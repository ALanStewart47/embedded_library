#include "led.h"


unsigned int SP[8]={0,0,0,0,0,0,0,0};  //频闪延时数组
unsigned int DP[8]={0,0,0,0,0,0,0,0};  //数字模式PWM等级
LED_Parameter LED_para;

void init_LEDparameter(void)
{
	LED_para.CH_num_now = 1;  //后续用作flash存储的初始化
	LED_para.DP_ro_SP = 1;
	LED_para.horl = 1;

	if(LED_para.DP_ro_SP == DP_MODE)
		HAL_GPIO_WritePin(DPorSP_GPIO_Port, DPorSP_Pin, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(DPorSP_GPIO_Port, DPorSP_Pin, GPIO_PIN_RESET);

	if(LED_para.horl == 1)
		HAL_GPIO_WritePin(HorL_GPIO_Port, HorL_Pin, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(HorL_GPIO_Port, HorL_Pin, GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
}

void set_DPmode(void)
{
	LED_para.DP_ro_SP = DP_MODE;
	HAL_GPIO_WritePin(DPorSP_GPIO_Port, DPorSP_Pin, GPIO_PIN_SET);
}

void set_SPmode(void)
{
	LED_para.DP_ro_SP = SP_MODE;
	HAL_GPIO_WritePin(DPorSP_GPIO_Port, DPorSP_Pin, GPIO_PIN_RESET);
}

unsigned char get_DPSP_mode(void)
{
	return LED_para.DP_ro_SP;
}

unsigned char get_horl(void)
{
	return LED_para.horl;
}

void set_horl(unsigned int value)
{
	LED_para.horl = value;
}

unsigned int get_DP_PWM(unsigned char seq)
{
	return DP[seq];
}

unsigned int get_SP_time(unsigned char seq)
{
	return SP[seq];
}

void set_DP_PWM(unsigned char seq, unsigned int value)
{
	DP[seq] = value;
}

void set_SP_PWM(unsigned char seq, unsigned int value)
{
	SP[seq] = value;
}

void ADD_CH_num_now(void)
{
	if(LED_para.CH_num_now == CHANNEL_8)
	{
		if(LED_para.DP_ro_SP == DP_MODE){
			LED_para.CH_num_now = HORL_SELECT;
		}else{
			LED_para.CH_num_now = CHANNEL_1;
		}
	}else
	{
		LED_para.CH_num_now++;
	}
}

void ADD_SPDP_ShortPress(void)
{
	if(LED_para.CH_num_now == HORL_SELECT)
	{
		if(LED_para.horl == 0){
			LED_para.horl = 1;
			HAL_GPIO_WritePin(HorL_GPIO_Port, HorL_Pin, GPIO_PIN_SET);
		}else{
			LED_para.horl = 0;
			HAL_GPIO_WritePin(HorL_GPIO_Port, HorL_Pin, GPIO_PIN_RESET);
		}
	}else
	{
		if(LED_para.DP_ro_SP == DP_MODE){
			DP[LED_para.CH_num_now-1] = DP[LED_para.CH_num_now-1] + 1;
			if(DP[LED_para.CH_num_now-1] > 255)
			{
				DP[LED_para.CH_num_now-1] = 255;
			}
		}else if(LED_para.DP_ro_SP == SP_MODE){
			SP[LED_para.CH_num_now-1] = SP[LED_para.CH_num_now-1] + 1;
			if(SP[LED_para.CH_num_now-1] > 999)
			{
				SP[LED_para.CH_num_now-1] = 999;
			}
		}
	}
}

void DELETE_SPDP_ShortPress(void)
{
	if(LED_para.CH_num_now == HORL_SELECT)
	{
		if(LED_para.horl == 0){
			LED_para.horl = 1;
			HAL_GPIO_WritePin(HorL_GPIO_Port, HorL_Pin, GPIO_PIN_SET);
		}else{
			LED_para.horl = 0;
			HAL_GPIO_WritePin(HorL_GPIO_Port, HorL_Pin, GPIO_PIN_RESET);
		}
	}else
	{
		if(LED_para.DP_ro_SP == DP_MODE){
			if(DP[LED_para.CH_num_now-1] > 0)
			{
				DP[LED_para.CH_num_now-1]--;
			}
		}else if(LED_para.DP_ro_SP == SP_MODE){
			if(SP[LED_para.CH_num_now-1] > 0)
			{
				SP[LED_para.CH_num_now-1]--;
			}
		}
	}
}

void led_show(void)
{
	if(LED_para.DP_ro_SP == DP_MODE)
	{
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
	}else if(LED_para.DP_ro_SP == SP_MODE){
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
	}
}

void LED595_show(void)
{
	static unsigned char led_num = 1;
	unsigned char channel = LED_para.CH_num_now;
	unsigned char hunderd_temp = 0;
	unsigned char ten_temp = 0;
	unsigned char ind_temp = 0;

	if((LED_para.DP_ro_SP == SP_MODE)&&(LED_para.CH_num_now == HORL_SELECT))
	{
		LED_para.CH_num_now = CHANNEL_1;   //防止闪频模式出现H通道
		channel = LED_para.CH_num_now;
	}

	if(channel == HORL_SELECT)
	{
		if((led_num == CH_SELECT)||(led_num == HUNDRED_SELECT)){
			LED_display(CH_SELECT, 10); //11是字母H
		}else{
			LED_display(IND_SELECT, LED_para.horl);
		}
	}else
	{
		if(LED_para.DP_ro_SP == DP_MODE){
			hunderd_temp = DP[LED_para.CH_num_now-1]/100;
			ten_temp = DP[LED_para.CH_num_now-1]%100/10;
			ind_temp = DP[LED_para.CH_num_now-1]%10;
		}else if(LED_para.DP_ro_SP == SP_MODE){
			hunderd_temp = SP[LED_para.CH_num_now-1]/100;
			ten_temp = SP[LED_para.CH_num_now-1]%100/10;
			ind_temp = SP[LED_para.CH_num_now-1]%10;
		}

		switch (led_num)
		{
			case CH_SELECT:
				LED_display(led_num, LED_para.CH_num_now);
				break;
			case HUNDRED_SELECT:
				LED_display(led_num, hunderd_temp);
				break;
			case TEN_SELECT:
				LED_display(led_num, ten_temp);
				break;
			case IND_SELECT:
				LED_display(led_num, ind_temp);
				break;
			default:
				break;
		}
	}
	
	if(led_num == IND_SELECT){
		led_num = CH_SELECT;
	}else{
		led_num++;
	}
}

void led_task(void)
{
	LED595_show();
	led_show();
}


