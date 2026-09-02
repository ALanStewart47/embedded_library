#ifndef __PROTOCOL_PUBLIC_H
#define __PROTOCOL_PUBLIC_H

#ifdef __cplusplus
extern "C" {
#endif

#define		CHANNEL_NUM		12              //????·??????????????
#include "main.h"

#define		SX0XXX_ENABLE			0
#define		SPX0XXX_ENABLE			0
#define		SPUXX_ENABLE			0
#define		ST0XXX_ENABLE			0
#define		SXXXXX_ENABLE			0
#define		SWXXXX_ENABLE			0
#define		SAVE_ENABLE				0
#define		SWTRIG_ENABLE			0

#define		TRX_ENABLE				0
#define		TX_ENABLE				0
#define		TPX_ENABLE				0

#define		DLX0XXX_ENABLE			0
#define		DCX0XXX_ENABLE			0
#define		DC0XXX_ENABLE			0

#define		CTX_ENABLE				0

#define		PRCLX_ENABLE				0
#define		PRWX_ENABLE				0
#define		PRNXXX_ENABLE			0
#define		PRENCLRX_ENABLE			0
#define		PRNCLRX_ENABLE			0
#define		SFIL_ENABLE			0//可编程指令——模式
#define		P_SPL_ENABLE			0//可编程指令——光源
#define		P_STP_14_ENABLE			0//可编程指令——触发源
#define		P_SW_4_ENABLE			0//可编程指令——模式
#define		P_SOFTTRI_ENABLE			0//可编程指令——模式
#define		SWX_ENABLE				0
#define     VER_ENABLE				0//VER#---版本号   1.0.0

#define     READALL_ENABLE			0 //读取所有参数----除可编程数据，模式数据

#define    MAX_CHANNEL_LETTER		'A'-1+CHANNEL_NUM//最大通道数
   


#define    SUM_SIZE			2200
#define    FREE				0
#define    BUSY				1


typedef struct UART_Prepare_Buf{
	unsigned int state;
	unsigned int available_length;
	unsigned char rx_buffer[SUM_SIZE];
	unsigned int data_start_position;
	unsigned int data_end_position;
}UART_Prepare_Buf;


void init_protocol_para(void);
void input_data(unsigned char *sdata,unsigned int length);
unsigned char* get_prepare_tx_buffer(unsigned int *length);
void analysis_command(void);
void clear_tx_buff( unsigned int num);
void set_state_clean(unsigned char channel,unsigned int value);
extern unsigned char cmd_up[15];
extern unsigned char prepare_tx_buffer[SUM_SIZE] ;
extern unsigned int prepare_tx_length ;
extern UART_Prepare_Buf prepare_data; 
void set_state_clean_(unsigned char channel,unsigned int value);


void cmd_40_263_up(void);
void cmd_40_7_up(void);
void cler_cnt_c(void);
void up_D2_data(unsigned char data);
void up_D1_data(unsigned int data);

void analysis_command_BOOT(void);
typedef struct
{
	/********可编程光源参数*********/
	uint32_t  addr;
	uint8_t   spi_data[2048];
	uint8_t   up_cmd;
	uint32_t  size_t;
	uint32_t  block_size_t;

}spi_w;

extern spi_w spi_w_handle;
#ifdef __cplusplus
}
#endif

#endif 
