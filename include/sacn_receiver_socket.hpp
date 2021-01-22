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
 * @brief A wrapper around an asio::ip::udp::socket to receive data from a sACN universe
 * 
 */
class sACNReceiverSocket
{
    public:
        /**
         * @brief Construct a new sACNReceiverSocket object
         * 
         * @param context the asio::io_context to use
         * @param interface the interface to bind to. if empty, the default interface will be used.
         */
        sACNReceiverSocket(std::shared_ptr<asio::io_context> context, std::string interface = "") : 
            m_interface(interface)
        {
            socket = std::make_unique<asio::ip::udp::socket>(*context);
        }

        /**
         * @brief prepares the socket to receive sACN. This involves opening the socket and joining the correct
         * multicast group for the specified universe.
         * 
         * @return true the socket was openend successfully, no error occurred.
         * @return false an error occurred. check the logs.
         */
        bool start()
        {
            try
            {
                socket->open(asio::ip::udp::v4());
                Logger::Log(LogLevel::Info, "Opened socket.");
            }
            catch(const std::exception& e)
            {
                Logger::Log(LogLevel::Critical, "Could not open socket! " + std::string(e.what()));
                return false;
            }

            try
            {               
                socket->bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 5568));
                
                Logger::Log(LogLevel::Info, "Bound socket.");
            }
            catch(const std::exception& e)
            {
                Logger::Log(LogLevel::Critical, "Could not bind socket! " + std::string(e.what()));
                return false;
            }

            return true;
        }

        bool joinUniverse(uint16_t universe)
        {
            try
            {
                if(m_interface == "")
                    socket->set_option(asio::ip::multicast::join_group(
                        asio::ip::make_address_v4(0xefff0000 | universe)));
                else
                    socket->set_option(asio::ip::multicast::join_group(
                        asio::ip::make_address_v4(0xefff0000 | universe), 
                        asio::ip::make_address_v4(m_interface)));

                Logger::Log(LogLevel::Info, "Joined multicast group for universe " + std::to_string(universe));
            }
            catch(const std::exception& e)
            {                
                Logger::Log(LogLevel::Critical, "Could not join multicast group! " + std::string(e.what()));
                return false;
            }  
            return true;        
        }

        /**
         * @brief Checks if a new packet can be received.
         * 
         * @return true a packet is ready to be processed
         * @return false no data available
         */
        bool packetAvailable()
        {
            return socket->available() > 0;
        }

        /**
         * @brief Received a packet into the packet structure provided
         * 
         * @param buffer the packet to receive data into
         * @return true no error occurred, receiving of the packet complete
         * @return false an error occurred while receiving the packet
         */
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
            Logger::Log(LogLevel::Debug, "Received new packet.");    
            return true;
        }

    private:
        /**
         * @brief the asio::ip::udp::socket to use
         * 
         */
        std::unique_ptr<asio::ip::udp::socket> socket;

        /**
         * @brief the interface to bind to
         * 
         */
        std::string m_interface;

        /**
         * @brief the universe to receive
         * 
         */
        uint16_t m_universe;
};

}