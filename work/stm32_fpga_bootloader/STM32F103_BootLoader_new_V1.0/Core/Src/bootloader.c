/******************************************************************************
 * Copyright (C) 2024 CST, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file bootloader.c
 *
 * @par dependencies
 * - bootloader.h
 *
 * @author  imphx
 * 		    Alan 		| R&D Dept. | CST
 *
 * @brief Provide the HAL APIs of the bootloader update.
 *
 * Processing flow:
 * @version 	V1.0 	2026-09-01 		Alan
 * @note 
 *       1 tab == 4 spaces!
 *
 *****************************************************************************/
//******************************** Includes *********************************//
#include "bootloader.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define TURE                        1
#define FALSE                       2

#define STM32F103_FLASH_SIZE_ADDR   0x1FFFF7E0U
#define STM32F103_FLASH_BASE        0x08000000U
#define BOOT_WAIT_TIME               2000U

typedef void (*pFunction)(void);
pFunction JumpToApplication;
uint32_t JumpAddress;

unsigned char urat_tx_buffer[100] = {0};
unsigned char urat_tx_length = 0;

unsigned int rx_length;
unsigned char rx_buffer[BUFFER_SIZE];
unsigned char rx_endFlag;

unsigned char need_to_upgrade = 1;
unsigned int package_sum = 0;
unsigned char upgrade_bin_flag = 0;
unsigned char success_flag = 0;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_uart4_rx;

unsigned int dma_ms = 0;
unsigned int dma_ms_flag = 0;
unsigned char dma_number = 1;

FLASH_EraseInitTypeDef My_Flash;
uint32_t PageError = 0;
unsigned int pack_num = 0;

uint32_t flash_page_size = 0;
uint32_t flash_end_address = 0;
unsigned char flash_info_ok = 0;

#if BOOTLOADER_ENABLE_GOWIN_FPGA
typedef struct
{
    uint32_t addr;
    unsigned char spi_data[2048];
    unsigned char up_cmd;
    uint32_t size_t;
    uint32_t block_size_t;
} spi_w;

spi_w spi_w_handle;

typedef enum
{
    TEST_LOGIC_RESET,
    RUN_TEST_IDLE,
    SELECT_DR_SCAN,
    CAPTURE_DR,
    SHIFT_DR,
    UPDATE_DR,
    SELECT_IR_SCAN,
    CAPTURE_IR,
    SHIFT_IR,
    UPDATE_IR
} TAPState;

#define TMS_GPIO_Port               GPIOA
#define TMS_Pin                     GPIO_PIN_4
#define TCK_GPIO_Port               GPIOA
#define TCK_Pin                     GPIO_PIN_5
#define TDI_GPIO_Port               GPIOA
#define TDI_Pin                     GPIO_PIN_6
#define TDO_GPIO_Port               GPIOA
#define TDO_Pin                     GPIO_PIN_7

#define TCK_HIGH                    (TCK_GPIO_Port->BSRR = TCK_Pin)
#define TCK_LOW                     (TCK_GPIO_Port->BSRR = (uint32_t)TCK_Pin << 16)
#define TMS_HIGH                    (TMS_GPIO_Port->BSRR = TMS_Pin)
#define TMS_LOW                     (TMS_GPIO_Port->BSRR = (uint32_t)TMS_Pin << 16)
#define TDI_HIGH                    (TDI_GPIO_Port->BSRR = TDI_Pin)
#define TDI_LOW                     (TDI_GPIO_Port->BSRR = (uint32_t)TDI_Pin << 16)
#define READ_TDO()                  (((TDO_GPIO_Port->IDR & TDO_Pin) != 0U) ? 1U : 0U)
#endif
//******************************** Defines **********************************//

#if BOOTLOADER_ENABLE_GOWIN_FPGA
void delay_us(uint32_t delay_us);
void my_gpio_init(void);
void fpga_update_task(void);
void program_internal_flash(uint16_t cnt_num);
#endif
void uart_data_handle(void);

