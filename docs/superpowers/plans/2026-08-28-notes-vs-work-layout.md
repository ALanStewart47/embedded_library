# notes vs work 目录迁移 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 按 spec 把现有散文件迁进 `notes/`，建空的 `work/` 占位，根上只留两棵树加说明。

**Architecture:** 只用 `git mv` / 新建文件搬家，不改任何 `.c` / `.h` 逻辑。`work/` 用 `.gitkeep` 占位，因为 Git 不跟踪空目录。

**Tech Stack:** Git、PowerShell、现有 C 源文件。

## Global Constraints

- 只搬家，不改函数实现，不修已有笔误
- 不把 ADC 校准改成完整工程
- 不往 `work/` 放入现有源码
- 不按外设切开 `work/`
- 未经用户明确要求不 `git commit`

---

### Task 1: 建立 notes / work 骨架并迁文件

**Files:**
- Create: `notes/snippets/`
- Create: `notes/modules/adc_two_point_cal/`
- Create: `work/.gitkeep`
- Create: `README.md`
- Move: `02_Unit_Data_Processing/fine_end.c` → `notes/snippets/find_end.c`
- Move: `02_Unit_Data_Processing/parse_uint_decimal.c` → `notes/snippets/parse_uint_decimal.c`
- Move: `02_Unit_Data_Processing/reply_u32_to_char.c` → `notes/snippets/reply_u32_to_char.c`
- Move: `01_ADC/Two-point linear calibration/two_point_linear_calibration.c` → `notes/modules/adc_two_point_cal/two_point_linear_calibration.c`
- Move: `01_ADC/Two-point linear calibration/two_point_linear_calibration.h` → `notes/modules/adc_two_point_cal/two_point_linear_calibration.h`
- Delete: `01_ADC/.gitignore`
- Delete: empty `01_ADC/`、`02_Unit_Data_Processing/`

**Interfaces:**
- Consumes: spec `docs/superpowers/specs/2026-08-28-notes-vs-work-layout-design.md`
- Produces: 根目录仅含 `notes/`、`work/`、`docs/`、`README.md`、`.git`

- [ ] **Step 1: 建目录**

```powershell
New-Item -ItemType Directory -Force -Path notes\snippets, notes\modules\adc_two_point_cal, work | Out-Null
```

- [ ] **Step 2: git mv 片段与模块**

```powershell
git mv 02_Unit_Data_Processing/fine_end.c notes/snippets/find_end.c
git mv 02_Unit_Data_Processing/parse_uint_decimal.c notes/snippets/parse_uint_decimal.c
git mv 02_Unit_Data_Processing/reply_u32_to_char.c notes/snippets/reply_u32_to_char.c
git mv "01_ADC/Two-point linear calibration/two_point_linear_calibration.c" notes/modules/adc_two_point_cal/two_point_linear_calibration.c
git mv "01_ADC/Two-point linear calibration/two_point_linear_calibration.h" notes/modules/adc_two_point_cal/two_point_linear_calibration.h
```

- [ ] **Step 3: 删旧 .gitignore 和空目录**

```powershell
git rm 01_ADC/.gitignore
Remove-Item -Recurse -Force 01_ADC, 02_Unit_Data_Processing
```

- [ ] **Step 4: 写 `work/.gitkeep`（空文件）和根 `README.md`**

`README.md` 全文：

```markdown
# embedded_library

- `notes/` 平时记录：片段放 `notes/snippets/`，小模块放 `notes/modules/`。不要求能编译。
- `work/` 公司可复用的完整工程。每个子目录必须能打开、能编，并持续在这里更新。

从笔记进入产品：拷进某个 `work` 工程再改，不要在 `notes` 里原地长大。
```

- [ ] **Step 5: 核对树与内容未改**

```powershell
git status
git ls-files
fc /b notes\snippets\find_end.c NUL > $null; git diff -- notes/snippets notes/modules
```

Expected:
- `git ls-files` 含 `notes/snippets/find_end.c`、`parse_uint_decimal.c`、`reply_u32_to_char.c`、`notes/modules/adc_two_point_cal/two_point_linear_calibration.c`、`.h`、`work/.gitkeep`、`README.md`、`docs/superpowers/...`
- 不含 `01_ADC/`、`02_Unit_Data_Processing/`
- `git diff` 对迁过去的源文件无内容变更（rename 不改字节）
- 不 commit（用户未要求）
