# 三路 UART Bootloader 传输层修复

## 目标

在 STM32F103 Bootloader 中同时支持 USART1、USART2、USART3 的 DMA 接收与协议回复，并修复下列已确认问题：

- FPGA 协议回复将栈变量交给 DMA 发送；
- USART 中断中有 3 ms 忙等；
- `URAT_NUMBER` 实际仅支持 USART1；
- `my_memset()` 的 8 位长度造成大于 255 字节的数据不能完整清零。

## 范围和约束

- 修改范围限定在 `stm32f103_bootloader_v1` 工程。
- MCU 和 Gowin FPGA 的既有帧格式保持不变。
- MCU 升级会话保持唯一；不允许来自不同 UART 的数据帧交错写入 Flash。
- `BOOTLOADER_ENABLE_GOWIN_FPGA` 默认值为 `0`；定义为 `1` 时才编译 FPGA 升级代码。
- USART1、USART2、USART3 分别由独立宏控制，默认全部启用；三个宏全部关闭时编译报错。
- 移除 Bootloader 对 UART4 的全部声明和分支。
- 删除确认未被工程引用的变量、类型、宏、函数声明和空函数，不改变仍在使用的接口。
- 不处理本轮范围外的问题，例如 MCU 固件完整性校验或包序号校验。

## 传输层设计

建立 3 个持久 UART 上下文，分别绑定 USART1、USART2、USART3。每个上下文包含：

- `UART_HandleTypeDef` 和 RX DMA 句柄；
- 独立 RX 缓冲区、接收长度和完成标志；
- 独立且持久的 TX 缓冲区。

三个端口通过以下可由 MDK 预定义宏覆盖的默认配置控制：

```c
#ifndef BOOTLOADER_ENABLE_USART1
#define BOOTLOADER_ENABLE_USART1 1
#endif
#ifndef BOOTLOADER_ENABLE_USART2
#define BOOTLOADER_ENABLE_USART2 1
#endif
#ifndef BOOTLOADER_ENABLE_USART3
#define BOOTLOADER_ENABLE_USART3 1
#endif
```

宏为 `0` 时，`main.c` 不初始化该 UART，Bootloader 不创建对应上下文、不启动 DMA，也不处理对应 UART 和 DMA IRQ。CubeMX 的公共 UART MSP 函数允许保留少量共享生成代码，以降低重新生成工程时的维护风险。

启动时，为 3 路 UART 同时开启 RX DMA 与 IDLE 中断。各 UART IRQ 只负责清除 IDLE、终止本路 RX、读取 DMA 剩余计数并置完成标志；IRQ 中不延时，也不停止 TX DMA。

主循环轮询三个上下文。每一帧的 ACK、NACK 和 FPGA 协议回复均通过该帧所属 UART 发回。TX 缓冲属于对应上下文，DMA 完成前不会引用局部栈内存。

## 会话归属

尚未开始升级时，任一 UART 的有效“开始升级”命令可成为会话拥有者。拥有者确定后，仅它的数据帧被写入 Flash；其他 UART 的帧不参与该会话，避免数据交错。Bootloader 因 Flash 标志进入升级模式时，会向三路各发送一次初始 ACK，便于上位机发现端口。

## 清理与跳转

`my_memset()` 采用至少 16 位的长度。跳转 APP 前，停止并反初始化 USART1/2/3，之后沿用既有的 NVIC、时钟和向量表清理流程。

## 验证

- 先添加传输层的失败检查，覆盖三路上下文、三路 IRQ、无 ISR 忙等、持久 TX 缓冲和 16 位清零长度；实现后检查应通过。
- 覆盖三个 UART 使能宏的典型组合，并确认三个宏全关闭时编译失败。
- 静态检查 UART4 路径和确认未使用的遗留声明已经移除。
- 使用 Keil MDK 分别构建 FPGA 宏为 `0` 和 `1` 的目标配置。
- 静态确认 USART1/2/3 的 DMA 和 IRQ 路径均连接到对应上下文。
