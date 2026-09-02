#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "hc595d.h"

#define    DP_MODE		1
#define    SP_MODE		0
#define LIGHT_UP		1
#define LIGHT_OUT		0


typedef struct __LED_Parameter
{
	unsigned char horl;
	unsigned char DP_ro_SP;
	unsigned char CH_num_now;
} LED_Parameter;

typedef enum
{
  HORL_SELECT = 0,
  CHANNEL_1,
  CHANNEL_2,
  CHANNEL_3,
  CHANNEL_4,
  CHANNEL_5,
  CHANNEL_6,
  CHANNEL_7,
  CHANNEL_8
} SELECT_CHANNLE;

void led_show(void);
void init_LEDparameter(void);
void set_DPmode(void);
void set_SPmode(void);
void LED595_show(void);
void ADD_CH_num_now(void);
void ADD_SPDP_ShortPress(void);
void DELETE_SPDP_ShortPress(void);
unsigned int get_DP_PWM(unsigned char seq);
unsigned int get_SP_time(unsigned char seq);
unsigned char get_DPSP_mode(void);
unsigned char get_horl(void);
void set_DP_PWM(unsigned char seq, unsigned int value);
void set_SP_PWM(unsigned char seq, unsigned int value);
void set_horl(unsigned int value);
void led_task(void);



	
#ifdef __cplusplus
}
#endif

#endif  
