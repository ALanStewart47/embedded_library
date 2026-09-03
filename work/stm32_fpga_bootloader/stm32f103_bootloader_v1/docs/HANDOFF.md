# STM32F103 Bootloader 安路 FPGA 升级交接

日期：2026-09-03  
当前分支：`fpga_`  
最新相关提交：`bbc2b8e fix: match Anlogic flash update flow`

## 当前阶段

安路（Anlogic）FPGA 升级功能已移植进 `stm32f103_bootloader_v1`，已完成源码静态核查与 Keil 全量编译；需要下一轮硬件复测确认完整擦除、分包写入及 FPGA 重载。

本轮修复前的硬件日志已证明：进入升级、块大小协商、总包数协商均正常；擦除进行时的轮询也已能回复 `FB 01 FB`。修复前在擦除开始约 5 秒后收到 `FB 02 FB`。该故障的代码根因已经定位并删除，但修复后的固件尚未完成新的板级闭环验证。

## 已完成内容

1. 新增宏 `BOOTLOADER_ENABLE_ANLOGIC_FPGA`，并与 `BOOTLOADER_ENABLE_GOWIN_FPGA` 做编译期互斥检查。
2. 在 `Core/Src/bootloader.c` 内移植安路所需的：
   - W25Q128 软件 SPI；
   - 固定外部 Flash 区域 `0x0C0000` 起的 12 个 64 KiB 块擦除；
   - 256 字节页写；
   - FPGA 控制帧（`0xFFF2 = 0x0C`、`0xFFF1 = 0x00`）。
3. 移植了源工程忙状态处理：擦除或写入时，接收 `FB FF FB` 查询即回复 `FB 01 FB`；完成后回复 `FB 00 FB`。
4. 保持 MCU 升级和高云 FPGA 升级路径不变；安路与高云只能择一编译。
5. 调整代码大小后，安路配置的 Bootloader IROM 使用量为 `0x2F30 / 0x3000`，余 208 字节。

## 已定位的问题及处理

### 问题一：擦除期间不响应轮询

现象：主机每约 400 ms 发送 `FB FF FB`，但最初的移植代码在擦除循环中没有回应。

根因：通用 Bootloader 将收到的帧留给主循环处理；主循环正在同步等待 W25Q 擦除，无法进入协议解析。原 `Boot_STM32_anlu` 在 UART IDLE 中直接执行协议处理，因此忙状态能够立即返回。

处理：在安路升级已激活且 `up_cmd != 0` 时，UART 接收中断内调用 `CMD_40_handle()`，发送源代码定义的忙状态后释放并重新启动接收。

相关提交：`cef7914 fix: reply to Anlogic busy queries during erase`。

### 问题二：约 5 秒后返回 `FB 02 FB`

现象：最近一次日志中，擦除于 `18:31:28.025` 开始，持续收到 `FB 01 FB`，在 `18:31:33.400` 返回 `FB 02 FB`，间隔约 5.375 秒。

根因：移植版本自行增加了 `ANLOGIC_BLOCK_TIMEOUT_MS = 5000`。W25Q 的 64 KiB 块擦除尚未完成时，该超时将 `up_cmd` 置为 `0xFF`，随后查询协议返回 `FB 02 FB`。原安路 `W25Q128_Block_Erase_64K()` 对 WIP 位无限轮询，不存在该超时和失败状态。

处理：提交 `bbc2b8e` 恢复源代码语义：

- 删除页写和块擦除的软件超时；WIP 位清零前持续等待；
- 删除写后回读比较（原安路代码对应校验是注释状态）；
- 删除 FPGA 重载三次重试及 ACK 成败分支；源代码仅发送一次两条控制命令，忽略其返回值；
- 最后一包写入后按源代码顺序置 BootLoader 标志、延时、设置 `FAULTMASK` 并复位。

后续维护不得重新加入以上超时、回读校验或重试逻辑，除非需求明确允许偏离 `Boot_STM32_anlu`。

## 关键代码位置

- 宏定义及互斥约束：`Core/Inc/bootloader_config.h`
- 安路升级状态机、W25Q 操作、FPGA 重载：`Core/Src/bootloader.c`
- 忙查询静态检查：`tests/check_anlogic_busy_query.ps1`
- 通用静态检查：`tests/check_bootloader_static.ps1`
- 对照源工程：`../Boot_STM32_anlu/MDK-ARM/APP/Src/uart.c`、`protocol_public.c`、`W25Q128.c`、`Data_for_fpga.c`

## 验证记录

已完成：

- `tests/check_anlogic_busy_query.ps1`：6/6 通过，覆盖忙状态中断应答、固定 12 块擦除、无 WIP 超时、无写后回读。
- Keil `rebuild`（安路宏启用）：0 error、0 warning；IROM `0x2F30 / 0x3000`。
- `git diff --check`：通过。

说明：`tests/check_bootloader_static.ps1` 在当前工作区会报告“安路 FPGA support must default to disabled”，原因是用户本地将 `BOOTLOADER_ENABLE_ANLOGIC_FPGA` 改为 `1` 以进行安路构建；该本地配置没有被提交，也不应覆盖。干净默认配置下该项检查设计为要求默认值为 `0`。

## 下一步：硬件复测

1. 使用当前安路宏启用的构建产物烧录 Bootloader。
2. 发起同一份 626,777 字节（2,449 包）的 FPGA 固件升级。
3. 擦除阶段确认：在超过 5 秒时仍保持 `FB 01 FB`，不得出现 `FB 02 FB`；仅在全部 12 块擦除结束后出现 `FB 00 FB`。
4. 继续发送数据包，确认每包应答、最后一包后的 MCU 复位、以及 FPGA 从 `0x0C0000` 指向的镜像成功启动。
5. 若 WIP 永远不清零，采集完整串口日志和 W25Q 的 CS/SCK/MOSI/MISO 波形；此时应排查硬件连线或 SPI 时序，不应再次以软件超时将其伪装成校验失败。

## 工作区注意事项

以下是用户本地未提交配置，当前未被本次工作修改，也不要在交接后直接覆盖：

- `Core/Inc/bootloader_config.h`
- `MDK-ARM/stm32f103_bootloader_v1.uvprojx`

相关提交链：`24ce918`、`435120e`、`e678aca`、`89b3bdf`、`cef7914`、`bbc2b8e`。
