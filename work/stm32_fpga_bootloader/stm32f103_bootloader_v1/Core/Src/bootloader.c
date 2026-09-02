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
#include "bootloader.h"

/* File-wide constants */
#define TURE                         1
#define FALSE                        2

#define STM32F103_FLASH_SIZE_ADDR    0x1FFFF7E0U
#define STM32F103_FLASH_BASE         0x08000000U
#define BOOT_WAIT_TIME               2000U

/* UART transport constants */
#define BOOT_UART_TX_BUFFER_SIZE     10U

typedef void (*pFunction)(void);

/* Bootloader update state */
unsigned char need_to_upgrade   = 1;
unsigned int package_sum        = 0;
unsigned char upgrade_bin_flag  = 0;
unsigned char success_flag      = 0;

/* Enabled UART handles and receive DMA channels */
#if BOOTLOADER_ENABLE_USART1
    extern UART_HandleTypeDef huart1;
    extern DMA_HandleTypeDef hdma_usart1_rx;
#endif
#if BOOTLOADER_ENABLE_USART2
    extern UART_HandleTypeDef huart2;
    extern DMA_HandleTypeDef hdma_usart2_rx;
#endif
#if BOOTLOADER_ENABLE_USART3
    extern UART_HandleTypeDef huart3;
    extern DMA_HandleTypeDef hdma_usart3_rx;
#endif

/* UART DMA transport context */
typedef struct
{
    uint8_t number;
    UART_HandleTypeDef *huart;
    DMA_HandleTypeDef *hdma_rx;
    volatile uint16_t rx_length;
    volatile uint8_t rx_ready;
    uint8_t rx_buffer[BUFFER_SIZE];
    uint8_t tx_buffer[BOOT_UART_TX_BUFFER_SIZE];
} boot_uart_context_t;

static boot_uart_context_t boot_uarts[] =
{
    #if BOOTLOADER_ENABLE_USART1
        {1U, &huart1, &hdma_usart1_rx, 0U, 0U, {0}, {0}},
    #endif
    #if BOOTLOADER_ENABLE_USART2
        {2U, &huart2, &hdma_usart2_rx, 0U, 0U, {0}, {0}},
    #endif
    #if BOOTLOADER_ENABLE_USART3
        {3U, &huart3, &hdma_usart3_rx, 0U, 0U, {0}, {0}},
    #endif
};

/* Depends on boot_uarts[] and therefore remains immediately below it. */
#define BOOT_UART_COUNT ((uint8_t)(sizeof(boot_uarts) / sizeof(boot_uarts[0])))

static boot_uart_context_t *active_uart = NULL;

/* Flash update state */
FLASH_EraseInitTypeDef My_Flash;
uint32_t PageError          = 0;
unsigned int pack_num       = 0;

uint32_t flash_page_size    = 0;
uint32_t flash_end_address  = 0;
unsigned char flash_info_ok = 0;

#if BOOTLOADER_ENABLE_GOWIN_FPGA
/* FPGA update state */
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

/* FPGA JTAG pin */
#define TMS_GPIO_Port           GPIOA
#define TMS_Pin                 GPIO_PIN_4
#define TCK_GPIO_Port           GPIOA
#define TCK_Pin                 GPIO_PIN_5
#define TDI_GPIO_Port           GPIOA
#define TDI_Pin                 GPIO_PIN_6
#define TDO_GPIO_Port           GPIOA
#define TDO_Pin                 GPIO_PIN_7

/* FPGA JTAG pin operation */
#define TCK_HIGH                (TCK_GPIO_Port->BSRR = TCK_Pin)
#define TCK_LOW                 (TCK_GPIO_Port->BSRR = (uint32_t)TCK_Pin << 16)
#define TMS_HIGH                (TMS_GPIO_Port->BSRR = TMS_Pin)
#define TMS_LOW                 (TMS_GPIO_Port->BSRR = (uint32_t)TMS_Pin << 16)
#define TDI_HIGH                (TDI_GPIO_Port->BSRR = TDI_Pin)
#define TDI_LOW                 (TDI_GPIO_Port->BSRR = (uint32_t)TDI_Pin << 16)
#define READ_TDO()              (((TDO_GPIO_Port->IDR & TDO_Pin) != 0U) ? 1U : 0U)
#endif

