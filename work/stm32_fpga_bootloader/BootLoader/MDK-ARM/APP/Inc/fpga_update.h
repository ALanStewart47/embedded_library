/******************************************************************************
 * @file fpga_update.h
 * @brief FPGA(GW1N-9C) 在线升级: 上位机 FA/FB 协议 + 块缓冲 + 升级主循环。
 *        协议与 GD32 参考(参考/boot参考/protocol_public.c CMD_40_handle)一致。
 *****************************************************************************/
#ifndef __FPGA_UPDATE_H
#define __FPGA_UPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

 
typedef struct
{
    uint32_t addr;             
    uint8_t  spi_data[2048];   
    uint8_t  up_cmd;           
    uint32_t size_t;           
    uint32_t block_size_t;     
} spi_w;

extern spi_w spi_w_handle;

uint16_t modbus_crc(uint8_t *data, uint16_t len);
void     CMD_40_handle(unsigned char *uart_buf, unsigned int lenght);
void     fpga_update_task(void);    

#ifdef __cplusplus
}
#endif

#endif  
