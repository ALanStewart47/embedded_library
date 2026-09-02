# Findings

- The target is part of the parent repository on branch `fpga_`, not a nested Git repository.
- USART1/2/3 GPIO, RX/TX DMA, NVIC setup, and handlers already exist in CubeMX-generated files.
- Existing Bootloader transport uses one global RX buffer and compile-time `URAT_NUMBER`; only the USART1 receive implementation exists.
- `CMD_40_handle()` passes a local TX buffer to asynchronous DMA.
- `my_memset()` takes an 8-bit length, truncating 262 to 6.
- The target copy currently defaults `BOOTLOADER_ENABLE_GOWIN_FPGA` to 1, contrary to the approved default-off design.
