#include "upgrade_internal.h"

#include <string.h>

#define UPGRADE_MARKER_MCU  0x00AAU
#define UPGRADE_MARKER_FPGA 0x0055U

upgrade_context_t g_upgrade;

static uint32_t upgrade_flash_end(const upgrade_config_t *config)
{
    return (config->app_end_address != 0U) ? config->app_end_address : (FLASH_BANK1_END + 1U);
}

static bool upgrade_flash_erase_page(uint32_t address)
{
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t page_error = 0U;
    HAL_StatusTypeDef result;

    if ((address % FLASH_PAGE_SIZE) != 0U)
    {
        return false;
    }

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = address;
    erase.NbPages = 1U;

    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return false;
    }

    result = HAL_FLASHEx_Erase(&erase, &page_error);
    (void)HAL_FLASH_Lock();
    return result == HAL_OK;
}

static bool upgrade_flash_program(uint32_t address, const uint8_t *data, uint16_t length)
{
    uint16_t offset;

    if (((address & 1U) != 0U) || ((length & 1U) != 0U))
    {
        return false;
    }

    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return false;
    }

    for (offset = 0U; offset < length; offset += 2U)
    {
        uint16_t value = (uint16_t)((uint16_t)data[offset] | ((uint16_t)data[offset + 1U] << 8U));
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address + offset, value) != HAL_OK)
        {
            (void)HAL_FLASH_Lock();
            return false;
        }
    }

    (void)HAL_FLASH_Lock();
    return true;
}

static bool upgrade_marker_write(const upgrade_config_t *config, uint16_t marker)
{
    uint8_t value[2];

    if ((config == 0) || (config->metadata_address >= config->app_address) ||
        ((config->metadata_address % FLASH_PAGE_SIZE) != 0U) ||
        ((config->metadata_address + FLASH_PAGE_SIZE) > config->app_address))
    {
        return false;
    }

    value[0] = (uint8_t)marker;
    value[1] = (uint8_t)(marker >> 8U);
    return upgrade_flash_erase_page(config->metadata_address) &&
           upgrade_flash_program(config->metadata_address, value, sizeof(value));
}

static void upgrade_send(const uint8_t *data, uint16_t length)
{
    if (g_upgrade.config.transmit != 0)
    {
        g_upgrade.config.transmit(data, length, g_upgrade.config.transmit_user);
    }
}

static void upgrade_send_mcu_ack(void)
{
    static const uint8_t response[] = {0x72U, 0x68U, 0x01U, 0x16U};
    upgrade_send(response, sizeof(response));
}

static void upgrade_send_mcu_ready(void)
{
    static const uint8_t response[] = {0x72U, 0x68U, 0x02U, 0x16U};
    upgrade_send(response, sizeof(response));
}

static void upgrade_send_mcu_packet(uint16_t packet)
{
    uint8_t response[] = {0x72U, 0x69U, (uint8_t)(packet >> 8U), (uint8_t)packet, 0x16U};
    upgrade_send(response, sizeof(response));
}

static void upgrade_send_mcu_error(void)
{
    static const uint8_t response[] = {0x72U, 0x68U, 0x03U, 0x16U};
    upgrade_send(response, sizeof(response));
}

static void upgrade_send_fpga_reply(uint8_t value)
{
    upgrade_send(&value, 1U);
}

static void upgrade_send_fpga_status(void)
{
    uint8_t response[] = {0xFBU, 0x00U, 0xFBU};

    if (g_upgrade.status == UPGRADE_STATUS_BUSY)
    {
        response[1] = 0x01U;
    }
    else if (g_upgrade.status == UPGRADE_STATUS_DONE)
    {
        response[1] = 0x02U;
        g_upgrade.status = UPGRADE_STATUS_WAITING;
    }
    else if (g_upgrade.status == UPGRADE_STATUS_ERROR)
    {
        response[1] = 0x03U;
    }

    upgrade_send(response, sizeof(response));
}

static void upgrade_send_fpga_crc_error(void)
{
    uint8_t response[] = {0xFAU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0U, 0U};
    uint16_t crc = upgrade_modbus_crc16(response, 5U);

    response[5] = (uint8_t)(crc >> 8U);
    response[6] = (uint8_t)crc;
    upgrade_send(response, sizeof(response));
}

static bool upgrade_mcu_write_packet(uint16_t packet, const uint8_t *payload)
{
    uint32_t offset = ((uint32_t)packet - 1U) * 256U;
    uint32_t address = g_upgrade.config.app_address + offset;
    uint32_t end = upgrade_flash_end(&g_upgrade.config);

    if ((packet == 0U) || (address < g_upgrade.config.app_address) ||
        ((address + 256U) > end))
    {
        return false;
    }

    if ((offset % FLASH_PAGE_SIZE) == 0U && !upgrade_flash_erase_page(address))
    {
        return false;
    }

    return upgrade_flash_program(address, payload, 256U);
}