uint32_t GetFlashPageSize(void)
{
    uint16_t flash_size_kb;

    flash_size_kb = *(volatile uint16_t *)STM32F103_FLASH_SIZE_ADDR;

    switch(flash_size_kb)
    {
        case 16:
        case 32:
        case 64:
        case 128:
            return 1024U;

        case 256:
        case 384:
        case 512:
        case 768:
        case 1024:
            return 2048U;

        default:
            return 0U;
    }
}

void init_Flash_info(void)
{
    uint16_t flash_size_kb;

    flash_size_kb = *(volatile uint16_t *)STM32F103_FLASH_SIZE_ADDR;
    flash_page_size = GetFlashPageSize();

    if(flash_page_size != 0U)
    {
        flash_end_address = STM32F103_FLASH_BASE + (uint32_t)flash_size_kb * 1024U;
        flash_info_ok = 1;
    }
    else
    {
        flash_end_address = 0;
        flash_info_ok = 0;
    }
}

uint32_t GetPageAddress(uint32_t Addr)
{
    if(flash_page_size == 0U)
    {
        return 0U;
    }

    return Addr - (Addr % flash_page_size);
}

unsigned char FlashAddressCheck(uint32_t Addr, uint32_t len)
{
    if((flash_info_ok == 0U) || (Addr < STM32F103_FLASH_BASE))
    {
        return 0;
    }

    if((Addr + len) > flash_end_address)
    {
        return 0;
    }

    return 1;
}

unsigned char EraseOnePage(uint32_t Addr)
{
    if((flash_info_ok == 0U) || (Addr != GetPageAddress(Addr)) ||
       (FlashAddressCheck(Addr, flash_page_size) == 0U))
    {
        return 0;
    }

    HAL_FLASH_Unlock();

    My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;
    My_Flash.PageAddress = Addr;
    My_Flash.NbPages = 1;

    if(HAL_FLASHEx_Erase(&My_Flash, &PageError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return 0;
    }

    HAL_FLASH_Lock();
    return 1;
}

unsigned char WriteFlashData(uint32_t Addr, unsigned char *data, uint16_t len)
{
    uint16_t i;
    uint16_t Write_Flash_Data;

    if((flash_info_ok == 0U) || ((Addr & 1U) != 0U) ||
       ((len & 1U) != 0U) || (FlashAddressCheck(Addr, len) == 0U))
    {
        return 0;
    }

    HAL_FLASH_Unlock();

    for(i = 0; i < len; i += 2)
    {
        Write_Flash_Data = (uint16_t)data[i] + ((uint16_t)data[i + 1] << 8);
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                             Addr + i, Write_Flash_Data) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return 0;
        }
    }

    HAL_FLASH_Lock();
    return 1;
}

#if BOOTLOADER_ENABLE_GOWIN_FPGA
void delay_us(uint32_t delay_us)
{
    volatile unsigned int num;
    volatile unsigned int t;

    for(num = 0; num < delay_us; num++)
    {
        t = 11;
        while(t != 0)
        {
            t--;
        }
    }
}
#endif

void delay_ms(unsigned int time)
{
    unsigned int i = 0;

    while(time--)
    {
        i = 12000;
        while(i--);
    }
}

void poweron_to_app(void)
{
    uint32_t i;

    if(((*(__IO uint32_t *)APPLICATION_ADDRESS_A) & 0x2FFE0000) == 0x20000000)
    {
        __disable_irq();

        HAL_UART_DMAStop(&huart1);
        HAL_UART_DeInit(&huart1);

        SysTick->CTRL = 0U;
        SysTick->LOAD = 0U;
        SysTick->VAL = 0U;

        for(i = 0; i < (sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0])); i++)
        {
            NVIC->ICER[i] = 0xFFFFFFFFU;
            NVIC->ICPR[i] = 0xFFFFFFFFU;
        }

        HAL_RCC_DeInit();

        JumpAddress = *(__IO uint32_t *)(APPLICATION_ADDRESS_A + 4);
        JumpToApplication = (pFunction)JumpAddress;
        SCB->VTOR = APPLICATION_ADDRESS_A;
        __DSB();
        __ISB();
        __set_MSP(*(__IO uint32_t *)APPLICATION_ADDRESS_A);
        __enable_irq();
        JumpToApplication();
    }
}

