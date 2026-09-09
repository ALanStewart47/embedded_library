# Boot 版本查询协议（v1.2）

给 App 和上位机。Boot 与 App 认同一条指令。旧设备（含 v1.1）不回复即视为旧 boot。

上位机**只在网口升级 MCU 前**查一次。FPGA 升级、RS232 升级 MCU 不查，直接发。

## 帧格式

查询：

```
72 68 F0 16
```

应答：

```
72 68 F0 <src> <ver> 16
```

| 字段 | 值 | 含义 |
|---|---|---|
| `src` | `01` | App 在答 |
| `src` | `02` | boot 在答 |
| `ver` | `0x12` | boot v1.2，`(major << 4) \| minor` |
| `ver` | `00` | 旧 / 未知 |

查询必须是 **4 字节**。boot 里 5 字节 `72 68 xx xx 16` 是开始升级，`≥6` 字节且带 `aa` 是取消。v1.1 及更早都不匹配，静默。

## 谁在什么时候答

| 状态 | F0 |
|---|---|
| App 在跑 | App 回，`src = 01` |
| 上电等待窗口（`BOOT_WAIT_TIME`，默认 5 s） | boot 回，`src = 02` |
| 已置 `0x00aa`，停在 boot 等 MCU 升级 | boot 回，`src = 02` |
| MCU 升级会话已开始 | 回 NACK `72 68 03 16` |
| FPGA 升级模式（`0x0055`） | 不应答 F0 |
| 旧 boot（v1.1 及更早） | 静默 |

## Boot（v1.2 已实现）

MCU 命令路径收到查询后回：

```
72 68 F0 02 12 16
```

版本来自 `BL_VERSION_MAJOR/MINOR`。不占升级会话（不改 `upgrade_bin_flag` / `active_uart`）。

无 flash 身份牌，App 不能靠读 boot 区判断新旧。

## App

收到查询后回 `72 68 F0 01 <ver> 16`。`ver` 用本工程编译常量，与出厂配套 boot 一致（当前 `0x12`）：

```c
#define APP_BOOT_VER  0x12U   /* 与配套 boot 版本一致 */

uint8_t reply[] = {0x72U, 0x68U, 0xF0U, 0x01U, APP_BOOT_VER, 0x16U};
```

旧 App 不认识 F0 → 超时。新 App 刷到旧 boot 上会误报 `0x12`，这类板请用 RS232。

## 上位机

```
连上设备（通常 App 在跑）
  → 发 72 68 F0 16，等 500 ms
```

| 结果 | 动作 |
|---|---|
| `ver != 00` | 放行，走现有 MCU 升级 |
| `ver == 00` | 拒绝：boot 仅支持串口升级 MCU，请改用 RS232 |
| 超时 | 拒绝：App 过旧或不支持查询，请用 RS232 |

`src` 只记日志，不参与放行。也可以在上电 5 s 窗口直接问 boot（`src = 02`）。

FPGA 网口升级不要发这条查询。查询必须在开始升级之前发。
