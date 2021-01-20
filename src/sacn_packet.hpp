#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <string>

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

/* E1.31 Public Constants */
const uint16_t E131_DEFAULT_PORT = 5568;
const uint8_t E131_DEFAULT_PRIORITY = 0x64;

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

/* E1.31 Packet Type */
/* All packet contents shall be transmitted in network byte order (big endian) */
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

class sACNPacket 
{

    public:
        sACNPacket(uint16_t num_slots = 512, uint16_t universe = 1)
        {
            packedPacket = new sacn_packet_struct();
            //   // compute packet layer lengths
            uint16_t prop_val_cnt = num_slots + 1;
            uint16_t dmp_length = prop_val_cnt +
                sizeof packedPacket->dmp - sizeof packedPacket->dmp.prop_val;
            uint16_t frame_length = sizeof packedPacket->frame + dmp_length;
            uint16_t root_length = sizeof packedPacket->root.flength +
                sizeof packedPacket->root.vector + sizeof packedPacket->root.cid + frame_length;

            // clear packet
            memset(packedPacket, 0, sizeof *packedPacket);

            // set Root Layer values
            packedPacket->root.preamble_size = htons(_E131_PREAMBLE_SIZE);
            packedPacket->root.postamble_size = htons(_E131_POSTAMBLE_SIZE);
            memcpy(packedPacket->root.acn_pid, _E131_ACN_PID, sizeof packedPacket->root.acn_pid);
            packedPacket->root.flength = htons(0x7000 | root_length);
            packedPacket->root.vector = htonl(_E131_ROOT_VECTOR);

            // set Framing Layer values 
            packedPacket->frame.flength = htons(0x7000 | frame_length);
            packedPacket->frame.vector = htonl(_E131_FRAME_VECTOR);
            packedPacket->frame.priority = E131_DEFAULT_PRIORITY;
            packedPacket->frame.universe = htons(universe);

            // set Device Management Protocol (DMP) Layer values
            packedPacket->dmp.flength = htons(0x7000 | dmp_length);
            packedPacket->dmp.vector = _E131_DMP_VECTOR;
            packedPacket->dmp.type = _E131_DMP_TYPE;
            packedPacket->dmp.first_addr = htons(_E131_DMP_FIRST_ADDR);
            packedPacket->dmp.addr_inc = htons(_E131_DMP_ADDR_INC);
            packedPacket->dmp.prop_val_cnt = htons(prop_val_cnt);

        }

        ~sACNPacket()
        {
            delete(packedPacket);
        }

        static size_t packetSize()
        {
            return sizeof(sacn_packet_struct);
        }

        sacn_packet_struct* getPackedPacket() 
        { 
            return packedPacket; 
        }

        const sacn_packet_struct* getPackedPacket() const
        { 
            return packedPacket; 
        }

        uint8_t dmx(size_t channel)
        {
            return packedPacket->dmp.prop_val[channel-1];
        }

        std::string sourceName()
        {
            return std::string((char*)&packedPacket->frame.source_name);
        }

        int universe()
        {
            return ntohs(packedPacket->frame.universe);
        }

    private:
        sacn_packet_struct* packedPacket;
        
};