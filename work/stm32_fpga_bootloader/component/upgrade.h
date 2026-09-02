#ifndef STM32F103_GW1N_UPGRADE_H
#define STM32F103_GW1N_UPGRADE_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "upgrade_protocol.h"

#define UPGRADE_MAX_FRAME_SIZE 2148U

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} upgrade_gpio_t;

/* The callback must copy or finish transmitting the buffer before it returns. */
typedef void (*upgrade_tx_fn)(const uint8_t *data, uint16_t length, void *user);

typedef struct
{
    uint32_t app_address;
    uint32_t app_end_address;
    uint32_t metadata_address;
    upgrade_tx_fn transmit;
    void *transmit_user;
    upgrade_gpio_t tms;
    upgrade_gpio_t tck;
    upgrade_gpio_t tdi;
    upgrade_gpio_t tdo;
    uint32_t expected_gowin_device_id; /* 0 disables the ID comparison. */
} upgrade_config_t;

typedef enum
{
    UPGRADE_STATUS_WAITING = 0,
    UPGRADE_STATUS_BUSY,
    UPGRADE_STATUS_DONE,
    UPGRADE_STATUS_ERROR
} upgrade_status_t;

/* Call before HAL_Init().  A normal marker returns UPGRADE_MODE_APP. */
upgrade_mode_t upgrade_boot_decide(const upgrade_config_t *config);
bool upgrade_app_valid(const upgrade_config_t *config);
void upgrade_jump_to_app(const upgrade_config_t *config);

/* Call after CubeMX has initialized the selected UART, DMA and GPIO clocks. */
bool upgrade_init(const upgrade_config_t *config, upgrade_mode_t mode);

/* Submit exactly one UART-idle-delimited frame. */
bool upgrade_rx_feed(const uint8_t *data, uint16_t length);
void upgrade_process(void);
upgrade_status_t upgrade_get_status(void);

/* This function erases the configured metadata page and resets the MCU. */
bool upgrade_request(const upgrade_config_t *config, upgrade_mode_t mode);

#endif
