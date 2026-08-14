# HANDOFF —— 2026-08-14 会话交接

> 用途：v0.3 OLED_UI 框架 C51 移植进度交接。新会话先读此文件，再读 `项目介绍.md` 与 `可行性评估报告.md`。
> 环境：Keil C51 9.60（D:/APP/C51V961）、STC8H8K64U 实验箱 9.62、0.96" SSD1306 OLED（硬件 I2C：SCL=P2.5、SDA=P2.4、0x78）、git 仓库（main + dev 分支，GitHub: FullingMaple/C51Project）。

---

## 1. 当前总体状态

**v0.3 里程碑（屏幕切换 OLED）进行中：驱动层 ✅ 完成并实测点亮；OLED_UI 框架 C51 移植 🔧 进行中——编译 0 Error，链接阶段剩平台层接口未实现。**

| 模块 | 状态 |
|---|---|
| SSD1306 驱动（OLED_driver.c，硬件 I2C + diff + 双后端） | ✅ 实体屏棋盘格验证通过（32 格完整、方向正确） |
| Keil 工程（Project.uvproj） | ✅ 编译 0 Error（含框架，除链接） |
| 图形层 OLED.c（上游完整版） | ✅ C51 兼容化完成 |
| 字库 OLED_Fonts.c/.h | ✅ const → code 完成（约 29KB 字模） |
| 框架核心 OLED_UI.c（1748 行） | 🔧 C51 兼容化完成（块内声明提升、fmin/vsnprintf/round 替换、patterns 提升、HAL 删除、ShowFps 撞名修复）——**但 OLED_UI.h 刚被写坏，需从上游重拷**（见 §3） |
| 菜单数据 OLED_UI_MenuData.c | ✅ 已重写为本项目菜单（磁贴 6 项/设置/关于，C51 位置初始化，GBK） |
| 平台层 OLED_UI_Driver.c | ⛔ **未实现**（16 键 ADC 键盘 + Timer0 20ms）——当前链接失败的根源 |
| OLED_UI_Launcher.c | ✅ 已 C51 化（Timer0 中断回调已接） |

## 2. 已完成的关键工作（背景）

- **驱动层**：`src/ui/Driver/Hardware_Driver/OLED_driver.c` —— STC8H 硬件 I2C（`P_SW2|=0x10`、`I2CCFG=0xe0`）、影子缓冲 diff 刷新（逐页比较只发变化页）、`VIRTUAL_OLED` 双后端（0=实体硬件 I2C，1=虚拟 AiCube-ISP USB-CDC）、方向修正 0xA1 + 0xC8（实测左右镜像改 0xA0→0xA1 后正常）。
- **接线**：J10 4 针座出厂未焊，实体屏接 7 孔座（J11）1-4 位（GND/VCC/SCL/SDA）或飞线；I2C 模式需改焊 R175/R176（出厂 SPI 焊 R173/R174）。
- **工程配置**：uvproj 基于 69 号例程模板改造；MODP2=1、RegisterFile=STC8.H、LARGE、3 分组（app/ui/ui_framework/driver）；IncludePath 顺序：`.\src\inc;...;D:\APP\C51V961\C51\INC;D:\APP\C51V961\C51\INC\STC`（**标准 INC 必须在前**，STC 增强版 math.h/string.h 会污染）。
- **测试图案**：棋盘格（16×16 格 × 32 格）验证显示完整性与方向。

## 3. 当前阻塞与精确下一步

### 3.1 紧急：OLED_UI.h 被写坏

最后一步 python 改名 `OLED_UI_ShowFps → OLED_UI_FpsShow` 时 GBK 写入失败（文件含非法字符），**OLED_UI.h 当前可能已损坏**（未验证）。修复：

