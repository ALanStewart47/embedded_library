#ifndef __HC595D_H
#define __HC595D_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"


typedef enum
{
  CH_SELECT = 1,
  HUNDRED_SELECT,
  TEN_SELECT,
  IND_SELECT
} SELECT_LED;

void write595_byte(unsigned char dat);
void LED_select(unsigned char channel);
void LED_display(unsigned char channel,unsigned char num);


	
#ifdef __cplusplus
}
#endif

#endif  
