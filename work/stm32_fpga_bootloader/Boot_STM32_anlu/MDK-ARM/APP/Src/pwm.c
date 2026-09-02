#include "pwm.h"

#define    PWM1   0
#define    PWM2   1
#define    PWM3   2
#define    PWM4   3
#define    PWM5   4
#define    PWM6   5
#define    PWM7   6
#define    PWM8   7

#define    SIGNAL_SUCCESS   1
#define    SIGNAL_NO	   	0


unsigned int signal_array[8]={0,0,0,0,0,0,0,0};


void pwm_start(void)
{
	unsigned char i = 0;

	for(i = 0; i < 8; i++)
	{
		signal_array[i] = 0;
		
	}
	
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  //TIME1		PWM8
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);  //TIME2		PWM3
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);  //TIME3		PWM2
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);  //TIME4		PWM1
	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);  //TIME5		PWM4
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);  //TIME8		PWM7
	HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);  //TIME9		PWM5
	HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);  //TIME12	PWM6

	HAL_GPIO_WritePin(OCP_RESET_GPIO_Port, OCP_RESET_Pin, GPIO_PIN_RESET);
}

void set_pwm_drive(unsigned char pwm_num, unsigned char duty ,unsigned char mode)
{
	if((signal_array[pwm_num] != SIGNAL_SUCCESS)||(mode == SP_MODE))
	{
		switch (pwm_num)
		{
			case PWM1:
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,duty);
				break;
			case PWM2:
				__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,duty);
				break;
			case PWM3:
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,duty);
				break;
			case PWM4:
				__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1,duty);
				break;
			case PWM5:
				__HAL_TIM_SET_COMPARE(&htim9,TIM_CHANNEL_1,duty);
				break;
			case PWM6:
				__HAL_TIM_SET_COMPARE(&htim12,TIM_CHANNEL_1,duty);
				break;
			case PWM7:
				__HAL_TIM_SET_COMPARE(&htim8,TIM_CHANNEL_1,duty);
				break;
			case PWM8:
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,duty);  //最�?000 255
				break;
		}
	}
}


void pwm_update() //数字模式下是占空�?闪频模式是百分百输出
{
	unsigned char i = 0;
	unsigned char horl = 1;

	horl = get_horl();

	if(horl == LIGHT_UP)		//常亮
	{	
		for(i=0; i<8; i++)
		{
			set_pwm_drive(i, get_DP_PWM(i), DP_MODE);
		}
	}else if(horl == LIGHT_OUT)
	{
		for(i=0; i<8; i++)
		{
			if(signal_array[i] != SIGNAL_SUCCESS){   
				set_pwm_drive(i, 0 ,DP_MODE);
			}
		}
	}
}

unsigned int sp_cishu[8]={1,1,1,1,1,1,1,1};
void SPmode(void)
{
	unsigned char i = 0;
	
	for(i=0; i<8; i++)
	{
		if(SP_time_out_flag(i) == 1)
		{
			set_pwm_drive(i, 0 ,SP_MODE);
		}
		if(signal_array[i] == SIGNAL_NO)
		{
			sp_cishu[i] = 1;
		}
	}
}

void PWM_task(void)
{
	unsigned char mode = 1;

	mode = get_DPSP_mode();


	if(mode == DP_MODE)			//数字模式

	{
		pwm_update();
	}
	else if(mode == SP_MODE)

	{
		SPmode();
	}
}

void check_signal(unsigned char horl)
{


	if(HAL_GPIO_ReadPin(Trigger2_GPIO_Port,Trigger2_Pin) == 1)
	{
		if(horl == LIGHT_UP){
			signal_array[PWM2] = SIGNAL_SUCCESS;
		}else{
			signal_array[PWM2] = SIGNAL_NO;
		}
	}else{
		if(horl == LIGHT_OUT){
			signal_array[PWM2] = SIGNAL_SUCCESS;
		}else{
			signal_array[PWM2] = SIGNAL_NO;
		}
	}

	if(HAL_GPIO_ReadPin(Trigger3_GPIO_Port,Trigger3_Pin) == 1)
	{
		if(horl == LIGHT_UP){
			signal_array[PWM3] = SIGNAL_SUCCESS;
		}else{
			signal_array[PWM3] = SIGNAL_NO;
		}
	}else{
		if(horl == LIGHT_OUT){
			signal_array[PWM3] = SIGNAL_SUCCESS;
		}else{
			signal_array[PWM3] = SIGNAL_NO;
		}
	}

	if(HAL_GPIO_ReadPin(Trigger4_GPIO_Port,Trigger4_Pin) == 1)
	{
		if(horl == LIGHT_UP){
			signal_array[PWM4] = SIGNAL_SUCCESS;
		}else{
			signal_array[PWM4] = SIGNAL_NO;
		}
	}else{
		if(horl == LIGHT_OUT){
			signal_array[PWM4] = SIGNAL_SUCCESS;
		}else{
			signal_array[PWM4] = SIGNAL_NO;
		}
	}

	



	if(HAL_GPIO_ReadPin(Trigger7_GPIO_Port,Trigger7_Pin) == 1)
	{
		if(horl == LIGHT_UP){
			signal_array[PWM7] = SIGNAL_SUCCESS;
		}else{
			signal_array[PWM7] = SIGNAL_NO;
		}
	}else{
		if(horl == LIGHT_OUT){
			signal_array[PWM7] = SIGNAL_SUCCESS;
		}else{
			signal_array[PWM7] = SIGNAL_NO;
		}
	}


}


