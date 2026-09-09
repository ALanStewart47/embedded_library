# STM32F103 Bootloader v1.2 交接

日期：2026-09-08  
工程：`stm32f103_bootloader_v1.2`  
交付目录：`../stm32f103_bootloader_v1.2_release/`

## 当前阶段

工作已转到 `stm32f103_bootloader_v1.2`。相对 v1.1，本版本增加 boot 版本查询（给 App / 上位机拦「网口升 MCU 到旧 boot」）。CH9121 与安路 FPGA 升级路径与 v1.1 相同。

源码静态检查已通过。本机无 Keil `UV4.exe`，`.hex` 需你本地 rebuild 后再拷进 release。

## 本轮做了什么（2026-09-08）

现场会混用只开 USART1 的旧 boot 和开了 USART3（CH9121）的新 boot。旧 boot 不能现场更新。上位机若走网口升 MCU，App 置 `0x00aa` 复位后旧 boot 收不到 USART3 数据，设备卡在 boot。FPGA 网口升级必须继续放行。

处理方式：App 和 boot 认同一条查询，旧 boot 不回就是旧的。上位机**只在网口升 MCU 前**查一次；FPGA 升级和 RS232 升 MCU 不查。

Boot 侧已落地：

1. `bootloader.h` 增加 `BL_VERSION_MAJOR/MINOR`（1.2）。
2. `BootLoaderCmd_handle()` 认 4 字节 `72 68 F0 16`，回 `72 68 F0 02 12 16`，不占升级会话。会话已开始仍回 NACK；FPGA 模式（`0x0055`）不应答。
3. 不做 flash 身份牌（固定地址对象会把 12 KiB IROM 垫满导致 `L6406E`）。App 用编译期常量答 F0。
4. 协议说明：`docs/boot_version_query_protocol.md`（release 不含 docs，单独发给同事）。
5. 源码已同步到 `stm32f103_bootloader_v1.2_release/Core/`。

判定：超时或不认识 F0 视为旧 boot。FPGA 网口升级不查。

## 此前 v1.1 已有（2026-09-07）

- 可选 CH9121：`BL_ENABLE_CH9121`，默认 CFG=PC5、RST=PB2，SysInit 里拉高，不打复位脉冲。
- `BOOT_WAIT_TIME` 挪到 `bootloader.h`，默认 5000 ms。
- CubeMX RCC/GPIO 走 LL；USART 仍是 HAL。

## 此前 v1 安路收尾与 Release（2026-09-04）

1. 配置已集中至 `Core/Inc/bootloader.h`；旧的 `bootloader_config.h` 已删除，不再提供兼容入口。
2. 配置宏已统一为简短的 `BL_ENABLE_*` 形式，例如 `BL_ENABLE_USART1`、`BL_ENABLE_ANLOGIC_FPGA`。
3. `bootloader.h` 和 `bootloader.c` 已进行轻量风格整理，仅调整注释、空白和文件头说明；保留原有 `Defines/Declaring` 区块及全部功能逻辑。
4. 已生成交付目录：`../stm32f103_bootloader_v1_release/`。该目录不含 `docs/`、`tests/` 和 AI 工作记录；工程源代码与原工程的 `Core/`、`Drivers/` 完全一致。
5. release 工程已被 Keil 重新构建，因此其中包含可交付的 `.hex`、`.axf`、`.map` 等构建产物。
6. 对比 release 与源工程时，仅发现两项工程配置差异：
   - `UserProg2Name` 已按预期改为 release 目录中的大小显示工具路径；
   - `Flash2` 从 `BIN\\UL2V8.DLL` 变为 `BIN\\UL2CM3.DLL`。这是 Keil 下载/调试算法配置差异，不影响 C 源代码或升级协议；尚未回改，后续如需严格保持工程配置一致再决定。

## 已完成内容

1. 新增宏 `BL_ENABLE_ANLOGIC_FPGA`，并与 `BL_ENABLE_GOWIN_FPGA` 做编译期互斥检查。
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

- 宏定义、版本、CH9121 引脚：`Core/Inc/bootloader.h`
- MCU / FPGA 升级、CH9121 拉高、F0 查询：`Core/Src/bootloader.c`
- 版本查询协议（给 App / 上位机）：`docs/boot_version_query_protocol.md`
- 版本查询设计：`docs/superpowers/specs/2026-09-08-boot-version-query-design.md`
- 忙查询静态检查：`tests/check_anlogic_busy_query.ps1`
- 通用静态检查：`tests/check_bootloader_static.ps1`
- 对照源工程：`../Boot_STM32_anlu/MDK-ARM/APP/Src/uart.c`、`protocol_public.c`、`W25Q128.c`、`Data_for_fpga.c`

## 验证记录

已完成：

- `tests/check_anlogic_busy_query.ps1`：14/14 通过。
- `tests/check_bootloader_static.ps1`：89/89 通过。
- Keil `rebuild`：请本地 rebuild，确认无 `L6406E`，再把 `.hex` 拷到 release。

说明：串口、FPGA、CH9121 开关全部位于 `Core/Inc/bootloader.h`。

## 下一步

1. 本地 Keil rebuild v1.2，确认无 `L6406E`，把新 `.hex` 拷进 `stm32f103_bootloader_v1.2_release`。
2. 把 `docs/boot_version_query_protocol.md` 发给 App / 上位机同事；App 用编译期常量答 F0，上位机只在网口升 MCU 前查一次。
3. 板级确认：新 boot 上电窗口或 `0x00aa` 后，`72 68 F0 16` 回 `72 68 F0 02 12 16`；FPGA 网口升级路径不变。
4. 安路 FPGA 升级硬件复测仍建议做：擦除超过 5 秒须保持 `FB 01 FB`，不得出现 `FB 02 FB`。

## 工作区注意事项

以下是需要保留的用户本地未提交配置或工程修改，不要在交接后直接覆盖：

- `Core/Inc/bootloader.h`（串口及 FPGA 配置区）
- `MDK-ARM/stm32f103_bootloader_v1.uvprojx`

相关提交链：`24ce918`、`435120e`、`e678aca`、`89b3bdf`、`cef7914`、`bbc2b8e`。
