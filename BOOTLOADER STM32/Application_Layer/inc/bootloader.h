#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#include "stm32f10x.h"
#include <stdint.h>

/* Memory map
   0x08000000 - 0x08003FFF : Bootloader (16KB)
   0x08004000 - 0x08007FFF : Temp area  (16KB)
   0x08008000 - 0x0800FFFF : App area   (32KB)
*/

#define BOOT_START_ADDR   0x08000000U
#define BOOT_END_ADDR     0x08004000U

#define TEMP_START_ADDR   0x08004000U
#define TEMP_END_ADDR     0x08008000U

#define APP_START_ADDR    0x08008000U
#define APP_END_ADDR      0x08010000U

typedef enum
{
    BL_STATE_IDLE = 0,
    BL_STATE_WAIT_START,
    BL_STATE_RECEIVING,
    BL_STATE_FINISHED,
    BL_STATE_ERROR
} BL_State_t;

void Bootloader_Run(void);
void BL_ProcessPacket(void);
uint8_t BL_CheckAppValid(void);
void BL_JumpToApp(void);

#endif