#ifndef __CRC_H
#define __CRC_H

#include <stdint.h>

uint8_t CRC_CalcChecksum(uint8_t cmd, uint8_t len, uint8_t *data);

#endif