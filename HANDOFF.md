# HANDOFF —— 2026-08-15 会话交接

> 用途：v0.3 OLED_UI 框架 C51 移植进度交接。新会话先读此文件，再读 `项目介绍.md` 与 `可行性评估报告.md`。
> 环境：Keil C51 9.60（D:/APP/C51V961）、STC8H8K64U 实验箱 9.62、0.96" SSD1306 OLED（硬件 I2C：SCL=P2.5、SDA=P2.4、0x78）、git 仓库（main + dev 分支，GitHub: FullingMaple/C51Project）。

---

## 1. 当前总体状态

**v0.3 里程碑（屏幕切换 OLED）✅ 完成：驱动 + 框架 + 平台层全部就绪，实体屏验证通过——静态帧率 80+，动画 30+，按键正常。**

| 模块 | 状态 |
|---|---|
| SSD1306 驱动（OLED_driver.c） | ✅ 实体屏点亮；硬件 I2C **400kHz**（I2CCFG=0xCD）；diff 逐页刷新；**总线自动恢复**（超时+GPIO 9 脉冲+重初始化） |
| Keil 工程（Project.uvproj） | ✅ 0 Error / 6 Warning（L13 递归静态检测×4 + L15 中断共享×2，均可接受），flash 61.6KB/64KB |
| 图形层 OLED.c | ✅ C51 兼容；**ShowImageArea 整列快路径 + ClearArea 页级掩码**（绘制 8 倍加速） |
| 字库 OLED_Fonts.c/.h | ✅ 裁剪未引用（Gif/LOGO/QR 等省 8.8KB）+ **补 29 字**（SimSun 点阵渲染，与原字库风格一致） |
| 框架核心 OLED_UI.c | ✅ 从上游重拷 + 39 处 C51 兼容化（块内声明提升、for 初始化区、C248、fmin/vsnprintf、patterns static code、HAL 删除、ShowFps 撞名改名） |
| OLED_UI.h | ✅ 从上游重拷修复（原文件被 python 写坏） |
| 菜单数据 OLED_UI_MenuData.c | ✅ 本项目菜单（磁贴 6 项：设置/关于/测温/串口/计算器/游戏）；List_MenuArea 已填实际区域；磁贴动画 UNLINEAR（无过冲） |
| 平台层 OLED_UI_Driver.c/.h | ✅ Timer0 20ms（12T）+ 16 键 ADC 键盘（P1.0 三态滤波）+ 空编码器 + 软延时 + GetTick + 总线恢复 |
| main.c | ✅ SP 不手动设（C51 运行库 0x22 已设）；OLED_UI_init/start + EA=1 |
| 帧率显示 | ✅ 默认开启（OLED_UI_FpsShow=true），设置页可关 |

## 2. 实测性能（用户上板确认，最终版 2026-08-15）

- **静态 80+fps、动画 35fps**（I2C 400kHz 整页 diff + 周期全刷 + 总线恢复，最终确认稳定）
- **按键正常**（上下/进入/返回），进入设置/关于页 ~1.5s（SPEED 8）
- **虚拟屏**（VIRTUAL_OLED=1）：显示正常（整屏发送 + 2 帧节流 + bUsbInBusy 忙跳过）
- **功能图标**：6 磁贴手绘图标（齿轮/温度计/DB9/计算器/手柄/关于）

## 2b. "按进入卡死"完整复盘（本次会话最重要的教训）

**症状演进**：400kHz 提速后按进入卡死 → 总线恢复修复（治标）→ 列级 diff 后复发 → 273kHz 稳定 → 400kHz 最终成功。

**根因**：400kHz 是 SSD1306 规格上限，实验箱**飞线条件下 I2C 时序余量不足，偶发事务失败一直存在**（R 计数诊断证实：静态下 I2C 就在持续超时，超时率 ~1/3）。"卡死"真相 = 每事务 2ms 超时 × 渐隐全刷大量事务 = 每帧几十毫秒 → 主循环被拖到 7-10 秒/帧。

**教训**：
1. **列级 diff（0x21 短事务）放大事务次数 → 失败频率超过容错能力**——整页 diff 事务少，400kHz 可稳定；列级 diff 必须与降速搭配
2. **容错 ≠ 根治**：总线恢复只是"失败能爬回来"，信号余量不足的病因要靠降速/硬件（SPI）解决
3. **诊断方法**：MainLoop D-A/B/C/D 标记定位卡死阶段 + **I2C 超时计数显示（R 值）确诊失败率**——不猜，用数据
4. **当前最优配置**：400kHz（0xCD）+ 整页 diff + 每 50 帧周期全刷 + 总线恢复（超时 2ms + 9 脉冲 + 重初始化）
5. **兜底方案**：若 400kHz 再出问题 → 273kHz（0xD4，MSSPEED=20）动画 30fps 稳定；或等电烙铁切 SPI（根治）

## 3. 本次会话踩坑（真金白银，新会话必读）

