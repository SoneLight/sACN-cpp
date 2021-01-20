#pragma once
#include <sacn_packet.hpp>
#include <string>
#include <asio_standalone_or_boost.hpp>
#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <sys/types.h>
#include <logger.hpp>

namespace sACNcpp {



class sACNReceiverSocket
{
    public:
        sACNReceiverSocket(uint16_t universe, std::shared_ptr<asio::io_context> context, std::string interface = "") : 
            m_interface(interface), m_universe(universe)
        {
            socket = std::make_unique<asio::ip::udp::socket>(*context);
        }

        bool start()
        {
            try
            {               
                if(m_interface == "")
                    socket->bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 5568));
                else
                    socket->bind(asio::ip::udp::endpoint(asio::ip::make_address(m_interface), 5568));
                
                Logger::Log(LogLevel::Info, "Bound socket to interface " + m_interface);
            }
            catch(const std::exception& e)
            {
                Logger::Log(LogLevel::Critical, "Could not bind socket! " + std::string(e.what()));
                return false;
            }

            try
            {
                socket->set_option(asio::ip::multicast::join_group(
                    asio::ip::make_address_v4(0xefff0000 | m_universe), 
                    asio::ip::make_address_v4(m_interface)));

                Logger::Log(LogLevel::Info, "Joined multicast group for universe " + m_universe);
            }
            catch(const std::exception& e)
            {                
                Logger::Log(LogLevel::Critical, "Could not join multicast group! " + std::string(e.what()));
                return false;
            }           

            return true;
        }

        bool packetAvailable()
        {
            return socket->available() > 0;
        }

        bool receivePacket(sACNPacket& buffer)
        {
            try
            {
                socket->receive(asio::buffer(buffer.getPackedPacket()->raw));
            }
            catch(const std::exception& e)
            {                
                Logger::Log(LogLevel::Warning, "Exception while receiving packet! " + std::string(e.what()));
                return false;
            }       
            Logger::Log(LogLevel::Debug, "Received new packet for universe " + m_universe);    
            return true;
        }

    private:
        std::unique_ptr<asio::ip::udp::socket> socket;
        std::string m_interface;
        uint16_t m_universe;
};

}