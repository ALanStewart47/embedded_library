#include "uart.h"
 



#define APPLICATION_ADDRESS_A  0x08003800

#define BOOT_ADDRESS	0x8003400


#define TURE	  1
#define FALSE	  2

typedef  void (*pFunction)(void);
pFunction JumpToApplication;
uint32_t JumpAddress;

unsigned char urat_tx_buffer[100] = {0};    
unsigned char urat_tx_length = 0;

unsigned int rx_length;
unsigned char rx_buffer[BUFFER_SIZE];
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
extern TIM_HandleTypeDef htim6;


unsigned int dma_ms = 0;
unsigned int dma_ms_flag = 0;
unsigned char  dma_number = 1;

void init_Uart_data(void)
{
	my_memset(urat_tx_buffer,0,100);


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
	HAL_UART_Receive_DMA(&huart1,rx_buffer,BUFFER_SIZE);
	#endif
	#if (URAT_NUMBER == 2)
	HAL_UART_Receive_DMA(&huart2,rx_buffer,BUFFER_SIZE);
	#endif
	#if (URAT_NUMBER == 3)
	HAL_UART_Receive_DMA(&huart3,rx_buffer,BUFFER_SIZE);
	#endif
	#if (URAT_NUMBER == 4)
	HAL_UART_Receive_DMA(&huart4,rx_buffer,BUFFER_SIZE);
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
	 
	}

}
void delay_ms(unsigned int time)
{    
   unsigned int i=0;  
   while(time--)
   {
      i=12000;   
      while(i--) ;    
   }
}

void dma_delay_jisuan(void)
{
	if(dma_ms_flag == 1)
	{
		delay_ms(3);
		 
		 
		{
			uart_receive(dma_number);
			dma_ms_flag = 0;
			dma_ms = 0;
		}
	}
}
void uart_receive_number(unsigned char uart_number)
{
	dma_ms_flag = 1;
	 
	dma_number = uart_number;
	dma_delay_jisuan();
}

#if (URAT_NUMBER == 1)
void uart_receive(unsigned char uart_number)
{
	unsigned char recv_flag = 0;
	uint16_t numb = 0;
	
	recv_flag =__HAL_UART_GET_FLAG(&huart1,UART_FLAG_IDLE);

	if((recv_flag != 0))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);
		HAL_UART_DMAStop(&huart1);

		numb = __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);	 

		rx_length = BUFFER_SIZE - numb;

		rx_endFlag =1;
	}

	HAL_UART_IRQHandler(&huart1);
}
#endif
#if (URAT_NUMBER == 2)
void uart_receive(unsigned char uart_number)
{
	unsigned char recv_flag = 0;
	uint16_t numb = 0;
	
	recv_flag =__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE);

	if((recv_flag != 0))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart2);
		HAL_UART_DMAStop(&huart2);

		numb = __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);	 

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
	uint16_t numb = 0;
	
	recv_flag =__HAL_UART_GET_FLAG(&huart3,UART_FLAG_IDLE);

	if((recv_flag != 0))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart3);
		HAL_UART_DMAStop(&huart3);

		numb = __HAL_DMA_GET_COUNTER(&hdma_usart3_rx);	 

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
	uint16_t numb = 0;
	
	recv_flag =__HAL_UART_GET_FLAG(&huart4,UART_FLAG_IDLE);

	if((recv_flag != 0))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart4);
		HAL_UART_DMAStop(&huart4);

		numb = __HAL_DMA_GET_COUNTER(&hdma_uart4_rx);	 

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
		 
	     HAL_RCC_DeInit(); 
	     HAL_TIM_Base_DeInit(&htim6);    
	     
		 
		 
	     
	     
		__disable_irq();
	    JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS_A + 4); 
	    JumpToApplication = (pFunction) JumpAddress; 
	     
	    __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS_A);  
	    __enable_irq();
	    JumpToApplication();  
	}

}

void ACK_cmd(void)   
{
	urat_tx_buffer[0] = 0x72;
	urat_tx_buffer[1] = 0x68;
	urat_tx_buffer[2] = 0x01;
	urat_tx_buffer[3] = 0x16;
	urat_tx_length = 4;

	DMA_Usart_Send(urat_tx_buffer, urat_tx_length);
}
void ACK2_cmd(void) 
{
	urat_tx_buffer[0] = 0x72;
	urat_tx_buffer[1] = 0x68;
	urat_tx_buffer[2] = 0x02;
	urat_tx_buffer[3] = 0x16;
	urat_tx_length = 4;

	DMA_Usart_Send(urat_tx_buffer, urat_tx_length);
}

