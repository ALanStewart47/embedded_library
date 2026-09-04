# Bootloader 轻量风格整理设计

## 目标

在不改变 Bootloader 功能、协议、时序和配置结果的前提下，整理 `bootloader.c` 与 `bootloader.h` 中过度模板化或解释过满的文字，使代码更接近项目维护者长期使用的嵌入式 C 风格。

## 修改范围

- 仅修改 `Core/Src/bootloader.c` 和 `Core/Inc/bootloader.h`。
- 保留现有 `Defines`、`Declaring` 分区，以及当前缩进和大括号风格。
- 注释使用简短英文。
- 整理文件头中的重复字段、大小写错误、不自然表述和多余空行。
- 删除或缩短没有提供额外信息的模板化注释。
- 保留硬件引脚、Flash 地址、协议状态和关键时序等必要说明。
- 在 FPGA 配置旁说明：两个后端都关闭表示 MCU-only；两个后端同时开启才触发编译错误。

## 明确不修改

- 不修改任何宏值、地址、缓冲区大小或编译条件。
- 不重命名函数、变量、类型或宏。
- 不改变函数可见性、声明顺序或调用关系。
- 不调整协议判断、错误处理、Flash 操作、UART/DMA 行为和 FPGA 时序。
- 不修改 CubeMX 生成的 `main.c`、`dma.c` 和 `stm32f1xx_it.c`。

## 验证

- 审查最终差异，确认 `bootloader.c` 与 `bootloader.h` 只有注释和空白变化。
- 运行 `tests/check_bootloader_static.ps1`。
- 运行 `tests/check_anlogic_busy_query.ps1`。
- 运行 `git diff --check`。
- 若当前环境可用，执行 Keil rebuild；工具不可用时明确记录环境限制。
