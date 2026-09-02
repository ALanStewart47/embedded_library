/******************************************************************************
 * Copyright (C) 2024 CST, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file boot_uart.c
 *
 * @par dependencies
 * - boot_uart.h
 *
 * @author imphx
 * 		   Alan 
 * 					| R&D Dept. | CST
 *
 * @brief Provide the HAL APIs of the bootloader update.
 *
 * Processing flow:
 * add poweron_self_check() into  USER CODE BEGIN 1
 * add boot_uart_config();  into USER CODE BEGIN 2
 * add bsp_uart_baud_reinit(); if you need it.
 * add bsp_ota_handle(); into while(1)
 * add uart_interrupt_handle(x); into USART IRQHandler ,x:1~4 
 * 		e.g. using uart_1 , add uart_interrupt_handle(1); 
 * 			into USART1_IRQHandler(); and under the HAL_UART_IRQHandler(&huart1);
 *
 * @version 	V1.0 	2025-03-17 		Alan
 * @note 
 *       1 tab == 4 spaces!
 *
 *****************************************************************************/
#include "boot_uart.h"

#define TURE	  1
#define FALSE	  2

typedef  void (*pFunction)(void);
pFunction JumpToApplication;
uint32_t JumpAddress;

unsigned char urat_tx_buffer[100] = {0};   //uart3 == 232  uart4 ==缃戝彛閫忎紶
unsigned char urat_tx_length = 0;

unsigned int rx_length;
unsigned char rx_buffer[300];
unsigned char rx_endFlag;

unsigned char need_to_upgrade = 1;
//unsigned char version[2] = {1,0};
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


//extern DMA_HandleTypeDef hdma_usart1_tx;
//extern TIM_HandleTypeDef htim6;

unsigned int dma_ms = 0;
unsigned int dma_ms_flag = 0;
unsigned char  dma_number = 1;

uint32_t g_rs232_baud_buf[BAUD_MAX-1] = {
	4800,
	9600,
	14400,
	19200,
	28800,
	38400,
	57600,
	115200
};

void delay_us(uint32_t delay_us)
{    
  volatile unsigned int num;
  volatile unsigned int t;

  for (num = 0; num < delay_us; num++)
  {
    t = 11;
    while (t != 0){
      t--;}
  }
}

void delay_ms(unsigned int time)
{    
   unsigned int i=0;  
   while(time--)
   {
      i=12000;  //鑷?繁瀹氫箟
      while(i--) ;    
   }
}


typedef  void (*pFunction)(void);
pFunction JumpToApplication1;
uint32_t JumpAddress1;

void  poweron_to_app(void)
{
	if (((*(__IO uint32_t*)APPLICATION_ADDRESS_A) & 0x2FFE0000 ) == 0x20000000) 
	{
	     
	    /* Jump to user application */
		__disable_irq();
	    JumpAddress1 = *(__IO uint32_t*) (APPLICATION_ADDRESS_A + 4);// ?
	    JumpToApplication1 = (pFunction) JumpAddress1;//?
	    /* Initialize user application's Stack Pointer */
	    __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS_A); // ?
	    __enable_irq();
	    JumpToApplication1(); // ?
	}
}

void poweron_self_check(void)
{
	uint8_t data;

	data = *(uint16_t *)(BOOT_ADDRESS);

	if(data == 0xaa){
		
	}
	else
	{
		poweron_to_app();
	}
}


void init_Uart_data(void)
{
	my_memset(urat_tx_buffer,0,100);
}

void boot_uart_config(void)
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
	//Error_Handler();
	}
}


void dma_delay_jisuan(void)
{
	if(dma_ms_flag == 1)
	{
		delay_ms(3);
		//dma_ms++;
		//if(dma_ms == 2000)
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
	//dma_ms = 0;
	dma_number = uart_number;
	dma_delay_jisuan();
}


#if (URAT_NUMBER == 1)
void uart_receive(unsigned char uart_number)
{
	unsigned char recv_flag = 0;
	unsigned char numb = 0;
	
	recv_flag =__HAL_UART_GET_FLAG(&huart1,UART_FLAG_IDLE);

	if((recv_flag != 0))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);
		HAL_UART_DMAStop(&huart1);

		numb = __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);	// 鑾峰彇DMA涓?鏈?浼犺緭鐨勬暟鎹?涓?鏁?

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
	unsigned char numb = 0;
	
	recv_flag =__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE);

	if((recv_flag != 0))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart2);
		HAL_UART_DMAStop(&huart2);

		numb = __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);	// 鑾峰彇DMA涓?鏈?浼犺緭鐨勬暟鎹?涓?鏁?

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

		numb = __HAL_DMA_GET_COUNTER(&hdma_usart3_rx);	// 鑾峰彇DMA涓?鏈?浼犺緭鐨勬暟鎹?涓?鏁?

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

		numb = __HAL_DMA_GET_COUNTER(&hdma_uart4_rx);	// 鑾峰彇DMA涓?鏈?浼犺緭鐨勬暟鎹?涓?鏁?

		rx_length = BUFFER_SIZE - numb;

		rx_endFlag =1;
	}

	HAL_UART_IRQHandler(&huart4);
}
#endif