/* Private function declarations */
#if BOOTLOADER_ENABLE_GOWIN_FPGA
void delay_us(uint32_t delay_us);
void my_gpio_init(void);
void fpga_update_task(void);
void program_internal_flash(uint16_t cnt_num);
#endif
static void boot_uart_poll_mcu(void);
static void uart_data_handle(boot_uart_context_t *uart);
static void my_memset(unsigned char *dest, unsigned char set, uint16_t len);

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

void poweron_to_app(void)
{
    uint32_t i;
    uint8_t uart_index;
    uint32_t jump_address;
    pFunction jump_to_application;

    if(((*(__IO uint32_t *)APPLICATION_ADDRESS_A) & 0x2FFE0000) == 0x20000000)
    {
        __disable_irq();

        for(uart_index = 0U; uart_index < BOOT_UART_COUNT; uart_index++)
        {
            HAL_UART_Abort(boot_uarts[uart_index].huart);
            HAL_UART_DeInit(boot_uarts[uart_index].huart);
        }

        SysTick->CTRL = 0U;
        SysTick->LOAD = 0U;
        SysTick->VAL = 0U;

        for(i = 0; i < (sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0])); i++)
        {
            NVIC->ICER[i] = 0xFFFFFFFFU;
            NVIC->ICPR[i] = 0xFFFFFFFFU;
        }

        HAL_RCC_DeInit();

        jump_address = *(__IO uint32_t *)(APPLICATION_ADDRESS_A + 4U);
        jump_to_application = (pFunction)jump_address;
        SCB->VTOR = APPLICATION_ADDRESS_A;
        __DSB();
        __ISB();
        __set_MSP(*(__IO uint32_t *)APPLICATION_ADDRESS_A);
        __enable_irq();
        jump_to_application();
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
        boot_uart_poll_mcu();
        if(upgrade_bin_flag == 1)
        {
            need_to_upgrade = 1;
            return;
        }
    }

    poweron_to_app();
    need_to_upgrade = 1;
}

static boot_uart_context_t *boot_uart_find(uint8_t uart_number)
{
    uint8_t uart_index;

    for(uart_index = 0U; uart_index < BOOT_UART_COUNT; uart_index++)
    {
        if(boot_uarts[uart_index].number == uart_number)
        {
            return &boot_uarts[uart_index];
        }
    }

    return NULL;
}

static void my_memset(unsigned char *dest, unsigned char set, uint16_t len)
{
    while(len != 0U)
    {
        *dest++ = set;
        len--;
    }
}

static void boot_uart_start_receive(boot_uart_context_t *uart)
{
    uart->rx_length = 0U;
    uart->rx_ready = 0U;
    HAL_UART_Receive_DMA(uart->huart, uart->rx_buffer, BUFFER_SIZE);
}

static void boot_uart_release_frame(boot_uart_context_t *uart)
{
    my_memset(uart->rx_buffer, 0U, uart->rx_length);
    boot_uart_start_receive(uart);
}

void boot_uart_config(void)
{
    uint8_t uart_index;

    for(uart_index = 0U; uart_index < BOOT_UART_COUNT; uart_index++)
    {
        my_memset(boot_uarts[uart_index].rx_buffer, 0U, BUFFER_SIZE);
        my_memset(boot_uarts[uart_index].tx_buffer, 0U,
                  BOOT_UART_TX_BUFFER_SIZE);
        __HAL_UART_ENABLE_IT(boot_uarts[uart_index].huart, UART_IT_IDLE);
        boot_uart_start_receive(&boot_uarts[uart_index]);
    }
}

static void boot_uart_send(boot_uart_context_t *uart, const uint8_t *data,
                           uint8_t len)
{
    uint8_t i;

    if((uart == NULL) || (data == NULL) ||
       (len == 0U) || (len > BOOT_UART_TX_BUFFER_SIZE) ||
       (uart->huart->gState != HAL_UART_STATE_READY))
    {
        return;
    }

    for(i = 0U; i < len; i++)
    {
        uart->tx_buffer[i] = data[i];
    }

    HAL_UART_Transmit_DMA(uart->huart, uart->tx_buffer, len);
}

