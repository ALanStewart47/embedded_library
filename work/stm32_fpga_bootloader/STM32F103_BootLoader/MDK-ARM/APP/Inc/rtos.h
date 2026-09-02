#ifndef __RTOS_H
#define __RTOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"


#define    KEY_TASK_MS  	 10
#define    LED_TASK_MS  	 2
#define    PWM_TASK_MS  	 1
#define    UART_TASK_MS      50
#define    SIGNAL_TASK_MS      5

#define    TASK_SUM  	 7

typedef void (*task_fun) (void);

typedef enum
{
	KEY_TASK_ID = 0,
	LED_TASK_ID,
	PWM_TASK_ID,
	UART_TASK_ID,
	SIGNAL_TASK_ID,
	ALL_TASK_SUM,
	FREE_TASK_ID
} TASK_ID;

typedef struct __TASK_Delay
{
	unsigned char task_ms;
	unsigned char task_flag;
} TASK_Delay;


typedef enum
{
	TASK_FREE = 0,
	TASK_CREATE,
	TASK_START,
	TASK_STOP
} TASK_STATE;


typedef struct __TASK_Parameter
{
	TASK_ID task_id;
	TASK_STATE state;
	TASK_Delay delay_count;
	unsigned char task_SumDelay_time;
	task_fun func;
} TASK_Parameter;



void init_task(void);
void Create_task(TASK_ID task_id, task_fun func, unsigned char task_time);
void Start_AllTask(void);
void systick_TaskHandler(void);


	
#ifdef __cplusplus
}
#endif

#endif  
