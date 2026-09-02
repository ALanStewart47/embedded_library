#include "uart.h"
//#include "protocol_public.h"



#define APPLICATION_ADDRESS_A  0x08003800

#define BOOT_ADDRESS	0x08003400


#define TURE	  1
#define FALSE	  2


#include "protocol_public.h"
#include "Data_for_fpga.h"
#include "W25Q128.h"
#include "main.h"

uint16_t RX_data_length=0;
uint8_t Uart_RxBuf[Uart_Rx_Length]={0};

uint8_t 	Uart_Up_IT=0;//串口更新标志位

typedef  void (*pFunction)(void);
pFunction JumpToApplication;
uint32_t JumpAddress;

//unsigned char prepare_tx_buffer[100] = {0};   //uart3 == 232  uart4 ==缃戝彛閫忎紶
unsigned char urat_tx_length = 0;

unsigned int rx_length;
unsigned char rx_endFlag;

unsigned char need_to_upgrade = 1;
unsigned char version[2] = {1,0};
unsigned int package_sum = 0;
unsigned char upgrade_bin_flag = 0;
unsigned char success_flag = 0;


extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_uart4_rx;


extern DMA_HandleTypeDef hdma_usart1_tx;
//extern TIM_HandleTypeDef htim6;


unsigned int dma_ms = 0;
unsigned int dma_ms_flag = 0;
unsigned char  dma_number = 1;

void init_Uart_data(void)
{
//	my_memset(prepare_tx_buffer,0,100);


}

void init_and_set_uart(unsigned char uart_number,UART_HandleTypeDef * huart,DMA_HandleTypeDef * hdma_usart_rx)
{
	#if (URAT_NUMBER == 1)
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE); 
	#endif
	#if (URAT_NUMBER == 2)
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
	#endif
	#if (URAT_NUMBER == 3)
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
	#endif
	#if (URAT_NUMBER == 4)
	__HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);
	#endif
	
	UART_Receive_DMA();
}

void UART_Receive_DMA(void)
{
	#if (URAT_NUMBER == 1)
	HAL_UART_Receive_DMA(&huart1,Uart_RxBuf,Uart_Rx_Length);
	#endif
	#if (URAT_NUMBER == 2)
	HAL_UART_Receive_DMA(&huart2,Uart_RxBuf,Uart_Rx_Length);
	#endif
	#if (URAT_NUMBER == 3)
	HAL_UART_Receive_DMA(&huart3,Uart_RxBuf,Uart_Rx_Length);
	#endif
	#if (URAT_NUMBER == 4)
	HAL_UART_Receive_DMA(&huart4,Uart_RxBuf,Uart_Rx_Length);
	#endif
}


void DMA_Usart_Send(uint8_t *buf,uint8_t len)
{
	#if (URAT_NUMBER == 1)
	if(HAL_UART_Transmit_DMA(&huart1, buf,len)!= HAL_OK) 
	#endif
	#if (URAT_NUMBER == 2)
	if(HAL_UART_Transmit_DMA(&huart2, buf,len)!= HAL_OK) 
	#endif
	#if (URAT_NUMBER == 3)
	if(HAL_UART_Transmit_DMA(&huart3, buf,len)!= HAL_OK) 
	#endif
	#if (URAT_NUMBER == 4)
	if(HAL_UART_Transmit_DMA(&huart4, buf,len)!= HAL_OK) 
	#endif
	{
	//Error_Handler();
	}

}
//void delay_ms(unsigned int time)
//{    
//   unsigned int i=0;  
//   while(time--)
//   {
//      i=12000;  //鑷繁瀹氫箟
//      while(i--) ;    
//   }
//}

//void dma_delay_jisuan(void)
//{
//	if(dma_ms_flag == 1)
//	{
//		delay_ms(3);
//		//dma_ms++;
//		//if(dma_ms == 2000)
//		{
//			uart_receive(dma_number);
//			dma_ms_flag = 0;
//			dma_ms = 0;
//		}
//	}
//}
//void uart_receive_number(unsigned char uart_number)
//{
//	if(*(uint16_t *)(0x8003400)==0xaa)
//	{
//		dma_ms_flag = 1;
//	//dma_ms = 0;
//		dma_number = uart_number;
//		dma_delay_jisuan();
//	}
//}

