#include "gpio_spi.h"
#include "W25Q128.h"
#include "sys.h"

// 定义宏操作（高低电平控制）
#define SCK_HIGH()      PCout(13)=1 //G16
#define SCK_LOW()       PCout(13)=0 
#define MOSI_HIGH()     PAout(1)=1 //K16
#define MOSI_LOW()      PAout(1)=0 
#define CS_HIGH()       PAout(0)=1 //K15
#define CS_LOW()        PAout(0)=0
#define READ_MISO()     PBin(9)    //F16
#define H_SPI 0
#if(H_SPI==0) //是否硬件发送接收数据
// 发送和接收 1 个字节数据
uint8_t SPI_Transfer(uint8_t data) {
    uint8_t receivedData = 0;

    for (int i = 0; i < 8; i++) {
        // 根据数据高位设置 MOSI
        if (data & 0x80) {
            MOSI_HIGH();
        } else {
            MOSI_LOW();
        }

        // 产生时钟上升沿
        SCK_HIGH();

        // 读取 MISO 的值
        receivedData <<= 1;  // 左移，准备存储下一位
        if (READ_MISO()) {
            receivedData |= 0x01;  // 若 MISO 为高，则设置最低位为 1
        }

        // 产生时钟下降沿
        SCK_LOW();


        data <<= 1;  // 左移一位，准备发送下一位
    }

    return receivedData;
}

// 发送数据
void SPI_SendData(uint8_t* data, uint16_t size) {
   // CS_LOW();  // 选中从设备

    for (int i = 0; i < size; i++) {
        SPI_Transfer(data[i]);  // 逐字节发送数据
    }

   // CS_HIGH();  // 取消选中从设备
}

// 接收数据
void SPI_ReceiveData(uint8_t* buffer, uint16_t size) {
    //CS_LOW();  // 选中从设备

    for (int i = 0; i < size; i++) {
        buffer[i] = SPI_Transfer(0xFF);  // 发送空数据，同时接收从设备数据
    }

    //CS_HIGH();  // 取消选中从设备
}

#endif

#if(H_SPI==1) 
// 发送和接收 1 个字节数据
uint8_t SPI_Transfer(uint8_t data) {
    uint8_t receivedData = 0;

    for (int i = 0; i < 8; i++) {
        // 根据数据高位设置 MOSI
        if (data & 0x80) {
            MOSI_HIGH();
        } else {
            MOSI_LOW();
        }

        // 产生时钟上升沿
        SCK_HIGH();

        // 读取 MISO 的值
        receivedData <<= 1;  // 左移，准备存储下一位
        if (READ_MISO()) {
            receivedData |= 0x01;  // 若 MISO 为高，则设置最低位为 1
        }

        // 产生时钟下降沿
        SCK_LOW();


        data <<= 1;  // 左移一位，准备发送下一位
    }

    return receivedData;
}

// 发送数据
void SPI_SendData(uint8_t* data, uint16_t size) {
   // CS_LOW();  // 选中从设备

    for (int i = 0; i < size; i++) {
        SPI_Transfer(data[i]);  // 逐字节发送数据
    }

   // CS_HIGH();  // 取消选中从设备
}

// 接收数据
void SPI_ReceiveData(uint8_t* buffer, uint16_t size) {
    //CS_LOW();  // 选中从设备

    for (int i = 0; i < size; i++) {
        buffer[i] = SPI_Transfer(0xFF);  // 发送空数据，同时接收从设备数据
    }

    //CS_HIGH();  // 取消选中从设备
}

#endif

