/******************************************************************************
 * @file fpga_update.c
 * @brief FPGA 升级: FA/FB 上位机协议解析 + 调度 JTAG 内部 Flash 编程。
 *        CMD_40_handle 移植自 GD32 参考 protocol_public.c, 协议字节级保持一致;
 *        回复改为复用 BootLoader 现有 uart.c 的 DMA 发送(DMA_Usart_Send)。
 *
 *  上位机协议(均 modbus_crc, crc_hi crc_lo 结尾):
 *    FA 68 AA <blk_hi blk_lo> crc  -> 设块大小(256~2048,256倍数), 回 0x73
 *    FA 00 <size24> crc            -> 设总块数, 回 0x73, up_cmd=1(擦除)
 *    FA <addr32> <block数据> crc   -> 收一块, 回 0x73, up_cmd=2(写块)
 *    FB FF FB / 任意(up_cmd!=0)     -> 状态查询: fb 01 fb(忙) / fb 00 fb(空闲) / fb 02 fb(完成)
 *****************************************************************************/
#include "fpga_update.h"
#include "jtag_gaoyun.h"
#include "uart.h"

uint32_t page_size = FLASH_PAGE_SIZE;
extern unsigned int   rx_length;
extern unsigned char  rx_buffer[];
extern unsigned char  rx_endFlag;
void DMA_Usart_Send(uint8_t *buf, uint8_t len);

spi_w spi_w_handle;

static unsigned char fpga_tx_buffer[16];
static unsigned char fpga_tx_length;

static void package(unsigned char *buf, unsigned char len)
{
    for (unsigned char i = 0; i < len; i++)
        fpga_tx_buffer[fpga_tx_length++] = buf[i];
}

uint16_t modbus_crc(uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF, i, j;
    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
            else              crc = crc >> 1;
        }
    }
    return crc;
}

static uint16_t crc_temp = 0;

void CMD_40_handle(unsigned char *uart_buf, unsigned int lenght)
{
    unsigned char tx_buf[10];
    uint8_t h = 0;

    if (lenght >= 3)
    {
        crc_temp = modbus_crc(uart_buf, lenght - 2);

        if ((uart_buf[0] == 0xFB && uart_buf[1] == 0xFf && uart_buf[0] == 0xFB) || spi_w_handle.up_cmd != 0)
        {
            if (spi_w_handle.up_cmd == 1 || spi_w_handle.up_cmd == 2)
            {
                tx_buf[h++] = 0xfb; tx_buf[h++] = 0x01; tx_buf[h++] = 0xfb;
                package(tx_buf, h);
            }
            else if (spi_w_handle.up_cmd == 0)
            {
                tx_buf[h++] = 0xfb; tx_buf[h++] = 0x00; tx_buf[h++] = 0xfb;
                package(tx_buf, h);
            }
            else if (spi_w_handle.up_cmd == 0xff)
            {
                tx_buf[h++] = 0xfb; tx_buf[h++] = 0x02; tx_buf[h++] = 0xfb;
                spi_w_handle.up_cmd = 0;
                package(tx_buf, h);
            }
        }
        else if (crc_temp == (uart_buf[lenght - 2] * 256 + uart_buf[lenght - 1]) && uart_buf[0] == 0xFA)
        {
            if (lenght == 7)
            {
                if (uart_buf[1] == 0x68 && uart_buf[2] == 0xAA)    
                {
                    uint16_t block_size_temp = (uint32_t)(uart_buf[3] << 8) + (uint32_t)uart_buf[4];
                    if (block_size_temp >= 256 && block_size_temp <= 2048 && block_size_temp % 256 == 0)
                    {
                        spi_w_handle.block_size_t = block_size_temp;
                        tx_buf[h++] = 0x73; package(tx_buf, h);
                    }
                    else { tx_buf[h++] = 0x65; package(tx_buf, h); }
                }
                else if (uart_buf[1] == 0x00)                      
                {
                    uint32_t size_temp = (uint32_t)(uart_buf[2] << 16) + (uint32_t)(uart_buf[3] << 8) + (uint32_t)uart_buf[4];
                    spi_w_handle.size_t = size_temp;
                    tx_buf[h++] = 0x73; package(tx_buf, h);
                    spi_w_handle.up_cmd = 1;
                }
            }
            else if (lenght == 7 + spi_w_handle.block_size_t)      
            {
                spi_w_handle.up_cmd = 2;
                spi_w_handle.addr = (uint32_t)(uart_buf[1] << 24) + (uint32_t)(uart_buf[2] << 16)
                                  + (uint32_t)(uart_buf[3] << 8)  + (uint32_t)uart_buf[4];
                for (int i = 0; i < spi_w_handle.block_size_t; i++)
                    spi_w_handle.spi_data[i] = uart_buf[5 + i];
                tx_buf[h++] = 0x73; package(tx_buf, h);
            }
        }
        else                                                       
        {
            tx_buf[h++] = 0xfa; tx_buf[h++] = 0xff; tx_buf[h++] = 0xff;
            tx_buf[h++] = 0xff; tx_buf[h++] = 0xff;
            crc_temp = modbus_crc(tx_buf, h);
            tx_buf[h++] = crc_temp >> 8; tx_buf[h++] = crc_temp;
            package(tx_buf, h);
        }
    }
}

void fpga_update_task(void)
{
    spi_w_handle.up_cmd = 0;
    spi_w_handle.block_size_t = 0;

    while (1)
    {
        if (rx_endFlag == 1)
        {
            fpga_tx_length = 0;
            CMD_40_handle(rx_buffer, rx_length);
            if (fpga_tx_length != 0)
                DMA_Usart_Send(fpga_tx_buffer, fpga_tx_length);

             

            rx_length  = 0;
            rx_endFlag = 0;
            UART_Receive_DMA();
        }

        if (spi_w_handle.up_cmd == 1)             
        {
            program_internal_flash(0);
            spi_w_handle.up_cmd = 0;
        }
        else if (spi_w_handle.up_cmd == 2)        
        {
            cnt_max = spi_w_handle.size_t;
            program_internal_flash(spi_w_handle.addr);
            spi_w_handle.up_cmd = 0;
        }
    }
}
