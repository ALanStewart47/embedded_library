#ifndef STM32F103_GW1N_UPGRADE_INTERNAL_H
#define STM32F103_GW1N_UPGRADE_INTERNAL_H

#include "upgrade.h"

typedef struct
{
    upgrade_config_t config;
    upgrade_mode_t mode;
    upgrade_status_t status;
    uint8_t frame[UPGRADE_MAX_FRAME_SIZE];
    uint16_t frame_length;
    bool frame_ready;
    bool mcu_active;
    uint16_t mcu_total_packets;
    uint16_t mcu_next_packet;
    uint16_t fpga_block_size;
    uint32_t fpga_total_blocks;
    uint32_t fpga_next_block;
} upgrade_context_t;

extern upgrade_context_t g_upgrade;

bool upgrade_gowin_init(void);
bool upgrade_gowin_begin(void);
bool upgrade_gowin_program_block(uint32_t block_number, const uint8_t *data,
                                  uint16_t block_size, uint32_t total_blocks);
bool upgrade_gowin_finish(void);

#endif
