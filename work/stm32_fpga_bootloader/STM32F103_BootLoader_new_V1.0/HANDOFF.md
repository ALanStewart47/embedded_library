# STM32F103 Bootloader 交接说明

## 目的

本工程用于 STM32F103 串口 Bootloader：保留原 `work/stm32_fpga_bootloader/bootloader.c` 的协议组织和调用方式，并合入 MCU APP 升级、高云 FPGA 升级，以及 F103 不同 Flash 容量的页大小自适应。

目标是仅在 CubeMX/MDK 工程中加入 `bootloader.c`、`bootloader.h`，并在 `main.c` 与 USART1 中断中调用既有接口即可使用。

## 已完成内容

- 已在 `Core/Src/bootloader.c`、`Core/Inc/bootloader.h` 中实现 F103 版本的串口协议、MCU 升级和 FPGA 升级。
- MCU Flash 页大小不再使用 `FLASH_PAGE_SIZE`。代码读取 `0x1FFFF7E0` 的实际 Flash 容量：16/32/64/128 KB 使用 1 KB 页，256/384/512/768/1024 KB 使用 2 KB 页；未知容量禁止擦写。
- `main.c` 已按以下顺序集成：初始化 HAL、时钟、GPIO、DMA、USART1，调用 `boot_uart_config()`，再调用 `poweron_self_check()`；主循环调用 `bsp_ota_handle()`。
- `stm32f1xx_it.c` 的 `USART1_IRQHandler()` 在 `HAL_UART_IRQHandler(&huart1)` 后调用 `uart_interrupt_handle(1)`。
- 启动标志保持原逻辑：`0x00AA` 进入 MCU 升级，`0x0055` 进入 FPGA 升级。
- 无升级标志时，启动后等待 2 秒；仅收到既有 MCU 升级起始帧 `72 68 [包总数高] [包总数低] 16` 才留在 MCU 升级流程，其他串口数据不会阻止跳 APP。
- 跳 APP 前已加入 USART1 DMA 停止、UART DeInit、SysTick 停止、NVIC 关闭/清挂起、RCC DeInit、VTOR/MSP 设置及重新开启全局中断。

## 已遇到并处理的问题

1. **F103 Flash 页大小与工程芯片宏不一致**

   已改为读取实际 Flash 容量寄存器，不依赖工程的 `FLASH_PAGE_SIZE`。

2. **跳 APP 后 APP 不运行、读不到版本号**

   原因是跳转代码执行 `__disable_irq()` 后没有 `__enable_irq()`。已在设置 VTOR 和 MSP 后、调用 APP Reset_Handler 前恢复全局中断。

3. **Bootloader 与 APP 共用 USART1 后，APP 串口异常**

   原因是 Bootloader 的 UART DMA、DMA/NVIC、SysTick 和时钟状态遗留到 APP。跳转前已加入完整清理。此项仅完成静态检查，尚未完成新的硬件回归验证。

## 当前阻塞问题（重要）

当前现场现象：首次串口升级成功后，APP 未正常运行，复位后两秒内发送升级起始帧也无响应。

高概率原因是**升级完成时擦除了 Bootloader 尾部**：

- `BOOT_ADDRESS = 0x08003400`；
- 对 2 KB 页 F103，清除此标志时实际擦除页为 `0x08003000 ~ 0x080037FF`；
- 当前 APP 起始地址为 `0x08003800`；
- 因此 Bootloader 的有效链接范围必须严格小于 `0x08003000`。

工程中的 `MDK-ARM/DSP/DSP.sct` 确实定义了 `0x08000000 + 0x3000`，但 `MDK-ARM/DSP.uvprojx` 的 `<ScatterFile>` 为空，说明该 scatter 文件**没有被当前 MDK Target 使用**。该 Target 的有效 `IROM` 仍是 `0x80000`，因此链接器允许 Bootloader 代码落到 `0x08003000` 之后。第一次升级结束时，`set_BootLoader_flag()` 擦除标志页，会同时擦掉这部分代码。

该判断与“第一次升级完成后 APP 不运行、Bootloader 也无法再次响应”的现场现象一致。

## 当前需要做的事

1. 先用 ST-Link/J-Link 完整重新烧录 Bootloader；串口方式无法恢复可能已被擦坏的 Bootloader。
2. 在 MDK 中让**实际使用的 Target**执行下列两种方式之一，不能只改未启用的 `.sct`：
   - 把有效 IROM 容量设置为 `0x3000`；或
   - 在 Target 中明确启用 `MDK-ARM/DSP/DSP.sct`。
3. 编译后检查 `.map`：Bootloader 的 RO 末地址必须不超过 `0x08002FFF`。
4. 若 Bootloader 无法放入 12 KB，不能继续使用当前地址布局。建议统一改为：

   | 项目 | 地址/容量 |
   | --- | --- |
   | Bootloader 链接区 | `0x08000000`，大小 `0x3800` |
   | 升级标志地址 | `0x08003C00` |
   | APP 起始地址 | `0x08004000` |

   此布局在 2 KB 页芯片上把标志页放在 `0x08003800 ~ 0x08003FFF`，Bootloader 最大可到 `0x080037FF`；APP 工程也必须重新链接到 `0x08004000`。
5. 在内存布局确认后，做硬件回归：连续升级两次、升级后等待 2 秒跳 APP、读取 APP 版本、验证 APP 串口收发、验证两秒升级窗口。

## 验证状态

