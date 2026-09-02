#include "app.h"
#include "protocol_public.h"


void set_uart(unsigned char uart_number,UART_HandleTypeDef * huart,DMA_HandleTypeDef * hdma_usart_rx)
{
	init_and_set_uart(uart_number,huart,hdma_usart_rx);
}

void init_all(void)
{
	//pwm_start();
//	init_LEDparameter();

//	init_task();
//	load_task();

	init_Uart_data();
	init_protocol_para();
}


void load_task(void)
{
	//Create_task(KEY_TASK_ID, key_monitor_task, KEY_TASK_MS);
	//Create_task(LED_TASK_ID, led_task, LED_TASK_MS);
	//Create_task(PWM_TASK_ID, PWM_task, PWM_TASK_MS);
	//Create_task(UART_TASK_ID, signal_trigger_task, UART_TASK_MS);
	//Create_task(SIGNAL_TASK_ID, uart_task, SIGNAL_TASK_MS);

//	Start_AllTask();
}