#if (URAT_NUMBER == 1)
void uart_receive(unsigned char uart_number)
{
	unsigned char recv_flag = 0;
	unsigned short int numb = 0;
	
	recv_flag =__HAL_UART_GET_FLAG(&huart1,UART_FLAG_IDLE);

	if((recv_flag != 0))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);
		HAL_UART_DMAStop(&huart1);

		numb = __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);	// 鑾峰彇DMA涓湭浼犺緭鐨勬暟鎹釜鏁?

		rx_length = (uint16_t)BUFFER_SIZE - numb;

		rx_endFlag =1;
	}

	HAL_UART_IRQHandler(&huart1);
}
#endif
#if (URAT_NUMBER == 2)
void uart_receive(unsigned char uart_number)
{
	unsigned char recv_flag = 0;
	unsigned char numb = 0;
	
	recv_flag =__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE);

	if((recv_flag != 0))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart2);
		HAL_UART_DMAStop(&huart2);

		numb = __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);	// 鑾峰彇DMA涓湭浼犺緭鐨勬暟鎹釜鏁?

		rx_length = BUFFER_SIZE - numb;

		rx_endFlag =1;
	}

	HAL_UART_IRQHandler(&huart2);
}
#endif
#if (URAT_NUMBER == 3)
void uart_receive(unsigned char uart_number)
{
	unsigned char recv_flag = 0;
	unsigned char numb = 0;
	
	recv_flag =__HAL_UART_GET_FLAG(&huart3,UART_FLAG_IDLE);

	if((recv_flag != 0))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart3);
		HAL_UART_DMAStop(&huart3);

		numb = __HAL_DMA_GET_COUNTER(&hdma_usart3_rx);	// 鑾峰彇DMA涓湭浼犺緭鐨勬暟鎹釜鏁?

		rx_length = BUFFER_SIZE - numb;

		rx_endFlag =1;
	}

	HAL_UART_IRQHandler(&huart3);
}
#endif
#if (URAT_NUMBER == 4)
void uart_receive(unsigned char uart_number)
{
	unsigned char recv_flag = 0;
	unsigned char numb = 0;
	
	recv_flag =__HAL_UART_GET_FLAG(&huart4,UART_FLAG_IDLE);

	if((recv_flag != 0))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart4);
		HAL_UART_DMAStop(&huart4);

		numb = __HAL_DMA_GET_COUNTER(&hdma_uart4_rx);	// 鑾峰彇DMA涓湭浼犺緭鐨勬暟鎹釜鏁?

		rx_length = BUFFER_SIZE - numb;

		rx_endFlag =1;
	}

	HAL_UART_IRQHandler(&huart4);
}
#endif





void my_memset(unsigned char *dest, unsigned char set, unsigned char len)
{
	unsigned char *pdest = (unsigned char *)dest;
	
	while (len != 0)
	{
		*pdest++ = set;
		len--;
	}
	
}
void Jump_APP(void)
{

	//__HAL_UART_DISABLE_IT(&huart1, UART_IT_IDLE); 
	//	HAL_UART_DMAStop(&huart1);

	__HAL_RCC_DMA1_CLK_DISABLE();
	__HAL_RCC_DMA2_CLK_DISABLE();

  HAL_NVIC_DisableIRQ(DMA1_Channel2_IRQn);

  HAL_NVIC_DisableIRQ(DMA1_Channel3_IRQn);

  HAL_NVIC_DisableIRQ(DMA2_Channel3_IRQn);

  HAL_NVIC_DisableIRQ(DMA2_Channel4_5_IRQn);
	
	HAL_NVIC_DisableIRQ(DMA1_Channel4_IRQn);

  HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
	if (((*(__IO uint32_t*)APPLICATION_ADDRESS_A) & 0x2FFE0000 ) == 0x20000000) 
	{
		//MX_GPIO_DeInit();        
	     HAL_RCC_DeInit();//关闭外设 
	     //HAL_TIM_Base_DeInit(&htim6);    
	    // HAL_UART_MspDeInit(&huart1);            
		// HAL_UART_DMAStop(&huart1);
		 
	     
	    /* Jump to user application */
		__disable_irq();
	    JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS_A + 4);// ?
	    JumpToApplication = (pFunction) JumpAddress;//?
	    /* Initialize user application's Stack Pointer */
	    __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS_A); // ?
	    __enable_irq();
	    JumpToApplication(); // ?
	}

}