void uart_interrupt_handle(unsigned char uart_number)
{
	delay_ms(3);
	uart_receive(uart_number);
}



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
	if (((*(__IO uint32_t*)APPLICATION_ADDRESS_A) & 0x2FFE0000 ) == 0x20000000) 
	{   
	     HAL_RCC_DeInit();//关闭外设 
	    /* Jump to user application */
		__disable_irq();
	    JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS_A + 4);// 
	    JumpToApplication = (pFunction) JumpAddress;//?
	    /* Initialize user application's Stack Pointer */
	    __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS_A); // 
	    __enable_irq();
	    JumpToApplication(); // 
	}
}

void ACK_cmd(void)  //提醒上位机或者手持设备发送升级包
{
	urat_tx_buffer[0] = 0x72;
	urat_tx_buffer[1] = 0x68;
	urat_tx_buffer[2] = 0x01;
	urat_tx_buffer[3] = 0x16;
	urat_tx_length = 4;

	DMA_Usart_Send(urat_tx_buffer, urat_tx_length);
}
void ACK2_cmd(void)//收到升级包，返回上位机或者手持设备，发数据包
{
	urat_tx_buffer[0] = 0x72;
	urat_tx_buffer[1] = 0x68;
	urat_tx_buffer[2] = 0x02;
	urat_tx_buffer[3] = 0x16;
	urat_tx_length = 4;

	DMA_Usart_Send(urat_tx_buffer, urat_tx_length);
}

void ACK3_cmd(unsigned int uart_number)//返回每一包的应答
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
		//if((version[0] == rx_buffer[2])&&(version[1] == rx_buffer[3]))
		{
			//Jump_APP();
		}
		package_sum = rx_buffer[2]*256+rx_buffer[3];
		upgrade_bin_flag = 1;
		ACK2_cmd();
		//ACK2_cmd();
	}
	if((rx_buffer[0] == 0x72)&&(rx_buffer[1] == 0x68)&&(rx_buffer[2] == 0xaa)&&(rx_buffer[5] == 0x16))
	{
		upgrade_bin_flag = 0;
		ACK_cmd();
	}
}


uint32_t GetPage(uint32_t Addr)
{
  return (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
}


FLASH_EraseInitTypeDef My_Flash;
uint32_t PageError = 0; 

void set_BootLoader_flag(void)
{
	uint16_t Write_Flash_Data = 0x00; 
	
    /* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock(); 
    
        FLASH->ACR &= ~(1 << 10);
    
    
    /* Clear OPTVERR bit set on virgin samples */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
     __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGAERR|FLASH_FLAG_PGSERR|FLASH_FLAG_WRPERR);
	
    
	My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;     //标明Flash执行页面只作擦除操做
    My_Flash.Page = GetPage(BOOT_ADDRESS);          //声明要擦除的地址
    My_Flash.NbPages = 1; 
	
    if (HAL_FLASHEx_Erase(&My_Flash, &PageError) != HAL_OK)
    {
        while(1);  
    }
    
	if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, BOOT_ADDRESS, Write_Flash_Data) !=  HAL_OK)
    {
        while(1);
    }
    
     __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGAERR|FLASH_FLAG_PGSERR|FLASH_FLAG_WRPERR);
	
        FLASH->ACR |= 1 << 10;
    
	HAL_FLASH_Lock();
}

unsigned int pack_num = 0;