void uart_interrupt_handle(uint8_t uart_number)
{
    boot_uart_context_t *uart;
    uint16_t remaining;

    uart = boot_uart_find(uart_number);
    if((uart == NULL) || (uart->rx_ready != 0U) ||
       (__HAL_UART_GET_FLAG(uart->huart, UART_FLAG_IDLE) == RESET))
    {
        return;
    }

    remaining = __HAL_DMA_GET_COUNTER(uart->hdma_rx);
    __HAL_UART_CLEAR_IDLEFLAG(uart->huart);
    HAL_UART_AbortReceive(uart->huart);

    if(remaining <= BUFFER_SIZE)
    {
        uart->rx_length = (uint16_t)(BUFFER_SIZE - remaining);
        uart->rx_ready = 1U;
    }
    else
    {
        boot_uart_start_receive(uart);
    }
}

static void ACK_cmd(boot_uart_context_t *uart)
{
    static const uint8_t ack[] = {0x72U, 0x68U, 0x01U, 0x16U};

    boot_uart_send(uart, ack, sizeof(ack));
}

static void ACK2_cmd(boot_uart_context_t *uart)
{
    static const uint8_t ack[] = {0x72U, 0x68U, 0x02U, 0x16U};

    boot_uart_send(uart, ack, sizeof(ack));
}

static void ACK3_cmd(boot_uart_context_t *uart, uint16_t packet_number)
{
    uint8_t ack[5];

    ack[0] = 0x72U;
    ack[1] = 0x69U;
    ack[2] = (uint8_t)(packet_number >> 8);
    ack[3] = (uint8_t)packet_number;
    ack[4] = 0x16U;
    boot_uart_send(uart, ack, sizeof(ack));
}

static void false_cmd(boot_uart_context_t *uart)
{
    static const uint8_t nack[] = {0x72U, 0x68U, 0x03U, 0x16U};

    boot_uart_send(uart, nack, sizeof(nack));
}

