# Bootloader Readability Cleanup Design

## Scope

Reorganize declarations, macros, comments, and whitespace in `Core/Src/bootloader.c` only. Preserve all existing runtime behavior, macro values, control flow, and conditional-compilation conditions.

## Layout

- Keep independent file-wide constants together near the includes.
- Keep a macro immediately after the declaration it depends on; `BOOT_UART_COUNT` therefore stays after `boot_uarts`.
- Group UART transport declarations in the order: external handles, buffer-size macro, context type, context table, dependent count macro, active context.
- Keep FPGA-only types, pin definitions, and JTAG helper macros together inside the FPGA compile-time block.
- Use one consistent spacing and blank-line convention; remove redundant section banners only where they add no structure.

## Validation

Run the existing static regression script and whitespace check. No compiler, hardware, DMA, UART, or FPGA validation is part of this cleanup.