void ACK3_cmd(unsigned int uart_number) 
{
	urat_tx_buffer[0] = 0x72;
	urat_tx_buffer[1] = 0x69;
	urat_tx_buffer[2] = uart_number/256;
	urat_tx_buffer[3] = uart_number%256;
	urat_tx_buffer[4] = 0x16;
	urat_tx_length = 5;

	DMA_Usart_Send(urat_tx_buffer, urat_tx_length);
}

void false_cmd()
{
	urat_tx_buffer[0] = 0x72;
	urat_tx_buffer[1] = 0x68;
	urat_tx_buffer[2] = 0x03;
	urat_tx_buffer[3] = 0x16;
	urat_tx_length = 4;

	DMA_Usart_Send(urat_tx_buffer, urat_tx_length);
}

void BootLoaderCmd_handle(void)
{
	if((rx_buffer[0] == 0x72)&&(rx_buffer[1] == 0x68)&&(rx_buffer[4] == 0x16))
	{
		 
		{
			 
		}
		package_sum = rx_buffer[2]*256+rx_buffer[3];
		upgrade_bin_flag = 1;
		ACK2_cmd();
		 
	}
	if((rx_buffer[0] == 0x72)&&(rx_buffer[1] == 0x68)&&(rx_buffer[2] == 0xaa)&&(rx_buffer[5] == 0x16))
	{
		upgrade_bin_flag = 0;
		ACK_cmd();
	}
}

void delay_us(uint32_t delay_us)
{    
  volatile unsigned int num;
  volatile unsigned int t;
 
  
  for (num = 0; num < delay_us; num++)
  {
    t = 11;
    while (t != 0)
    {
      t--;
    }
  }
}


FLASH_EraseInitTypeDef My_Flash;
uint32_t PageError = 0; 

void set_BootLoader_flag(void)
{
	uint16_t Write_Flash_Data = 0x00; 
	
	HAL_FLASH_Unlock(); 
	My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;   
    My_Flash.PageAddress = BOOT_ADDRESS;   
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
	uint16_t pack_numb = rx_buffer[2]*256+rx_buffer[3]; 
	#if (FLASH_PAGES == 1024)
	uint16_t fash_page = (rx_buffer[2]*256+rx_buffer[3])/4; 
	#endif
	#if (FLASH_PAGES == 2048)
	uint16_t fash_page = (rx_buffer[2]*256+rx_buffer[3])/8; 
	#endif
	unsigned char res = 0x00;


	if((rx_buffer[0] == 0x72)&&(rx_buffer[1] == 0x68)&&(rx_buffer[2] == 0xaa)&&(rx_buffer[5] == 0x16))
	{
		upgrade_bin_flag = 0;
		ACK_cmd();
	}
	if((rx_buffer[0] == 0x72)&&(rx_buffer[1] == 0x69)&&(rx_buffer[261] == 0x16))
	{
		for ( i = 0; i < 256; i++)   
		{
			res ^= rx_buffer[i+4];   
		}
		if(res != rx_buffer[260])
		{
			false_cmd();
			success_flag = FALSE;
			return;
		}
		HAL_FLASH_Unlock(); 
		#if (FLASH_PAGES == 1024)
		if(pack_numb%4 == 1)
		{
			My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;   
			My_Flash.PageAddress = APPLICATION_ADDRESS_A+fash_page*1024;   
	        My_Flash.NbPages = 1; 
			HAL_FLASHEx_Erase(&My_Flash, &PageError);
		}

		for(i=0; i<256; i=i+2)
		{
			Write_Flash_Data = rx_buffer[i+4]+rx_buffer[i+5]*256;
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APPLICATION_ADDRESS_A+fash_page*1024+(pack_numb%4-1)*256+i, Write_Flash_Data);

		}
		#endif
		#if (FLASH_PAGES == 2048)
		if(pack_numb%8 == 1)
		{
			My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;   
			My_Flash.PageAddress = APPLICATION_ADDRESS_A+fash_page*2048;   
	        My_Flash.NbPages = 1; 
			HAL_FLASHEx_Erase(&My_Flash, &PageError);
		}

		for(i=0; i<256; i=i+2)
		{
			Write_Flash_Data = rx_buffer[i+4]+rx_buffer[i+5]*256;
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APPLICATION_ADDRESS_A+fash_page*2048+(pack_numb%8-1)*256+i, Write_Flash_Data);

		}
		#endif
		HAL_FLASH_Lock();

		 
		pack_num = rx_buffer[2]*256+rx_buffer[3];
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
	
	if(rx_endFlag == 1)
	{
		urat_tx_length = 0;
		if(upgrade_bin_flag == 0)
		{
			BootLoaderCmd_handle();
		}else
		{
			upgradeCmd_handle();
		}

		my_memset(rx_buffer,0,rx_length);
		rx_length = 0;
		rx_endFlag = 0;

		UART_Receive_DMA();
		
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

