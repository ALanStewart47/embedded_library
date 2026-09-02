/******************************************************************************
 * Copyright (C) 2024 CST, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file bootloader.h
 *
 * @par dependencies
 *
 * @author imphx
 * 		   Alan 
 * 					| R&D Dept. | CST
 *
 * @brief Provide the HAL APIs of the bootloader update.
 *
 * Processing flow:
 * add poweron_self_check() into  USER CODE BEGIN 1
 * add boot_uart_config();  into USER CODE BEGIN 2
 * add bsp_uart_baud_reinit(); if you need it.
 * add bsp_ota_handle(); into while(1)
 * add uart_interrupt_handle(x); into USART IRQHandler ,x:1~4 
 * 		e.g. using uart_1 , add uart_interrupt_handle(1); 
 * 			into USART1_IRQHandler(); and under the HAL_UART_IRQHandler(&huart1);
 *
 * @version 	V1.0 	2025-08-04 		Alan
 * @note 
 *       1 tab == 4 spaces!
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

#define	FLASH_PAGES				2048	//1024or2048
#define URAT_NUMBER				2		//串口号 1\2\3\4


#define BUFFER_SIZE				300
#define UART_SUM				1

#define APPLICATION_ADDRESS_A  	0x8003800	

#define BOOT_ADDRESS			0x8003400

#define BAUD_DATA_SAVE_ADDR		((uint32_t)(0x801F000))		//波特率数据保存地址		//2K

#define	BaudDataSucFlag			0x99						//波特率-写入成功标志位

#define RE_UART                 huart2

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
	BAUD_DETECT 	= 0,
	BAUD_4800 		= 1,
	BAUD_9600 		= 2,
	BAUD_14400 		= 3,
	BAUD_19200 		= 4, 
	BAUD_28800 		= 5,
	BAUD_38400 		= 6, 
	BAUD_57600 		= 7, 
	BAUD_115200 	= 8,
    BAUD_MAX
}uart_bauds_t;

typedef struct __UART_RX_BUF{
	unsigned char enable;
	unsigned char uart_number;
	unsigned char rx_length;
	unsigned char rx_buffer[BUFFER_SIZE];
	unsigned char rx_endFlag;
	UART_HandleTypeDef * huart;
	DMA_HandleTypeDef * hdma_usart_rx;
}UART_rx_buf;

void UART_Receive_DMA(void);
void my_memset(unsigned char *dest, unsigned char set, unsigned char len);
void init_Uart_data(void);
void boot_uart_config(void);
void uart_receive(unsigned char uart_number);
void poweron_self_check(void);
void uart_task(void);
void ACK_cmd(void);
void bsp_ota_handle(void);
void bsp_uart_baud_reinit(void );
void uart_interrupt_handle(unsigned char uart_number);
	
//******************************** Declaring ********************************//

#ifdef __cplusplus
}
#endif

#endif /* __BOOTLOADER_H */
