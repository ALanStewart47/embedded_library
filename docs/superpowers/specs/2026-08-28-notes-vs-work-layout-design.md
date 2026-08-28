# notes vs work 目录划分

日期：2026-08-28  
状态：待实现

## 目标

把本仓库拆成两棵互不混放的树：

- `notes/`：平时记录。可以是片段，也可以是小模块。不要求能编译。
- `work/`：公司可长期复用的完整工程。可持续更新。每个子目录都必须是能打开、能编的工程。

根目录不再放 `.c`，也不再按 `01_ADC` 这类主题编号做顶层分类。

## 目录

```
embedded_library/
  notes/
    snippets/          # 单文件、半截函数、备忘
    modules/           # 一组 .h/.c，不绑板、不绑 IDE
  work/
    <工程名>/          # 完整工程（Keil / CMake / EIDE）
  README.md            # 只说明 notes 与 work 的区别
```

主题（ADC、协议等）不是顶层分类。笔记多了以后，可以在 `notes/snippets/` 或 `notes/modules/` 下再加主题子目录，不改顶层。

## 进出规则

1. 编不过、或不打算当产品用 → `notes/`
   - 单函数、半截协议、魔法数说明 → `notes/snippets/`
   - 已有 `.h` + `.c`，但没有工程文件 → `notes/modules/`（一个模块一个子目录）
2. 公司以后还要改、要当可复用工程 → `work/<工程名>/`
   - 只收完整工程，不收散文件
3. 从笔记进入产品是拷贝，不是原地长大
   - 在 `notes` 写熟后，拷进某个 `work` 工程再改接口、补构建、补板级
   - `notes` 里的原稿可保留作草稿，不强制删除

## work 工程门槛

`work/<工程名>/` 必须同时具备：

- 能打开的工程文件：Keil `.uvprojx`、CMake、或 EIDE `eide.yml`，三选一
- 源码（`.c` / `.h`），含启动/板级/构建，不只是核心算法
- 工程根上的 `README.md`：干什么、哪块板/哪套工具链、怎么编
- 工程自己的 `.gitignore`（Keil 产物、`Objects/`、`Listings/` 等）

工程名：英文蛇形或短横线，按产品或可复用能力命名（如 `adc_cal_board`）。禁止 `01_ADC`、禁止 `test1`。

本次不往 `work/` 塞现有代码。建空目录占位，等第一个完整公司工程再放入。

## notes 命名

- `snippets/`：一个主题一个 `.c`，文件名说清干什么
- `modules/`：一个模块一个子目录，至少有 `.h` + `.c`
- 不强制编号前缀

## 本次迁移（只搬家，不改逻辑）

| 现在 | 迁到 |
|------|------|
| `02_Unit_Data_Processing/fine_end.c` | `notes/snippets/find_end.c`（按函数名重命名） |
| `02_Unit_Data_Processing/parse_uint_decimal.c` | `notes/snippets/parse_uint_decimal.c` |
| `02_Unit_Data_Processing/reply_u32_to_char.c` | `notes/snippets/reply_u32_to_char.c` |
| `01_ADC/Two-point linear calibration/` | `notes/modules/adc_two_point_cal/` |
| `01_ADC/.gitignore` | 删除（根上没有 Keil 工程；规则由未来的 work 工程各自携带） |

迁完后删除空的 `01_ADC`、`02_Unit_Data_Processing`。根目录只留 `notes/`、`work/`、`docs/`、`README.md`、`.git`。

根 `README.md` 只说明：`notes` 是草稿，`work` 是公司可复用完整工程。

## 不做

- 不把 ADC 校准或其它笔记改成完整工程
- 不按外设切开 `work/`
- 不在本次修改函数实现、不修 `parse_uint_decimal.c` 等已有笔误
- 不把个人笔记和公司工程放进同一个子目录