void signal_handle(unsigned char mode, unsigned char horl)
{
	if(signal_array[PWM1] == SIGNAL_SUCCESS){
		if(mode == DP_MODE)
		{
			if(horl == LIGHT_UP){
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,0);
			}else{
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,get_DP_PWM(PWM1));
			}
		}else{
			if(sp_cishu[PWM1] == 1){
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,255);
				SP_delay_start(PWM1,get_SP_time(PWM1));
			}
			sp_cishu[PWM1] = 2;
		}
	}

	if(signal_array[PWM2] == SIGNAL_SUCCESS){
		if(mode == DP_MODE)
		{
			if(horl == LIGHT_UP){
				__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,0);
			}else{
				__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,get_DP_PWM(PWM2));
			}
		}else{
			if(sp_cishu[PWM2] == 1){
				__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,255);
				SP_delay_start(PWM2,get_SP_time(PWM2));
			}
			sp_cishu[PWM2] = 2;
		}
	}

	if(signal_array[PWM3] == SIGNAL_SUCCESS){
		if(mode == DP_MODE)
		{
			if(horl == LIGHT_UP){
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,0);
			}else{
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,get_DP_PWM(PWM3));
			}
		}else{
			if(sp_cishu[PWM3] == 1){
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,255);
				SP_delay_start(PWM3,get_SP_time(PWM3));
			}
			sp_cishu[PWM3] = 2;
		}
	}

	if(signal_array[PWM4] == SIGNAL_SUCCESS){
		if(mode == DP_MODE)
		{
			if(horl == LIGHT_UP){
				__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1,0);
			}else{
				__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1,get_DP_PWM(PWM4));
			}
		}else{
			if(sp_cishu[PWM4] == 1){
				__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1,255);
				SP_delay_start(PWM4,get_SP_time(PWM4));
			}
			sp_cishu[PWM4] = 2;
		}
	}

	if(signal_array[PWM5] == SIGNAL_SUCCESS){
		if(mode == DP_MODE)
		{
			if(horl == LIGHT_UP){
				__HAL_TIM_SET_COMPARE(&htim9,TIM_CHANNEL_1,0);
			}else{
				__HAL_TIM_SET_COMPARE(&htim9,TIM_CHANNEL_1,get_DP_PWM(PWM5));
			}
		}else{
			if(sp_cishu[PWM5] == 1){
				__HAL_TIM_SET_COMPARE(&htim9,TIM_CHANNEL_1,255);
				SP_delay_start(PWM5,get_SP_time(PWM5));
			}
			sp_cishu[PWM5] = 2;
		}
	}

	if(signal_array[PWM6] == SIGNAL_SUCCESS){
		if(mode == DP_MODE)
		{
			if(horl == LIGHT_UP){
				__HAL_TIM_SET_COMPARE(&htim12,TIM_CHANNEL_1,0);
			}else{
				__HAL_TIM_SET_COMPARE(&htim12,TIM_CHANNEL_1,get_DP_PWM(PWM6));
			}
		}else{
			if(sp_cishu[PWM6] == 1){
				__HAL_TIM_SET_COMPARE(&htim12,TIM_CHANNEL_1,255);
				SP_delay_start(PWM6,get_SP_time(PWM6));
			}
			sp_cishu[PWM6] = 2;
		}
	}

	if(signal_array[PWM7] == SIGNAL_SUCCESS){
		if(mode == DP_MODE)
		{
			if(horl == LIGHT_UP){
				__HAL_TIM_SET_COMPARE(&htim8,TIM_CHANNEL_1,0);
			}else{
				__HAL_TIM_SET_COMPARE(&htim8,TIM_CHANNEL_1,get_DP_PWM(PWM7));
			}
		}else{
			if(sp_cishu[PWM7] == 1){
				__HAL_TIM_SET_COMPARE(&htim8,TIM_CHANNEL_1,255);
				SP_delay_start(PWM7,get_SP_time(PWM7));
			}
			sp_cishu[PWM7] = 2;
		}
	}

	if(signal_array[PWM8] == SIGNAL_SUCCESS){
		if(mode == DP_MODE)
		{
			if(horl == LIGHT_UP){
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,0);
			}else{
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,get_DP_PWM(PWM8));
			}
		}else{
			if(sp_cishu[PWM8] == 1){
				__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,255);
				SP_delay_start(PWM8,get_SP_time(PWM8));
			}
			sp_cishu[PWM8] = 2;
		}
	}
}

void signal_trigger_task(void)
{
	unsigned char horl = 1;
	unsigned char mode = 1;

	horl = get_horl();
	mode = get_DPSP_mode();
	
	check_signal(horl);
	signal_handle(mode, horl);
}

