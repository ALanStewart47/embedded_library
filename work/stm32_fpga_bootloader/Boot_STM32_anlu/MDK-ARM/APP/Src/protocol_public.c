#include "protocol_public.h"
//#include "STC8.h"
#include "Data_for_fpga.h"
//#include "uart.h"
//extern controller_data Controller_data;//数据结构体

extern uint8_t VERSION[];//版本号
extern uint8_t Pmode_data;
uint8_t cmd_up[15]={0};  
UART_Prepare_Buf prepare_data; 
unsigned char prepare_tx_buffer[SUM_SIZE] = {0};
unsigned int prepare_tx_length = 0;
uint8_t fpga_it2=1;
extern uint8_t stp_num_temp;
extern uint8_t spl_num_temp;
unsigned char calculateBCC(const unsigned char *data, unsigned short int length) {
    unsigned char bcc = 0; // 初始化BCC值为0
    for (unsigned short int i = 0; i < length; i++) {
        bcc ^= data[i]; // 对每个字节进行异或运算
    }
    return bcc; // 返回计算得到的BCC值
}
uint16_t modbus_crc(uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF,i,j;
		
    for ( i = 0; i < len; i++) {
        crc ^= data[i];
        for ( j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

void init_protocol_para(void)
{
	unsigned int i = 0;
	
	prepare_data.available_length = SUM_SIZE;
	prepare_data.data_end_position = 0;
	prepare_data.data_start_position = 0;
	prepare_data.state = 0;

	for(i = 0; i < SUM_SIZE; i++)
	{
		prepare_data.rx_buffer[i] = 0;
	}

	prepare_tx_length = 0;

	for(i = 0; i < SUM_SIZE; i++)
	{
		prepare_tx_buffer[i] = 0;
	}
}

void input_data(unsigned char *sdata,unsigned int length)
{
	unsigned int j = 0;
	unsigned int now_position = 0;
	
	if(prepare_data.state == FREE)
	{
		if(prepare_data.available_length < length)
		{
			return ;  //导入的数据比剩下的空间大
		}
		now_position = prepare_data.data_end_position;
		for(j = 0; j < length; j++)
		{
			prepare_data.rx_buffer[j+now_position] = sdata[j];
		}
		prepare_data.data_end_position = now_position + length;
		prepare_data.available_length = prepare_data.available_length - length;
	}
}

void package_tx_buffer(unsigned char *buf,unsigned int len)
{
	unsigned int i = 0;

	for(i=0; i < len; i++)
	{
		prepare_tx_buffer[prepare_tx_length] = buf[i];
		prepare_tx_length++;
	}

	
}

uint16_t err=0;
uint16_t bcc_temp=0;
uint16_t lenght_temp;
uint16_t crc_temp=0;
spi_w spi_w_handle;
void cmd_40_7_up(void)
{
	unsigned char tx_buf[10];
	uint8_t h=0;
			tx_buf[h++]=0xfa;
			tx_buf[h++]=0x00;
			tx_buf[h++]=0x00;
			tx_buf[h++]=0x00;
			tx_buf[h++]=0x00;
			crc_temp=modbus_crc(tx_buf,h);
	tx_buf[h++]=crc_temp>>8;
	tx_buf[h++]=crc_temp;
	package_tx_buffer(tx_buf, h);
}
void cmd_40_263_up(void)
{
	unsigned char tx_buf[10];
	uint8_t h=0;
			tx_buf[h++]=0xfa;
			tx_buf[h++]=spi_w_handle.addr>>24;
			tx_buf[h++]=spi_w_handle.addr>>16;
			tx_buf[h++]=spi_w_handle.addr>>8;
			tx_buf[h++]=spi_w_handle.addr;
			
			crc_temp=modbus_crc(tx_buf,h);
			tx_buf[h++]=crc_temp>>8;
			tx_buf[h++]=crc_temp;
//			package_tx_buffer(tx_buf, h);
	package_tx_buffer(tx_buf, h);
}
unsigned int CMD_40_handle(unsigned char *uart_buf,unsigned int lenght)
{

	unsigned char tx_buf[10];
	uint8_t h=0;
	lenght_temp=lenght;
	if(lenght>=3)
	{
		crc_temp=modbus_crc(uart_buf,lenght-2);
	
		if((uart_buf[0]==0xFB&&uart_buf[1]==0xFf&&uart_buf[0]==0xFB)||spi_w_handle.up_cmd!=0)
		{
			if(spi_w_handle.up_cmd==1||spi_w_handle.up_cmd==2)
			{
				tx_buf[h++]=0xfb;
				tx_buf[h++]=0x01;
				tx_buf[h++]=0xfb;
				package_tx_buffer(tx_buf, h);
			}
			else if(spi_w_handle.up_cmd==0)
			{
				tx_buf[h++]=0xfb;
				tx_buf[h++]=0x00;
				tx_buf[h++]=0xfb;	
				package_tx_buffer(tx_buf, h);			
				
			}
			else if(spi_w_handle.up_cmd==0xff)
			{
				tx_buf[h++]=0xfb;
				tx_buf[h++]=0x02;
				tx_buf[h++]=0xfb;	
				spi_w_handle.up_cmd=0;				
				package_tx_buffer(tx_buf, h);
			}
		}
		else if(crc_temp==(uart_buf[lenght-2]*256+uart_buf[lenght-1])&&uart_buf[0]==0xFA)
		{
			if(lenght==7)//清空对应的flash空间
			{
				if(uart_buf[1]==0x68&&uart_buf[2]==0xAA)
				{
						uint16_t block_size_temp=(uint32_t)(uart_buf[3]<<8)+(uint32_t)uart_buf[4];
						if(block_size_temp>=256&&block_size_temp<=2048&&block_size_temp%256==0)
						{
							spi_w_handle.block_size_t=block_size_temp;
							tx_buf[h++]=0x73;
							package_tx_buffer(tx_buf, h);
						}
						else
						{
							tx_buf[h++]=0x65;
							package_tx_buffer(tx_buf, h);
						}
				}
				else if(uart_buf[1]==0x00)
				{
					uint32_t size_temp=(uint32_t)(uart_buf[2]<<16)+(uint32_t)(uart_buf[3]<<8)+(uint32_t)uart_buf[4];
					spi_w_handle.size_t=size_temp;
					tx_buf[h++]=0x73;
					package_tx_buffer(tx_buf, h);
					spi_w_handle.up_cmd=1;
				}
			}	
			else if(lenght==7+spi_w_handle.block_size_t)//开始依次写入数据
			{
				spi_w_handle.up_cmd=2;
				spi_w_handle.addr=(uint32_t)(uart_buf[1]<<24)+(uint32_t)(uart_buf[2]<<16)+(uint32_t)(uart_buf[3]<<8)+(uint32_t)uart_buf[4];
				for(int i=0;i<spi_w_handle.block_size_t;i++)
				spi_w_handle.spi_data[i]=uart_buf[5+i];
				tx_buf[h++]=0x73;
				package_tx_buffer(tx_buf, h);
				
//				for(int j=0;j<spi_w_handle.block_size_t;j++)
//				{
//					
//				}
				//将数据拷贝到指定地址
				
//				tx_buf[h++]=uart_buf[0];
//				tx_buf[h++]=uart_buf[1];
//				tx_buf[h++]=uart_buf[2];
//				tx_buf[h++]=uart_buf[3];
//				tx_buf[h++]=uart_buf[4];
//				crc_temp=modbus_crc(tx_buf,h);
//				tx_buf[h++]=crc_temp>>8;
//				tx_buf[h++]=crc_temp;
//				package_tx_buffer(tx_buf, h);
			}
		}
		else//返回失败 
		{
				tx_buf[h++]=0xfa;
				tx_buf[h++]=0xff;
				tx_buf[h++]=0xff;
				tx_buf[h++]=0xff;
				tx_buf[h++]=0xff;
				crc_temp=modbus_crc(tx_buf,h);
				tx_buf[h++]=crc_temp>>8;
				tx_buf[h++]=crc_temp;
				package_tx_buffer(tx_buf, h);
		}
	}
	return 0;
}
unsigned char* get_prepare_tx_buffer(unsigned int *length)
{
	*length = prepare_tx_length;

	return prepare_tx_buffer;
}

void analysis_command(void)
{
	unsigned int i = 0;
	unsigned int length = SUM_SIZE - prepare_data.available_length;
	//unsigned char tx_buffer[5] = {0};
	if(length == 0){
		return;
	}
	prepare_tx_length = 0;
	prepare_data.state = BUSY;

	CMD_40_handle(prepare_data.rx_buffer,length);
	for(i = 0; i < length; i++)
	{
		prepare_data.rx_buffer[i] = 0;
	}
	prepare_data.state = FREE;
	prepare_data.data_end_position = 0;
	prepare_data.available_length  = SUM_SIZE;
}

void analysis_command_BOOT(void)
{
	unsigned int i = 0;
	unsigned int length = SUM_SIZE - prepare_data.available_length;
	//unsigned char tx_buffer[5] = {0};
	if(length == 0){
		return;
	}
	prepare_tx_length = 0;
	prepare_data.state = BUSY;

	//CMD_40_handle(prepare_data.rx_buffer,length);
	for(i = 0; i < length; i++)
	{
		prepare_data.rx_buffer[i] = 0;
	}
	prepare_data.state = FREE;
	prepare_data.data_end_position = 0;
	prepare_data.available_length  = SUM_SIZE;
}