# Boot version query

## Goal

One command for App and boot so the host can refuse a network MCU
upgrade when the bootloader is old (USART1-only). FPGA upgrades and
RS232 MCU upgrades stay unrestricted.

## Required behaviour

- Query: `72 68 F0 16` (4 bytes). Old boots ignore it; it does not match
  the 5-byte start-upgrade frame or the `aa` cancel frame.
- Reply: `72 68 F0 <src> <ver> 16`.
  - `src`: `01` App, `02` boot.
  - `ver`: `(major << 4) | minor`. This boot reports `0x12`.
- Boot answers on the MCU command path only (power-on wait window, or
  flag `0x00aa`). It must not take the upgrade session.
- Active MCU session: existing NACK `72 68 03 16`. FPGA mode (`0x0055`):
  no F0 reply.
- No flash identity block. Reply packs `BL_VERSION_MAJOR/MINOR`.

## Design

`BootLoaderCmd_handle()` matches the 4-byte F0 frame first, sends the
reply on the same UART, and returns. App answers with a compile-time
constant for the boot it ships with. Host treats timeout as old.

Host queries only before a **network MCU** upgrade.

## Verification

Static checks require `BL_VERSION_*`, no identity symbols, and an F0
branch that does not assign `upgrade_bin_flag` or `active_uart`. Run
`check_bootloader_static.ps1` and `check_anlogic_busy_query.ps1`. Rebuild
in Keil and confirm there is no `L6406E`.
