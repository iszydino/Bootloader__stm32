#include "crc.h"

uint8_t CRC_CalcChecksum(uint8_t cmd, uint8_t len, uint8_t *data)
{
    uint16_t sum;
    uint8_t i;

    sum = 0U;
    sum += cmd;
    sum += len;

    for (i = 0U; i < len; i++)
    {
        sum += data[i];
    }

    return (uint8_t)(sum & 0xFFU);
}