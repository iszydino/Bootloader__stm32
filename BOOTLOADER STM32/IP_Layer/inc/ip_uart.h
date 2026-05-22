#ifndef __IP_UART_H
#define __IP_UART_H

#include "stm32f10x.h"
#include <stdint.h>

void IP_UART_Init(void);
void IP_UART_SendByte(uint8_t data);
uint8_t IP_UART_ReceiveByte(void);
uint8_t IP_UART_Available(void);

#endif