void ACK_cmd(void)  //提醒上位机或者手持设备发送升级包
{
	
	prepare_tx_buffer[0] = 0x72;
	prepare_tx_buffer[1] = 0x68;
	prepare_tx_buffer[2] = 0x01;
	prepare_tx_buffer[3] = 0x16;
	urat_tx_length = 4;

	DMA_Usart_Send(prepare_tx_buffer, urat_tx_length);
}
void ACK2_cmd(void)//收到升级包，返回上位机或者手持设备，发数据包
{
	prepare_tx_buffer[0] = 0x72;
	prepare_tx_buffer[1] = 0x68;
	prepare_tx_buffer[2] = 0x02;
	prepare_tx_buffer[3] = 0x16;
	urat_tx_length = 4;

	DMA_Usart_Send(prepare_tx_buffer, urat_tx_length);
}

void ACK3_cmd(unsigned int uart_number)//返回每一包的应答
{
	prepare_tx_buffer[0] = 0x72;
	prepare_tx_buffer[1] = 0x69;
	prepare_tx_buffer[2] = uart_number/256;
	prepare_tx_buffer[3] = uart_number%256;
	prepare_tx_buffer[4] = 0x16;
	urat_tx_length = 5;

	DMA_Usart_Send(prepare_tx_buffer, urat_tx_length);
}

void false_cmd()
{
	prepare_tx_buffer[0] = 0x72;
	prepare_tx_buffer[1] = 0x68;
	prepare_tx_buffer[2] = 0x03;
	prepare_tx_buffer[3] = 0x16;
	urat_tx_length = 4;

	DMA_Usart_Send(prepare_tx_buffer, urat_tx_length);
}

void BootLoaderCmd_handle(void)
{
	if((prepare_data.rx_buffer[0] == 0x72)&&(prepare_data.rx_buffer[1] == 0x68)&&(prepare_data.rx_buffer[4] == 0x16))
	{
		package_sum = prepare_data.rx_buffer[2]*256+prepare_data.rx_buffer[3];
		upgrade_bin_flag = 1;
		ACK2_cmd();
	}
	if((prepare_data.rx_buffer[0] == 0x72)&&(prepare_data.rx_buffer[1] == 0x68)&&(prepare_data.rx_buffer[2] == 0xaa)&&(prepare_data.rx_buffer[5] == 0x16))
	{
		upgrade_bin_flag = 0;
		ACK_cmd();
	}
}

void delay_us(uint32_t delay_us)
{    
//  volatile unsigned int num;
//  volatile unsigned int t;
// 
//  
//  for (num = 0; num < delay_us; num++)
//  {
//    t = 11;
//    while (t != 0)
//    {
//      t--;
//    }
//  }
}


FLASH_EraseInitTypeDef My_Flash;
uint32_t PageError = 0; 

void set_BootLoader_flag(void)
{
	uint16_t Write_Flash_Data = 0x00; 
	
	HAL_FLASH_Unlock(); 
	My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;  //标明Flash执行页面只作擦除操做
    My_Flash.PageAddress = BOOT_ADDRESS;  //声明要擦除的地址
    My_Flash.NbPages = 1; 
	HAL_FLASHEx_Erase(&My_Flash, &PageError);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, BOOT_ADDRESS, Write_Flash_Data);
	HAL_FLASH_Lock();
}

unsigned int pack_num = 0;

