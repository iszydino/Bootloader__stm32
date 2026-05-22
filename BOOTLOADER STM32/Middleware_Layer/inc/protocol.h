#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>

#define SOF                0xAAU

#define CMD_START_UPDATE   0x01U
#define CMD_WRITE_DATA     0x02U
#define CMD_END_UPDATE     0x03U
#define CMD_JUMP_APP       0x04U

#define ACK                0x79U
#define NACK               0x1FU

#define PROTOCOL_MAX_DATA  32U

typedef struct
{
    uint8_t sof;
    uint8_t cmd;
    uint8_t len;
    uint8_t data[PROTOCOL_MAX_DATA];
    uint8_t checksum;
} Protocol_Packet_t;

uint8_t Protocol_ReceivePacket(Protocol_Packet_t *packet);

#endif