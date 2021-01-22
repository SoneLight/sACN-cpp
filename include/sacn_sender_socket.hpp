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

/**
 * @brief A wrapper around an asio::ip::udp::socket for sending sACN.
 * 
 */
class sACNSenderSocket
{
    public:

        /**
         * @brief Construct a new sACNSenderSocket object
         * 
         * @param context the asio context to use to construct the asio::ip::udp::socket
         * @param interface the interface to use to send sACN from. if empty, the default interface will be used.
         */
        sACNSenderSocket(std::shared_ptr<asio::io_context> context, std::string interface="") : m_interface(interface)
        {
            socket = std::make_unique<asio::ip::udp::socket>(*context);            
        }

        /**
         * @brief prepares the socket for sending sacn
         * 
         * @return true creation of the socket successful
         * @return false an error occurred, check logs
         */
        bool start()
        {
               
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
            
            if(m_interface != "")
            {
                try
                {
                    socket->bind(asio::ip::udp::endpoint(asio::ip::make_address(m_interface), 34567));
                    Logger::Log(LogLevel::Info, "Bound socket to interface " + m_interface);
                }
                catch(const std::exception& e)
                {
                    Logger::Log(LogLevel::Critical, "Could not bind socket! " + std::string(e.what()));
                    return false;
                }                
            }
            
            return true;
        }

        /**
         * @brief Sends a sACN packet to the multicast address corresponding to the universe
         * set in the packet.
         * 
         * @param packet the packet to send.
         * @return true the packet was successfully sent
         * @return false an error occurred while sending the packet
         */
        bool sendPacketMulticast(const sACNPacket& packet)
        {
            uint16_t universe = packet.universe();
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

        /**
         * @brief Sends a sACN packet to the hostname and port provided. If no port is provided, the 
         * default port for sACN (5568) is used.
         * 
         * @param packet the packet to send
         * @param hostname the hostname to send to
         * @param port the port to send to, or 5568, if none is provided
         * @return true packet sent successfully
         * @return false an error occurred while sending the packet
         */
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
        /**
         * @brief the socket sent to send packets
         * 
         */
        std::unique_ptr<asio::ip::udp::socket> socket;

        /**
         * @brief the interface to use to send packets
         * 
         */
        std::string m_interface;
};
}