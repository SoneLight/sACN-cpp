#pragma once
#include <array>
#include <shared_mutex>
#include <iostream>
#include <iomanip>

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
        m_changed.store(false);
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
        m_changed.store(true);        
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
        m_changed.store(true);
        m_data[channel] = value;
    }

    /**
     * @brief returns if this instance contains data that was changed
     * sind setUnchanged was last called.
     * 
     * @return true new data available
     * @return false nothing changed
     */
    bool changed() const
    {
        return m_changed.load();
    }

    /**
     * @brief Set the changed flag to false.
     * 
     */
    void setUnchanged()
    {
        m_changed.store(false);
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

    /**
     * @brief an atomic bool indicating this instance contains 
     * data that changed since setUnchanged was last called 
     * 
     */
    std::atomic_bool m_changed;
};

}