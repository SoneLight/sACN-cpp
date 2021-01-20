#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <string>
#include <dmx_universe_data.hpp>

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

namespace sACNcpp {

/* E1.31 Public Constants */
const uint16_t E131_DEFAULT_PORT = 5568;
const uint8_t E131_DEFAULT_PRIORITY = 100;

/* E1.31 Private Constants */
const uint16_t _E131_PREAMBLE_SIZE = 0x0010;
const uint16_t _E131_POSTAMBLE_SIZE = 0x0000;
const uint8_t _E131_ACN_PID[] = {0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00};
const uint32_t _E131_ROOT_VECTOR = 0x00000004;
const uint32_t _E131_FRAME_VECTOR = 0x00000002;
const uint8_t _E131_DMP_VECTOR = 0x02;
const uint8_t _E131_DMP_TYPE = 0xa1;
const uint16_t _E131_DMP_FIRST_ADDR = 0x0000;
const uint16_t _E131_DMP_ADDR_INC = 0x0001;

/**
 * @brief A struct describing the E1.31 packet type
 * All packet contents are in network byte order/big endian
 * 
 */
typedef union {
PACK(struct {
    PACK(struct { /* ACN Root Layer: 38 bytes */
    uint16_t preamble_size;    /* Preamble Size */
    uint16_t postamble_size;   /* Post-amble Size */
    uint8_t  acn_pid[12];      /* ACN Packet Identifier */
    uint16_t flength;          /* Flags (high 4 bits) & Length (low 12 bits) */
    uint32_t vector;           /* Layer Vector */
    uint8_t  cid[16];          /* Component Identifier (UUID) */
    }) root;

    PACK(struct { /* Framing Layer: 77 bytes */
    uint16_t flength;          /* Flags (high 4 bits) & Length (low 12 bits) */
    uint32_t vector;           /* Layer Vector */
    uint8_t  source_name[64];  /* User Assigned Name of Source (UTF-8) */
    uint8_t  priority;         /* Packet Priority (0-200, default 100) */
    uint16_t reserved;         /* Reserved (should be always 0) */
    uint8_t  seq_number;       /* Sequence Number (detect duplicates or out of order packets) */
    uint8_t  options;          /* Options Flags (bit 7: preview data, bit 6: stream terminated) */
    uint16_t universe;         /* DMX Universe Number */
    }) frame;

    PACK(struct { /* Device Management Protocol (DMP) Layer: 523 bytes */
    uint16_t flength;          /* Flags (high 4 bits) / Length (low 12 bits) */
    uint8_t  vector;           /* Layer Vector */
    uint8_t  type;             /* Address Type & Data Type */
    uint16_t first_addr;       /* First Property Address */
    uint16_t addr_inc;         /* Address Increment */
    uint16_t prop_val_cnt;     /* Property Value Count (1 + number of slots) */
    uint8_t  prop_val[513];    /* Property Values (DMX start code + slots data) */
    }) dmp;
});

uint8_t raw[638]; /* raw buffer view: 638 bytes */
} sacn_packet_struct;

/**
 * @brief A class containing the raw sACN packet and accessor/setter helper methods
 * 
 */
class sACNPacket 
{

    public:
        /**
         * @brief Construct a new sACNPacket object and initalizes the packet structure
         * 
         * @param universe the universe this packet represents
         * @param num_slots the slots to send, defaults to 512
         */
        sACNPacket(uint16_t universe = 1, uint16_t num_slots = 512)
        {
            packedPacket = new sacn_packet_struct();

            // clear packet
            memset(packedPacket, 0, sizeof *packedPacket);

            setNumDMXSlots(num_slots);

            // set Root Layer values
            packedPacket->root.preamble_size = htons(_E131_PREAMBLE_SIZE);
            packedPacket->root.postamble_size = htons(_E131_POSTAMBLE_SIZE);
            memcpy(packedPacket->root.acn_pid, _E131_ACN_PID, sizeof packedPacket->root.acn_pid);
            packedPacket->root.vector = htonl(_E131_ROOT_VECTOR);

            // set Framing Layer values 
            packedPacket->frame.vector = htonl(_E131_FRAME_VECTOR);
            packedPacket->frame.priority = E131_DEFAULT_PRIORITY;
            packedPacket->frame.universe = htons(universe);

            // set Device Management Protocol (DMP) Layer values
            packedPacket->dmp.vector = _E131_DMP_VECTOR;
            packedPacket->dmp.type = _E131_DMP_TYPE;
            packedPacket->dmp.first_addr = htons(_E131_DMP_FIRST_ADDR);
            packedPacket->dmp.addr_inc = htons(_E131_DMP_ADDR_INC);

        }

        /**
         * @brief Destroy the sACNPacket object and deletes to paket structure
         * 
         */
        ~sACNPacket()
        {
            delete(packedPacket);
        }

        /**
         * @brief the (maximum) size of the sacn packet, in bytes
         * 
         * @return size_t 
         */
        size_t packetSize()
        {
            return sizeof(sacn_packet_struct);
        }

        /**
         * @brief number of dmx slots in this packet
         * 
         * @return uint16_t 
         */
        uint16_t numDMXSlots() const
        {
            return ntohs(packedPacket->dmp.prop_val_cnt)-1;
        }

