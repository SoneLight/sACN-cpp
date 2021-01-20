#pragma once
#include <sacn_packet.hpp>
#include <string>
#include <asio_standalone_or_boost.hpp>
#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <sys/types.h>

typedef unsigned char uint8_t;

class sACNSocket
{
    public:
        sACNSocket(std::string interface, std::shared_ptr<asio::io_context> context)
        {
            socket = std::make_unique<asio::ip::udp::socket>(*context);
        }

        bool prepareToReceive(uint16_t universe)
        {
            socket->bind(asio::ip::udp::endpoint(asio::ip::make_address(interface), 5568));

            socket->set_option(asio::ip::multicast::join_group(
                asio::ip::make_address_v4(0xefff0000 | universe), 
                asio::ip::make_address_v4(interface)));
        }

        bool prepareToSend()
        {
            socket->open();
        }

        bool packetAvailable()
        {
            return socket->available() >= sACNPacket::packetSize();
        }

        bool receivePacket(sACNPacket& buffer)
        {
            socket->receive(asio::buffer(buffer.getPackedPacket()->raw));
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