- 已完成代码静态检查：函数调用顺序、页大小运行时判断、两秒窗口、APP 跳转清理顺序。
- 未完成本工程的 MDK 编译、map 地址确认与硬件回归。
- 当前不应把“串口升级可稳定使用”视为已验证结论，必须先解决实际链接范围未生效的问题。

## 2026-09-02 Bootloader 瘦身静态评估（未修改代码）

本次仅阅读和分析 `Core/Src/bootloader.c`、工程配置及已有 `MDK-ARM/DSP/DSP.map`；未修改任何源码、工程配置或编译产物。

### 现有容量基线

- 现有 map 显示 Load Region 大小为 `0x2E18`（11800 B），上限为 `0x3000`（12288 B），仅剩 **488 B**。
- `DSP.map` 与 build log 的时间早于当前 `bootloader.c`，因此该容量是参考基线；后续每次修改都必须重新编译并以新 map 的 `Total ROM Size` 和 Load Region 末地址为准。
- map 显示 FPGA 更新相关、已链接的 `bootloader.c` 函数段合计约 **1264 B**。若产品允许删除 FPGA 在线升级，预计 ROM 约降至 **10536 B**，可用余量约 **1752 B**；同时可释放约 3.95 KiB RAM（FPGA 双缓冲和较大的接收缓冲）。

### 保留 MCU + FPGA 升级时的建议

1. 仅删除无用源码没有 ROM 收益：`jtag_read_data()`（96 B）和空的 `bsp_uart_baud_reinit()`（2 B）已被链接器移除；保留或删除只影响可读性。
2. 第一阶段可做低风险整理，预计节约 **220～285 B**：
   - 合并 `ACK_cmd()`、`ACK2_cmd()`、`false_cmd()` 的重复应答逻辑，并把发送缓存从 100 B 缩至实际所需大小；
   - 删除 BSS 接收/发送缓冲的重复清零，并删除处理完成后的无必要清零；协议继续以 `rx_length` 判定有效数据；
   - 对固定 USART1 路径删除 `dma_number`、`dma_ms`、`dma_ms_flag` 伪状态及 3 ms 忙等；此项必须做串口分包回归；
   - 用页大小掩码替换取模计算，并压缩 Flash 页大小判断；未知容量禁止擦写的保护逻辑必须保留；
   - 让 `jtag_erase_flash()` 复用已有的 32-bit JTAG 移位流程。
3. 第二阶段可取消 FPGA 数据从 `rx_buffer` 复制到 `spi_w_handle.spi_data[2048]`：停止 DMA 后直接消费接收缓冲，JTAG 编程结束再重启 DMA。预计额外节约 **80～140 B Flash** 和 **2048 B RAM**，但必须验证接收与编程时序。
4. 若目标是至少留出 1 KiB Flash 余量，当前基线至少需节约 **536 B**。仅第一、二阶段未必足够；还需压缩 FPGA 命令解析/JTAG 状态序列（预计总节约约 450～675 B），实际收益只能由重新编译后的 map 确认。

### 结论

瘦身可以改善 12 KB 布局下的余量，但不应替代本交接文档前述的内存布局修复。尤其在有效链接范围尚未确认前，仍优先采用 `Bootloader 0x08000000 + 0x3800`、标志页 `0x08003C00`、APP 起始 `0x08004000` 的布局，并验证实际 Target 使用该范围。

## 2026-09-02 后续进度：`stm32f103_bootloader_v1` 副本

以下进度适用于 `work/stm32_fpga_bootloader/stm32f103_bootloader_v1`，不是对本目录原工程的覆盖或硬件验证结果。

### 已完成

- 已保存目标副本的初始源码基线：`134b7ae`。
- 串口传输层已改为可同时支持 USART1、USART2、USART3；每路 UART 具有独立的 RX/TX DMA 上下文，升级会话通过活动端口隔离，回复从接收该命令的端口发出。
- 新增 `Core/Inc/bootloader_config.h`：`BOOTLOADER_ENABLE_USART1/2/3` 可独立控制；三个端口同时关闭时编译报错。`BOOTLOADER_ENABLE_GOWIN_FPGA` 默认关闭，关闭时不编译 FPGA 升级代码；UART4 支持已移除。
- 修复了 DMA 异步发送引用局部 TX 缓冲、`my_memset()` 长度为 8 位导致截断、RX 中断路径延时及停止全部 UART DMA 等问题。
- 已补齐 USART2/USART3 的初始化、DMA/NVIC 和 IDLE IRQ 条件编译路径。
- 已按依赖关系整理 `bootloader.c` 的定义、宏和注释；`BOOT_UART_COUNT` 保持在 `boot_uarts[]` 后，避免与其依赖关系分离。

### 配置与提交

- 多 UART 实现提交：`1437cd6 feat: add configurable multi-UART bootloader transport`。
- 可读性整理提交：`6bf7e74 style: improve bootloader definition layout`。
- 当前工作区配置为 USART1 默认启用、USART2/USART3 默认关闭；这只是默认配置，仍可通过 `bootloader_config.h` 的宏启用任意组合。

### 验证状态

- 已运行 `stm32f103_bootloader_v1/tests/check_bootloader_static.ps1`，25 项静态断言通过；同时完成条件编译配对和 Git 空白检查。
- 按当前任务约定，**未执行** MDK/Keil 编译、map 容量检查、实机 UART DMA 测试、MCU 升级测试或 FPGA/JTAG 测试。
- 本节不改变上文原工程关于链接范围、升级标志页和硬件回归的阻塞结论；对目标副本也应在实机验证前确认实际链接范围与 APP 地址布局。
