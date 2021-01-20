#pragma once
#include <stdint.h>
#include <asio_standalone_or_boost.hpp>
#include <sacn_socket.hpp>
#include <atomic>
#include <thread>
#include <memory>
#include <chrono>

class sACNInput {

public:
    sACNInput(uint16_t universe, std::string networkInterface, std::shared_ptr<asio::io_context> io_context)
    {
        m_socket = std::make_unique<sACNSocket>(networkInterface, io_context);
        m_socket->prepareToReceive(universe);
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



private:

    void run()
    {
        while(m_running.load())
        {
            while(m_socket->packetAvailable())
            {
                m_socket->receivePacket(m_tempPacket);
                handleNewPacket();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    void handleNewPacket()
    {
        //TODO
    }

    uint16_t m_universe;
    std::thread m_thread;
    std::atomic_bool m_running;
    std::unique_ptr<sACNSocket> m_socket;
    sACNPacket m_tempPacket;
};