#include "protocol.h"
#include "ip_uart.h"
#include "crc.h"

uint8_t Protocol_ReceivePacket(Protocol_Packet_t *packet)
{
    uint8_t calc;
    uint8_t i;
    uint8_t byteIn;

    if (packet == 0)
    {
        return 0U;
    }

    /* Tim byte SOF */
    while (1)
    {
        byteIn = IP_UART_ReceiveByte();

        if (byteIn == SOF)
        {
            packet->sof = byteIn;
            break;
        }
    }

    packet->cmd = IP_UART_ReceiveByte();
    packet->len = IP_UART_ReceiveByte();

    if (packet->len > PROTOCOL_MAX_DATA)
    {
        return 0U;
    }

    for (i = 0U; i < packet->len; i++)
    {
        packet->data[i] = IP_UART_ReceiveByte();
    }

    packet->checksum = IP_UART_ReceiveByte();

    calc = CRC_CalcChecksum(packet->cmd, packet->len, packet->data);

    if (calc != packet->checksum)
    {
        return 0U;
    }

    return 1U;
}