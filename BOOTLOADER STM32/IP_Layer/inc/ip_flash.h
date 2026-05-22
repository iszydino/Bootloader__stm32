#ifndef __IP_FLASH_H
#define __IP_FLASH_H

#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include <stdint.h>

#define FLASH_PAGE_SIZE   1024U

void IP_FLASH_Unlock(void);
void IP_FLASH_Lock(void);
uint8_t IP_FLASH_ErasePage(uint32_t address);
uint8_t IP_FLASH_ProgramWord(uint32_t address, uint32_t data);

#endif