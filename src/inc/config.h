/*==============================================================================
 * config.h —— 引脚/按键/显示后端统一配置
 * 项目：电子手表风格 UI 菜单系统（STC8H8K64U 实验箱 9.62 + 0.96" SSD1306 OLED）
 *============================================================================*/
#ifndef CONFIG_H
#define CONFIG_H

#include "stdint.h"
#include "stdbool.h"
#include "stc8h.h"              /* 寄存器定义（sbit 引脚声明依赖） */

#define MAIN_Fosc   24000000L

/* ================= 显示后端选择 =================
 * 1 = 虚拟 OLED（AiCube-ISP：工具→调试仿真接口→OLED-128*64，经 USB-CDC）
 *     开发期无实体屏调试全部 UI 逻辑；帧率数字失真，性能须实体屏验证
 * 0 = 实体 OLED（软件 I2C，SSD1306，SCL=P2.5 / SDA=P2.4 / 0x78）
 */
#define VIRTUAL_OLED    0

/* ================= SSD1306 OLED（软件 I2C）================= */
#define OLED_ADDR   0x78        /* 0x3C<<1；模块焊盘在 0x7A 侧则改 0x7A */
sbit OLED_SCL = P2^5;           /* 时钟 */
sbit OLED_SDA = P2^4;           /* 数据 */
/* P2.2~P2.5 需配开漏 + 外部上拉（实验箱已接 10K 到 3.3V）*/
#define OLED_IO_MODE()  do { P2M1 |= 0x3c; P2M0 |= 0x3c; } while(0)

/* ================= 16 键 ADC 键盘 =================
 * 占位键值，按实验箱 16 键实际布局调整（见上机实测清单）
 * 应用态（计算器/游戏）直读原始键值 1~16，与菜单态 4 键不冲突
 */
#define KEY_ADC_UP      1
#define KEY_ADC_DOWN    2
#define KEY_ADC_ENTER   3
#define KEY_ADC_BACK    4

#endif
