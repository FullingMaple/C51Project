# HANDOFF —— 2026-08-14 会话交接

> 用途：v0.3 OLED_UI 框架 C51 移植进度交接。新会话先读此文件，再读 `项目介绍.md` 与 `可行性评估报告.md`。
> 环境：Keil C51 9.60（D:/APP/C51V961）、STC8H8K64U 实验箱 9.62、0.96" SSD1306 OLED（硬件 I2C：SCL=P2.5、SDA=P2.4、0x78）、git 仓库（main + dev 分支，GitHub: FullingMaple/C51Project）。

---

## 1. 当前总体状态

**v0.3 里程碑（屏幕切换 OLED）：驱动层 ✅ 实测点亮；OLED_UI 框架 C51 移植 ✅ 编译 0 Error + 链接通过 + HEX 已产出——剩上板烧录验证。**

| 模块 | 状态 |
|---|---|
| SSD1306 驱动（OLED_driver.c，硬件 I2C + diff + 双后端） | ✅ 实体屏棋盘格验证通过 |
| Keil 工程（Project.uvproj） | ✅ 编译 0 Error，链接通过，flash 60.7KB/64KB（含裁剪） |
| 图形层 OLED.c | ✅ C51 兼容化 |
| 字库 OLED_Fonts.c/.h | ✅ const→code；**已裁剪未引用字库/图片省 ~8.8KB** |
| 框架核心 OLED_UI.c | ✅ **本次从上游重拷并重做全部 C51 兼容化**（上次会话被 python 写空，见 §4.1） |
| OLED_UI.h | ✅ **本次从上游重拷修复**（上次会话被写坏，GBK 解码失败，见 §4.1） |
| 菜单数据 OLED_UI_MenuData.c | ✅ 重写为本项目菜单；变量 `OLED_UI_FpsShow` 已同步改名 |
| 平台层 OLED_UI_Driver.c/.h | ✅ **本次实现**：Timer0 20ms（12T）+ 16 键 ADC 键盘（P1.0，三态滤波）+ 空编码器 + 软延时 + GetTick |
| OLED_UI_Launcher.c | ✅ Timer0 中断已接 tick + 键扫 + InterruptHandler |
| main.c | ✅ 接入 OLED_UI_init/start + SP=0x80 + EA=1 |

## 2. 本次会话完成的关键工作（2026-08-14 第二段）

1. **发现并修复 OLED_UI.c 被写空**（git 提交 2696338 里它已是 0 字节空文件！）：从上游 `D:/Document/本科/单片机课程/OLED_UI_upstream/OLED_UI_Core/HAL/OLED_UI_Core/OLED_UI/OLED_UI.c` 重拷，重做全部 C51 兼容化（39 处转换）：
   - include：`"..\OLED_UI_Launcher.h"` → `"OLED_UI_Launcher.h"`；新增 `"c51lib.h"` + `<stdarg.h>`
   - `OLED_UI_ShowFps`（变量）→ `OLED_UI_FpsShow`（C51 大小写不敏感，与函数 OLED_UI_ShowFPS 撞名）
   - 删 `HAL_TIM_Base_Start_IT(&htim1)`；`HAL_GetTick()` → `GetTick()`（平台层实现）
   - `vsnprintf` → `vsprintf`（配 PRINTF_LARGE）；`fmin` 用文件级宏替代
   - patterns 局部 const 数组 → 文件级 `static code`（C51 局部 const 进 RAM）
   - **全部块内声明提升**：for 初始化区声明（`for(MenuID i...`、`for(int i...`）×5、语句后声明 ×10+、`MenuID_Type IncreaseID = OLED_KeyAndEncoderRecord()` 改声明+赋值（C248）
   - **注意：文件顶部 include 必须在 fmin 宏/patterns 数组之前**（否则 uint8_t 未定义 → C129）
2. **修复 OLED_UI.h**：从上游重拷（本地文件 GBK 解码失败，含 UTF-8 替换字符 U+FFFD 混入），改 2 处 include 为本地路径，删掉无 xdata 的重复 `extern OLED_DisplayBuf`（OLED_driver.h 已有 xdata 声明）
3. **实现 OLED_UI_Driver.c**（见 §3.1）
4. **裁剪 OLED_Fonts.c 未引用字库/图片省 ~8.8KB**（flash 超 64KB 问题，见 §4.3）
5. **uvproj**：C51 Define 加 `PRINTF_LARGE`；driver 组加 OLED_UI_Driver.c
6. **main.c**：`SP = 0x80`（无 STARTUP.A51 手动设栈）+ `OLED_UI_init()` + `OLED_UI_start()` + EA=1
7. **构建结果**：`0 Error(s), 4 Warning(s)`，Program Size: data=15.1 xdata=3853 const=20801 code=41315（共 60.7KB < 64KB），HEX 已生成 Objects/Project.hex

## 3. 平台层实现要点（OLED_UI_Driver.c/.h）