void upgradeCmd_handle(void)
{
	uint16_t Write_Flash_Data = 0; 
	uint16_t i = 0; 
	uint16_t pack_numb = prepare_data.rx_buffer[2]*256+prepare_data.rx_buffer[3]; 
	#if (FLASH_PAGES == 1024)
	uint16_t fash_page = (prepare_data.rx_buffer[2]*256+prepare_data.rx_buffer[3])/4; 
	#endif
	#if (FLASH_PAGES == 2048)
	uint16_t fash_page = (prepare_data.rx_buffer[2]*256+prepare_data.rx_buffer[3])/8; 
	#endif
	unsigned char res = 0x00;


	if((prepare_data.rx_buffer[0] == 0x72)&&(prepare_data.rx_buffer[1] == 0x68)&&(prepare_data.rx_buffer[2] == 0xaa)&&(prepare_data.rx_buffer[5] == 0x16))
	{
		upgrade_bin_flag = 0;
		ACK_cmd();
	}
	if((prepare_data.rx_buffer[0] == 0x72)&&(prepare_data.rx_buffer[1] == 0x69)&&(prepare_data.rx_buffer[261] == 0x16))
	{
		for ( i = 0; i < 256; i++)  //计算checksum
		{
			res ^= prepare_data.rx_buffer[i+4];  //异或
		}
		if(res != prepare_data.rx_buffer[260])
		{
			false_cmd();
			success_flag = FALSE;
			return;
		}
		HAL_FLASH_Unlock(); 
		#if (FLASH_PAGES == 1024)
		if(pack_numb%4 == 1)
		{
			My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;  //标明Flash执行页面只作擦除操做
			My_Flash.PageAddress = APPLICATION_ADDRESS_A+fash_page*1024;  //声明要擦除的地址
	        My_Flash.NbPages = 1; 
			HAL_FLASHEx_Erase(&My_Flash, &PageError);
		}

		for(i=0; i<256; i=i+2)
		{
			Write_Flash_Data = prepare_data.rx_buffer[i+4]+prepare_data.rx_buffer[i+5]*256;
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APPLICATION_ADDRESS_A+fash_page*1024+(pack_numb%4-1)*256+i, Write_Flash_Data);

		}
		#endif
		#if (FLASH_PAGES == 2048)
		if(pack_numb%8 == 1)
		{
			My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;  //标明Flash执行页面只作擦除操做
			My_Flash.PageAddress = APPLICATION_ADDRESS_A+fash_page*2048;  //声明要擦除的地址
	        My_Flash.NbPages = 1; 
			HAL_FLASHEx_Erase(&My_Flash, &PageError);
		}

		for(i=0; i<256; i=i+2)
		{
			Write_Flash_Data = prepare_data.rx_buffer[i+4]+prepare_data.rx_buffer[i+5]*256;
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APPLICATION_ADDRESS_A+fash_page*2048+(pack_numb%8-1)*256+i, Write_Flash_Data);

		}
		#endif
		HAL_FLASH_Lock();

		//ACK3_cmd(prepare_data.rx_buffer[2]);
		pack_num = prepare_data.rx_buffer[2]*256+prepare_data.rx_buffer[3];
		if(pack_num == package_sum)
		{
			ACK3_cmd(pack_num);
			need_to_upgrade = 0;
			success_flag = TURE;
			set_BootLoader_flag();
			delay_us(300);
			delay_us(500);
			__set_FAULTMASK(1);
			HAL_NVIC_SystemReset();
		}else
		{
			ACK3_cmd(pack_num);
		}
	}
}

void uart_data_handle(void)
{
	
	if(Uart_Up_IT == 1)
	{
//		urat_tx_length = 0;
		if(upgrade_bin_flag == 0)
		{
			BootLoaderCmd_handle();
		}else
		{
			upgradeCmd_handle();
		}

		analysis_command_BOOT();
		Uart_Up_IT = 0;
		
	}
}

void uart_task(void)
{
	uint16_t data;

	data = *(uint16_t *)(BOOT_ADDRESS);
	if(data == 0xaa){
		need_to_upgrade=1;
		ACK_cmd();
	}else
	{
		need_to_upgrade=0;
	}
	while(1)
	{
		if(need_to_upgrade == 1){
			uart_data_handle();
			
		}else
		{
			
		}
		if(success_flag == FALSE)
		{
			success_flag = 0;
			need_to_upgrade=1;
			ACK_cmd();
		}
	}
}



































uint8_t UART_ID=0;

