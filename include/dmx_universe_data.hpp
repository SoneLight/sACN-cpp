#pragma once
#include <array>
#include <shared_mutex>
#include <iostream>
#include <iomanip>
#include <cmath>

namespace sACNcpp {

class DMXUniverseData {
    /**
     * @brief A wrapper around a std::array providing 
     * helper functions for DMX usage
     * 
     */

public:

    /**
     * @brief Construct a new DMXUniverseData object
     * 
     */
    DMXUniverseData() 
    {
        m_data.fill(0);
    }

    /**
     * @brief Reads DMX data from a given buffer
     * 
     * @param data the buffer to read from
     * @param length number of bytes to read
     */
    void read(const uint8_t * data, uint16_t length)
    {
        std::lock_guard<std::shared_timed_mutex> writeLock(m_mutex);
        
        bool changed = false;     
        for(uint16_t i = 0; i < length; i++)
        {
            m_data[i] = data[i];
        }
    }

    /**
     * @brief writes DMX data to a given buffer
     * 
     * @param data the buffer to write to
     * @param length the length to write. if none is 
     * specified, all 512 channels will be written. 
     */
    void write(uint8_t * data, uint16_t length)
    {
        std::shared_lock<std::shared_timed_mutex> readLock(m_mutex);
        
        for(uint16_t i = 0; i < length; i++)
        {
            data[i] = m_data[i];
        }
    }

    /**
     * @brief copies the contents of this DMXUniverseData to an other instance
     * 
     * @param target the instance to copy to
     */
    void copyTo(DMXUniverseData& target)
    {
        std::shared_lock<std::shared_timed_mutex> readLock(m_mutex);
        std::lock_guard<std::shared_timed_mutex> writeLock(target.m_mutex);

        for(uint16_t i = 0; i < 512; i++)
        {
            target.m_data[i] = m_data[i];
        }
    }

    /**
     * @brief Reads a multichannel dmx value (fine/ultra/.. resolution) from this packet.
     * 
     * @param channel the first channel the value will be read from
     * @param resolution the resolution of the value. should be greater than 0, 1 equals operator[].
     * @return double, will be between 0 and 1
     */
    double readVariableResolutionValue(uint16_t channel, uint8_t resolution)
    {
        if(channel+resolution > 512)
            throw std::out_of_range("channel+resolution would read from channel > 512");

        long rawValue = 0;
        std::shared_lock<std::shared_timed_mutex> readLock(m_mutex);
        for(uint8_t i = 0; i < resolution; i++)
        {
            rawValue += m_data[channel+resolution-i-1]*pow(256, i);
        }

        return rawValue / (pow(256, resolution)-1);
    }

    /**
     * @brief Writes a multichannel dmx value (fine/ultra/.. resolution) to this packet.
     * 
     * @param value the value to write. should be between 0 and 1.
     * @param channel the first channel the value should be written to
     * @param resolution the resolution to write the value in. should be greater than 0, 1 equals set().
     */
    void writeVariableResolutionValue(double value, uint16_t channel, uint8_t resolution)
    {
        if(channel+resolution > 512)
            throw std::out_of_range("channel+resolution would write to channel > 512");

        long val = value * (pow(256, resolution)-1);

        std::lock_guard<std::shared_timed_mutex> writeLock(m_mutex);
        for(uint8_t i = 0; i < resolution; i++)
        {
            m_data[channel+resolution-i-1] = val % 256;
            val /= 256;
        }
    }

    /**
     * @brief gets the dmx value of a channel
     * 
     * @param channel the channel to return the value of
     * @return const uint8_t value of channel
     */
    const uint8_t operator[](uint16_t channel)
    {
        std::shared_lock<std::shared_timed_mutex> readLock(m_mutex);
        return m_data[channel];
    }

    /**
     * @brief sets a dmx channel to a given value
     * 
     * @param channel channel to set
     * @param value value to store
     */
    void set(uint16_t channel, uint8_t value)
    {
        std::lock_guard<std::shared_timed_mutex> writeLock(m_mutex);
        m_data[channel] = value;
    }

    /**
     * @brief prints the stored dmx data to the console.
     * 
     */
    void print()
    {
        std::shared_lock<std::shared_timed_mutex> readLock(m_mutex);
        std::cout << "---------------------------------" << std::endl;
        for(int i = 0; i < 32; i++)
        {
            std::cout << std::setw(3) << i*16 << " | ";
            for(int j = 0; j < 16; j++)
            {
                uint16_t channel = i*16+j;

                std::cout << std::setw(3) << (int)m_data[channel] << " ";
            }

            std::cout << std::endl;
        }
        std::cout << "---------------------------------" << std::endl;
    }

private:

    /**
     * @brief the array storing the dmx data
     * 
     */
    std::array<uint8_t, 512> m_data;

    /**
     * @brief a mutex protecting the data array
     * 
     */
    std::shared_timed_mutex m_mutex;

};

}