static void upgrade_handle_mcu(void)
{
    const uint8_t *frame = g_upgrade.frame;
    uint16_t length = g_upgrade.frame_length;
    uint16_t packet;

    if ((length == 5U) && (frame[0] == 0x72U) && (frame[1] == 0x68U) && (frame[4] == 0x16U))
    {
        g_upgrade.mcu_total_packets = (uint16_t)(((uint16_t)frame[2] << 8U) | frame[3]);
        g_upgrade.mcu_next_packet = 1U;
        g_upgrade.mcu_active = g_upgrade.mcu_total_packets != 0U;
        upgrade_send_mcu_ready();
        return;
    }

    if ((length >= 6U) && (frame[0] == 0x72U) && (frame[1] == 0x68U) &&
        (frame[2] == 0xAAU) && (frame[length - 1U] == 0x16U))
    {
        g_upgrade.mcu_active = false;
        upgrade_send_mcu_ack();
        return;
    }

    if ((length != 262U) || !g_upgrade.mcu_active || (frame[0] != 0x72U) ||
        (frame[1] != 0x69U) || (frame[261] != 0x16U))
    {
        upgrade_send_mcu_error();
        return;
    }

    packet = (uint16_t)(((uint16_t)frame[2] << 8U) | frame[3]);
    if ((packet != g_upgrade.mcu_next_packet) || (packet > g_upgrade.mcu_total_packets) ||
        (upgrade_xor8(&frame[4], 256U) != frame[260]) || !upgrade_mcu_write_packet(packet, &frame[4]))
    {
        g_upgrade.status = UPGRADE_STATUS_ERROR;
        upgrade_send_mcu_error();
        return;
    }

    upgrade_send_mcu_packet(packet);
    g_upgrade.mcu_next_packet++;
    if (packet == g_upgrade.mcu_total_packets)
    {
        g_upgrade.mcu_active = false;
        if (!upgrade_app_valid(&g_upgrade.config) || !upgrade_marker_write(&g_upgrade.config, 0U))
        {
            g_upgrade.status = UPGRADE_STATUS_ERROR;
            return;
        }

        g_upgrade.status = UPGRADE_STATUS_DONE;
        HAL_Delay(1U);
        NVIC_SystemReset();
    }
}

static void upgrade_handle_fpga(void)
{
    const uint8_t *frame = g_upgrade.frame;
    uint16_t length = g_upgrade.frame_length;
    uint16_t block_size;
    uint32_t total_blocks;
    uint32_t block_number;

    if ((length == 3U) && (frame[0] == 0xFBU) && (frame[1] == 0xFFU) && (frame[2] == 0xFBU))
    {
        upgrade_send_fpga_status();
        return;
    }

    if (!upgrade_modbus_frame_valid(frame, length) || (frame[0] != 0xFAU))
    {
        upgrade_send_fpga_crc_error();
        return;
    }

    if ((length == 7U) && (frame[1] == 0x68U) && (frame[2] == 0xAAU))
    {
        block_size = (uint16_t)(((uint16_t)frame[3] << 8U) | frame[4]);
        if ((block_size < 256U) || (block_size > 2048U) || ((block_size % 256U) != 0U))
        {
            upgrade_send_fpga_reply(0x65U);
            return;
        }

        g_upgrade.fpga_block_size = block_size;
        upgrade_send_fpga_reply(0x73U);
        return;
    }

    if ((length == 7U) && (frame[1] == 0x00U) && (g_upgrade.fpga_block_size != 0U))
    {
        total_blocks = ((uint32_t)frame[2] << 16U) | ((uint32_t)frame[3] << 8U) | frame[4];
        if (total_blocks == 0U)
        {
            upgrade_send_fpga_reply(0x65U);
            return;
        }

        g_upgrade.fpga_total_blocks = total_blocks;
        g_upgrade.fpga_next_block = 1U;
        upgrade_send_fpga_reply(0x73U);
        g_upgrade.status = UPGRADE_STATUS_BUSY;
        if (!upgrade_gowin_begin())
        {
            g_upgrade.status = UPGRADE_STATUS_ERROR;
        }
        else
        {
            g_upgrade.status = UPGRADE_STATUS_WAITING;
        }
        return;
    }

    if ((g_upgrade.fpga_block_size == 0U) || (g_upgrade.fpga_total_blocks == 0U) ||
        (length != (uint16_t)(7U + g_upgrade.fpga_block_size)))
    {
        upgrade_send_fpga_reply(0x65U);
        return;
    }

    block_number = ((uint32_t)frame[1] << 24U) | ((uint32_t)frame[2] << 16U) |
                   ((uint32_t)frame[3] << 8U) | frame[4];
    if ((block_number != g_upgrade.fpga_next_block) || (block_number > g_upgrade.fpga_total_blocks))
    {
        upgrade_send_fpga_reply(0x65U);
        return;
    }

    g_upgrade.status = UPGRADE_STATUS_BUSY;
    if (!upgrade_gowin_program_block(block_number, &frame[5], g_upgrade.fpga_block_size,
                                     g_upgrade.fpga_total_blocks))
    {
        g_upgrade.status = UPGRADE_STATUS_ERROR;
        upgrade_send_fpga_reply(0x65U);
        return;
    }

    upgrade_send_fpga_reply(0x73U);
    g_upgrade.fpga_next_block++;
    g_upgrade.status = UPGRADE_STATUS_WAITING;
    if (block_number == g_upgrade.fpga_total_blocks)
    {
        if (!upgrade_gowin_finish() || !upgrade_marker_write(&g_upgrade.config, 0U))
        {
            g_upgrade.status = UPGRADE_STATUS_ERROR;
            return;
        }

        g_upgrade.status = UPGRADE_STATUS_DONE;
        HAL_Delay(10U);
        NVIC_SystemReset();
    }
}

