# Findings

- The target is part of the parent repository on branch `fpga_`, not a nested Git repository.
- USART1/2/3 GPIO, RX/TX DMA, NVIC setup, and handlers already exist in CubeMX-generated files.
- Existing Bootloader transport uses one global RX buffer and compile-time `URAT_NUMBER`; only the USART1 receive implementation exists.
- `CMD_40_handle()` passes a local TX buffer to asynchronous DMA.
- `my_memset()` takes an 8-bit length, truncating 262 to 6.
- The target copy currently defaults `BL_ENABLE_GOWIN_FPGA` to 0 and `BL_ENABLE_ANLOGIC_FPGA` to 1 for the Anlogic build.

## 2026-09-08 Boot query

- F0 is a 4-byte command; boot replies `72 68 F0 02 12 16` from `BL_VERSION_*`.
- Flash identity at `0x08002FF0` overflowed IROM (`L6406E`) and was removed.
- App cannot probe boot from flash; it answers F0 with a compile-time constant.
- Existing HEX predates this source; rebuild before treating the image as valid.
