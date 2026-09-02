#include "protocol_public.h"



UART_Prepare_Buf prepare_data; 
unsigned char prepare_tx_buffer[SUM_SIZE] = {0};
unsigned char prepare_tx_length = 0;

void init_protocol_para(void)
{
	unsigned char i = 0;
	
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

void input_data(unsigned char *data,unsigned char length)
{
	unsigned char j = 0;
	unsigned char now_position = 0;
	
	if(prepare_data.state == FREE)
	{
		if(prepare_data.available_length < length)
		{
			return ;   
		}
		now_position = prepare_data.data_end_position;
		for(j = 0; j < length; j++)
		{
			prepare_data.rx_buffer[j+now_position] = data[j];
		}
		prepare_data.data_end_position = now_position + length;
		prepare_data.available_length = prepare_data.available_length - length;
	}
}

void package_tx_buffer(unsigned char *buf,unsigned char len)
{
	unsigned char i = 0;

	for(i=0; i < len; i++)
	{
		prepare_tx_buffer[prepare_tx_length] = buf[i];
		prepare_tx_length++;
	}
	
}
void set_brightness_API(unsigned char channel,unsigned int value)
{
	unsigned char tx_len = 1;
	unsigned char tx_buffer[1] = {0};
	
	 
	tx_buffer[0] = 'a' + channel;

	package_tx_buffer(tx_buffer, tx_len);
}
void read_brightness_API(unsigned char channel)
{
	unsigned char tx_len = 5;
	unsigned char tx_buffer[5] = {0};
	unsigned int temp = 0;

	 
	tx_buffer[0] = 'a' + channel;
	tx_buffer[1] = '0';
	tx_buffer[2] = temp/100 + '0';
	tx_buffer[3] = temp%100/10 + '0';
	tx_buffer[4] = temp%10 + '0';
	
	package_tx_buffer(tx_buffer, tx_len);
}

void set_trigger_cycle_API(unsigned int value)
{
	unsigned char tx_len = 1;
	unsigned char tx_buffer[1] = {0};
	
	 
	tx_buffer[0] = 't' ;

	package_tx_buffer(tx_buffer, tx_len);
}

void read_trigger_cycle_API(void)
{
	unsigned char tx_len = 5;
	unsigned char tx_buffer[5] = {0};
	unsigned int temp = 0;

	 
	tx_buffer[0] = 't';
	tx_buffer[1] = '0';
	tx_buffer[2] = temp/100 + '0';
	tx_buffer[3] = temp%100/10 + '0';
	tx_buffer[4] = temp%10 + '0';
	
	package_tx_buffer(tx_buffer, tx_len);
}
void set_area_brightness_API(unsigned char channel)
{
	unsigned char tx_len = 1;
	unsigned char tx_buffer[1] = {0};
	
	 
	tx_buffer[0] = 'a'+ channel ;

	package_tx_buffer(tx_buffer, tx_len);
}
void read_area_brightness_API(void)
{
	unsigned char tx_len = 6;
	unsigned char tx_buffer[6] = {0};
	unsigned int temp = 0;

	 
	tx_buffer[0] = 's';
	tx_buffer[1] = 'w';
	tx_buffer[2] = temp/100 + '0';
	tx_buffer[3] = temp%100/10 + '0';
	tx_buffer[4] = temp%10 + '0';
	tx_buffer[5] = temp%10 + '0';
	
	package_tx_buffer(tx_buffer, tx_len);
}

void soft_trigger_API(void)
{
	unsigned char tx_len = 1;
	unsigned char tx_buffer[1] = {0};

	 
	tx_buffer[0] = '#';
	package_tx_buffer(tx_buffer, tx_len);
}

void save_para_API(void)
{
	unsigned char tx_len = 1;
	unsigned char tx_buffer[1] = {0};

	 
	tx_buffer[0] = '#';
	package_tx_buffer(tx_buffer, tx_len);
}

void read_SP_pulseWidth_unit_API(unsigned char channel)
{
	unsigned char tx_len = 4;
	unsigned char tx_buffer[4] = {0};
	unsigned int temp = 0;

	 
	tx_buffer[0] = 'p';
	tx_buffer[1] = 'u';
	tx_buffer[2] = channel + 'a';
	tx_buffer[3] = temp + '0';
	
	package_tx_buffer(tx_buffer, tx_len);
}

void set_SP_pulseWidth_unit_API(unsigned char channel,unsigned int value)
{
	unsigned char tx_len = 3;
	unsigned char tx_buffer[3] = {0};

	 
	tx_buffer[0] = 'p';
	tx_buffer[1] = 'u';
	tx_buffer[2] = channel + 'a';

	package_tx_buffer(tx_buffer, tx_len);
}


void read_SP_pulseWidth_API(unsigned char channel)
{
	unsigned char tx_len = 6;
	unsigned char tx_buffer[6] = {0};
	unsigned int temp = 0;

	 
	tx_buffer[0] = 'p';
	tx_buffer[1] = channel + 'a';
	tx_buffer[2] = '0';
	tx_buffer[3] = temp/100 + '0';
	tx_buffer[4] = temp%100/10 + '0';
	tx_buffer[5] = temp%10 + '0';
	
	package_tx_buffer(tx_buffer, tx_len);
}

void set_SP_pulseWidth_API(unsigned char channel,unsigned int value)
{
	unsigned char tx_len = 2;
	unsigned char tx_buffer[2] = {0};

	 
	tx_buffer[0] = 'p';
	tx_buffer[1] =  channel + 'a';

	package_tx_buffer(tx_buffer, tx_len);
}

void read_trigger_mode_API(void)
{
	unsigned char tx_len = 3;
	unsigned char tx_buffer[3] = {0};

	 
	tx_buffer[0] = 't';
	tx_buffer[1] = 'r';
	tx_buffer[2] = '0';
	package_tx_buffer(tx_buffer, tx_len);
}

void set_trigger_mode_API(unsigned int value)
{
	unsigned char tx_len = 2;
	unsigned char tx_buffer[2] = {0};

	 
	tx_buffer[0] = 't';
	tx_buffer[1] = 'r';
	package_tx_buffer(tx_buffer, tx_len);
}

void set_horl_mode_API(unsigned int value)
{
	unsigned char tx_len = 1;
	unsigned char tx_buffer[1] = {0};

	 
	tx_buffer[0] = value-'A'+'a';
	package_tx_buffer(tx_buffer, tx_len);
}

void read_horl_mode_API(void)
{
	unsigned char tx_len = 1;
	unsigned char tx_buffer[1] = {0};

	 
	tx_buffer[0] = 'H';
	package_tx_buffer(tx_buffer, tx_len);
}

void set_triggerpower_API(unsigned int value)  
{
	unsigned char tx_len = 2;
	unsigned char tx_buffer[2] = {0};

	 
	tx_buffer[0] = 't';
	tx_buffer[1] = 'p';
	package_tx_buffer(tx_buffer, tx_len);
}

void read_triggerpower_API(void)
{
	unsigned char tx_len = 3;
	unsigned char tx_buffer[3] = {0};

	 
	tx_buffer[0] = 't';
	tx_buffer[1] = 'p';
	tx_buffer[2] = '0';
	package_tx_buffer(tx_buffer, tx_len);
}

void set_light_delay_API(unsigned char channel,unsigned int value)
{
	unsigned char tx_len = 3;
	unsigned char tx_buffer[3] = {0};

	 
	tx_buffer[0] = 'd';
	tx_buffer[1] = 'l';
	tx_buffer[2] =  channel + 'a';

	package_tx_buffer(tx_buffer, tx_len);
}

void set_camera_delay_API(unsigned char channel,unsigned int value)
{
	unsigned char tx_len = 3;
	unsigned char tx_buffer[3] = {0};

	 
	tx_buffer[0] = 'd';
	tx_buffer[1] = 'c';
	tx_buffer[2] =	channel + 'a';

	package_tx_buffer(tx_buffer, tx_len);
}

void set_AllCamera_delay_API(unsigned int value)
{
	unsigned char tx_len = 1;
	unsigned char tx_buffer[1] = {0};

	 
	tx_buffer[0] = '#';

	package_tx_buffer(tx_buffer, tx_len);
}

void read_light_delay_API(unsigned char channel)
{
	unsigned char tx_len = 7;
	unsigned char tx_buffer[7] = {0};

	 
	tx_buffer[0] = 'd';
	tx_buffer[1] = 'l';
	tx_buffer[2] = channel + 'a';
	tx_buffer[3] = '0';
	tx_buffer[4] = '0';
	tx_buffer[5] = '0';
	tx_buffer[6] = '0';
	
	package_tx_buffer(tx_buffer, tx_len);
}

void read_camera_delay_API(unsigned char channel)
{
	unsigned char tx_len = 7;
	unsigned char tx_buffer[7] = {0};

	 
	tx_buffer[0] = 'd';
	tx_buffer[1] = 'c';
	tx_buffer[2] = channel + 'a';
	tx_buffer[3] = '0';
	tx_buffer[4] = '0';
	tx_buffer[5] = '0';
	tx_buffer[6] = '0';
	
	package_tx_buffer(tx_buffer, tx_len);
}

void read_AllCamera_delay_API(void)
{
	unsigned char tx_len = 6;
	unsigned char tx_buffer[6] = {0};

	 
	tx_buffer[0] = 'd';
	tx_buffer[1] = 'c';
	tx_buffer[2] = '0';
	tx_buffer[3] = '0';
	tx_buffer[4] = '0';
	tx_buffer[5] = '0';
	
	package_tx_buffer(tx_buffer, tx_len);
}

void set_trigger_UpRoDpwm_API(unsigned int value)
{
	unsigned char tx_len = 2;
	unsigned char tx_buffer[2] = {0};

	 
	tx_buffer[0] = 'c';
	tx_buffer[1] = 't';

	package_tx_buffer(tx_buffer, tx_len);
}

void read_trigger_UpRoDpwm_API(void)
{
	unsigned char tx_len = 3;
	unsigned char tx_buffer[3] = {0};

	 
	tx_buffer[0] = 'c';
	tx_buffer[1] = 't';
	tx_buffer[2] = '0';

	package_tx_buffer(tx_buffer, tx_len);
}



unsigned char SX0XXX_CMD(unsigned char *uart_buf,unsigned char i)
{
	unsigned char brightness = 0;
	if (uart_buf[i+2] == '0')
	{
		brightness = (uart_buf[i+3]- '0')*100+(uart_buf[i+4]- '0')*10+(uart_buf[i+5]- '0');
		if(brightness <= 255)
		{
			set_brightness_API(uart_buf[i+1]-'A',brightness); 
			return i+6;
		}
	}
	return i;
}
unsigned char SX_CMD(unsigned char *uart_buf,unsigned char i)
{	
	read_brightness_API(uart_buf[i+1]-'A'); 
	return i+2;		
}

unsigned char ST0XXX_CMD(unsigned char *uart_buf,unsigned char i)
{
	unsigned char trigger_cycle  = 0;

	if(uart_buf[i+2] == '0')
	{
		trigger_cycle= (uart_buf[i+3]- '0')*100+(uart_buf[i+4]- '0')*10+(uart_buf[i+5]- '0');
		if(trigger_cycle <= 999)
		{
			set_trigger_cycle_API(trigger_cycle); 
			return i+6;
		}
	}
	return i;
}
unsigned char ST_CMD(unsigned char *uart_buf,unsigned char i)
{
	read_trigger_cycle_API(); 
	return i+2;
}

unsigned char SWXXXX_CMD(unsigned char *uart_buf,unsigned char i)
{
	if(((uart_buf[i+2] >= '0')&&(uart_buf[i+2] <= '9'))&&((uart_buf[i+3] >= '0')&&(uart_buf[i+3] <= '9'))&&
		((uart_buf[i+4] >= '0')&&(uart_buf[i+4] <= '9'))&&((uart_buf[i+5] >= '0')&&(uart_buf[i+5] <= '9')))
	{
		set_area_brightness_API(uart_buf[i+1]-'A'); 
		return i+6;
	}
	return i;
}
unsigned char SW_CMD(unsigned char *uart_buf,unsigned char i)
{
	read_area_brightness_API(); 
	return i+2;
}

unsigned char SWTRIG_CMD(unsigned char *uart_buf,unsigned char i)
{
	
	if((uart_buf[i+2] == 'T')&&(uart_buf[i+3] == 'R')&&(uart_buf[i+4] == 'I')&&(uart_buf[i+5] == 'G'))
	{
		soft_trigger_API();
		return i+6;
	}
	return i;
}

unsigned char SAVE_CMD(unsigned char *uart_buf,unsigned char i)
{
	
	if((uart_buf[i+2] == 'V')&&(uart_buf[i+3] == 'E'))
	{
		save_para_API();
		return i+4;
	}
	return i;
}

unsigned char SPUX_CMD(unsigned char *uart_buf,unsigned char i)
{
	read_SP_pulseWidth_unit_API(uart_buf[i+3]-'A');
	return i+4;
}

unsigned char SXXX_CMD(unsigned char *uart_buf,unsigned char i)
{	
	 
	return i+4;		
}

unsigned char SPX_CMD(unsigned char *uart_buf,unsigned char i)
{
	read_SP_pulseWidth_API(uart_buf[i+2]-'A');
	return i+3;
}

unsigned char SPUXX_CMD(unsigned char *uart_buf,unsigned char i)
{
	if((uart_buf[i+4] == '0')||(uart_buf[i+4] == '1'))
	{
		set_SP_pulseWidth_unit_API(uart_buf[i+3]-'A',uart_buf[i+4]);
		return i+5;
	}
	return i;
}

unsigned char SPX0XXX_CMD(unsigned char *uart_buf,unsigned char i)
{
	unsigned char SP = 0;

	SP = (uart_buf[i+4]- '0')*100+(uart_buf[i+5]- '0')*10+(uart_buf[i+6]- '0');
	if(SP <= 999)
	{
		set_SP_pulseWidth_API(uart_buf[i+2]-'A',SP);
		return i+7;
	}
	return i;
}

unsigned char TR_CMD(unsigned char *uart_buf,unsigned char i)
{

	read_trigger_mode_API();
	return i+2;
}

unsigned char TRX_CMD(unsigned char *uart_buf,unsigned char i)
{

	set_trigger_mode_API(uart_buf[i+2]-'0');
	return i+3;
}

unsigned char TX_CMD(unsigned char *uart_buf,unsigned char i)
{

	set_horl_mode_API(uart_buf[i+1]);
	return i+2;
}
unsigned char T_CMD(unsigned char *uart_buf,unsigned char i)
{

	read_horl_mode_API();
	return i+1;
}
unsigned char TPX_CMD(unsigned char *uart_buf,unsigned char i)
{

	set_triggerpower_API(uart_buf[i+1]);
	return i+3;
}
unsigned char TP_CMD(unsigned char *uart_buf,unsigned char i)
{

	read_triggerpower_API();
	return i+2;
}

unsigned char DLX0XXX_CMD(unsigned char *uart_buf,unsigned char i)
{
	unsigned char light_delay = 0;

	if((uart_buf[i+2] >= 'A')&&(uart_buf[i+2] <= MAX_CHANNEL_LETTER)&&(uart_buf[i+3] == '0'))
	{
		light_delay = (uart_buf[i+4]- '0')*100+(uart_buf[i+5]- '0')*10+(uart_buf[i+6]- '0');
		if(light_delay <= 999)
		{
			set_light_delay_API(uart_buf[i+2]-'A',light_delay);
			return i+7;
		}
	}
	return i;
}

unsigned char DCX0XXX_CMD(unsigned char *uart_buf,unsigned char i)
{
	unsigned char camera_delay = 0;

	if((uart_buf[i+2] >= 'A')&&(uart_buf[i+2] <= MAX_CHANNEL_LETTER)&&(uart_buf[i+3] == '0'))
	{
		camera_delay = (uart_buf[i+4]- '0')*100+(uart_buf[i+5]- '0')*10+(uart_buf[i+6]- '0');
		if(camera_delay <= 999)
		{
			set_camera_delay_API(uart_buf[i+2]-'A',camera_delay);
			return i+7;
		}
	}
	return i;
}

unsigned char DC0XXX_CMD(unsigned char *uart_buf,unsigned char i)
{
	unsigned char camera_delay = 0;

	if(uart_buf[i+2] == '0')
	{
		camera_delay = (uart_buf[i+3]- '0')*100+(uart_buf[i+4]- '0')*10+(uart_buf[i+5]- '0');
		if(camera_delay <= 999)
		{
			set_AllCamera_delay_API(camera_delay);
			return i+6;
		}
	}
	return i;
}

unsigned char DLX_CMD(unsigned char *uart_buf,unsigned char i)
{
	if((uart_buf[i+2] >= 'A')&&(uart_buf[i+2] <= MAX_CHANNEL_LETTER))
	{
		read_light_delay_API(uart_buf[i+2]-'A');
		return i+3;
	}
	return i;
}

unsigned char DCX_CMD(unsigned char *uart_buf,unsigned char i)
{
	if((uart_buf[i+2] >= 'A')&&(uart_buf[i+2] <= MAX_CHANNEL_LETTER))
	{
		read_camera_delay_API(uart_buf[i+2]-'A');
		return i+3;
	}
	return i;
}

unsigned char DC_CMD(unsigned char *uart_buf,unsigned char i)
{
	read_AllCamera_delay_API();
	return i+2;
}

unsigned char CTX_CMD(unsigned char *uart_buf,unsigned char i)
{
	if((uart_buf[i+2] == '0')||(uart_buf[i+2] == '1'))
	{
		set_trigger_UpRoDpwm_API(uart_buf[i+2]);
		return i+3;
	}
	return i;
}

unsigned char CT_CMD(unsigned char *uart_buf,unsigned char i)
{
	read_trigger_UpRoDpwm_API();
	return i+2;
}



unsigned char S_cmdhandle(unsigned char *uart_buf,unsigned char i)
{
	if(uart_buf[i+6] == '#')
	{
		if(uart_buf[i+1] == 'T')
		{
			#if (ST0XXX_ENABLE == 1)
				return ST0XXX_CMD(uart_buf,i);
			#endif
		}
		if(uart_buf[i+1] == 'W')
		{
			if(uart_buf[i+2] == 'T'){
				#if (SWTRIG_ENABLE == 1)
					return SWTRIG_CMD(uart_buf,i);
				#endif
			}
			if((uart_buf[i+2] >= '0')&&(uart_buf[i+2] <= '9')){
				#if (SWXXXX_ENABLE == 1)
					return SWXXXX_CMD(uart_buf,i);
				#endif
			}
		}
		if((uart_buf[i+1] >= 'A')&&(uart_buf[i+1] <= MAX_CHANNEL_LETTER))
		{
			#if (SX0XXX_ENABLE == 1)
				return SX0XXX_CMD(uart_buf,i);
			#endif
		}		
	}
	if(uart_buf[i+2] == '#')
	{
		if(uart_buf[i+1] == 'T')
		{
			#if (ST0XXX_ENABLE == 1)
				return ST_CMD(uart_buf,i);
			#endif
		}
		if(uart_buf[i+1] == 'W')
		{
			#if (SWXXXX_ENABLE == 1)
				return SW_CMD(uart_buf,i);
			#endif
		}
		if((uart_buf[i+1] >= 'A')&&(uart_buf[i+1] <= MAX_CHANNEL_LETTER))
		{
			#if (SX0XXX_ENABLE == 1)
				return SX_CMD(uart_buf,i);
			#endif
		}		
	}
	if(uart_buf[i+4] == '#')
	{
		if(uart_buf[i+1] == 'A')
		{
			#if (SAVE_ENABLE == 1)
				return SAVE_CMD(uart_buf,i);
			#endif
		}
		if((uart_buf[i+1] == 'P')&&(uart_buf[i+2] == 'U')&&(uart_buf[i+3] >= 'A')&&(uart_buf[i+3] <= MAX_CHANNEL_LETTER))
		{
			#if (SPUXX_ENABLE == 1)
			return SPUX_CMD(uart_buf,i);
			#endif
		}
		if((uart_buf[i+1] >= '1')&&(uart_buf[i+1] <= '8'))
		{
			return SXXX_CMD(uart_buf,i);
		}		
	}
	if(uart_buf[i+3] == '#')
	{
		if((uart_buf[i+1] == 'P')&&(uart_buf[i+2] >= 'A')&&(uart_buf[i+2] <= MAX_CHANNEL_LETTER))
		{
			#if (SPX0XXX_ENABLE == 1)
			return SPX_CMD(uart_buf,i);
			#endif
		}		
	}
	if(uart_buf[i+5] == '#')
	{
		if((uart_buf[i+1] == 'P')&&(uart_buf[i+2] == 'U')&&(uart_buf[i+3] >= 'A')&&(uart_buf[i+3] <= MAX_CHANNEL_LETTER))
		{
			#if (SPUXX_ENABLE == 1)
			return SPUXX_CMD(uart_buf,i);
			#endif
		}		
	}
	if(uart_buf[i+7] == '#')
	{
		if((uart_buf[i+1] == 'P')&&(uart_buf[i+3] == '0')&&(uart_buf[i+2] >= 'A')&&(uart_buf[i+2] <= MAX_CHANNEL_LETTER))
		{
			#if (SPX0XXX_ENABLE == 1)
			return SPX0XXX_CMD(uart_buf,i);
			#endif
		}		
	}
	return i;
}

unsigned char T_cmdhandle(unsigned char *uart_buf,unsigned char i)
{
	#if (TRX_ENABLE == 1)
	if((uart_buf[i+1] == 'R')&&(uart_buf[i+2] == '#'))    
	{
		return TR_CMD(uart_buf,i);
	}
	if((uart_buf[i+1] == 'R')&&(uart_buf[i+2] >= '0')&&(uart_buf[i+2] <= '4')&&(uart_buf[i+3] == '#'))    
	{
		return TRX_CMD(uart_buf,i);
	}
	#endif
	#if (TX_ENABLE == 1)
	if(((uart_buf[i+1] == 'L')||(uart_buf[i+1] == 'H'))&&(uart_buf[i+2] == '#'))    
	{
		return TX_CMD(uart_buf,i);
	}
	if(uart_buf[i+1] == '#')   
	{
		return T_CMD(uart_buf,i);
	}
	#endif
	#if (TPX_ENABLE == 1)
	if((uart_buf[i+1] == 'P')&&(uart_buf[i+2] >= '0')&&(uart_buf[i+2] <= '1')&&(uart_buf[i+3] == '#'))    
	{
		return TPX_CMD(uart_buf,i);
	}
	if((uart_buf[i+1] == 'P')&&(uart_buf[i+2] == '#'))    
	{
		return TP_CMD(uart_buf,i);
	}
	#endif
	return i;
}

unsigned char P_cmdhandle(unsigned char *uart_buf,unsigned char i)
{
	#if (PRCL_ENABLE == 1)
	if((uart_buf[i+1] == 'R')&&(uart_buf[i+2] == 'C')&&(uart_buf[i+3] == 'L')&&(uart_buf[i+4] == '#'))    
	{
		return TPX_CMD(uart_buf,i);
	}
	#endif
	#if (PRWX_ENABLE == 1)
	if((uart_buf[i+1] == 'R')&&(uart_buf[i+2] == 'W')&&(uart_buf[i+3] >= 'A')&&(uart_buf[i+3] <= MAX_CHANNEL_LETTER)&&(uart_buf[i+4] == '#'))    
	{
		return TPX_CMD(uart_buf,i);
	}
	#endif
	#if (PRNXXX_ENABLE == 1)
	if((uart_buf[i+1] == 'R')&&(uart_buf[i+2] == 'N')&&(uart_buf[i+3] >= 'A')&&(uart_buf[i+3] <= MAX_CHANNEL_LETTER)&&(uart_buf[i+6] == '#'))    
	{
		return TPX_CMD(uart_buf,i);
	}
	if((uart_buf[i+1] == 'R')&&(uart_buf[i+2] == 'N')&&(uart_buf[i+3] >= 'A')&&(uart_buf[i+3] <= MAX_CHANNEL_LETTER)&&(uart_buf[i+4] == '#'))    
	{
		return TPX_CMD(uart_buf,i);
	}
	#endif
	#if (PRENCLRX_ENABLE == 1)
	if((uart_buf[i+1] == 'R')&&(uart_buf[i+2] == 'E')&&(uart_buf[i+3] == 'N')&&(uart_buf[i+4] == 'C')
		&&(uart_buf[i+5] == 'L')&&(uart_buf[i+6] == 'R')&&(uart_buf[i+7] >= 'A')&&(uart_buf[i+7] <= MAX_CHANNEL_LETTER)&&(uart_buf[i+8] == '#'))    
	{
		return TPX_CMD(uart_buf,i);
	}
	#endif
	return i;
}

unsigned char D_cmdhandle(unsigned char *uart_buf,unsigned char i)
{
	#if (DLX0XXX_ENABLE == 1)
	if((uart_buf[i+1] == 'L')&&(uart_buf[i+7] == '#'))   
	{
		return DLX0XXX_CMD(uart_buf,i);
	}
	if((uart_buf[i+1] == 'L')&&(uart_buf[i+3] == '#'))   
	{
		return DLX_CMD(uart_buf,i);
	}
	#endif
	#if (DCX0XXX_ENABLE == 1)
	if((uart_buf[i+1] == 'C')&&(uart_buf[i+7] == '#'))   
	{
		return DCX0XXX_CMD(uart_buf,i);
	}
	if((uart_buf[i+1] == 'C')&&(uart_buf[i+3] == '#'))   
	{
		return DCX_CMD(uart_buf,i);
	}
	#endif
	#if (DC0XXX_ENABLE == 1)
	if((uart_buf[i+1] == 'C')&&(uart_buf[i+6] == '#'))   
	{
		return DC0XXX_CMD(uart_buf,i);
	}
	if((uart_buf[i+1] == 'C')&&(uart_buf[i+2] == '#'))   
	{
		return DC_CMD(uart_buf,i);
	}
	#endif
	return i;
}

unsigned char C_cmdhandle(unsigned char *uart_buf,unsigned char i)
{
	#if (CTX_ENABLE == 1)
	if((uart_buf[i+1] == 'T')&&(uart_buf[i+3] == '#'))   
	{
		return CTX_CMD(uart_buf,i);
	}
	if((uart_buf[i+1] == 'T')&&(uart_buf[i+2] == '#'))   
	{
		return CT_CMD(uart_buf,i);
	}
	#endif
	return i;
}


unsigned char cmd_handle_func(unsigned char *uart_buf,unsigned char i)
{

	switch (uart_buf[i])
	{
		case 'S':
			i = S_cmdhandle(uart_buf,i);
			break;
		case 'T':
			i = T_cmdhandle(uart_buf,i);
			break;
		case 'P':
			i = P_cmdhandle(uart_buf,i);
			break;
		case 'D':
			i = D_cmdhandle(uart_buf,i);
			break;
		case 'C':
			i = C_cmdhandle(uart_buf,i);
			break;
		case 'H':
			break;
		case 'N':
			break;
	}
	return i;
}

unsigned char* get_prepare_tx_buffer(unsigned char *length)
{
	*length = prepare_tx_length;

	return prepare_tx_buffer;
}

void analysis_command(void)
{
	unsigned char i = 0;
	unsigned char length = SUM_SIZE - prepare_data.available_length;

	if(length == 0){
		return;
	}
	prepare_tx_length = 0;
	prepare_data.state = BUSY;
	for(i = 0; i < length; i++)
	{
		i = cmd_handle_func(prepare_data.rx_buffer, i);;
	}

	 
	for(i = 0; i < length; i++)
	{
		prepare_data.rx_buffer[i] = 0;
	}
	prepare_data.state = FREE;
	prepare_data.data_end_position = 0;
	prepare_data.available_length  = SUM_SIZE;
}



