#ifndef __TIME_H
#define __TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "led.h"



typedef struct __SP_Parameter
{
	unsigned char sp_ms;
	unsigned char time;
	unsigned char time_out_flag;
	unsigned char enable;
} SP_Parameter;

void time6_handler(void);
extern void key_delay_start(void);
extern void set_key_short_success(unsigned char temp);
extern unsigned char read_key_short_success(void);
unsigned char SP_time_out_flag(unsigned char pwm_num);
void SP_delay_start(unsigned char pwm_num,unsigned int time);




	
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
