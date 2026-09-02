
#include "Data_for_fpga.h"
#include "sys.h"
#define FPGA_CLK_L PCout(13)=0; //data_clk G16-C13
#define FPGA_CLK_H PCout(13)=1;

#define FPGA_READ_OK PDin(0)  //J15-D0

#define FPGA_RST_L PCout(14)=0;//H15-C14
#define FPGA_RST_H PCout(14)=1;

#define FPGA_Data_L PCout(15)=0;//H16-C15  TX_DATA
#define FPGA_Data_H PCout(15)=1;


#define FPGA_CLK2_L PDout(1)=0;//J16
#define FPGA_CLK2_H PDout(1)=1;

#define FPGA_DATA_LINE PAin(5)//L16

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

u32 FPGA_Data_receive(u8 add)
{

	uint8_t i;
	u8 deay_time=2;
	uint32_t Data_H=0,Data_L=0;


	FPGA_CLK2_L;delay_us(deay_time);
	FPGA_RST_L;delay_us(deay_time);
	FPGA_RST_H;delay_us(deay_time);

	for(i=0;i<=15;i++)
	{

		FPGA_CLK2_H;delay_us(deay_time);
		Data_L=Data_L|((u16)FPGA_DATA_LINE<<i);
		FPGA_CLK2_L;delay_us(deay_time); 
	}
	
	for(i=0;i<=15;i++)
	{
		
		FPGA_CLK2_H;delay_us(deay_time);
		Data_H=Data_H|((u16)FPGA_DATA_LINE<<i);
		FPGA_CLK2_L;delay_us(deay_time);
		
	}
	return ((Data_H)<<16)+Data_L;

}
int FPGA_Data_send(uint16_t add,uint32_t data_1)
{
	uint32_t data32 = 0;
	uint8_t i;
	u8 deay_time=3;
	data32 =data_1;

	FPGA_CLK_L;delay_us(deay_time);
	FPGA_RST_L;delay_us(deay_time);
	FPGA_RST_H;delay_us(deay_time);
	//SBUF=data32>>16;
	for(i=0;i<=31;i++)
	{
		if((data32>>i)&0x00000001)
		{
			FPGA_Data_H;
		}
		else
		{
			FPGA_Data_L;

		}
		delay_us(deay_time);
		FPGA_CLK_H;
		delay_us(deay_time);
		FPGA_CLK_L;
		delay_us(deay_time);
		
		
		

	}
	data32  =add;
	for(i=0;i<=15;i++)
	{
		if((data32>>i)&0x00000001)
		{
			FPGA_Data_H;
		}
		else
		{
			FPGA_Data_L;
		}
		delay_us(deay_time);
		FPGA_CLK_H;
		delay_us(deay_time);
		FPGA_CLK_L;
		delay_us(deay_time);
	}
	delay_us(deay_time);

	if(FPGA_READ_OK)
	{
		FPGA_RST_L; delay_us(deay_time);
		FPGA_RST_H;	delay_us(deay_time);
		return 0;
	}
	else
	{
		FPGA_RST_L;delay_us(deay_time);
		FPGA_RST_H;delay_us(deay_time);
		return -1;
	}
}
