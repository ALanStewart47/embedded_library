#include "hc595d.h"

#define    LED595_DATA_SET     HAL_GPIO_WritePin(DA_GPIO_Port,DA_Pin,GPIO_PIN_SET)
#define    LED595_DATA_RESET   HAL_GPIO_WritePin(DA_GPIO_Port,DA_Pin,GPIO_PIN_RESET)
#define    LED595_CLK_SET      HAL_GPIO_WritePin(SH_GPIO_Port,SH_Pin,GPIO_PIN_SET)
#define    LED595_CLK_RESET    HAL_GPIO_WritePin(SH_GPIO_Port,SH_Pin,GPIO_PIN_RESET)
#define    LED595_LATCH_SET    HAL_GPIO_WritePin(ST_GPIO_Port,ST_Pin,GPIO_PIN_SET)
#define    LED595_LATCH_RESET  HAL_GPIO_WritePin(ST_GPIO_Port,ST_Pin,GPIO_PIN_RESET)

unsigned char DAT[12]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x76,0x00};

void write595_byte(unsigned char dat)
{
	unsigned char i; //发送数据时做循环使用临时变量
	
    for(i=0; i<8; i++) //将8位数据按位发送,先发送高字节后发送低字节
    {
        //LED595_CLK_RESET;//时钟线低电平
        if( (dat & 0x80) == 0x80)//判断数据高低位
        {
            LED595_DATA_SET;                   //数据线高电平
        }
        else
        {
             LED595_DATA_RESET;          //数据线低电平
        }
        dat = dat << 1;         //数据左移1位
        //LED595_CLK_SET;                 //时钟线高电平
    }
    //上升沿输出数据
    LED595_LATCH_RESET;  
    LED595_LATCH_SET;

} 


void LED_select(unsigned char channel)
{
	if(channel == CH_SELECT)
	{
		HAL_GPIO_WritePin(sel4_GPIO_Port, sel4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(sel3_GPIO_Port, sel3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(sel2_GPIO_Port, sel2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(sel1_GPIO_Port, sel1_Pin, GPIO_PIN_SET);
	}else if(channel == HUNDRED_SELECT)
	{
		HAL_GPIO_WritePin(sel4_GPIO_Port, sel4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(sel3_GPIO_Port, sel3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(sel2_GPIO_Port, sel2_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(sel1_GPIO_Port, sel1_Pin, GPIO_PIN_RESET);
	}else if(channel == TEN_SELECT)
	{
		HAL_GPIO_WritePin(sel4_GPIO_Port, sel4_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(sel3_GPIO_Port, sel3_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(sel2_GPIO_Port, sel2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(sel1_GPIO_Port, sel1_Pin, GPIO_PIN_RESET);
	}else if(channel == IND_SELECT)
	{
		HAL_GPIO_WritePin(sel4_GPIO_Port, sel4_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(sel3_GPIO_Port, sel3_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(sel2_GPIO_Port, sel2_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(sel1_GPIO_Port, sel1_Pin, GPIO_PIN_RESET);
	}
}

void LED_display(unsigned char channel,unsigned char num)
{
	LED_select(channel);
	write595_byte(DAT[num]);
}

