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

class sACNSenderSocket
{
    public:
        sACNSenderSocket(std::shared_ptr<asio::io_context> context, std::string interface="") : m_interface(interface)
        {
            socket = std::make_unique<asio::ip::udp::socket>(*context);            
        }

        bool start()
        {
            if(m_interface != "")
            {
                try
                {
                    socket->bind(asio::ip::udp::endpoint(asio::ip::make_address(m_interface), 5569));
                    Logger::Log(LogLevel::Info, "Bound socket to interface " + m_interface);
                }
                catch(const std::exception& e)
                {
                    Logger::Log(LogLevel::Critical, "Could not bind socket! " + std::string(e.what()));
                    return false;
                }                
            }
                
            try
            {
                socket->open(asio::ip::udp::v4());
                Logger::Log(LogLevel::Info, "Openend socket.");
            }
            catch(const std::exception& e)
            {
                Logger::Log(LogLevel::Critical, "Could not open socket! " + std::string(e.what()));
                return false;
            }
            
            return true;
        }

        bool sendPacketMulticast(const sACNPacket& packet, uint16_t universe)
        {
            asio::ip::udp::endpoint endpoint(asio::ip::make_address_v4(0xefff0000 | universe), 5568);

            try
            {
                socket->send_to(asio::buffer(packet.getPackedPacket()->raw), endpoint);
            }
            catch(const std::exception& e)
            {                
                Logger::Log(LogLevel::Warning, "Could not send packet! " + std::string(e.what()));
                return false;
            }
            Logger::Log(LogLevel::Debug, "Sent packet! ");
            return true;           
        }

        bool sendPacketUnicast(const sACNPacket& packet, std::string hostname, uint16_t port=5568)
        {
            asio::ip::udp::endpoint endpoint(asio::ip::make_address(hostname), port);
            try
            {
                socket->send_to(asio::buffer(packet.getPackedPacket()->raw), endpoint);
            }
            catch(const std::exception& e)
            {
                Logger::Log(LogLevel::Warning, "Could not send packet! " + std::string(e.what()));
                return false;
            }
            Logger::Log(LogLevel::Debug, "Sent packet! ");
            return true;         
        }

    private:
        std::unique_ptr<asio::ip::udp::socket> socket;
        std::string m_interface;
};
}