upgrade_mode_t upgrade_boot_decide(const upgrade_config_t *config)
{
    if (config == 0)
    {
        return UPGRADE_MODE_APP;
    }

    return upgrade_mode_from_marker(*((const uint16_t *)config->metadata_address));
}

bool upgrade_app_valid(const upgrade_config_t *config)
{
    uint32_t stack_pointer;

    if (config == 0)
    {
        return false;
    }

    stack_pointer = *((const uint32_t *)config->app_address);
    return (stack_pointer & 0x2FFE0000U) == 0x20000000U;
}

void upgrade_jump_to_app(const upgrade_config_t *config)
{
    void (*reset_handler)(void);

    if (!upgrade_app_valid(config))
    {
        return;
    }

    reset_handler = (void (*)(void))*((const uint32_t *)(config->app_address + 4U));
    __disable_irq();
    SysTick->CTRL = 0U;
    SCB->VTOR = config->app_address;
    __DSB();
    __ISB();
    __set_MSP(*((const uint32_t *)config->app_address));
    reset_handler();
}

bool upgrade_init(const upgrade_config_t *config, upgrade_mode_t mode)
{
    if ((config == 0) || (config->transmit == 0) || (mode == UPGRADE_MODE_APP) ||
        (config->app_address <= config->metadata_address) ||
        ((config->app_address % FLASH_PAGE_SIZE) != 0U) ||
        ((config->metadata_address % FLASH_PAGE_SIZE) != 0U) ||
        ((config->metadata_address + FLASH_PAGE_SIZE) > config->app_address))
    {
        return false;
    }

    memset(&g_upgrade, 0, sizeof(g_upgrade));
    g_upgrade.config = *config;
    g_upgrade.mode = mode;
    g_upgrade.status = UPGRADE_STATUS_WAITING;
    return (mode != UPGRADE_MODE_FPGA) || upgrade_gowin_init();
}

bool upgrade_rx_feed(const uint8_t *data, uint16_t length)
{
    if ((data == 0) || (length == 0U) || (length > UPGRADE_MAX_FRAME_SIZE) ||
        (g_upgrade.mode == UPGRADE_MODE_APP) || g_upgrade.frame_ready)
    {
        return false;
    }

    memcpy(g_upgrade.frame, data, length);
    g_upgrade.frame_length = length;
    g_upgrade.frame_ready = true;
    return true;
}

void upgrade_process(void)
{
    if (!g_upgrade.frame_ready)
    {
        return;
    }

    g_upgrade.frame_ready = false;
    if (g_upgrade.mode == UPGRADE_MODE_MCU)
    {
        upgrade_handle_mcu();
    }
    else if (g_upgrade.mode == UPGRADE_MODE_FPGA)
    {
        upgrade_handle_fpga();
    }
}

upgrade_status_t upgrade_get_status(void)
{
    return g_upgrade.status;
}

bool upgrade_request(const upgrade_config_t *config, upgrade_mode_t mode)
{
    uint16_t marker;

    if (mode == UPGRADE_MODE_MCU)
    {
        marker = UPGRADE_MARKER_MCU;
    }
    else if (mode == UPGRADE_MODE_FPGA)
    {
        marker = UPGRADE_MARKER_FPGA;
    }
    else
    {
        return false;
    }

    if (!upgrade_marker_write(config, marker))
    {
        return false;
    }

    NVIC_SystemReset();
    return true;
}
