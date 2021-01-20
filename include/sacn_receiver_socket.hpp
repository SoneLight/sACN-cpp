#pragma once
#include <sacn_packet.hpp>
#include <string>
#include <asio_standalone_or_boost.hpp>
#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <sys/types.h>

class sACNReceiverSocket
{
    public:
        sACNReceiverSocket(uint16_t universe, std::shared_ptr<asio::io_context> context, std::string interface = "")
        {
            if(interface == "")
                socket = std::make_unique<asio::ip::udp::socket>(*context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 5568));
            else
                socket = std::make_unique<asio::ip::udp::socket>(*context, asio::ip::udp::endpoint(asio::ip::make_address(interface), 5568));

            socket->set_option(asio::ip::multicast::join_group(
                asio::ip::make_address_v4(0xefff0000 | universe), 
                asio::ip::make_address_v4(interface)));
        }

        bool packetAvailable()
        {
            return socket->available() > 0;
        }

        bool receivePacket(sACNPacket& buffer)
        {
            socket->receive(asio::buffer(buffer.getPackedPacket()->raw));
        }

    private:
        std::unique_ptr<asio::ip::udp::socket> socket;
        std::string interface;
};