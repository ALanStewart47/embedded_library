# Task Plan

## Goal

Implement configurable simultaneous USART1/2/3 Bootloader transport support, remove UART4 and unused declarations, and fix review items 2, 4, 6, and 8 using static verification only.

## Phases

- [x] Confirm design and scope.
- [x] Save the target source baseline in Git.
- [x] Add static regression checks and verify they fail on the baseline.
- [x] Implement the UART transport changes.
- [x] Run static regression checks and source inspection.
- [x] Commit the implementation and report unverified hardware/build scope.

## Decisions

- Work in the user-selected `stm32f103_bootloader_v1` directory.
- USART1/2/3 are independently controlled by compile-time macros and default enabled.
- A single upgrade session is owned by the first UART that sends a valid start command.
- No Keil build or hardware test is performed at the user's request.
- IDLE uses `HAL_UART_AbortReceive()` to preserve any active TX DMA transfer.

## Errors Encountered

- Initial target path omitted the `stm32_fpga_bootloader` parent directory; resolved by locating the folder with `rg --files`.
- Baseline `git diff --check` reported pre-existing CRLF/trailing-whitespace warnings in CubeMX files; the baseline commit was retained unchanged.
- Final static-check command initially failed because PowerShell parsed `$file:` as an invalid variable reference; retry uses `${file}`.
