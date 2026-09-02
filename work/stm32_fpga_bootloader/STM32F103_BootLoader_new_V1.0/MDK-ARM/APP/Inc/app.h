#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "time.h"
#include "key.h"
#include "led.h"
#include "pwm.h"
#include "uart.h"
#include "rtos.h"


#define    TASK_TIME_OUT		1
#define NVIC_VectTab_RAM             ((uint32_t)0x20000000)
#define NVIC_VectTab_FLASH           ((uint32_t)0x08000000)

#define APPLICATION_POSADDR 	  (0x0000C000)  



void set_uart(unsigned char uart_number,UART_HandleTypeDef * huart,DMA_HandleTypeDef * hdma_usart_rx);
void init_all(void);
void app_task_polling(void);


void load_task(void);

	
#ifdef __cplusplus
}
#endif

#endif  
