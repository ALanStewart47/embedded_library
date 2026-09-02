#include "main.h"
#include "W25Q128.h"
#include "gpio_spi.h"
#include "sys.h"
// W25Q128 命令定义
#define W25Q128_WRITE_ENABLE    0x06//0x06  // 写使能命令: 允许后续的数据写入或擦除
#define W25Q128_READ_DATA       0x03//0x13  // 读取数据命令: 从指定地址读取数据
#define W25Q128_WRITE_DATA      0x02//0x12  // 写数据命令: 向指定地址写入数据
#define W25Q128_CHIP_ERASE      0xC7//0xC7  // 全片擦除命令: 擦除整个闪存芯片的内容
#define W25Q128_READ_STATUS_REG 0x05//0x05  // 读取状态寄存器命令: 检查设备状态（如写入进行中）
#define W25Q128_QUICK_READ      0x0B//0x0B  // 快速读取命令: 快速读取指定地址的数据
#define W25Q128_SECTOR_ERASE    0x20//0x21  // 扇区擦除命令: 擦除指定的 4KB 扇区
#define W25Q128_BLOCK_ERASE_32K 0x52//0x52  // 32KB 块擦除命令: 擦除指定的 32KB 块
#define W25Q128_BLOCK_ERASE_64K 0xD8//0xDc  // 64KB 块擦除命令: 擦除指定的 64KB 块

//3字节地址和4字节地址的区别
//void W25Q128_WaitForWriteEnd();
//extern SPI_HandleTypeDef hspi3; // SPI 句柄，用于 SPI 通信
#define CS_HIGH()       PAout(0)=1 //K15
#define CS_LOW()        PAout(0)=0
// SPI Flash CS 引脚控制
void W25Q128_CS_Low(void) {
    CS_LOW(); // 将 CS 引脚拉低，开始 SPI 通信
}

void W25Q128_CS_High(void) {
    CS_HIGH(); // 将 CS 引脚拉高，结束 SPI 通信
}

// 写使能
void W25Q128_Write_Enable(void) {
    uint8_t cmd = W25Q128_WRITE_ENABLE; // 写使能命令
    W25Q128_CS_Low(); // 拉低 CS 引脚
    SPI_SendData(&cmd, 1); // 发送写使能命令
    W25Q128_CS_High(); // 拉高 CS 引脚
}

// 读取状态寄存器函数
uint8_t W25Q128_Read_Status(void) {
    uint8_t cmd = W25Q128_READ_STATUS_REG; // 读取状态寄存器命令
    uint8_t status = 0; // 存储状态值
    W25Q128_CS_Low(); // 拉低 CS 引脚
    SPI_SendData(&cmd,1); // 发送读取状态命令
    SPI_ReceiveData(&status, 1); // 接收状态值
    W25Q128_CS_High(); // 拉高 CS 引脚
    return status; // 返回状态值
}

// 读取数据
void W25Q128_Read(uint32_t address, uint8_t *data, uint16_t size) {
    uint8_t cmd[5]; // 存储命令和地址
		uint8_t h=0;
    cmd[h++] = W25Q128_READ_DATA; // 读取数据命令
		//cmd[h++] = (address >> 24) & 0xFF; // 地址高字节
    cmd[h++] = (address >> 16) & 0xFF; // 地址高字节
    cmd[h++] = (address >> 8) & 0xFF; // 地址中字节
    cmd[h++] = address & 0xFF; // 地址低字节

    W25Q128_CS_Low(); // 拉低 CS 引脚
    SPI_SendData(cmd, h); // 发送读取命令和地址
    SPI_ReceiveData(data, size); // 接收数据
    W25Q128_CS_High(); // 拉高 CS 引脚
}

// 写数据
void W25Q128_Write(uint32_t address, uint8_t *data, uint16_t size) {
    // 使能写操作
   // W25Q128_Write_Enable();

    // 计算每次写入的字节数（最大为256字节）
    uint16_t remaining = size;
    uint16_t pageSize = 256; // 页大小
    uint16_t offset = 0; // 当前写入偏移量
		uint8_t h=0;
    while (remaining > 0) {
        // 计算本次写入的字节数
        uint16_t writeSize = (remaining > pageSize) ? pageSize : remaining;

        // 计算当前写入的地址和数据指针
        uint8_t cmd[5]; // 存储命令和地址
        cmd[h++] = W25Q128_WRITE_DATA; // 写数据命令
				//cmd[h++] = (address >> 24) & 0xFF; // 地址高字节
        cmd[h++] = (address >> 16) & 0xFF; // 地址高字节
        cmd[h++] = (address >> 8) & 0xFF; // 地址中字节
        cmd[h++] = address & 0xFF; // 地址低字节
				W25Q128_Write_Enable();
        // 拉低 CS 引脚
        W25Q128_CS_Low();
        // 发送写命令和地址
				SPI_SendData(cmd, h); // 发送读取命令和地址
				SPI_SendData(data + offset, writeSize); // 接收数据

        // 拉高 CS 引脚
        W25Q128_CS_High();
			
//				 W25Q128_CS_Low();
//				 uint8_t data_temp[200]={0};
//				 for(int i=0;i<90;i++)
//				 {
//						data_temp[i*2]=5;
//						data_temp[i*2+1]=0;
//				 }
//				 HAL_SPI_Transmit(&hspi3, data_temp, 180, HAL_MAX_DELAY);
//				 W25Q128_CS_High();
			
				W25Q128_WaitForWriteEnd();
        // 更新剩余字节数和偏移量
        remaining -= writeSize;
        offset += writeSize;
        address += writeSize; // 更新地址
    }
}

