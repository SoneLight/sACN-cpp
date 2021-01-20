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

class sACNUniverseOutput {

public:
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

    void start()
    {
        if(m_running.load())
            return;

        if(!m_socket->start())
            return;

        m_running.store(true);
        m_thread = std::thread([this]() {this->run(); });
    }

    void stop()
    {
        if(!m_running.load())
            return;

        m_running.store(false);
        m_thread.join();
    }

    DMXUniverseData& dmx()
    {
        return m_universeValues;
    }


private:

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

    uint16_t m_universe;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastPacket;
    uint16_t m_unchangedRefreshRate;
    std::thread m_thread;
    std::atomic_bool m_running;
    std::unique_ptr<sACNSenderSocket> m_socket;
    sACNPacket m_tempPacket;
    DMXUniverseData m_universeValues;

};
}