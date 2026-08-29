#include "time.h"
#include "app.h"



unsigned char key_delay_ms = 0;
unsigned char key_delay_flag = 0;
unsigned char key_short_delay_success = 0;


SP_Parameter SP_Para[8];
unsigned char pwm_number = 0;


unsigned char read_key_short_success(void)
{
	return key_short_delay_success;
}
void set_key_short_success(unsigned char temp)
{
	key_short_delay_success = temp;
}


void key_delay_start(void)
{
	key_delay_flag = 1;
}

void key_delay(void)
{
	if(key_delay_flag == 1)
	{
		key_delay_ms++;
		if(key_delay_ms >= 10)
		{
			key_delay_flag = 0;
			key_delay_ms = 0;
			key_short_delay_success = 1;
		}
	}
}

void SP_delay_start(unsigned char pwm_num,unsigned int time)
{
	SP_Para[pwm_num].enable = 1;
	SP_Para[pwm_num].time_out_flag = 0;
	SP_Para[pwm_num].sp_ms = 0;
	SP_Para[pwm_num].time = time;
}

unsigned char SP_time_out_flag(unsigned char pwm_num)
{
	return SP_Para[pwm_num].time_out_flag;
}

void SP_time(void)
{
	for(pwm_number=0;pwm_number<8;pwm_number++)
	{
		if(SP_Para[pwm_number].enable == 1)
		{
			SP_Para[pwm_number].sp_ms++;
			if(SP_Para[pwm_number].sp_ms >= SP_Para[pwm_number].time)
			{
				SP_Para[pwm_number].time_out_flag = 1;
				SP_Para[pwm_number].enable = 0;
			}
		}
	}
}

void time6_handler(void)
{
	key_delay();
	SP_time();
}