void poweron_self_check(void)
{
    uint16_t data;
    uint32_t start_time;

    init_Flash_info();
    data = *(uint16_t *)BOOT_ADDRESS;

    if(data == 0x00aa)
    {
        return;
    }
#if BOOTLOADER_ENABLE_GOWIN_FPGA
    if(data == 0x0055)
    {
        return;
    }
#endif

    start_time = HAL_GetTick();
    while((HAL_GetTick() - start_time) < BOOT_WAIT_TIME)
    {
        if(rx_endFlag == 1)
        {
            uart_data_handle();
            if(upgrade_bin_flag == 1)
            {
                need_to_upgrade = 1;
                return;
            }
        }
    }

    poweron_to_app();
    need_to_upgrade = 1;
}

void init_Uart_data(void)
{
    uint16_t i;

    my_memset(urat_tx_buffer, 0, 100);
    for(i = 0; i < BUFFER_SIZE; i++)
    {
        rx_buffer[i] = 0;
    }
}

void UART_Receive_DMA(void)
{
    #if (URAT_NUMBER == 1)
    HAL_UART_Receive_DMA(&huart1, rx_buffer, BUFFER_SIZE);
    #endif
    #if (URAT_NUMBER == 2)
    HAL_UART_Receive_DMA(&huart2, rx_buffer, BUFFER_SIZE);
    #endif
    #if (URAT_NUMBER == 3)
    HAL_UART_Receive_DMA(&huart3, rx_buffer, BUFFER_SIZE);
    #endif
    #if (URAT_NUMBER == 4)
    HAL_UART_Receive_DMA(&huart4, rx_buffer, BUFFER_SIZE);
    #endif
}

void boot_uart_config(void)
{
    #if (URAT_NUMBER == 1)
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
    #endif
    #if (URAT_NUMBER == 2)
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
    #endif
    #if (URAT_NUMBER == 3)
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
    #endif
    #if (URAT_NUMBER == 4)
    __HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);
    #endif

    init_Uart_data();
    UART_Receive_DMA();
}

void DMA_Usart_Send(uint8_t *buf, uint8_t len)
{
    #if (URAT_NUMBER == 1)
    HAL_UART_Transmit_DMA(&huart1, buf, len);
    #endif
    #if (URAT_NUMBER == 2)
    HAL_UART_Transmit_DMA(&huart2, buf, len);
    #endif
    #if (URAT_NUMBER == 3)
    HAL_UART_Transmit_DMA(&huart3, buf, len);
    #endif
    #if (URAT_NUMBER == 4)
    HAL_UART_Transmit_DMA(&huart4, buf, len);
    #endif
}

void uart_receive_number(unsigned char uart_number)
{
    dma_number = uart_number;
    dma_ms_flag = 1;

    if(dma_ms_flag == 1)
    {
        delay_ms(3);
        uart_receive(dma_number);
        dma_ms_flag = 0;
        dma_ms = 0;
    }
}

#if (URAT_NUMBER == 1)
void uart_receive(unsigned char uart_number)
{
    unsigned char recv_flag;
    uint16_t numb;

    recv_flag = __HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE);

    if(recv_flag != 0)
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);
        HAL_UART_DMAStop(&huart1);

        numb = __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
        rx_length = BUFFER_SIZE - numb;
        rx_endFlag = 1;
    }
}
#endif

void uart_interrupt_handle(unsigned char uart_number)
{
    uart_receive_number(uart_number);
}

void my_memset(unsigned char *dest, unsigned char set, unsigned char len)
{
    unsigned char *pdest = dest;

    while(len != 0)
    {
        *pdest++ = set;
        len--;
    }
}

void ACK_cmd(void)
{
    urat_tx_buffer[0] = 0x72;
    urat_tx_buffer[1] = 0x68;
    urat_tx_buffer[2] = 0x01;
    urat_tx_buffer[3] = 0x16;
    urat_tx_length = 4;

    DMA_Usart_Send(urat_tx_buffer, urat_tx_length);
}

