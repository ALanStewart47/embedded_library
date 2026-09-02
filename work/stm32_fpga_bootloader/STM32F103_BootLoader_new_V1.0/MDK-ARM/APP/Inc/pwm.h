#ifndef __PWM_H
#define __PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "led.h"
#include "time.h"


typedef struct __SIGNAL_Array
{
	unsigned char signal_1;
	unsigned char signal_2;
	unsigned char signal_3;
	unsigned char signal_4;
	unsigned char signal_5;
	unsigned char signal_6;
	unsigned char signal_7;
	unsigned char signal_8;
} SIGNAL_Array;


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim9;
extern TIM_HandleTypeDef htim12;

void pwm_start(void);
void PWM_task(void);
void signal_trigger_task(void);

	
#ifdef __cplusplus
}
#endif

#endif  