```bash
cd "d:/Document/本科/单片机课程/实训项目"
cp "D:/Document/本科/单片机课程/OLED_UI_upstream/OLED_UI_Core/HAL/OLED_UI_Core/OLED_UI/OLED_UI.h" src/ui/OLED_UI/OLED_UI.h
# python GBK 读写应用修改：
# 1) "#include \"Driver\\Hardware_Driver\\OLED_UI_Driver.h\"" → "#include \"OLED_UI_Driver.h\""
# 2) "#include \"Driver\\Software_Driver\\OLED.h\"" → "#include \"OLED.h\""
# 3) "OLED_UI_ShowFps" → "OLED_UI_FpsShow"（C51 大小写不敏感，避免与 OLED_UI_ShowFPS() 函数撞名）
```
（OLED_UI.c 里的 `OLED_UI_ShowFps → OLED_UI_FpsShow` 改名**已写入成功**，勿重复。）

### 3.2 链接失败原因：平台层接口未实现

最后编译的链接错误（0 Error 编译通过后）：
```
L127 UNRESOLVED: _OLED_UI_Init, OLED_UI_InterruptHandler, OLED_UI_MainLoop,
                 ColorMode, OLED_UI_Brightness, OLED_UI_FpsShow ...
                 Key_Init, Key_GetUpStatus, Key_GetDownStatus, Key_GetEnterStatus,
                 Key_GetBackStatus, Encoder_Init, Encoder_Get, Encoder_Enable,
                 Encoder_Disable, Timer_Init, Delay_ms, Delay_s
```
- `_OLED_UI_Init` 等带下划线 = OLED_UI.c 编译失败（OLED_UI.h 损坏导致）→ 修 3.1 后应消失
- **`Key_* / Encoder_* / Timer_Init / Delay_*` 是真缺**：`src/ui/Driver/Hardware_Driver/OLED_UI_Driver.h` 已声明接口，**需要写 OLED_UI_Driver.c**：
  - `Timer_Init()`：Timer0 20ms 中断（1T 模式，24MHz，重载值），中断里调 `OLED_UI_InterruptHandler()`（Launcher.c 已有 `Timer0_Isr(void) interrupt 1`）
  - `Key_Init()`：P1.0 ADC 初始化（参考官方 17 号例程 ADC_KeyScan.c：ADCTIM/ADCCFG/ADC_CONTR + 256 分档阈值 + 三态滤波）
  - `Key_GetUpStatus/DownStatus/EnterStatus/BackStatus()`：返回 `OLED_UI_Key.Up/.Down/.Enter/.Back` 的按下状态（0=按下，1=松开，对照 OLED_UI.c 里的判断 `OLED_UI_Key.Up == 0`）
  - `Encoder_Init/Get/Enable/Disable()`：编码器，实验箱无 → 空实现（Init 空、Get 返回 0、Enable/Disable 空）
  - `Delay_ms/Delay_s()`：软件延时
  - 按键轮询在 `OLED_UI_MainLoop` 中调用（看 OLED_UI.c 对 Key_Get 的调用时机——main loop 里调用 Key_Get* 更新 OLED_UI_Key？对照上游 OLED_UI_Driver.c：`OLED_UI_Driver.c` 里的 `BtnTask()` 在 Launcher 的 while 循环里调用，更新 OLED_UI_Key）
  - **参考**：上游 `OLED_UI_upstream/OLED_UI_Core/HAL/OLED_UI_Core/Driver/Hardware_Driver/OLED_UI_Driver.c`（STM32 版接口语义）+ 官方 17 号例程 `D:/edge_download/STC8H8K64U-DEMO-CODE-V9.6/17-ADC键盘扫描数码管显示键值和调整时间/C语言/ADC_KeyScan.c`（ADC 键盘算法）
- 写完把 `OLED_UI_Driver.c` 加入 uvproj 的 driver 组

### 3.3 下一步顺序（完成后）

1. 修 OLED_UI.h（3.1）→ 编译确认 0 Error
2. 写 OLED_UI_Driver.c（3.2）→ 编译链接通过 → HEX
3. `src/app/main.c`：`System_Init` 里加 `Timer_Init(); Key_Init();` 和 `OLED_UI_init()`；main 循环改 `OLED_UI_start()`（或直接 OLED_UI_MainLoop）——参考 Launcher.c 的 OLED_UI_init/start
4. 烧录验证：磁贴主屏 6 项显示、按键导航（ADC 键盘键值 1-4 对应上/下/确认/返回，`config.h` 的 KEY_ADC_* 占位值按实验箱实际布局调整）
5. 提交 git

## 4. 踩坑清单（新会话必读，都是真金白银）