void ACK2_cmd(void)
{
    urat_tx_buffer[0] = 0x72;
    urat_tx_buffer[1] = 0x68;
    urat_tx_buffer[2] = 0x02;
    urat_tx_buffer[3] = 0x16;
    urat_tx_length = 4;

    DMA_Usart_Send(urat_tx_buffer, urat_tx_length);
}

void ACK3_cmd(unsigned int uart_number)
{
    urat_tx_buffer[0] = 0x72;
    urat_tx_buffer[1] = 0x69;
    urat_tx_buffer[2] = uart_number / 256;
    urat_tx_buffer[3] = uart_number % 256;
    urat_tx_buffer[4] = 0x16;
    urat_tx_length = 5;

    DMA_Usart_Send(urat_tx_buffer, urat_tx_length);
}

void false_cmd(void)
{
    urat_tx_buffer[0] = 0x72;
    urat_tx_buffer[1] = 0x68;
    urat_tx_buffer[2] = 0x03;
    urat_tx_buffer[3] = 0x16;
    urat_tx_length = 4;

    DMA_Usart_Send(urat_tx_buffer, urat_tx_length);
}

void BootLoaderCmd_handle(void)
{
    if((rx_length == 5) && (rx_buffer[0] == 0x72) &&
       (rx_buffer[1] == 0x68) && (rx_buffer[4] == 0x16))
    {
        package_sum = rx_buffer[2] * 256 + rx_buffer[3];
        upgrade_bin_flag = 1;
        ACK2_cmd();
    }

    if((rx_length >= 6) && (rx_buffer[0] == 0x72) &&
       (rx_buffer[1] == 0x68) && (rx_buffer[2] == 0xaa) &&
       (rx_buffer[rx_length - 1] == 0x16))
    {
        upgrade_bin_flag = 0;
        ACK_cmd();
    }
}

void set_BootLoader_flag(void)
{
    unsigned char Write_Flash_Data[2] = {0x00, 0x00};

    if(flash_info_ok == 0U)
    {
        return;
    }

    if((EraseOnePage(GetPageAddress(BOOT_ADDRESS)) == 0U) ||
       (WriteFlashData(BOOT_ADDRESS, Write_Flash_Data, 2) == 0U))
    {
        success_flag = FALSE;
    }
}

void upgradeCmd_handle(void)
{
    uint16_t i;
    uint16_t pack_numb;
    uint32_t write_address;
    unsigned char res = 0x00;

    if((rx_length >= 6) && (rx_buffer[0] == 0x72) &&
       (rx_buffer[1] == 0x68) && (rx_buffer[2] == 0xaa) &&
       (rx_buffer[rx_length - 1] == 0x16))
    {
        upgrade_bin_flag = 0;
        ACK_cmd();
        return;
    }

    if((rx_length != 262) || (rx_buffer[0] != 0x72) ||
       (rx_buffer[1] != 0x69) || (rx_buffer[261] != 0x16))
    {
        false_cmd();
        return;
    }

    for(i = 0; i < 256; i++)
    {
        res ^= rx_buffer[i + 4];
    }

    if((res != rx_buffer[260]) || (flash_info_ok == 0U))
    {
        false_cmd();
        success_flag = FALSE;
        return;
    }

    pack_numb = rx_buffer[2] * 256 + rx_buffer[3];
    if((pack_numb == 0) || (pack_numb > package_sum))
    {
        false_cmd();
        success_flag = FALSE;
        return;
    }

    write_address = APPLICATION_ADDRESS_A + ((uint32_t)pack_numb - 1U) * 256U;

    if(FlashAddressCheck(write_address, 256U) == 0U)
    {
        false_cmd();
        success_flag = FALSE;
        return;
    }

    if(((write_address - APPLICATION_ADDRESS_A) % flash_page_size) == 0U)
    {
        if(EraseOnePage(write_address) == 0U)
        {
            false_cmd();
            success_flag = FALSE;
            return;
        }
    }

    if(WriteFlashData(write_address, &rx_buffer[4], 256) == 0U)
    {
        false_cmd();
        success_flag = FALSE;
        return;
    }

    pack_num = pack_numb;
    ACK3_cmd(pack_num);

    if(pack_num == package_sum)
    {
        need_to_upgrade = 0;
        success_flag = TURE;
        set_BootLoader_flag();

        if(success_flag == TURE)
        {
            HAL_NVIC_SystemReset();
        }
    }
}