1. **OLED_UI.c 曾被写空、OLED_UI.h 被写坏**（python GBK 写失败）——已从上游重拷并重新兼容化（提交 7f14906）。**处理 GBK 文件必须 rb/wb + 同编码 + 统一 LF**；**不要用 Edit 工具改 GBK 文件**（会把中文注释写坏——OLED.c 因此坏过一次，git checkout 恢复后改用 python）
2. **I2CCFG 的 bit5-0 是 MSSPEED[5:0]（等待时钟数）**：速度 = FOSC/2/(MSSPEED×2+4)。**0xe0 在 24MHz 只有 176kHz**（官方 0xe0 是针对 11.0592MHz 的 100kHz 配置）！**400kHz 需 MSSPEED=13 → 0xCD**。参考：STC8H 硬件 IIC（CSDN）、STC 官方论坛
3. **I2C 总线偶发失败会"卡死"**：每个事务 13ms 超时（40000 次空循环）累积 → 画面冻结（诊断定位 D-D = OLED_Update）。**修复**：超时缩短 6000（~2ms）+ 连续失败 5 次触发 **I2C_BusRecover**（GPIO 9 脉冲释放从机 + 重新初始化 SSD1306，I2C_Recovering 防递归）——注意 I2C_Wait 调用 BusRecover 需要前向声明
4. **C51 声明位置**：块内任意位置声明报错；**连"赋值语句后声明"都报 C141**；for 初始化区声明不支持；结构体不能声明时用对象/函数返回值初始化（C248）
5. **未引用 const 数组不会被 REMOVEUNUSED 移除**——必须手动删（字库裁剪省 8.8KB 才装进 64KB）
6. **补字模**：SimSun 在 16px/12px 有内嵌点阵——大画布渲染 + bbox 居中 + **1:1 不缩放**（缩放有灰阶 → 笔画断/糊）。格式：**列式**（每列垂直 8bit 一字节，bit0 在顶；先全部列上半再全部列下半）
7. **ClearArea 页级掩码**：rows 必须限制在本页剩余行（min(8-j%8, y_end-j)），否则跨页清错位 → 界面残留（曾误判为按键失效）
8. **SP 不要手动设 0x80**：C51 运行库 ?C_C51STARTUP 已设 SP=0x22（栈 221B），手动 0x80 砍栈到 128B 深调用易溢出
9. **按键失效的排查规律**：改刷新/绘制路径（OLED.c/OLED_driver.c）后按键异常 → 先查 ClearArea/绘制函数的内存问题（越界/清错位），不要先怀疑按键链
10. **诊断方法**：MainLoop 加 D-A/B/C/D 标记定位卡死点（卡在 D-D = OLED_Update/I2C）；帧率判断用 P 值（刷新页数）区分 I2C/CPU 瓶颈
11. **LCD1602 打印调试**：无串口/无仿真器时可用 OLED 显示调试信息（FPS 位置 110,0）

## 3b. 本会话后半段新踩坑（虚拟屏 + 图标 + 卡死复盘）

1. **虚拟屏按进入卡死**：USB-CDC 库 `OLED12864_ShowPicture` 是**阻塞式发送**（等 EP 缓冲），渐隐每帧整屏 1024B、主机读取不及时 → 死等。修复：**整屏发送 + 每 2 帧节流 + bUsbInBusy 忙跳过**（忙则丢帧不阻塞）
2. **虚拟屏 OLED12864_ShowPicture 的 y/cy 参数语义未知**（库头无注释）——diff 页发送（`(0, page*8, 128, 1)`）导致显示错乱——**只信任整屏调用 (0,0,128,8)**
3. **Edit 工具会破坏 GBK 文件**（中文注释写坏，decode 失败）——**GBK 文件一律用 python rb/wb + 同编码处理**；main.c 被 Edit 改成了 UTF-8（编译无碍但 python 处理要用 UTF-8）
4. **printf 的 %d 读 uint8_t/uint16_t 显示错位**（0x40xx 恒定高位）——诊断计数显示用 uint16_t + extern 类型同步
5. **手绘图标**（PIL）：32×32 列式字模（先上半页再下半页）；齿轮用 8 径向梯形齿 + 细环壁（emoji 风格）；emoji 字符（🔧⚙️）不能出现在 GBK 文件/print 里（编码错误）
6. **SPEED 4→8**：页面切换动画从 3-5 秒降到 ~1.5s（UNLINEAR 收敛帧数减半）
7. **帧率显示默认开启**：OLED_UI_FpsShow 默认 true

## 4. 剩余事项（可选）

1. 动画帧率 30fps 已到 SSD1306 规格上限（400kHz）——若想更高：减少动画期全屏刷新（区域刷新）或接受现状
2. 6 个 Warning 可接受（L13 递归为静态检测，运行时有 I2C_Recovering 重入保护；L15 中断共享为框架架构）
3. 设置页"语言/提示音"占位项未实现功能（回调为 NULL）
4. "测温/串口"磁贴无子菜单（按进入无反应是预期行为——需要的话加子菜单/回调）
5. 字库仍是子集（~600 字）——菜单新增汉字需补字模（见 §3.6 方法）
6. 提交 git 后建议打 tag（v0.3）

## 5. 构建与验证命令

```bash
# 构建（命令行，无需打开 Keil）
"D:/APP/C51V961/UV4/UV4.exe" -b "Project.uvproj" -o "build_log.txt" && sleep 25 && grep -E "error|Program Size" build_log.txt
# 产物：Objects/Project.hex（STC-ISP 烧录，IRC 24MHz）

# git（dev 分支）
git add -A && git commit -m "..." && git push origin dev
```

## 6. 参考资料位置

- 上游框架镜像：`D:/Document/本科/单片机课程/OLED_UI_upstream/`（OLED_UI_Core/HAL/OLED_UI_Core/）
- 官方例程：`D:/edge_download/STC8H8K64U-DEMO-CODE-V9.6/`（17=ADC键盘、28=I2C主机、69=I2C OLED 双后端）
- 实验箱原理图：`课程资料/单片机/STC8H8K64U-DEMO-CODE-V9.6/实验箱9.62_2023-4-25-SCH.pdf`
- STC8H 手册（I2CCFG 定义）：https://www.stcmicro.com/datasheet/STC8C-cn.pdf ；STC 官方论坛 https://www.stcaimcu.com
- 项目文档：`项目介绍.md`（v0.3）、`可行性评估报告.md`（OLED v3）
