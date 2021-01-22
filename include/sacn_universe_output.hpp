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
     * @param unchangedRefreshRate the refresh rate to send packets when no changes are made to the DMXUniverseData class
     */
    sACNUniverseOutput(uint16_t universe, 
        uint16_t unchangedRefreshRate=5) 
        :
        m_universe(universe),
        m_unchangedRefreshRate(unchangedRefreshRate)
    {       

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

    /**
     * @brief fills the passed sACNPacket with new data to send, if sending a packet is necessary.
     * 
     * @param dataPacket packet reference to modify and fill with new data if new data should be sent
     * @return true: if a new packet should be sent and the contents of dataPacket were modified
     * @return false: if now new data needs to be sent
     */
    bool getNewPacketData(sACNPacket& dataPacket)
    {
        if(m_universeValues.changed() || 
            std::chrono::high_resolution_clock::now()-m_lastPacket > std::chrono::milliseconds(1000/m_unchangedRefreshRate))
        {
            dataPacket.setDMXDataCopy(m_universeValues);
            dataPacket.setUniverse(m_universe);
            dataPacket.setSequenceNumber(m_sequenceNumber);

            m_sequenceNumber++;

            m_universeValues.setUnchanged();
            m_lastPacket = std::chrono::high_resolution_clock::now();

            return true;
        }
        return false;
    }


private:

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

    uint8_t m_sequenceNumber = 0;

    /**
     * @brief The values to send
     * 
     */
    DMXUniverseData m_universeValues;

};
}