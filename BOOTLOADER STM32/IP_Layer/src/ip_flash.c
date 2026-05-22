#include "ip_flash.h"

void IP_FLASH_Unlock(void)
{
    FLASH_Unlock();
}

void IP_FLASH_Lock(void)
{
    FLASH_Lock();
}

uint8_t IP_FLASH_ErasePage(uint32_t address)
{
    FLASH_Status status;

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    status = FLASH_ErasePage(address);

    if (status == FLASH_COMPLETE)
    {
        return 1U;
    }

    return 0U;
}

uint8_t IP_FLASH_ProgramWord(uint32_t address, uint32_t data)
{
    FLASH_Status status;

    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    status = FLASH_ProgramWord(address, data);

    if (status == FLASH_COMPLETE)
    {
        return 1U;
    }

    return 0U;
}