/******************************************************************************
 * @file jtag_gaoyun.h
 * @brief STM32(HAL) 位带 JTAG 主机, 对 Gowin GW1N-9C 做内部 Flash ISP 编程。
 *        由 GD32 参考(参考/boot参考/jtag_gaoyun.c)移植, JTAG 时序/ISP 指令保持一致。
 *
 * ⚠ JTAG 引脚默认沿用参考(TMS=PA5/TCK=PA6/TDI=PA7/TDO=PB0), 上板前务必按本板原理图核实,
 *   引脚集中在 jtag_gaoyun.c 顶部宏, 改线只改一处。
 *****************************************************************************/
#ifndef __JTAG_GAOYUN_H
#define __JTAG_GAOYUN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void     my_gpio_init(void);                  
uint32_t readFPGA_ID(void);
void     writeSRAM(uint8_t *bitstream, uint32_t length);   
void     eraseflash(void);
void     program_internal_flash(uint16_t cnt_num);         
uint32_t read_Status(void);
uint32_t read_device_ID(void);
void     eraseSPIFlash(void);

extern uint16_t cnt, cnt_max, cnt_max_decimal;

#ifdef __cplusplus
}
#endif

#endif  
