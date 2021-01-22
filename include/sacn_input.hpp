#pragma once
#include <stdint.h>
#include <asio_standalone_or_boost.hpp>
#include <sacn_receiver_socket.hpp>
#include <sacn_universe_input.hpp>
#include <atomic>
#include <thread>
#include <memory>
#include <chrono>
#include <array>
#include <shared_mutex>

namespace sACNcpp {

/**
 * @brief A class receiving all dmx data from sACN.
 * 
 * A separate thread is used in the background.
 * sACN is only received (and the DMXUniverseData accessible by dmx() filled) when start() was called. 
 * 
 */
class sACNInput {

public:

    /**
     * @brief Construct a new sACNInput object.
     * 
     * @param io_context the asio iocontext object to use for the underlying socket (optional)
     */
    sACNInput(std::shared_ptr<asio::io_context> io_context=nullptr) : m_iocontext(io_context)
    {
        if(!m_iocontext)
            m_iocontext = std::make_unique<asio::io_context>();
        m_running.store(false);
    }

    /**
     * @brief Destroy the sACNUniverseInput object. This will stop the receiver thread.
     * 
     */
    ~sACNInput()
    {
        stop();
    }

    /**
     * @brief Starts execution of the receiver. This will spawn an additional thread to receive sACN in the background.
     * @param networkInterface the network interface to bind to. if empty, the default interface will be chosen
     * @return true: creation of the socket was successful
     * @return false: there was an error constructing the socket
     */
    bool start(std::string networkInterface="")
    {
        if(m_running.load())
            return false;

        m_socket = std::make_unique<sACNReceiverSocket>(m_iocontext, networkInterface);

        if(!m_socket->start())
            return false;

        m_running.store(true);
        m_thread = std::thread([this]() {this->run(); });

        return true;
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
     * @brief Adds a universe to listen to. This will join the corresponding multicast group.
     * 
     * @param universe the universe to listen to
     * @return true: creation of the socket receiver was successful
     * @return false: there was an error joing the multicast group
     */
    bool addUniverse(const uint16_t& universe)
    {
        std::lock_guard<std::shared_timed_mutex> writeLock(m_mutex); 

        if(m_universes.find(universe) != m_universes.end())
            return false;

        if(!m_socket->joinUniverse(universe))
            return false;

        m_universes.emplace(universe, new sACNUniverseInput());

        return true;
    }

    /**
     * @brief gets a pointer to the universe with the given id. If this universe was not intialized on this object with addUniverse(), an exception will be thrown.
     * 
     * @throw std::out_of_range exception if the universe with id universe was not first added to the input with addUnvierse()
     * @param universe the id of the universe to get
     * @return sACNUniverseInput* a pointer to the universe
     */
    sACNUniverseInput* operator[](const uint16_t& universe)
    {
        std::shared_lock<std::shared_timed_mutex> readLock(m_mutex);                
        return m_universes.at(universe);
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
                int universe = m_tempPacket.universe();
                {
                    std::shared_lock<std::shared_timed_mutex> readLock(m_mutex);

                    auto it = m_universes.find(universe);
                    
                    if(it == m_universes.end())
                        continue;
                    
                    else
                    {
                        m_universes.at(universe)->handleNewPacket(m_tempPacket); 
                    }    
                }    
                Logger::Log(LogLevel::Debug, "Universe " + std::to_string(universe) + " received new packet.");             

            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    
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
     * @brief a packet used to receive packets. 
     * The data will be copied from here.
     * 
     */
    sACNPacket m_tempPacket;

    /**
     * @brief IO context used by the asio socket
     * 
     */
    std::shared_ptr<asio::io_context> m_iocontext;

    /**
     * @brief The last DMX values received
     * 
     */
    std::map<uint16_t, sACNUniverseInput*> m_universes;

    /**
     * @brief A mutex protecting the m_universes member
     * 
     */
    std::shared_timed_mutex m_mutex;
};
}