#pragma once
#include <array>
#include <shared_mutex>

namespace sACNcpp {

class DMXUniverseData {
    /**
     * @brief A wrapper around a std::array providing 
     * helper functions for DMX usage
     * 
     */

public:

    DMXUniverseData() 
    {
        m_data.fill(0);
        m_changed.store(false);
    }

    void read(const uint8_t * data, uint16_t length)
    {
        std::lock_guard<std::shared_timed_mutex> writeLock(m_mutex);
        m_changed.store(true);        
        for(uint16_t i = 0; i < length; i++)
        {
            m_data[i] = data[i];
        }
    }

    void write(uint8_t * data, uint16_t length)
    {
        std::shared_lock<std::shared_timed_mutex> readLock(m_mutex);
        
        for(uint16_t i = 0; i < length; i++)
        {
            data[i] = m_data[i];
        }
    }

    const uint8_t operator[](uint16_t channel)
    {
        std::shared_lock<std::shared_timed_mutex> readLock(m_mutex);
        return m_data[channel];
    }

    void set(uint16_t channel, uint8_t value)
    {
        std::lock_guard<std::shared_timed_mutex> writeLock(m_mutex);
        m_changed.store(true);
        m_data[channel] = value;
    }

    bool changed() const
    {
        return m_changed.load();
    }

    void setUnchanged()
    {
        m_changed.store(false);
    }

private:

    std::array<uint8_t, 512> m_data;
    std::shared_timed_mutex m_mutex;
    std::atomic_bool m_changed;
};

}