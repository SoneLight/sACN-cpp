#pragma once
#include <stdint.h>
#include <asio_standalone_or_boost.hpp>
#include <sacn_sender_socket.hpp>
#include <dmx_universe_data.hpp>
#include <atomic>
#include <thread>
#include <memory>
#include <chrono>
#include <array>

namespace sACNcpp {

/**
 * @brief A class sending a single universe to sACN.
 * 
 * A separate thread is used in the background.
 * sACN is only sent (using the values in the DMXUniverseData accessible by dmx()) when start() was called. 
 * 
 */
class sACNUniverseOutput {

public:
    /**
     * @brief Construct a new sACNUniverseOutput object
     * 
     * @param universe sACN universe to output data to
     * @param sourceName the sourceName this sACN sender should appear as
     * @param io_context the asio iocontext object to use for the underlying socket
     * @param unchangedRefreshRate the refresh rate to send packets when no changes are made to the DMXUniverseData class
     * @param networkInterface The network interface to use. If none is provided, some interface/the default will be chosen. 
     */
    sACNUniverseOutput(uint16_t universe, 
        std::string sourceName,
        std::shared_ptr<asio::io_context> io_context, 
        uint16_t unchangedRefreshRate=5, 
        std::string networkInterface="") :
        m_universe(universe),
        m_unchangedRefreshRate(unchangedRefreshRate)
    {       
        m_socket = std::make_unique<sACNSenderSocket>(io_context, networkInterface);
        m_tempPacket.setSourceName(sourceName);
        m_tempPacket.setUniverse(universe);
    }

    /**
     * @brief Destroy the sACNUniverseOutput object. This will stop the receiver thread.
     * 
     */
    ~sACNUniverseOutput()
    {
        stop();
    }

    /**
     * @brief Starts execution of the sender. This will spawn an additional thread to send sACN in the background.
     * 
     */
    void start()
    {
        if(m_running.load())
            return;

        if(!m_socket->start())
            return;

        m_running.store(true);
        m_thread = std::thread([this]() {this->run(); });
    }

    /**
     * @brief Stops execution of the sACN sender.
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
     * @brief The DMX values this module is currently sending to sACN. 
     * This returns a mutable reference, to be used to set DMX values.
     * If a change in the DMXUniverseData is detected, the values will be sent to sACN,
     * with a refreshRate exceeding the unchangedRefreshRate.
     * 
     * @return DMXUniverseData& 
     */
    DMXUniverseData& dmx()
    {
        return m_universeValues;
    }


private:

    /**
     * @brief Runs the sending thread until m_running is set to false.
     * 
     */
    void run()
    {
        while(m_running.load())
        {
            if(m_universeValues.changed() || 
                std::chrono::high_resolution_clock::now()-m_lastPacket > std::chrono::milliseconds(1000/m_unchangedRefreshRate))
            {
                m_tempPacket.setDMXDataCopy(m_universeValues);
                m_socket->sendPacketMulticast(m_tempPacket, m_universe);

                m_universeValues.setUnchanged();
                m_lastPacket = std::chrono::high_resolution_clock::now();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    /**
     * @brief the universe to send to
     * 
     */
    uint16_t m_universe;

    /**
     * @brief the time point the last packet was sent
     * 
     */
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastPacket;

    /**
     * @brief The refresh rate to use when no changes are mad ein the DMXUniverseData object.
     * 
     */
    uint16_t m_unchangedRefreshRate;

    /**
     * @brief The thread used to send sACN
     * 
     */
    std::thread m_thread;

    /**
     * @brief An atomic bool indicating the thread should keep running.
     * 
     */
    std::atomic_bool m_running;

    /**
     * @brief The sACNSenderSocket used to send sACN
     * 
     */
    std::unique_ptr<sACNSenderSocket> m_socket;

    /**
     * @brief a packet used to send in every update. 
     * The data will be copied to this packet before sending.
     * 
     */
    sACNPacket m_tempPacket;

    /**
     * @brief The values to send
     * 
     */
    DMXUniverseData m_universeValues;

};
}