void uart_data_handle(void)
{
    if(rx_endFlag == 1)
    {
        urat_tx_length = 0;

        if(upgrade_bin_flag == 0)
        {
            BootLoaderCmd_handle();
        }
        else
        {
            upgradeCmd_handle();
        }

        my_memset(rx_buffer, 0, (unsigned char)rx_length);
        rx_length = 0;
        rx_endFlag = 0;
        UART_Receive_DMA();
    }
}

#if BOOTLOADER_ENABLE_GOWIN_FPGA
uint16_t modbus_crc(unsigned char *data, uint16_t len)
{
    uint16_t crc = 0xffff;
    uint16_t i;
    unsigned char j;

    for(i = 0; i < len; i++)
    {
        crc ^= data[i];
        for(j = 0; j < 8; j++)
        {
            if(crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xa001;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    }

    return crc;
}

void fpga_send(unsigned char *data, unsigned char len)
{
    DMA_Usart_Send(data, len);
}

void CMD_40_handle(unsigned char *uart_buf, unsigned int length)
{
    unsigned char tx_buf[10];
    unsigned char h = 0;
    uint16_t crc_temp;
    uint16_t block_size_temp;

    if(length < 3)
    {
        return;
    }

    if((uart_buf[0] == 0xfb) && (uart_buf[1] == 0xff) &&
       (uart_buf[2] == 0xfb))
    {
        tx_buf[h++] = 0xfb;
        if((spi_w_handle.up_cmd == 1) || (spi_w_handle.up_cmd == 2))
        {
            tx_buf[h++] = 0x01;
        }
        else if(spi_w_handle.up_cmd == 0xff)
        {
            tx_buf[h++] = 0x02;
            spi_w_handle.up_cmd = 0;
        }
        else
        {
            tx_buf[h++] = 0x00;
        }
        tx_buf[h++] = 0xfb;
        fpga_send(tx_buf, h);
        return;
    }

    crc_temp = modbus_crc(uart_buf, length - 2);
    if((crc_temp != ((uint16_t)uart_buf[length - 2] * 256 +
                     uart_buf[length - 1])) || (uart_buf[0] != 0xfa))
    {
        tx_buf[h++] = 0xfa;
        tx_buf[h++] = 0xff;
        tx_buf[h++] = 0xff;
        tx_buf[h++] = 0xff;
        tx_buf[h++] = 0xff;
        crc_temp = modbus_crc(tx_buf, h);
        tx_buf[h++] = crc_temp >> 8;
        tx_buf[h++] = crc_temp;
        fpga_send(tx_buf, h);
        return;
    }

    if((length == 7) && (uart_buf[1] == 0x68) && (uart_buf[2] == 0xaa))
    {
        block_size_temp = (uint16_t)uart_buf[3] * 256 + uart_buf[4];
        if((block_size_temp >= 256) && (block_size_temp <= 2048) &&
           ((block_size_temp % 256) == 0))
        {
            spi_w_handle.block_size_t = block_size_temp;
            tx_buf[h++] = 0x73;
        }
        else
        {
            tx_buf[h++] = 0x65;
        }
        fpga_send(tx_buf, h);
        return;
    }

    if((length == 7) && (uart_buf[1] == 0x00) &&
       (spi_w_handle.block_size_t != 0))
    {
        spi_w_handle.size_t = ((uint32_t)uart_buf[2] << 16) +
                              ((uint32_t)uart_buf[3] << 8) +
                              uart_buf[4];
        tx_buf[h++] = (spi_w_handle.size_t == 0) ? 0x65 : 0x73;
        fpga_send(tx_buf, h);
        if(spi_w_handle.size_t != 0)
        {
            spi_w_handle.up_cmd = 1;
        }
        return;
    }

    if((spi_w_handle.block_size_t != 0) &&
       (length == (spi_w_handle.block_size_t + 7)))
    {
        spi_w_handle.addr = ((uint32_t)uart_buf[1] << 24) +
                            ((uint32_t)uart_buf[2] << 16) +
                            ((uint32_t)uart_buf[3] << 8) +
                            uart_buf[4];
        if((spi_w_handle.addr == 0) ||
           (spi_w_handle.addr > spi_w_handle.size_t))
        {
            tx_buf[h++] = 0x65;
        }
        else
        {
            for(block_size_temp = 0; block_size_temp < spi_w_handle.block_size_t;
                block_size_temp++)
            {
                spi_w_handle.spi_data[block_size_temp] = uart_buf[5 + block_size_temp];
            }
            spi_w_handle.up_cmd = 2;
            tx_buf[h++] = 0x73;
        }
        fpga_send(tx_buf, h);
    }
}

void fpga_update_task(void)
{
    spi_w_handle.up_cmd = 0;
    spi_w_handle.block_size_t = 0;

    while(1)
    {
        if(rx_endFlag == 1)
        {
            CMD_40_handle(rx_buffer, rx_length);
            rx_length = 0;
            rx_endFlag = 0;
            UART_Receive_DMA();
        }

        if(spi_w_handle.up_cmd == 1)
        {
            program_internal_flash(0);
            spi_w_handle.up_cmd = 0;
        }
        else if(spi_w_handle.up_cmd == 2)
        {
            program_internal_flash((uint16_t)spi_w_handle.addr);
            if(spi_w_handle.addr == spi_w_handle.size_t)
            {
                spi_w_handle.up_cmd = 0xff;
                set_BootLoader_flag();
            }
            else
            {
                spi_w_handle.up_cmd = 0;
            }
        }
    }
}
#endif

void uart_task(void)
{
    uint16_t data;

    data = *(uint16_t *)BOOT_ADDRESS;
#if BOOTLOADER_ENABLE_GOWIN_FPGA
    if(data == 0x0055)
    {
        my_gpio_init();
        fpga_update_task();
    }
#endif

    if(data == 0x00aa)
    {
        need_to_upgrade = 1;
        ACK_cmd();
    }
    else if(upgrade_bin_flag == 1)
    {
        need_to_upgrade = 1;
    }
    else
    {
        need_to_upgrade = 0;
    }

    while(1)
    {
        if(need_to_upgrade == 1)
        {
            uart_data_handle();
        }

        if(success_flag == FALSE)
        {
            success_flag = 0;
            need_to_upgrade = 1;
            ACK_cmd();
        }
    }
}

void bsp_ota_handle(void)
{
    HAL_Delay(100);
    uart_task();
}

void bsp_uart_baud_reinit(void)
{
}

#if BOOTLOADER_ENABLE_GOWIN_FPGA
void jtag_clock(unsigned int clocks)
{
    unsigned int i;

    for(i = 0; i < clocks; i++)
    {
        TCK_HIGH;
        __NOP();
        TCK_LOW;
        __NOP();
    }
}

void jtag_state(TAPState state)
{
    unsigned int clocks = 1;

    if((state == TEST_LOGIC_RESET) || (state == RUN_TEST_IDLE))
    {
        clocks = 8;
    }

    if((state == RUN_TEST_IDLE) || (state == CAPTURE_DR) ||
       (state == SHIFT_DR) || (state == CAPTURE_IR) || (state == SHIFT_IR))
    {
        TMS_LOW;
    }
    else
    {
        TMS_HIGH;
    }

    jtag_clock(clocks);
}

void jtag_configure(unsigned char instruction)
{
    unsigned char bit;

    jtag_state(SELECT_DR_SCAN);
    jtag_state(SELECT_IR_SCAN);
    jtag_state(CAPTURE_IR);
    jtag_state(SHIFT_IR);

    for(bit = 0; bit < 8; bit++)
    {
        if(bit == 7)
        {
            TMS_HIGH;
        }

        if((instruction >> bit) & 1)
        {
            TDI_HIGH;
        }
        else
        {
            TDI_LOW;
        }
        jtag_clock(1);
    }

    jtag_state(UPDATE_IR);
    jtag_state(RUN_TEST_IDLE);
    delay_us(100);
}

uint32_t jtag_read_data(unsigned char width)
{
    uint32_t value = 0;
    unsigned char bit;

    jtag_state(SELECT_DR_SCAN);
    jtag_state(CAPTURE_DR);
    jtag_state(SHIFT_DR);

    for(bit = 0; bit < width; bit++)
    {
        if(bit == (width - 1))
        {
            TMS_HIGH;
        }

        TCK_HIGH;
        value = (value << 1) | READ_TDO();
        TCK_LOW;
    }

    jtag_state(UPDATE_DR);
    jtag_state(RUN_TEST_IDLE);
    return value;
}

void jtag_shift_u32(uint32_t value)
{
    unsigned char bit;

    jtag_state(SELECT_DR_SCAN);
    jtag_state(CAPTURE_DR);
    jtag_state(SHIFT_DR);

    for(bit = 0; bit < 32; bit++)
    {
        if(bit == 31)
        {
            TMS_HIGH;
        }

        if((value >> bit) & 1U)
        {
            TDI_HIGH;
        }
        else
        {
            TDI_LOW;
        }
        jtag_clock(1);
    }

    jtag_state(UPDATE_DR);
    jtag_state(RUN_TEST_IDLE);
    delay_us(20);
}

void jtag_erase_flash(void)
{
    unsigned char bit;

    jtag_configure(0x15);
    jtag_configure(0x75);
    jtag_state(RUN_TEST_IDLE);
    jtag_state(SELECT_DR_SCAN);
    jtag_state(CAPTURE_DR);
    jtag_state(SHIFT_DR);

    for(bit = 0; bit < 32; bit++)
    {
        if(bit == 31)
        {
            TMS_HIGH;
        }
        TDI_LOW;
        jtag_clock(1);
    }

    jtag_state(UPDATE_DR);
    jtag_state(RUN_TEST_IDLE);
    jtag_clock(230000);
    jtag_configure(0x3a);
    jtag_configure(0x02);
    jtag_configure(0x3c);
    HAL_Delay(15);
}

void jtag_program_page(uint32_t address, unsigned char *data)
{
    unsigned char word;
    uint32_t value;

    jtag_configure(0x15);
    jtag_configure(0x71);
    jtag_shift_u32(address);

    for(word = 0; word < 64; word++)
    {
        value = ((uint32_t)data[word * 4] << 24) |
                ((uint32_t)data[word * 4 + 1] << 16) |
                ((uint32_t)data[word * 4 + 2] << 8) |
                data[word * 4 + 3];
        jtag_shift_u32(value);
    }

    jtag_clock(9400);
}

void my_gpio_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pin = TMS_Pin | TCK_Pin | TDI_Pin;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    gpio.Pin = TDO_Pin;
    HAL_GPIO_Init(GPIOA, &gpio);
}

void program_internal_flash(uint16_t cnt_num)
{
    unsigned char page_data[256];
    uint16_t page;
    uint16_t i;
    uint16_t page_sum;
    uint32_t address;

    if(cnt_num == 0)
    {
        jtag_state(TEST_LOGIC_RESET);
        jtag_state(RUN_TEST_IDLE);
        jtag_erase_flash();
        return;
    }

    page_sum = spi_w_handle.block_size_t / 256;
    address = ((uint32_t)cnt_num - 1U) * 64U * page_sum;

    for(page = 0; page < page_sum; page++)
    {
        for(i = 0; i < 256; i++)
        {
            page_data[i] = spi_w_handle.spi_data[page * 256 + i];
        }

        if((cnt_num == 1) && (page == 0))
        {
            page_data[0] = 0x47;
            page_data[1] = 0x57;
            page_data[2] = 0x31;
            page_data[3] = 0x4e;
        }

        jtag_program_page(address + (uint32_t)page * 64U, page_data);
    }
}
#endif
