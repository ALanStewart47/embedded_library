/******************************************************************************
 * Copyright (C) 2024 CST, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file bootloader.h
 *
 *  * @author   imphx
 * 		        Alan    | R&D Dept. | CST
 *
 * @brief Provide the HAL APIs of the bootloader update.
 *
 * Processing flow:
 * add poweron_self_check() into USER CODE BEGIN 1
 * add boot_uart_config(); into USER CODE BEGIN 2
 * add bsp_ota_handle(); into while(1)
 * add uart_interrupt_handle(x); into USART IRQHandler, x:1~4
 * e.g. using uart_1, add uart_interrupt_handle(1);
 * into USART1_IRQHandler() and under HAL_UART_IRQHandler(&huart1);
 *
 * @version 	V1.0 	2026-09-01 		Alan
 * @note 
 *       1 tab == 4 spaces!
 *       Set BOOTLOADER_ENABLE_GOWIN_FPGA to 1 to compile Gowin FPGA upgrade.
 *       Default 0 is MCU-only and fits the current 12 KB Bootloader ROM.
 *       FPGA=1 currently overflows 0x3000; enlarge Bootloader IROM first.
 *       The MDK Target preprocessor can also define it to override the default.
 *
 *****************************************************************************/
#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#ifdef __cplusplus
extern "C" {
#endif
//******************************** Includes *********************************//
#include "main.h"
#include <stdint.h>
//******************************** Includes *********************************//

#define URAT_NUMBER                1       // UART number 1\2\3\4

#ifndef BOOTLOADER_ENABLE_GOWIN_FPGA
#define BOOTLOADER_ENABLE_GOWIN_FPGA  0    /* 1: compile Gowin FPGA upgrade */
#endif

#if BOOTLOADER_ENABLE_GOWIN_FPGA
#define BUFFER_SIZE                2148
#else
#define BUFFER_SIZE                272
#endif
#define UART_SUM                   1

#define APPLICATION_ADDRESS_A      0x08003800U
#define BOOT_ADDRESS               0x08003400U

#define RE_UART                    huart1

//******************************** Declaring ********************************//
typedef enum
{
    UART_0 = 0,
    UART_1,
    UART_2,
    UART_3,
    UART_4
} UART_NUMBER;

typedef enum
{
    BAUD_DETECT = 0,
    BAUD_4800 = 1,
    BAUD_9600 = 2,
    BAUD_14400 = 3,
    BAUD_19200 = 4,
    BAUD_28800 = 5,
    BAUD_38400 = 6,
    BAUD_57600 = 7,
    BAUD_115200 = 8,
    BAUD_MAX
} uart_bauds_t;

typedef struct __UART_RX_BUF
{
    unsigned char enable;
    unsigned char uart_number;
    unsigned char rx_length;
    unsigned char rx_buffer[BUFFER_SIZE];
    unsigned char rx_endFlag;
    UART_HandleTypeDef *huart;
    DMA_HandleTypeDef *hdma_usart_rx;
} UART_rx_buf;

void UART_Receive_DMA(void);
void my_memset(unsigned char *dest, unsigned char set, unsigned char len);
void init_Uart_data(void);
void boot_uart_config(void);
void uart_receive(unsigned char uart_number);
void poweron_self_check(void);
void uart_task(void);
void ACK_cmd(void);
void bsp_ota_handle(void);
void bsp_uart_baud_reinit(void);
void uart_interrupt_handle(unsigned char uart_number);
//******************************** Declaring ********************************//

#ifdef __cplusplus
}
#endif

#endif /* __BOOTLOADER_H */
