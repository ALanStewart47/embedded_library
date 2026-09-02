#include "upgrade_internal.h"

#include <string.h>

typedef enum
{
    TAP_TEST_LOGIC_RESET,
    TAP_RUN_TEST_IDLE,
    TAP_SELECT_DR_SCAN,
    TAP_CAPTURE_DR,
    TAP_SHIFT_DR,
    TAP_EXIT1_DR,
    TAP_UPDATE_DR,
    TAP_SELECT_IR_SCAN,
    TAP_CAPTURE_IR,
    TAP_SHIFT_IR,
    TAP_UPDATE_IR
} gowin_tap_state_t;

static void gowin_pin_write(const upgrade_gpio_t *pin, bool high)
{
    if (high)
    {
        pin->port->BSRR = pin->pin;
    }
    else
    {
        pin->port->BSRR = (uint32_t)pin->pin << 16U;
    }
}

static bool gowin_pin_read(const upgrade_gpio_t *pin)
{
    return (pin->port->IDR & pin->pin) != 0U;
}

static void gowin_delay_us(uint32_t microseconds)
{
    volatile uint32_t count;
    volatile uint32_t delay;

    for (count = 0U; count < microseconds; ++count)
    {
        delay = 11U;
        while (delay != 0U)
        {
            --delay;
        }
    }
}

static void gowin_clock(uint32_t cycles, uint32_t nops)
{
    uint32_t i;
    uint32_t j;

    for (i = 0U; i < cycles; ++i)
    {
        gowin_pin_write(&g_upgrade.config.tck, true);
        for (j = 0U; j < nops; ++j)
        {
            __NOP();
        }
        gowin_pin_write(&g_upgrade.config.tck, false);
        for (j = 0U; j < nops; ++j)
        {
            __NOP();
        }
    }
}

static void gowin_tap_transition(gowin_tap_state_t state)
{
    uint32_t clocks = 1U;

    if ((state == TAP_TEST_LOGIC_RESET) || (state == TAP_RUN_TEST_IDLE))
    {
        clocks = 8U;
    }

    switch (state)
    {
        case TAP_TEST_LOGIC_RESET:
            gowin_pin_write(&g_upgrade.config.tms, true);
            break;
        case TAP_RUN_TEST_IDLE:
        case TAP_CAPTURE_DR:
        case TAP_SHIFT_DR:
        case TAP_CAPTURE_IR:
        case TAP_SHIFT_IR:
            gowin_pin_write(&g_upgrade.config.tms, false);
            break;
        default:
            gowin_pin_write(&g_upgrade.config.tms, true);
            break;
    }

    gowin_clock(clocks, 1U);
}

static void gowin_configure(uint8_t instruction)
{
    uint8_t bit;

    gowin_tap_transition(TAP_SELECT_DR_SCAN);
    gowin_tap_transition(TAP_SELECT_IR_SCAN);
    gowin_tap_transition(TAP_CAPTURE_IR);
    gowin_tap_transition(TAP_SHIFT_IR);
    for (bit = 0U; bit < 8U; ++bit)
    {
        if (bit == 7U)
        {
            gowin_pin_write(&g_upgrade.config.tms, true);
        }
        gowin_pin_write(&g_upgrade.config.tdi, ((instruction >> bit) & 1U) != 0U);
        gowin_clock(1U, 1U);
    }
    gowin_tap_transition(TAP_UPDATE_IR);
    gowin_tap_transition(TAP_RUN_TEST_IDLE);
    gowin_delay_us(100U);
}

static uint32_t gowin_read_data(uint8_t width)
{
    uint32_t value = 0U;
    uint8_t bit;

    gowin_tap_transition(TAP_SELECT_DR_SCAN);
    gowin_tap_transition(TAP_CAPTURE_DR);
    gowin_tap_transition(TAP_SHIFT_DR);
    for (bit = 0U; bit < width; ++bit)
    {
        if (bit == (uint8_t)(width - 1U))
        {
            gowin_pin_write(&g_upgrade.config.tms, true);
        }
        gowin_pin_write(&g_upgrade.config.tck, true);
        value = (value << 1U) | (gowin_pin_read(&g_upgrade.config.tdo) ? 1U : 0U);
        gowin_pin_write(&g_upgrade.config.tck, false);
    }
    gowin_tap_transition(TAP_UPDATE_DR);
    gowin_tap_transition(TAP_RUN_TEST_IDLE);
    return value;
}

static uint32_t gowin_read_device_id(void)
{
    gowin_configure(0x11U);
    return gowin_read_data(32U);
}

static uint32_t gowin_read_status(void)
{
    gowin_configure(0x41U);
    return gowin_read_data(32U);
}

static void gowin_shift_u32(uint32_t value)
{
    uint8_t bit;

    gowin_tap_transition(TAP_SELECT_DR_SCAN);
    gowin_tap_transition(TAP_CAPTURE_DR);
    gowin_tap_transition(TAP_SHIFT_DR);
    for (bit = 0U; bit < 32U; ++bit)
    {
        if (bit == 31U)
        {
            gowin_pin_write(&g_upgrade.config.tms, true);
        }
        gowin_pin_write(&g_upgrade.config.tdi, ((value >> bit) & 1U) != 0U);
        gowin_clock(1U, 1U);
    }
    gowin_tap_transition(TAP_UPDATE_DR);
    gowin_tap_transition(TAP_RUN_TEST_IDLE);
    gowin_delay_us(20U);
}

