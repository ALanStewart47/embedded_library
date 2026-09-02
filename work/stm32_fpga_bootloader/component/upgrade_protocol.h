#ifndef STM32_FPGA_UPGRADE_PROTOCOL_H
#define STM32_FPGA_UPGRADE_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum
{
    UPGRADE_MODE_APP = 0,
    UPGRADE_MODE_MCU,
    UPGRADE_MODE_FPGA
} upgrade_mode_t;

upgrade_mode_t upgrade_mode_from_marker(uint16_t marker);
uint16_t upgrade_modbus_crc16(const uint8_t *data, size_t length);
bool upgrade_modbus_frame_valid(const uint8_t *frame, size_t length);
uint8_t upgrade_xor8(const uint8_t *data, size_t length);

#endif
