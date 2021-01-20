#pragma once
#include <stdint.h>
#include <asio_standalone_or_boost.hpp>
#include <sacn_receiver_socket.hpp>
#include <dmx_universe_data.hpp>
#include <atomic>
#include <thread>
#include <memory>
#include <chrono>
#include <array>
#include <mutex>

namespace sACNcpp {

/**
 * @brief A class receiving a single universe from sACN.
 * 
 * A separate thread is used in the background.
 * sACN is only received (and the DMXUniverseData accessible by dmx() filled) when start() was called. 
 * 
 */
class sACNUniverseInput {

public:

    /**
     * @brief Construct a new sACNUniverseInput object.
     * 
     * @param universe sACN universe to receive
     * @param io_context the asio iocontext to use for the underlying asio socket
     * @param networkInterface the network interface to bind to.
     */
    sACNUniverseInput(uint16_t universe, std::shared_ptr<asio::io_context> io_context, std::string networkInterface="")
    {
        m_socket = std::make_unique<sACNReceiverSocket>(universe, io_context, networkInterface);
    }

    /**
     * @brief Destroy the sACNUniverseInput object. This will stop the receiver thread.
     * 
     */
    ~sACNUniverseInput()
    {
        stop();
    }

    /**
     * @brief Starts execution of the receiver. This will spawn an additional thread to receive sACN in the background.
     * 
     */
    void start()
    {
        if(m_running.load())
            return;

        if(m_socket->start())
            return;

        m_running.store(true);
        m_thread = std::thread([this]() {this->run(); });
    }

    /**
     * @brief Stops execution of the receiver.
     * 
     */
    void stop()
    {
        if(!m_running.load())
            return;

        m_running.store(false);
        m_thread.join();
    }

    /**
     * @brief Returns the name of the sACN source this class is receiving sACN from.
     * 
     * @return std::string 
     */
    std::string currentDMXSource()
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_currentSource;
    }

    /**
     * @brief Returns a reference to the current dmx values for the received universe.
     * 
     * @return DMXUniverseData& the current dmx values for the received universe
     */
    DMXUniverseData& dmx()
    {
        return m_universeValues;
    }


private:

    /**
     * @brief Executes the receiver thread, until m_running is set to false.
     * 
     */
    void run()
    {
        while(m_running.load())
        {
            while(m_socket->packetAvailable())
            {
                if(!m_socket->receivePacket(m_tempPacket))
                    continue;

                if(!m_tempPacket.valid())
                {
                    Logger::Log(LogLevel::Warning, "Received invalid packet!");
                    continue;
                }

                if(m_tempPacket.universe() != m_universe)
                {
                    Logger::Log(LogLevel::Warning, "Received packet from wrong universe on this multicast group.");
                    continue;
                }

                {
                    std::lock_guard<std::mutex> lk(m_mutex);
                    m_currentSource = m_tempPacket.sourceName();
                }
                m_tempPacket.getDMXDataCopy(m_universeValues);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    
    /**
     * @brief The universe this receiver is listening to
     * 
     */
    uint16_t m_universe;

    /**
     * @brief the thread running the run method and so, the receiving thread
     * 
     */
    std::thread m_thread;

    /**
     * @brief An atomic boolean indicating that the thread should continue running.
     * 
     */
    std::atomic_bool m_running;

    /**
     * @brief The socket used for receiving sACN
     * 
     */
    std::unique_ptr<sACNReceiverSocket> m_socket;

    /**
     * @brief A packet to receive into, Data will be copied from this packet directly after receiving.
     * 
     */
    sACNPacket m_tempPacket;

    /**
     * @brief The last DMX values received
     * 
     */
    DMXUniverseData m_universeValues;

    /**
     * @brief A mutex protecting private data members, currently only the m_currentSource member
     * 
     */
    std::mutex m_mutex;

    /**
     * @brief The source name of the sACN source of the last packet received.
     * 
     */
    std::string m_currentSource = "None";
};
}