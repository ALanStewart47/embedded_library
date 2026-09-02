#ifndef __W25Q128_H__
#define __W25Q128_H__
#include "main.h"

void W25Q128_Read(uint32_t address, uint8_t *data, uint16_t size);
void W25Q128_Write(uint32_t address, uint8_t *data, uint16_t size);
void W25Q128_Chip_Erase(void);
// ÉÈÇø²Á³ý£¨4KB£©
void W25Q128_Sector_Erase(uint32_t address);
// 64KB ¿é²Á³ý
void W25Q128_Block_Erase_64K(uint32_t address);
void W25Q128_Init(void);
void W25Q128_WaitForWriteEnd(void);

#endif

