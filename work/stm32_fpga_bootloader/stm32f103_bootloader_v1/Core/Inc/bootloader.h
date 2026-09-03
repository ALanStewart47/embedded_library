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
 * add uart_interrupt_handle(x); into USART IRQHandler, x:1~3
 * e.g. using USART1, add uart_interrupt_handle(1);
 * into USART1_IRQHandler() and under HAL_UART_IRQHandler(&huart1);
 *
 * @version 	V1.0 	2026-09-01 		Alan
 * @note 
 *       1 tab == 4 spaces!
 *       Configure enabled UART ports in bootloader_config.h.
 *       Enable one FPGA backend in bootloader_config.h when required.
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
#include "bootloader_config.h"
#include <stdint.h>
//******************************** Includes *********************************//

#if (BOOTLOADER_ENABLE_GOWIN_FPGA || BOOTLOADER_ENABLE_ANLOGIC_FPGA)
#define BUFFER_SIZE                2148
#else
#define BUFFER_SIZE                272
#endif

#define APPLICATION_ADDRESS_A      0x08003800U
#define BOOT_ADDRESS               0x08003400U

//******************************** Declaring ********************************//
void boot_uart_config(void);
void poweron_self_check(void);
void bsp_ota_handle(void);
void uart_interrupt_handle(uint8_t uart_number);
//******************************** Declaring ********************************//

#ifdef __cplusplus
}
#endif

#endif /* __BOOTLOADER_H */
