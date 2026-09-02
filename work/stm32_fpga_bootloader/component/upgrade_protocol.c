#include "upgrade_protocol.h"

upgrade_mode_t upgrade_mode_from_marker(uint16_t marker)
{
    if (marker == 0x00AAU)
    {
        return UPGRADE_MODE_MCU;
    }

    if (marker == 0x0055U)
    {
        return UPGRADE_MODE_FPGA;
    }

    return UPGRADE_MODE_APP;
}

uint16_t upgrade_modbus_crc16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFFU;
    size_t i;
    uint8_t bit;

    for (i = 0U; i < length; ++i)
    {
        crc ^= data[i];
        for (bit = 0U; bit < 8U; ++bit)
        {
            crc = (crc & 1U) ? (uint16_t)((crc >> 1U) ^ 0xA001U) : (uint16_t)(crc >> 1U);
        }
    }

    return crc;
}

bool upgrade_modbus_frame_valid(const uint8_t *frame, size_t length)
{
    uint16_t expected;
    uint16_t actual;

    if ((frame == 0) || (length < 3U))
    {
        return false;
    }

    expected = upgrade_modbus_crc16(frame, length - 2U);
    actual = (uint16_t)(((uint16_t)frame[length - 2U] << 8U) | frame[length - 1U]);
    return expected == actual;
}

uint8_t upgrade_xor8(const uint8_t *data, size_t length)
{
    uint8_t value = 0U;
    size_t i;

    for (i = 0U; i < length; ++i)
    {
        value ^= data[i];
    }

    return value;
}
