# Anlogic FPGA busy-query compatibility

## Goal

Match `Boot_STM32_anlu` during the FPGA Flash erase phase. The FPGA update
protocol retains its fixed erase range: after a valid total-block command, erase
the twelve 64 KiB blocks beginning at `0x000C0000` before accepting image data.

## Required behaviour

- While `spi_w_handle.up_cmd` is busy (`1` for erase or `2` for write), a UART
  frame received on the active FPGA-update UART must immediately return
  `FB 01 FB`.
- The response must be emitted from the UART interrupt path, so it can preempt
  the synchronous W25Q operation.
- After the operation completes, normal main-loop parsing remains responsible
  for commands and returns the existing ready state `FB 00 FB`.
- The MCU-only and Gowin configurations are unchanged.

## Design

The generic UART ISR continues to capture frames as before. Under
`BL_ENABLE_ANLOGIC_FPGA`, when the FPGA update task is active and its
operation state is nonzero, the ISR consumes that received frame, sends the
three-byte busy status through the persistent per-UART DMA TX buffer, and
re-arms reception. This models the source project's interrupt-time
`Uart_Task()` response without restoring its unsafe shared stack/DMA buffers or
making normal command parsing run in an interrupt.

## Verification

Static checks require the Anlogic busy-state reply to be reachable from
`uart_interrupt_handle`, retain `FB 01 FB`, and keep the source-compatible
fixed twelve-block erase loop. Keil builds cover the default, Gowin-only, and
Anlogic-only macro configurations.
