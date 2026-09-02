#include "key.h"

#define  LOCK    0
#define  UNLOCK  1

unsigned char lock = 0;
unsigned char key_short_success = 0;

unsigned char key_long_success = 0;
unsigned int key_long_press = 0;
unsigned int key_long_press2 = 0;




void mode_key_monitor(void)
{
	if(HAL_GPIO_ReadPin(S_L_R_GPIO_Port,S_L_R_Pin) == 1)
	{
		set_SPmode();
	}else if(HAL_GPIO_ReadPin(S_L_R_GPIO_Port,S_L_R_Pin) == 0)
	{
		set_DPmode();
	}
}

void lock_key_monitor(void)
{
	if(HAL_GPIO_ReadPin(S_MODE_GPIO_Port,S_MODE_Pin) == 1)
	{
		lock = UNLOCK;
	}else if(HAL_GPIO_ReadPin(S_MODE_GPIO_Port,S_MODE_Pin) == 0)
	{	
		lock = LOCK;
	}
}

void CH_key_monitor(void)
{
	if(HAL_GPIO_ReadPin(Key1_GPIO_Port,Key1_Pin) == 0)
	{
		key_delay_start();
		if((read_key_short_success() == 1)&&(HAL_GPIO_ReadPin(Key1_GPIO_Port,Key1_Pin) == 0))
		{
			set_key_short_success(0);
			key_short_success = 1;
		}
	}

	if((key_short_success == 1)&&(HAL_GPIO_ReadPin(Key1_GPIO_Port,Key1_Pin) == 1))
	{
		key_short_success = 0;
		ADD_CH_num_now();
	}
}

void ADD_key_monitor(void)
{
	if(HAL_GPIO_ReadPin(Key2_GPIO_Port,Key2_Pin) == 0)
	{
		key_delay_start();
		if((read_key_short_success() == 1)||(HAL_GPIO_ReadPin(Key2_GPIO_Port,Key2_Pin) == 0))
		{
			set_key_short_success(0);
			key_short_success = 2;
			key_long_press++;
		}
	}

	if((key_short_success == 2)&&(HAL_GPIO_ReadPin(Key2_GPIO_Port,Key2_Pin) == 1)&&(key_long_success == 0))
	{
		key_short_success = 0;
		ADD_SPDP_ShortPress();
		key_long_press = 0;
	}
	
	if(key_long_press >= 200)
	{
		key_long_success = 2;
	}
	if((key_long_success == 2)&&(HAL_GPIO_ReadPin(Key2_GPIO_Port,Key2_Pin) == 0))
	{
		ADD_SPDP_ShortPress();
		
	}
	else if((key_long_success == 2)&&(HAL_GPIO_ReadPin(Key2_GPIO_Port,Key2_Pin) == 1))
	{
		key_long_success = 0;
		key_long_press = 0;
	}
}

void DELETE_key_monitor(void)
{
	if(HAL_GPIO_ReadPin(Key3_GPIO_Port,Key3_Pin) == 0)
	{
		key_delay_start();
		if((read_key_short_success() == 1)||(HAL_GPIO_ReadPin(Key3_GPIO_Port,Key3_Pin) == 0))
		{
			set_key_short_success(0);
			key_short_success = 3;
			key_long_press2++;
		}
	}
	if((key_short_success == 3)&&(HAL_GPIO_ReadPin(Key3_GPIO_Port,Key3_Pin) == 1))
	{
		key_short_success = 0;
		DELETE_SPDP_ShortPress();
		key_long_press2 = 0;
	}

	if(key_long_press2 >= 200)
	{
		key_long_success = 3;
	}
	if((key_long_success == 3)&&(HAL_GPIO_ReadPin(Key3_GPIO_Port,Key3_Pin) == 0))
	{
		DELETE_SPDP_ShortPress();
		
	}
	else if((key_long_success == 3)&&(HAL_GPIO_ReadPin(Key3_GPIO_Port,Key3_Pin) == 1))
	{
		key_long_success = 0;
		key_long_press2 = 0;
	}
}



void key_monitor_task(void)
{
	lock_key_monitor();

	if(lock == LOCK){
		return;
	}

	mode_key_monitor();

	CH_key_monitor();
	ADD_key_monitor();
	DELETE_key_monitor();
}