static void gowin_erase_sram(void)
{
    gowin_configure(0x15U);
    gowin_configure(0x05U);
    gowin_configure(0x02U);
    gowin_clock(100000U, 12U);
    gowin_delay_us(10000U);
    gowin_configure(0x09U);
    gowin_configure(0x02U);
    gowin_delay_us(10000U);
    gowin_configure(0x3AU);
    gowin_configure(0x02U);
    gowin_tap_transition(TAP_RUN_TEST_IDLE);
    gowin_clock(10000U, 12U);
}

static void gowin_erase_flash(void)
{
    uint8_t bit;

    gowin_configure(0x15U);
    gowin_configure(0x75U);
    gowin_tap_transition(TAP_RUN_TEST_IDLE);
    gowin_tap_transition(TAP_SELECT_DR_SCAN);
    gowin_tap_transition(TAP_CAPTURE_DR);
    gowin_tap_transition(TAP_SHIFT_DR);
    for (bit = 0U; bit < 32U; ++bit)
    {
        if (bit == 31U)
        {
            gowin_pin_write(&g_upgrade.config.tms, true);
        }
        gowin_pin_write(&g_upgrade.config.tdi, false);
        gowin_clock(1U, 1U);
    }
    gowin_tap_transition(TAP_UPDATE_DR);
    gowin_tap_transition(TAP_RUN_TEST_IDLE);
    gowin_clock(230000U, 12U);
    gowin_configure(0x3AU);
    gowin_configure(0x02U);
    gowin_configure(0x3CU);
    HAL_Delay(15U);
    (void)gowin_read_status();
    HAL_Delay(15U);
    (void)gowin_read_status();
}

static void gowin_program_page(uint32_t address, const uint8_t *data)
{
    uint8_t word;

    gowin_configure(0x15U);
    gowin_configure(0x71U);
    gowin_shift_u32(address);
    for (word = 0U; word < 64U; ++word)
    {
        uint32_t value = ((uint32_t)data[(uint16_t)word * 4U] << 24U) |
                         ((uint32_t)data[(uint16_t)word * 4U + 1U] << 16U) |
                         ((uint32_t)data[(uint16_t)word * 4U + 2U] << 8U) |
                         (uint32_t)data[(uint16_t)word * 4U + 3U];
        gowin_shift_u32(value);
    }
    gowin_clock(9400U, 1U);
}

bool upgrade_gowin_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    if ((g_upgrade.config.tms.port == 0) || (g_upgrade.config.tck.port == 0) ||
        (g_upgrade.config.tdi.port == 0) || (g_upgrade.config.tdo.port == 0))
    {
        return false;
    }

    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull = GPIO_NOPULL;
    gpio.Pin = g_upgrade.config.tms.pin;
    HAL_GPIO_Init(g_upgrade.config.tms.port, &gpio);
    gpio.Pin = g_upgrade.config.tck.pin;
    HAL_GPIO_Init(g_upgrade.config.tck.port, &gpio);
    gpio.Pin = g_upgrade.config.tdi.pin;
    HAL_GPIO_Init(g_upgrade.config.tdi.port, &gpio);
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    gpio.Pin = g_upgrade.config.tdo.pin;
    HAL_GPIO_Init(g_upgrade.config.tdo.port, &gpio);
    return true;
}

bool upgrade_gowin_begin(void)
{
    uint32_t id;

    gowin_tap_transition(TAP_TEST_LOGIC_RESET);
    gowin_tap_transition(TAP_RUN_TEST_IDLE);
    id = gowin_read_device_id();
    if ((g_upgrade.config.expected_gowin_device_id != 0U) &&
        (id != g_upgrade.config.expected_gowin_device_id))
    {
        return false;
    }

    (void)gowin_read_status();
    gowin_erase_sram();
    gowin_erase_flash();
    (void)gowin_read_status();
    return true;
}

bool upgrade_gowin_program_block(uint32_t block_number, const uint8_t *data,
                                  uint16_t block_size, uint32_t total_blocks)
{
    uint8_t page_data[256];
    uint16_t page;
    uint16_t page_count = block_size / 256U;
    uint32_t address = (block_number - 1U) * 64U * page_count;

    if ((data == 0) || (block_number == 0U) || (block_number > total_blocks))
    {
        return false;
    }

    for (page = 0U; page < page_count; ++page)
    {
        memcpy(page_data, &data[(uint32_t)page * 256U], sizeof(page_data));
        if ((block_number == 1U) && (page == 0U))
        {
            /* Required by the verified legacy GW1N-9C programming stream. */
            page_data[0] = 0x47U;
            page_data[1] = 0x57U;
            page_data[2] = 0x31U;
            page_data[3] = 0x4EU;
        }
        gowin_program_page(address + (uint32_t)page * 64U, page_data);
    }
    return true;
}

bool upgrade_gowin_finish(void)
{
    gowin_configure(0x3AU);
    gowin_configure(0x3CU);
    gowin_configure(0x02U);
    (void)gowin_read_device_id();
    (void)gowin_read_status();
    HAL_Delay(20U);
    HAL_Delay(100U);
    (void)gowin_read_status();
    return true;
}