static void BootLoaderCmd_handle(boot_uart_context_t *uart)
{
    if((uart->rx_length == 5U) && (uart->rx_buffer[0] == 0x72U) &&
       (uart->rx_buffer[1] == 0x68U) && (uart->rx_buffer[4] == 0x16U))
    {
        package_sum = (uint16_t)((uint16_t)uart->rx_buffer[2] * 256U +
                                 uart->rx_buffer[3]);
        upgrade_bin_flag = 1;
        active_uart = uart;
        ACK2_cmd(uart);
    }

    if((uart->rx_length >= 6U) && (uart->rx_buffer[0] == 0x72U) &&
       (uart->rx_buffer[1] == 0x68U) && (uart->rx_buffer[2] == 0xaaU) &&
       (uart->rx_buffer[uart->rx_length - 1U] == 0x16U))
    {
        upgrade_bin_flag = 0;
        ACK_cmd(uart);
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

static void upgradeCmd_handle(boot_uart_context_t *uart)
{
    uint16_t i;
    uint16_t pack_numb;
    uint32_t write_address;
    unsigned char res = 0x00;

    if((uart->rx_length >= 6U) && (uart->rx_buffer[0] == 0x72U) &&
       (uart->rx_buffer[1] == 0x68U) && (uart->rx_buffer[2] == 0xaaU) &&
       (uart->rx_buffer[uart->rx_length - 1U] == 0x16U))
    {
        upgrade_bin_flag = 0;
        active_uart = NULL;
        ACK_cmd(uart);
        return;
    }

    if((uart->rx_length != 262U) || (uart->rx_buffer[0] != 0x72U) ||
       (uart->rx_buffer[1] != 0x69U) || (uart->rx_buffer[261] != 0x16U))
    {
        false_cmd(uart);
        return;
    }

    for(i = 0; i < 256; i++)
    {
        res ^= uart->rx_buffer[i + 4U];
    }

    if((res != uart->rx_buffer[260]) || (flash_info_ok == 0U))
    {
        false_cmd(uart);
        success_flag = FALSE;
        return;
    }

    pack_numb = (uint16_t)((uint16_t)uart->rx_buffer[2] * 256U +
                           uart->rx_buffer[3]);
    if((pack_numb == 0) || (pack_numb > package_sum))
    {
        false_cmd(uart);
        success_flag = FALSE;
        return;
    }

    write_address = APPLICATION_ADDRESS_A + ((uint32_t)pack_numb - 1U) * 256U;

    if(FlashAddressCheck(write_address, 256U) == 0U)
    {
        false_cmd(uart);
        success_flag = FALSE;
        return;
    }

    if(((write_address - APPLICATION_ADDRESS_A) % flash_page_size) == 0U)
    {
        if(EraseOnePage(write_address) == 0U)
        {
            false_cmd(uart);
            success_flag = FALSE;
            return;
        }
    }

    if(WriteFlashData(write_address, &uart->rx_buffer[4], 256U) == 0U)
    {
        false_cmd(uart);
        success_flag = FALSE;
        return;
    }

    pack_num = pack_numb;
    ACK3_cmd(uart, pack_num);

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

static void uart_data_handle(boot_uart_context_t *uart)
{
    if(uart->rx_ready != 0U)
    {
        if((active_uart != NULL) && (active_uart != uart))
        {
            boot_uart_release_frame(uart);
            return;
        }

        if(upgrade_bin_flag == 0U)
        {
            BootLoaderCmd_handle(uart);
        }
        else
        {
            upgradeCmd_handle(uart);
        }

        boot_uart_release_frame(uart);
    }
}

static void boot_uart_poll_mcu(void)
{
    uint8_t uart_index;

    for(uart_index = 0U; uart_index < BOOT_UART_COUNT; uart_index++)
    {
        uart_data_handle(&boot_uarts[uart_index]);
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

static void fpga_send(boot_uart_context_t *uart, unsigned char *data,
                      unsigned char len)
{
    boot_uart_send(uart, data, len);
}

static void CMD_40_handle(boot_uart_context_t *uart)
{
    unsigned char *uart_buf = uart->rx_buffer;
    unsigned int length = uart->rx_length;
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
        fpga_send(uart, tx_buf, h);
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
        fpga_send(uart, tx_buf, h);
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
        fpga_send(uart, tx_buf, h);
        return;
    }

    if((length == 7) && (uart_buf[1] == 0x00) &&
       (spi_w_handle.block_size_t != 0))
    {
        spi_w_handle.size_t = ((uint32_t)uart_buf[2] << 16) +
                              ((uint32_t)uart_buf[3] << 8) +
                              uart_buf[4];
        tx_buf[h++] = (spi_w_handle.size_t == 0) ? 0x65 : 0x73;
        fpga_send(uart, tx_buf, h);
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
        fpga_send(uart, tx_buf, h);
    }
}

void fpga_update_task(void)
{
    uint8_t uart_index;

    spi_w_handle.up_cmd = 0;
    spi_w_handle.block_size_t = 0;
    active_uart = NULL;

    while(1)
    {
        for(uart_index = 0U; uart_index < BOOT_UART_COUNT; uart_index++)
        {
            if(boot_uarts[uart_index].rx_ready != 0U)
            {
                if((active_uart == NULL) ||
                   (active_uart == &boot_uarts[uart_index]))
                {
                    active_uart = &boot_uarts[uart_index];
                    CMD_40_handle(active_uart);
                }
                boot_uart_release_frame(&boot_uarts[uart_index]);
            }
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

static void boot_uart_broadcast_ack(void)
{
    uint8_t uart_index;

    for(uart_index = 0U; uart_index < BOOT_UART_COUNT; uart_index++)
    {
        ACK_cmd(&boot_uarts[uart_index]);
    }
}

static void uart_task(void)
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
        boot_uart_broadcast_ack();
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
            boot_uart_poll_mcu();
        }

        if(success_flag == FALSE)
        {
            success_flag = 0;
            need_to_upgrade = 1;
            if(active_uart != NULL)
            {
                ACK_cmd(active_uart);
            }
            else
            {
                boot_uart_broadcast_ack();
            }
        }
    }
}

void bsp_ota_handle(void)
{
    HAL_Delay(100);
    uart_task();
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
