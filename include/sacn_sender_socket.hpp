#pragma once
#include <sacn_packet.hpp>
#include <string>
#include <asio_standalone_or_boost.hpp>
#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <sys/types.h>

class sACNSenderSocket
{
    public:
        sACNSenderSocket(std::shared_ptr<asio::io_context> context, std::string interface="")
        {
            if(interface == "")
                socket = std::make_unique<asio::ip::udp::socket>(*context, asio::ip::udp::endpoint(asio::ip::make_address(interface), 5569));
            else
                socket = std::make_unique<asio::ip::udp::socket>(*context, asio::ip::udp::v4());
        }

        bool sendPacketMulticast(const sACNPacket& packet, uint16_t universe)
        {
            asio::ip::udp::endpoint endpoint(asio::ip::make_address_v4(0xefff0000 | universe), 5568);
            socket->send_to(asio::buffer(packet.getPackedPacket()->raw), endpoint);
        }

        bool sendPacketUnicast(const sACNPacket& packet, std::string hostname, uint16_t port=5568)
        {
            asio::ip::udp::endpoint endpoint(asio::ip::make_address(hostname), port);
            socket->send_to(asio::buffer(packet.getPackedPacket()->raw), endpoint);
        }

    private:
        std::unique_ptr<asio::ip::udp::socket> socket;
        std::string interface;
};