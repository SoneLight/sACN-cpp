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
 * @brief A class handle a single received DMX universe.
 * 
 * 
 */
class sACNUniverseInput {

public:

    /**
     * @brief Construct a new sACNUniverseInput object.
     * 
     */
    sACNUniverseInput()
    {
        m_lastPacket = std::chrono::high_resolution_clock::now() - std::chrono::seconds(100);
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

    /**
     * @brief handles a newly received DMX packet, and copies the contained DMX data.
     * 
     * @param newPacket the packet to handle
     */
    void handleNewPacket(const sACNPacket& newPacket)
    {
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_currentSource = newPacket.sourceName();
            m_lastPacket = std::chrono::high_resolution_clock::now();
        }
        newPacket.getDMXDataCopy(m_universeValues);
    }

    bool receivingData()
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        return std::chrono::high_resolution_clock::now()-m_lastPacket < std::chrono::seconds(2);
    }


private:
    
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
     * @brief the time point the last packet was received
     * 
     */
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastPacket;

    /**
     * @brief The source name of the sACN source of the last packet received.
     * 
     */
    std::string m_currentSource = "None";
};
}