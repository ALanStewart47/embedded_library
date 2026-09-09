# Bootloader 配置入口合并设计

## 目标

将 Bootloader 的串口和 FPGA 后端开关集中到 `Core/Inc/bootloader.h`，以后只需打开一个头文件即可配置，同时保持编译结果和当前启用状态不变。

## 方案选择

可选方案包括：继续保留独立配置头、彻底删除配置头、或将配置移入 `bootloader.h` 并保留兼容转发头。本次按后续确认采用彻底删除配置头，避免保留一个无配置内容的入口造成混淆。

## 修改范围

1. 把 USART1/2/3、Gowin、Anlogic 的默认宏以及“至少启用一个串口”和“两个 FPGA 后端互斥”的编译期检查移至 `bootloader.h` 的用户配置区。
2. 迁移时保留当前工作区配置值：USART1=1、USART2=0、USART3=1、Gowin=0、Anlogic=1。
3. 删除 `bootloader_config.h`，不再保留兼容转发入口。
4. `dma.c` 改为直接包含 `bootloader.h`。
5. 静态测试改为在 `bootloader.h` 检查全部配置项，并验证旧配置头不存在。

## 验证

先修改静态测试并确认旧结构失败，再迁移配置使其通过。随后运行安路专项检查、通用静态检查和 `git diff --check`；若 Keil 可用，再执行当前 Target 的 rebuild。