void ACK_cmd_for_fpag(void)  //提醒上位机或者手持设备发送升级包
{
	prepare_tx_buffer[0] = 0xfa;
	prepare_tx_buffer[1] = 0x68;
	prepare_tx_buffer[2] = 0x01;
	prepare_tx_buffer[3] = 0x16;
	urat_tx_length = 4;

	DMA_Usart_Send(prepare_tx_buffer, urat_tx_length);
	/* The TCP transparent serial module (CH9121) is wired to USART3. */
	HAL_UART_Transmit_DMA(&huart3, prepare_tx_buffer, urat_tx_length);
}
uint8_t data_temp_fpga[2048];
void uart_fpga_updata(void)
{
	spi_w_handle.up_cmd=1;
	rx_length=0;
	ACK_cmd_for_fpag();
//	FPGA_Data_send(0XFFF2,0x00);//地址
//	FPGA_Data_send(0XFFF1,0x00);//复位
	HAL_Delay(1000);
	HAL_Delay(1000);
	spi_w_handle.up_cmd=0;
//	HAL_Delay(1000);
//	ACK_cmd_for_fpag();
	while(1)
	{

		

		
		if(spi_w_handle.up_cmd==1)//清空数据
		{
		 //0XA000 开始  64K擦除  12次,FPGA固件地址跳转
			for(int k=0;k<12;k++)
			W25Q128_Block_Erase_64K(0x0c0000+k*64*1024);
			
//			cmd_40_7_up();

			spi_w_handle.up_cmd=0;
		}
		else if(spi_w_handle.up_cmd==2) //写数据
		{
			for(int k=0;k<spi_w_handle.block_size_t/256;k++)
				W25Q128_Write(0x0c0000+(spi_w_handle.addr-1)*spi_w_handle.block_size_t+k*256,spi_w_handle.spi_data+k*256,256);
			
//			W25Q128_Read(0x0c0000+(spi_w_handle.addr-1)*256,data_temp_fpga,256);//数据校验
//			for(int k=0;k<256;k++)
//			{
//				if(data_temp_fpga[k]!=spi_w_handle.spi_data[k])
//				{
//					spi_w_handle.up_cmd=0xff;
//					break;
//				}
//				else spi_w_handle.up_cmd=0;
//			}
			spi_w_handle.up_cmd=0;
			
//			cmd_40_263_up();
			if(spi_w_handle.size_t==spi_w_handle.addr&&spi_w_handle.size_t!=0)  //FPGA,跳转到APP区启动，同时MCU标记完成固件升级工作
			{
				FPGA_Data_send(0XFFF2,0x0C);//地址
				FPGA_Data_send(0XFFF1,0x00);//复位
				
				set_BootLoader_flag();
				delay_us(300);
				delay_us(500);
				__set_FAULTMASK(1);
				HAL_NVIC_SystemReset();
			}
			
		}
		
	}
}


extern 	uint16_t data;
void My_Uart_Handler(UART_HandleTypeDef *huart,DMA_HandleTypeDef *hdma_usart_rx)
{
  /* USER CODE BEGIN USART1_IRQn 0 */
	uint8_t temp_flag;

  /* USER CODE END USART1_IRQn 0 */
  /* USER CODE BEGIN USART1_IRQn 1 */
	temp_flag =__HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE); //获取IDLE状态
	if((temp_flag != RESET))//如果数据不为空
	{
		__HAL_UART_CLEAR_IDLEFLAG(huart);//清除标志位
		HAL_UART_DMAStop(huart);
		RX_data_length=Uart_Rx_Length-__HAL_DMA_GET_COUNTER(hdma_usart_rx); 
		if(RX_data_length!=0)
		{
				Uart_Up_IT=1;
				input_data(Uart_RxBuf,RX_data_length);

		}
		HAL_UART_Receive_DMA(huart,Uart_RxBuf,Uart_Rx_Length);//开启DMA接收
				if(data==0x55)
				Uart_Task();
	}
}
void Uart_Task(void)
{
//	u16 i;
	unsigned int  _RecByte;
	uint8_t *add_tx_data;
	
	//if(Uart_Up_IT>=1)Uart_Up_IT++;
	if(Uart_Up_IT==1)
	{
		analysis_command();
		RX_data_length=0;//接收长度清零
		Uart_Up_IT=0;//串口更新标志位清零
	}
	
	add_tx_data=&(*(get_prepare_tx_buffer(&_RecByte)));//传回地址+数量
	if(_RecByte!=0)
	{
		if(UART_ID == 1)
			HAL_UART_Transmit_DMA(&huart1, add_tx_data, _RecByte);
		else if(UART_ID == 3)
			HAL_UART_Transmit_DMA(&huart3, add_tx_data, _RecByte);
		prepare_tx_length=0;
		//将数据装载到DMA发送
	}
}
