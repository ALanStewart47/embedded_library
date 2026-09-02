#ifndef __DATA_FOR_FPGA_H
#define __DATA_FOR_FPGA_H
#include "app.h"


int FPGA_Data_send(uint16_t add,uint32_t data_1);
void FPGA_Data_Update(void);

//void Delay1000ms(void);
void delay_us(uint32_t us);

extern uint8_t sofe_run_it;
#endif
