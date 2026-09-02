# Progress

## 2026-09-02

- Read and approved the multi-UART design specification.
- Added independent UART-enable macros, UART4 removal, and unused-code cleanup to the specification.
- User limited verification to static checks; build and hardware validation remain for the user.
- Saved the five files that will be changed in baseline commit `134b7ae`.
- Added `tests/check_bootloader_static.ps1`; baseline run failed 22 assertions for the expected missing behavior.
- Replaced the single global RX path with an enabled-port context table and a single active upgrade owner.
- Replaced direct DMA sends with per-context persistent TX buffers.
- Added USART2/USART3 IDLE IRQ hooks, compile-time gates for initialization and DMA/IRQ paths, and removed UART4/legacy unused declarations.
- Static regression checks now pass; Keil and hardware verification were intentionally not run.
- Final aggregate static check passed: 25 assertions, balanced conditional-compilation directives, and no patch whitespace errors.