1. **STC 头文件是 `STC8H.H`（不是 STC8.H）**，在 `D:/APP/C51V961/C51/INC/STC/`；src/inc 下**不要建 stc8h.h shim**（与 STC8H.H 大小写撞车，include 命中自己导致内容被 guard 跳过）
2. **不要 include STC 增强版 `<math.h>/<string.h>/<stdio.h>`**（STC 补丁覆盖了标准头，含 C99/C++ 扩展 C51 编不过）→ 用 `src/inc/c51lib.h`（手动声明 strcmp/strlen/sprintf/vsprintf/atan2/sin/cos/fabs/ceil 原型）
3. **python 处理 GBK 文件必须 `rb` 读 + `wb` 写 + 同编码**；`rb` 读 + 文本写会把行尾 `\r\n → \r\r\n` 逐次加倍，C51 解析 include 错乱（报错行号错位到别的文件）→ 统一 LF（`s.replace('\r\n','\n')`）
4. **`bit`、`data`、`code` 是 C51 关键字**，不能做变量名（`uint8_t bit` → 改名 bitval）
5. **C51 不支持**：for 初始化区声明、语句后声明（必须提到函数头/块开头）、C99 designated initializer（`.field=`）、复合字面量（`{0,0}` 赋值）、非常量数组初始化（`int vx[] = {X0,X1,X2}`）、`fmin/fmax/round/vsnprintf/sinf/cosf`（C51 库没有或签名不同）
6. **`const` 默认进 RAM**，数组必须显式 `code`（字库/图标）；**extern 声明必须匹配**：`extern code uint8_t` 配 `code uint8_t` 定义（`extern const` + `code` 定义 → L102/L231 报错）
7. **C51 大小写不敏感**：`OLED_UI_ShowFps`（变量）与 `OLED_UI_ShowFPS()`（函数）撞名 → 变量已改名 `OLED_UI_FpsShow`
8. **结构体整体赋值**（`A = B`）C51 支持；但初始化 `= 其他结构体` 不支持（C248）→ 声明 + 赋值分离
9. OLED 方向：上下反改 0xC0↔0xC8，左右反改 0xA0↔0xA1（本模块实测 0xA1 + 0xC8）
10. `OLED_DisplayBuf` 定义在 OLED.c（`uint8_t xdata`），OLED.h/OLED_UI.h 的 extern 必须带 `xdata` 或不重复声明（OLED.h 里的重复 extern 已删）
11. uvproj 的 `MiscControls=REMOVEUNUSED` 会移除未引用函数/变量——链接报 unresolved 时先确认定义文件编译成功且被引用

## 5. 构建与验证命令

```bash
# 构建（命令行，无需打开 Keil）
"D:/APP/C51V961/UV4/UV4.exe" -b "Project.uvproj" -o "build_log.txt" && sleep 25 && grep -E "error|Program Size" build_log.txt
# 产物：Objects/Project.hex（STC-ISP 烧录，IRC 24MHz）

# git（当前在 dev 分支，有大量未提交改动）
git add -A && git commit -m "..." && git push origin dev
```

## 6. Git 状态

- 分支：dev（main 已有 2 次文档提交；dev 已有 12 次提交，最新 `b17f379`）
- **未提交改动**：src/ 下大量文件（框架移植全部改动 + OLED_UI.h 可能损坏 + OLED_UI_Driver.c 待建）——建议 3.3 完成后统一提交，提交信息注明"v0.3 框架移植"
- `.gitignore`：Objects/、Listings/、*.uvopt、build_log.txt

## 7. 参考资料位置

- 上游框架镜像：`D:/Document/本科/单片机课程/OLED_UI_upstream/`（OLED_UI_Core/HAL/OLED_UI_Core/）
- 官方例程：`D:/edge_download/STC8H8K64U-DEMO-CODE-V9.6/`（17=ADC键盘、36.2=ST7920、69=I2C OLED 双后端）
- 实验箱原理图：`.../SCH_实验箱9.62A_2026-1-27.pdf`（J10/J11/R173-R176）
- 项目文档：`项目介绍.md`（v0.3）、`可行性评估报告.md`（OLED v3）