// 全片擦除
void W25Q128_Chip_Erase(void) {
    W25Q128_Write_Enable(); // 使能写操作
    uint8_t cmd = W25Q128_CHIP_ERASE; // 全片擦除命令

    W25Q128_CS_Low(); // 拉低 CS 引脚
    SPI_SendData(&cmd, 1); // 发送全片擦除命令
    W25Q128_CS_High(); // 拉高 CS 引脚

    // 等待擦除完成
    uint8_t status;
    uint32_t timeout = 100000; // 超时计数
    do {
        status = W25Q128_Read_Status(); // 读取状态寄存器
        timeout--; // 减少超时计数
        if (timeout == 0) {
            // 超时处理
            break; // 超时退出
        }
    } while (status & 0x01); // WIP位为1时继续等待
}

// 扇区擦除（4KB）
void W25Q128_Sector_Erase(uint32_t address) {
    W25Q128_Write_Enable(); // 使能写操作
		uint8_t h=0;
    uint8_t cmd[5]; // 存储命令和地址
    cmd[h++] = W25Q128_SECTOR_ERASE; // 扇区擦除命令
		//cmd[h++] = (address >> 24) & 0xFF; // 地址高字节
    cmd[h++] = (address >> 16) & 0xFF; // 地址高字节
    cmd[h++] = (address >> 8) & 0xFF; // 地址中字节
    cmd[h++] = address & 0xFF; // 地址低字节

    W25Q128_CS_Low(); // 拉低 CS 引脚
    SPI_SendData( cmd, h); // 发送扇区擦除命令和地址
    W25Q128_CS_High(); // 拉高 CS 引脚

    // 等待擦除完成
    uint8_t status;
    do {
        status = W25Q128_Read_Status(); // 读取状态寄存器
    } while (status & 0x01); // WIP位为1时继续等待
}

// 32KB 块擦除
void W25Q128_Block_Erase_32K(uint32_t address) {
    W25Q128_Write_Enable(); // 使能写操作
		uint8_t h=0;
    uint8_t cmd[4]; // 存储命令和地址
    cmd[h++] = W25Q128_BLOCK_ERASE_32K; // 32KB块擦除命令
    cmd[h++] = (address >> 16) & 0xFF; // 地址高字节
    cmd[h++] = (address >> 8) & 0xFF; // 地址中字节
    cmd[h++] = address & 0xFF; // 地址低字节

    W25Q128_CS_Low(); // 拉低 CS 引脚
    SPI_SendData(cmd, h); // 发送块擦除命令和地址
    W25Q128_CS_High(); // 拉高 CS 引脚

    // 等待擦除完成
    uint8_t status;
    do {
        status = W25Q128_Read_Status(); // 读取状态寄存器
    } while (status & 0x01); // WIP位为1时继续等待
}

// 64KB 块擦除
void W25Q128_Block_Erase_64K(uint32_t address) {
    W25Q128_Write_Enable(); // 使能写操作
		uint8_t h=0;
    uint8_t cmd[5]; // 存储命令和地址
    cmd[h++] = W25Q128_BLOCK_ERASE_64K; // 64KB块擦除命令
		//cmd[h++] = (address >> 24) & 0xFF; // 地址高字节
    cmd[h++] = (address >> 16) & 0xFF; // 地址高字节
    cmd[h++] = (address >> 8) & 0xFF; // 地址中字节
    cmd[h++] = address & 0xFF; // 地址低字节

    W25Q128_CS_Low(); // 拉低 CS 引脚
    SPI_SendData(cmd, h); // 发送块擦除命令和地址
    W25Q128_CS_High(); // 拉高 CS 引脚

    // 等待擦除完成
    uint8_t status;
    do {
        status = W25Q128_Read_Status(); // 读取状态寄存器
    } while (status & 0x01); // WIP位为1时继续等待
}


// 初始化
void W25Q128_Init(void) {
    HAL_Delay(10); // 等待上电稳定

    W25Q128_Write_Enable(); // 可选: 在初始化时发送写使能命令

    // 读取状态寄存器并确认设备准备就绪
    uint8_t status;
    do {
        status = W25Q128_Read_Status(); // 读取状态寄存器
    } while (status & 0x01); // WIP位为1时继续等待
}
void W25Q128_WaitForWriteEnd(void) {
    uint8_t status;
    do {
        // 读取状态寄存器
        status=W25Q128_Read_Status();
    } while (status & 0x01);  // WIP位为1表示写入中
}