- `Timer_Init()`：**12T 模式**（1T 下 16 位定时器最大 2.73ms 无法 20ms），24MHz→2MHz，20ms=40000 计数，重载 65536-40000=25536（TH0=0x63/TL0=0xC0），TMOD 0x01，AUXR &= ~0x80
- `Key_Init()`：P1.0=ADC0 高阻（P1M1|=0x01），ADCTIM=0x3f / ADCCFG=0x2f / ADC_CONTR=0x80（官方 17 号例程参数）
- `Driver_KeyScan()`：20ms 中断内采样 → 256 分档（ADC_OFFSET=64）→ **三态滤波**（连续 3 次相同才确认，60ms 去抖）→ 更新 `Key_Hold`
- `Key_Get*Status()`：返回 0=按下 1=松开（与 OLED_UI_Key 语义一致），映射 config.h 的 `KEY_ADC_UP/DOWN/ENTER/BACK`（1/2/3/4，按实验箱实际布局调整）
- `GetTick()`：Timer0 20ms 计数 ×20（ms）；FADEOUT_TIME 40ms 节拍用
- Launcher.c 的 Timer0_Isr：`Driver_TickHandler(); Driver_KeyScan(); OLED_UI_InterruptHandler();`
- 编码器：实验箱无 → 空实现（Encoder_Get 返回 0）

## 4. 踩坑清单（新会话必读，都是真金白银）

1. **OLED_UI.c/.h 曾被 python GBK 写坏**：`rb` 读 + 文本写 → 文件变空/混入 U+FFFD。**处理 GBK 文件必须 rb/wb + 同编码**；恢复方法 = 从上游重拷 + 重做兼容化（§2 清单）。**建议以后把 OLED_UI.c 的兼容化改动提交 git（本会话已提交，勿再丢失）**
2. **C51 声明位置规则比想象的严**：不仅"块内任意位置"不行，**连"声明在赋值语句之后"都报错 C141**（expected '__asm'）——声明必须全部提到函数/块开头，赋值放后面。for 初始化区声明（`for(MenuID i = 0;`）同样不行
3. **C51 结构体初始化限制**：声明时不能 `= 结构体对象`（C248，如 `OLED_ChangePoint t = 全局结构体`）也不能 `= 函数返回值`（C248）——必须声明+赋值分离
4. **flash 超 64KB 是真实风险**：完整字库 29.1KB + 框架 code 41.3KB = 70.7KB。**未引用的 const 数组不会被 REMOVEUNUSED 移除**（它只移除未引用函数）——必须手动删定义+声明。本会话删：Gif_cube(9.6K)、LOGO×5(2K)、alipay_QR(2K)、32 版图片×6(0.8K) → const 20.8KB。**20x20/10x20 字体被 OLED.c 的字体选择代码引用，保留**
5. **OLED_Fonts.c 数据用大写 `0X`**（不是 0x）——正则/脚本统计字节数时注意
6. **OLED.h 是 LF 行尾**（不是 CRLF）；其 OLED_WIDTH/HEIGHT 宏会与 OLED_driver.h 重复定义（C317）——已加 #ifndef 保护
7. **PRINTF_LARGE 必须在 C51 Define 里配**（vsprintf/%f/%.2f 需要；printf_small 无 vsprintf）
8. **文件顶部 include 必须在 fmin 宏/patterns 数组之前**（uint8_t 未定义 → C129 missing ';'）
9. 剩余 4 Warning 可接受：L15 MULTIPLE CALL ×2（Timer0 中断与主循环共享 GetMenuItemNum/GetWindowDataStyle，框架设计使然，Keil 自动禁止 overlay 保护）、L25 DATA TYPES DIFFERENT ×2（OLED_UI_init/start 的 void 参数表编码差异，无害）
10. 其余沿用旧清单（STC8H.H、c51lib.h、bit/code 关键字、extern 匹配、OLED 方向、OLED_DisplayBuf xdata、uvproj MODP2 等）

## 5. 下一步（上板验证清单）

1. 烧录 Objects/Project.hex（STC-ISP，IRC 24MHz）
2. 验证磁贴主屏 6 项显示 + 文字图标正常、方向正确（0xA1+0xC8 已配置）
3. 按键导航：ADC 键盘键值 1/2/3/4 对应上/下/确认/返回——**若键位不对，调 config.h 的 KEY_ADC_***（实验箱 16 键布局，官方 17 号例程的键值 1-16 顺序）
4. 验证：进入设置（亮度窗口 +/-、深浅色、显示帧率开关）、返回动画、长按加速
5. 若方向/镜像有问题：左右改 0xA0↔0xA1、上下改 0xC0↔0xC8（OLED_driver.c）
6. 提交验证结果 + 更新文档

## 6. 构建与验证命令

```bash
# 构建（命令行，无需打开 Keil）
"D:/APP/C51V961/UV4/UV4.exe" -b "Project.uvproj" -o "build_log.txt" && sleep 25 && grep -E "error|Program Size" build_log.txt
# 产物：Objects/Project.hex（STC-ISP 烧录，IRC 24MHz）

# git（dev 分支）
git add -A && git commit -m "..." && git push origin dev
```

## 7. 参考资料位置

- 上游框架镜像：`D:/Document/本科/单片机课程/OLED_UI_upstream/`（OLED_UI_Core/HAL/OLED_UI_Core/）
- 官方例程：`D:/edge_download/STC8H8K64U-DEMO-CODE-V9.6/`（17=ADC键盘、36.2=ST7920、69=I2C OLED 双后端）
- 实验箱原理图：`.../SCH_实验箱9.62A_2026-1-27.pdf`（J10/J11/R173-R176）
- 项目文档：`项目介绍.md`（v0.3）、`可行性评估报告.md`（OLED v3）
