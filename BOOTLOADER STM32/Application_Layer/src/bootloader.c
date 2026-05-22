#include "bootloader.h"
#include "protocol.h"
#include "ip_uart.h"
#include "ip_flash.h"

static Protocol_Packet_t g_packet;
static BL_State_t g_blState = BL_STATE_IDLE;
static uint32_t g_writeAddress = TEMP_START_ADDR;
static uint32_t g_fwSize = 0U;

static void BL_SendAck(void)
{
    IP_UART_SendByte(ACK);
}

static void BL_SendNack(void)
{
    IP_UART_SendByte(NACK);
}

uint8_t BL_CheckAppValid(void)
{
    uint32_t appStack;
    uint32_t appEntry;

    appStack = *(volatile uint32_t*)APP_START_ADDR;
    appEntry = *(volatile uint32_t*)(APP_START_ADDR + 4U);

    /* Stack pointer phai nam trong RAM */
    if ((appStack & 0x2FFE0000U) != 0x20000000U)
    {
        return 0U;
    }

    /* Reset handler phai nam trong vung app */
    if ((appEntry < APP_START_ADDR) || (appEntry >= APP_END_ADDR))
    {
        return 0U;
    }

    return 1U;
}

void BL_JumpToApp(void)
{
    uint32_t appStack;
    uint32_t appEntry;
    void (*App_ResetHandler)(void);

    appStack = *(volatile uint32_t*)APP_START_ADDR;
    appEntry = *(volatile uint32_t*)(APP_START_ADDR + 4U);

    App_ResetHandler = (void (*)(void))appEntry;

    __disable_irq();

    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL  = 0U;

    SCB->VTOR = APP_START_ADDR;
    __set_MSP(appStack);

    App_ResetHandler();
}

static void BL_EraseTempArea(void)
{
    uint32_t addr;

    IP_FLASH_Unlock();

    for (addr = TEMP_START_ADDR; addr < TEMP_END_ADDR; addr += FLASH_PAGE_SIZE)
    {
        IP_FLASH_ErasePage(addr);
    }

    IP_FLASH_Lock();
}

static void BL_EraseAppArea(void)
{
    uint32_t addr;

    IP_FLASH_Unlock();

    for (addr = APP_START_ADDR; addr < APP_END_ADDR; addr += FLASH_PAGE_SIZE)
    {
        IP_FLASH_ErasePage(addr);
    }

    IP_FLASH_Lock();
}

static uint8_t BL_WriteDataToTemp(uint8_t *data, uint8_t len)
{
    uint32_t word;
    uint8_t i;

    IP_FLASH_Unlock();

    i = 0U;
    while (i < len)
    {
        if (g_writeAddress >= TEMP_END_ADDR)
        {
            IP_FLASH_Lock();
            return 0U;
        }

        word = 0xFFFFFFFFU;

        if (i < len)
        {
            word = (uint32_t)data[i];
        }
        if ((i + 1U) < len)
        {
            word |= ((uint32_t)data[i + 1U] << 8);
        }
        if ((i + 2U) < len)
        {
            word |= ((uint32_t)data[i + 2U] << 16);
        }
        if ((i + 3U) < len)
        {
            word |= ((uint32_t)data[i + 3U] << 24);
        }

        if (IP_FLASH_ProgramWord(g_writeAddress, word) == 0U)
        {
            IP_FLASH_Lock();
            return 0U;
        }

        g_writeAddress += 4U;
        i += 4U;
    }

    IP_FLASH_Lock();
    return 1U;
}

static void BL_CopyTempToApp(void)
{
    uint32_t src;
    uint32_t dst;
    uint32_t data;
    uint32_t copied;
    uint32_t copySize;

    BL_EraseAppArea();

    copySize = (g_fwSize + 3U) & (~3U);

    IP_FLASH_Unlock();

    src = TEMP_START_ADDR;
    dst = APP_START_ADDR;
    copied = 0U;

    while (copied < copySize)
    {
        data = *(volatile uint32_t*)src;

        if (IP_FLASH_ProgramWord(dst, data) == 0U)
        {
            break;
        }

        src += 4U;
        dst += 4U;
        copied += 4U;
    }

    IP_FLASH_Lock();
}

void BL_ProcessPacket(void)
{
    uint8_t ok;

    ok = Protocol_ReceivePacket(&g_packet);

    if (ok == 0U)
    {
        BL_SendNack();
        g_blState = BL_STATE_ERROR;
        return;
    }

    switch (g_packet.cmd)
    {
        case CMD_START_UPDATE:
            g_writeAddress = TEMP_START_ADDR;
            g_fwSize = 0U;
            BL_EraseTempArea();
            g_blState = BL_STATE_RECEIVING;
            BL_SendAck();
            break;

        case CMD_WRITE_DATA:
            if (g_blState == BL_STATE_RECEIVING)
            {
                if (BL_WriteDataToTemp(g_packet.data, g_packet.len) == 1U)
                {
                    g_fwSize += g_packet.len;
                    BL_SendAck();
                }
                else
                {
                    g_blState = BL_STATE_ERROR;
                    BL_SendNack();
                }
            }
            else
            {
                BL_SendNack();
            }
            break;

        case CMD_END_UPDATE:
            if (g_blState == BL_STATE_RECEIVING)
            {
                BL_CopyTempToApp();
                g_blState = BL_STATE_FINISHED;

                BL_SendAck();

                if (BL_CheckAppValid() == 1U)
                {
                    BL_JumpToApp();
                }
            }
            else
            {
                BL_SendNack();
            }
            break;

        case CMD_JUMP_APP:
            if (BL_CheckAppValid() == 1U)
            {
                BL_SendAck();
                BL_JumpToApp();
            }
            else
            {
                BL_SendNack();
            }
            break;

        default:
            BL_SendNack();
            break;
    }
}

void Bootloader_Run(void)
{
    uint32_t timeout;

    IP_UART_Init();

    g_blState = BL_STATE_WAIT_START;
    g_writeAddress = TEMP_START_ADDR;
    g_fwSize = 0U;

    /* cho 1 khoang thoi gian de PC gui lenh update */
    timeout = 0U;
    while (timeout < 3000000U)
    {
        if (IP_UART_Available() == 1U)
        {
            break;
        }
        timeout++;
    }

    /* neu khong co du lieu UART va app hop le thi nhay vao app */
    if (IP_UART_Available() == 0U)
    {
        if (BL_CheckAppValid() == 1U)
        {
            BL_JumpToApp();
        }
    }

    /* neu co du lieu thi vao bootloader de nhan firmware */
    while (1)
    {
        BL_ProcessPacket();
    }
}