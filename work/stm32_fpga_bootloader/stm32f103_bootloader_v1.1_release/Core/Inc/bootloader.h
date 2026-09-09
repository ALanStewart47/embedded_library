/******************************************************************************
 * Copyright (C) 2024 CST, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file bootloader.h
 *
 * @author  Alan    | R&D Dept. | CST
 *
 * @brief Bootloader update functions.
 *
 * Use poweron_self_check() in USER CODE BEGIN 1,
 * boot_uart_config() in USER CODE BEGIN 2, and bsp_ota_handle() in while(1).
 * Call uart_interrupt_handle(1..3) from the matching USART IRQ handler.
 *
 * @version 	V1.0 	2026-09-01 		Alan
 *				V1.1	2026-09-07		Alan	- Add CH9121 CFG/RST(PC5/PB2) hold high
 *												- BOOT_WAIT_TIME default 5 s
 * @note 
 *       1 tab == 4 spaces!
 *       Configure the options in the User configuration section below.
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

//******************************** Config **********************************//
#ifndef BL_ENABLE_USART1
#define BL_ENABLE_USART1                1
#endif

#ifndef BL_ENABLE_USART2
#define BL_ENABLE_USART2                0
#endif

#ifndef BL_ENABLE_USART3
#define BL_ENABLE_USART3                1
#endif

#ifndef BL_ENABLE_CH9121
#define BL_ENABLE_CH9121                1
#endif

/* Use LL_GPIO_PIN_n (not GPIO_PIN_n). CFG default PC5, RST default PB2. */
#ifndef BL_CH9121_CFG_PORT
#define BL_CH9121_CFG_PORT              GPIOC
#endif
#ifndef BL_CH9121_CFG_PIN
#define BL_CH9121_CFG_PIN               LL_GPIO_PIN_5
#endif
#ifndef BL_CH9121_RST_PORT
#define BL_CH9121_RST_PORT              GPIOB
#endif
#ifndef BL_CH9121_RST_PIN
#define BL_CH9121_RST_PIN               LL_GPIO_PIN_2
#endif

#if !(BL_ENABLE_USART1) && \
    !(BL_ENABLE_USART2) && \
    !(BL_ENABLE_USART3)
#error "At least one Bootloader USART must be enabled"
#endif

#ifndef BL_ENABLE_GOWIN_FPGA
#define BL_ENABLE_GOWIN_FPGA            0
#endif

#ifndef BL_ENABLE_ANLOGIC_FPGA
#define BL_ENABLE_ANLOGIC_FPGA          1
#endif

#ifndef BOOT_WAIT_TIME
#define BOOT_WAIT_TIME                  5000U /* milliseconds */
#endif

/* Both 0 = MCU-only; enable at most one FPGA backend. */
#if (BL_ENABLE_GOWIN_FPGA) && \
    (BL_ENABLE_ANLOGIC_FPGA)
#error "Gowin and Anlogic FPGA are mutually exclusive"
#endif
//******************************** Config **********************************//

#if (BL_ENABLE_GOWIN_FPGA || BL_ENABLE_ANLOGIC_FPGA)
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
#if BL_ENABLE_CH9121
void ch9121_gpio_init(void);
#endif
//******************************** Declaring ********************************//

#ifdef __cplusplus
}
#endif

#endif /* __BOOTLOADER_H */
