#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "led.h"

#define		FLASH_PAGES		1024		 
#define		URAT_NUMBER		1		 


#define    BUFFER_SIZE		2148	 
#define    UART_SUM		2


typedef enum
{
  UART_0 = 0,
  UART_1,
  UART_2,
  UART_3,
  UART_4
} UART_NUMBER;

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
void init_and_set_uart(unsigned char uart_number,UART_HandleTypeDef * huart,DMA_HandleTypeDef * hdma_usart_rx);

void uart_receive(unsigned char uart_number);

void uart_task(void);
void ACK_cmd(void);

void dma_delay_jisuan(void);
void uart_receive_number(unsigned char uart_number);


	
#ifdef __cplusplus
}
#endif

#endif  
