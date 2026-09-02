# STM32F103 + GW1N-9C 升级组件

外部工程只需包含 `upgrade.h`；`upgrade_protocol.h` 与 `upgrade_internal.h` 是组件内部文件。

## 文件

- `upgrade.c`：启动判定、MCU UART 升级、Flash 操作与公共 API。
- `upgrade_gowin.c`：GW1N-9C 的软件 JTAG ISP。
- `upgrade_protocol.c`：启动标志、Modbus CRC 与 XOR。

## 固定布局

此版本针对 STM32F103 HAL。默认推荐布局如下：

| 区域 | 地址 |
| --- | --- |
| Bootloader 代码 | `0x08000000` - `0x08002FFF`（最大 12 KB） |
| 升级标志页 | `0x08003000` - `0x080037FF` |
| MCU 应用 | 从 `0x08003800` 开始 |

F103ZE 的 Flash 页为 2 KB，所以升级标志页必须独占，不能放入 Bootloader 代码。Bootloader 链接脚本的 Flash 长度应设为 12 KB；应用工程的 Flash 起始地址应设为 `0x08003800`。

## CubeMX 接入

1. 将本目录三个 `.c` 文件和 `upgrade.h` 加入工程；启用 STM32F1 HAL Flash、GPIO、UART、DMA。
2. 在 CubeMX 中启用升级 UART 的 DMA 接收和 UART IDLE 中断，并启用 JTAG 四根 GPIO 的时钟。当前已验证板卡通常是 PA4/PA5/PA6/PA7，但以原理图为准。
3. 定义配置。发送回调应在返回前复制完数据；最简单的实现是阻塞式 `HAL_UART_Transmit`，不要直接把组件提供的临时缓冲交给异步 DMA。

```c
static void upgrade_tx(const uint8_t *data, uint16_t length, void *user)
{
    UART_HandleTypeDef *uart = user;
    (void)HAL_UART_Transmit(uart, (uint8_t *)data, length, 1000U);
}

static const upgrade_config_t upgrade_cfg = {
    .app_address = 0x08003800U,
    .app_end_address = 0U,       /* 0: use the F103 Flash end supplied by HAL */
    .metadata_address = 0x08003000U,
    .transmit = upgrade_tx,
    .transmit_user = &huart1,
    .tms = {GPIOA, GPIO_PIN_4},
    .tck = {GPIOA, GPIO_PIN_5},
    .tdi = {GPIOA, GPIO_PIN_6},
    .tdo = {GPIOA, GPIO_PIN_7},
    .expected_gowin_device_id = 0U /* 0 keeps legacy no-ID-check behavior */
};
```

4. 在 `main()` 的最前面、`HAL_Init()` 前读取模式。正常模式跳转成功后不会返回；若应用向量无效，可改为 MCU 升级模式以便救援。

```c
upgrade_mode_t mode = upgrade_boot_decide(&upgrade_cfg);
if (mode == UPGRADE_MODE_APP) {
    upgrade_jump_to_app(&upgrade_cfg);
    mode = UPGRADE_MODE_MCU;
}

HAL_Init();
SystemClock_Config();
MX_GPIO_Init();
MX_DMA_Init();
MX_USART1_UART_Init();

if (!upgrade_init(&upgrade_cfg, mode)) {
    Error_Handler();
}
```

5. 在现有 UART IDLE + DMA 接收处理里，计算本帧实际长度后调用 `upgrade_rx_feed(rx_buffer, rx_length)`，然后重启 DMA 接收。主循环只需调用：

```c
while (1) {
    upgrade_process();
}
```

6. 应用程序请求升级时调用 `upgrade_request(&upgrade_cfg, UPGRADE_MODE_MCU)` 或 `upgrade_request(&upgrade_cfg, UPGRADE_MODE_FPGA)`；它会擦除标志页、写入 `0xAA` 或 `0x55` 并复位。

## 兼容范围

- 保留原工程 MCU 的 `72 68/72 69` 和 FPGA 的 `FA/FB` 上位机协议。
- FPGA 仅支持已实机验证的 Gowin GW1N-9C 编程序列。
- 按现有机制，MCU 每包为 256 bytes，FPGA 块大小为 256 至 2048 bytes。
- 一次 Flash 擦写或 JTAG 编程期间会阻塞；不实现加密、签名、A/B 分区、回滚、断点续传。
