#ifndef __PROTOCOL_PUBLIC_H
#define __PROTOCOL_PUBLIC_H

#ifdef __cplusplus
extern "C" {
#endif

#define		CHANNEL_NUM		8               


#define		SX0XXX_ENABLE			1
#define		SPX0XXX_ENABLE			1
#define		SPUXX_ENABLE			1
#define		ST0XXX_ENABLE			1
#define		SXXXXX_ENABLE			1
#define		SWXXXX_ENABLE			1
#define		SAVE_ENABLE				1
#define		SWTRIG_ENABLE			1

#define		TRX_ENABLE				1
#define		TX_ENABLE				1
#define		TPX_ENABLE				1

#define		DLX0XXX_ENABLE			1
#define		DCX0XXX_ENABLE			1
#define		DC0XXX_ENABLE			1

#define		CTX_ENABLE				1

#define		PRCL_ENABLE				1
#define		PRWX_ENABLE				1
#define		PRNXXX_ENABLE			1
#define		PRENCLRX_ENABLE			1



#define    MAX_CHANNEL_LETTER		'A'-1+CHANNEL_NUM



#define    SUM_SIZE			200
#define    FREE				0
#define    BUSY				1


typedef struct UART_Prepare_Buf{
	unsigned char state;
	unsigned char available_length;
	unsigned char rx_buffer[SUM_SIZE];
	unsigned char data_start_position;
	unsigned char data_end_position;
}UART_Prepare_Buf;


void init_protocol_para(void);
void input_data(unsigned char *data,unsigned char length);
unsigned char* get_prepare_tx_buffer(unsigned char *length);
void analysis_command(void);

	
#ifdef __cplusplus
}
#endif

#endif 
