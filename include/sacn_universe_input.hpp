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

class sACNUniverseInput {

public:
    sACNUniverseInput(uint16_t universe, std::string networkInterface, std::shared_ptr<asio::io_context> io_context)
    {
        m_socket = std::make_unique<sACNReceiverSocket>(networkInterface, io_context);
    }

    void start()
    {
        if(m_running.load())
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

    std::string currentDMXSource()
    {
        //TODO
        return "None";
    }

    const DMXUniverseData& dmx() const
    {
        return m_universeValues;
    }


private:

    void run()
    {
        while(m_running.load())
        {
            while(m_socket->packetAvailable())
            {
                m_socket->receivePacket(m_tempPacket);
                if(m_tempPacket.valid())
                    m_tempPacket.getDMXDataCopy(m_universeValues);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    uint16_t m_universe;
    std::thread m_thread;
    std::atomic_bool m_running;
    std::unique_ptr<sACNReceiverSocket> m_socket;
    sACNPacket m_tempPacket;
    DMXUniverseData m_universeValues;
};