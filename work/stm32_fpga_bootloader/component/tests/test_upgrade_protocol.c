#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "upgrade_protocol.h"

static void test_boot_markers(void)
{
    assert(upgrade_mode_from_marker(0x0000U) == UPGRADE_MODE_APP);
    assert(upgrade_mode_from_marker(0x00AAU) == UPGRADE_MODE_MCU);
    assert(upgrade_mode_from_marker(0x0055U) == UPGRADE_MODE_FPGA);
}

static void test_modbus_crc(void)
{
    const uint8_t data[] = "123456789";
    assert(upgrade_modbus_crc16(data, sizeof(data) - 1U) == 0x4B37U);
}

static void test_fpga_frame_crc(void)
{
    uint8_t frame[] = {0xFAU, 0x68U, 0xAAU, 0x08U, 0x00U, 0x00U, 0x00U};
    uint16_t crc = upgrade_modbus_crc16(frame, 5U);

    frame[5] = (uint8_t)(crc >> 8);
    frame[6] = (uint8_t)crc;
    assert(upgrade_modbus_frame_valid(frame, sizeof(frame)));

    frame[4] ^= 0x01U;
    assert(!upgrade_modbus_frame_valid(frame, sizeof(frame)));
}

static void test_mcu_payload_xor(void)
{
    const uint8_t payload[] = {0x12U, 0x34U, 0x56U, 0x78U};
    assert(upgrade_xor8(payload, sizeof(payload)) == 0x08U);
}

int main(void)
{
    test_boot_markers();
    test_modbus_crc();
    test_fpga_frame_crc();
    test_mcu_payload_xor();
    puts("upgrade protocol tests passed");
    return 0;
}