        /**
         * @brief Set the number of DMX Slots stored
         * 
         */
        void setNumDMXSlots(uint16_t dmx_slots)
        {
            //   // compute packet layer lengths
            uint16_t prop_val_cnt = dmx_slots + 1;
            uint16_t dmp_length = prop_val_cnt +
                sizeof packedPacket->dmp - sizeof packedPacket->dmp.prop_val;
            uint16_t frame_length = sizeof packedPacket->frame + dmp_length;
            uint16_t root_length = sizeof packedPacket->root.flength +
                sizeof packedPacket->root.vector + sizeof packedPacket->root.cid + frame_length;

            packedPacket->root.flength = htons(0x7000 | root_length);
            packedPacket->frame.flength = htons(0x7000 | frame_length);
            packedPacket->dmp.flength = htons(0x7000 | dmp_length);
            packedPacket->dmp.prop_val_cnt = htons(prop_val_cnt);

        }

        /**
         * @brief Get a pointer to the packet object
         * 
         * @return sacn_packet_struct* 
         */
        sacn_packet_struct* getPackedPacket() 
        { 
            return packedPacket; 
        }

        /**
         * @brief Get a const pointer to the packet struct
         * 
         * @return const sacn_packet_struct* 
         */
        const sacn_packet_struct* getPackedPacket() const
        { 
            return packedPacket; 
        }

        /**
         * @brief gets a dmx channel value stored in this packet
         * 
         * @param channel the channel to get
         * @return uint8_t the value
         */
        uint8_t dmx(uint16_t channel) const
        {
            return packedPacket->dmp.prop_val[channel];
        }

        /**
         * @brief sets a dmx channel in this value to a specified value
         * 
         * @param channel 
         * @param value 
         */
        void setDMX(uint16_t channel, uint8_t value)
        {
            packedPacket->dmp.prop_val[channel] = value;
        }

        /**
         * @brief copies the dmx content of this packet to a DMXUniverseData object
         * 
         * @param result the object to store the dmx values in
         */
        void getDMXDataCopy(DMXUniverseData& result) const
        {
            result.read(&packedPacket->dmp.prop_val[1], numDMXSlots());            
        }

        /**
         * @brief writes the dmx data from a DMXUniverseData object to this packet
         * 
         * @param data the object to copy the data from
         */
        void setDMXDataCopy(DMXUniverseData& data)
        {
            setNumDMXSlots(512);
            data.write(&packedPacket->dmp.prop_val[1], 512);
        }

        /**
         * @brief the name of the sACN source that sent this packet
         * 
         * @return std::string source name
         */
        std::string sourceName() const
        {
            return std::string((char*)packedPacket->frame.source_name);
        }

        /**
         * @brief Set the Source Name objectSets the source name stored in this packet
         * 
         * @param name 
         */
        void setSourceName(const std::string& name)
        {
            if(name.size() >= 63)
            {
                throw std::invalid_argument("Source name too long! Maximum 63 chars.");
            }
            strcpy((char*)packedPacket->frame.source_name, name.c_str());            
        }

        /**
         * @brief gets the universe id stored in this packet
         * 
         * @return uint16_t the id
         */
        uint16_t universe() const
        {
            return ntohs(packedPacket->frame.universe);
        }

        /**
         * @brief Set the universe id to store in this packet.
         * 
         * @param universe the new universe id. hast to be between 1 and 63999
         */
        void setUniverse(uint16_t universe)
        {
            if(universe > 63999 || universe < 1)
            {
                throw std::invalid_argument("Invalid universe number, only universes between 1 and 63999 are allowed.");
            }
            packedPacket->frame.universe = htons(universe);
        }

        /**
         * @brief gets the priority of the source that sent this package
         * 
         * @return uint8_t 
         */
        uint8_t priority() const
        {
            return packedPacket->frame.priority;
        } 

        /**
         * @brief Sets the priority this package stores
         * 
         * @param priority 
         */
        void setPriority(uint8_t priority)
        {
            packedPacket->frame.priority = priority;
        } 

        /**
         * @brief checks this packet for validity
         * 
         * @return true looks like a valid sACN packet
         * @return false not a valid sACN packet, discard!
         */
        bool valid() const
        {
            if (ntohs(packedPacket->root.preamble_size) != _E131_PREAMBLE_SIZE)
                return false;
            if (ntohs(packedPacket->root.postamble_size) != _E131_POSTAMBLE_SIZE)
                return false;
            if (memcmp(packedPacket->root.acn_pid, _E131_ACN_PID, sizeof packedPacket->root.acn_pid) != 0)
                return false;
            if (ntohl(packedPacket->root.vector) != _E131_ROOT_VECTOR)
                return false;
            if (ntohl(packedPacket->frame.vector) != _E131_FRAME_VECTOR)
                return false;
            if (packedPacket->dmp.vector != _E131_DMP_VECTOR)
                return false;
            if (packedPacket->dmp.type != _E131_DMP_TYPE)
                return false;
            if (htons(packedPacket->dmp.first_addr) != _E131_DMP_FIRST_ADDR)
                return false;
            if (htons(packedPacket->dmp.addr_inc) != _E131_DMP_ADDR_INC)
                return false;
            return true;
        }

    private:

        /**
         * @brief struct containing the packet data
         * 
         */
        sacn_packet_struct* packedPacket;
        
};

}