void upgradeCmd_handle(void)
{
	uint64_t Write_Flash_Data = 0; 
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
		for ( i = 0; i < 256; i++)  //计算checksum
		{
			res ^= rx_buffer[i+4];  //异或
		}
		if(res != rx_buffer[260])
		{
			false_cmd();
			success_flag = FALSE;
			return;
		}
        
        /* Unlock the Flash to enable the flash control register access *************/
		HAL_FLASH_Unlock();
        
        FLASH->ACR &= ~(1 << 10);
        
        /* Clear OPTVERR bit set on virgin samples */
         __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
        
         __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGAERR|FLASH_FLAG_PGSERR|FLASH_FLAG_WRPERR);
	
		
		#if (FLASH_PAGES == 1024)
		if(pack_numb%4 == 1)
		{
			My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;  //标明Flash执行页面只作擦除操做
			My_Flash.Page = APPLICATION_ADDRESS_A+fash_page*1024;  //声明要擦除的地址
	        My_Flash.NbPages = 1; 
			HAL_FLASHEx_Erase(&My_Flash, &PageError);
		}

		for(i=0; i<256; i=i+2)
		{
			Write_Flash_Data = rx_buf fer[i+4]+rx_buffer[i+5]*256;
			//HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APPLICATION_ADDRESS_A+fash_page*1024+(pack_numb%4-1)*256+i, Write_Flash_Data);

		}
		#endif
		
		#if (FLASH_PAGES == 2048)
		if(pack_numb%8 == 1)
		{
			
			My_Flash.TypeErase = FLASH_TYPEERASE_PAGES;  //标明Flash执行页面只作擦除操做
			My_Flash.Page = GetPage(APPLICATION_ADDRESS_A)+fash_page;  //声明要擦除的地址
            My_Flash.NbPages = 1; 
			if(HAL_FLASHEx_Erase(&My_Flash, &PageError) != HAL_OK)
            {
                while(1);
            }
		}

		for(i=0; i<256; i=i+8)
		{
			Write_Flash_Data =  rx_buffer[i+4]   + 
                                ((uint64_t)rx_buffer[i+5]<<8)   + 
                                ((uint64_t)rx_buffer[i+6]<<16)  + 
                                ((uint64_t)rx_buffer[i+7]<<24)  + 
                                ((uint64_t)rx_buffer[i+8]<<32)  + 
                                ((uint64_t)rx_buffer[i+9]<<40)  + 
                                ((uint64_t)rx_buffer[i+10]<<48) + 
                                ((uint64_t)rx_buffer[i+11]<<56)   ;
			
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, APPLICATION_ADDRESS_A+fash_page*2048+(pack_numb%8-1)*256+i, Write_Flash_Data) != HAL_OK)
            {
                while(1);
            }
		}
		#endif
         __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGAERR|FLASH_FLAG_PGSERR|FLASH_FLAG_WRPERR);
	
        FLASH->ACR |= 1 << 10;
        
		HAL_FLASH_Lock();
	       
		pack_num = rx_buffer[2]*256+rx_buffer[3];
		if(pack_num == package_sum)
		{
			ACK3_cmd(pack_num);
            need_to_upgrade = 0;
			success_flag = TURE;
            set_BootLoader_flag();

            HAL_RCC_DeInit();
            HAL_DeInit();
 
            SysTick->CTRL = 0;
            SysTick->LOAD = 0;
            SysTick->VAL  = 0;
            
            my_memset(urat_tx_buffer,0,sizeof(urat_tx_buffer));
//            
//			__set_FAULTMASK(1);
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
	uint8_t data;

	data = *(uint16_t *)(BOOT_ADDRESS);
	if((data == 0xaa) || (0xFF == data)){
		need_to_upgrade=1;
		ACK_cmd();
	}else
	{
		need_to_upgrade=0;
	}
	while(1)
	{
		//set_fenqu_flag();
		if(need_to_upgrade == 1){
		uart_data_handle();
			//ACK_cmd();
		}else
		{
			//Jump_APP();
		}
		if(success_flag == FALSE)
		{
			success_flag = 0;
			need_to_upgrade=1;
			ACK_cmd();
		}
	}
}


void bsp_ota_handle(void)
{
	HAL_Delay(100);
	uart_task();
}


static uint32_t bsp_check_baud(void)
{
	uint8_t _usBaud_gear = 0;
	uint32_t _usBaud = 19200;
	uint16_t _usFlag	=	0;
	
	_usFlag	=	(uint16_t)	*(uint16_t *) (BAUD_DATA_SAVE_ADDR);
	
	if(_usFlag	==	0x99)
	{
		_usBaud_gear	=	(uint8_t)	*(uint16_t *) ( BAUD_DATA_SAVE_ADDR + 2 );
		if((_usBaud_gear < 9) && (_usBaud_gear >= 1))
		{
			_usBaud = g_rs232_baud_buf[_usBaud_gear-1];
			return	_usBaud;
		}
		else
		{
			_usBaud_gear = 4;
			_usBaud = 19200;
			return	_usBaud;
		}

	}
	else
	{
		_usBaud_gear = 4;
		_usBaud = g_rs232_baud_buf[_usBaud_gear-1];
		return	_usBaud;
	}
}
  uint32_t baudrate = 0;
void bsp_uart_baud_reinit(void )
{
  /* USER CODE BEGIN USART1_Init 0 */
  //uint32_t baudrate = 0;
  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */
	#if (URAT_NUMBER == 1)
		huart1.Instance = USART1;
	#endif
  
	#if (URAT_NUMBER == 2)
		huart2.Instance = USART2;
	#endif

	#if (URAT_NUMBER == 3)
		huart.Instance = USART3;
	#endif

	#if (URAT_NUMBER == 4)
		huart.Instance = UART4;
	#endif

  /* USER CODE END USART1_Init 1 */
	baudrate = bsp_check_baud();
	huart2.Init.BaudRate = baudrate;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}

