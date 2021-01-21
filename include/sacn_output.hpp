#pragma once
#include <stdint.h>
#include <asio_standalone_or_boost.hpp>
#include <sacn_sender_socket.hpp>
#include <sacn_universe_output.hpp>
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
class sACNOutput {

public:
    /**
     * @brief Construct a new sACNOutput object
     * 
     * @param sourceName the sourceName this sACN sender should appear as
     * @param io_context the asio iocontext object to use for the underlying socket, optional
     * @param unchangedRefreshRate the refresh rate to send packets when no changes are made to the DMXUniverseData class
     * @param networkInterface The network interface to use. If none is provided, some interface/the default will be chosen. 
     */
    sACNOutput( 
        std::string sourceName,
        std::shared_ptr<asio::io_context> io_context = nullptr, 
        uint16_t unchangedRefreshRate=5, 
        std::string networkInterface="") :
        m_iocontext(io_context),
        m_unchangedRefreshRate(unchangedRefreshRate)
    {       
        if(!m_iocontext)
            m_iocontext = std::make_shared<asio::io_context>();

        m_socket = std::make_unique<sACNSenderSocket>(m_iocontext, networkInterface);
        m_tempPacket.setSourceName(sourceName);
        m_running.store(false);
    }

    /**
     * @brief Destroy the sACNUniverseOutput object. This will stop the receiver thread.
     * 
     */
    ~sACNOutput()
    {
        stop();
    }

    /**
     * @brief Starts execution of the sender. This will spawn an additional thread to send sACN in the background.
     * @return true: creation of the socket was successful
     * @return false: there was an error constructing the socket
     */
    bool start()
    {
        if(m_running.load())
            return false;

        if(!m_socket->start())
            return false;

        m_running.store(true);
        m_thread = std::thread([this]() {this->run(); });

        return true;
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
     * @brief Adds a universe to send data to.
     * 
     * @param universe the id of the universe to start sending data to
     * @return true: if the universe sender was successfully added
     * @return false: the universe sender was already constructed
     */
    bool addUniverse(const uint16_t& universe)
    {
        std::lock_guard<std::shared_timed_mutex> writeLock(m_mutex); 

        if(m_universes.find(universe) != m_universes.end())
            return false;

        m_universes.emplace(universe, new sACNUniverseOutput(universe, m_unchangedRefreshRate));

        return true;
    }

    /**
     * @brief gets a reference to the universe with the given id. If this universe was not intialized on this object with addUniverse(), an exception will be thrown.
     * 
     * @throw std::out_of_range exception if the universe with id universe was not first added to the input with addUnvierse()
     * @param universe the id of the universe to get
     * @return sACNUniverseOutput* a pointer to the universe
     */
    sACNUniverseOutput* operator[](const uint16_t& universe)
    {
        std::shared_lock<std::shared_timed_mutex> readLock(m_mutex);                
        return m_universes.at(universe);
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
            {
                std::shared_lock<std::shared_timed_mutex> readLock(m_mutex);

                for(uint16_t k : m_universeIDs)
                {
                    if(m_universes.at(k)->getNewPacketData(m_tempPacket))
                    {
                        m_socket->sendPacketMulticast(m_tempPacket);
                    }
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    /**
     * @brief the universes to send to
     * 
     */
    std::vector<uint16_t> m_universeIDs;

    /**
     * @brief the universes objects
     * 
     */
    std::map<uint16_t, sACNUniverseOutput*> m_universes;

    /**
     * @brief A mutex protecting m_universes
     * 
     */
    std::shared_timed_mutex m_mutex;

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
     * @brief IO context used by the asio socket
     * 
     */
    std::shared_ptr<asio::io_context> m_iocontext;
    
    /**
     * @brief a packet used to send in every update. 
     * The data will be copied to this packet before sending.
     * 
     */
    sACNPacket m_tempPacket;

};
}