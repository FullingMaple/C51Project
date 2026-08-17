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

/* 实体 OLED 接口选择：
 * OLED_IF_SPI = 1 → SPI（SCLK=P2.5 MOSI=P2.3 RES=P2.4 DC=P3.4 CS=P1.1，
 *                  实验箱出厂 R173/R174 焊好，7 孔座直插，无需改焊）
 * OLED_IF_SPI = 0 → 硬件 I2C（SCL=P2.5 SDA=P2.4，需焊 R175/R176 或飞线）
 */
#define OLED_IF_SPI     1

/* ================= SSD1306/SSD1315 OLED ================= */
#define OLED_ADDR   0x78        /* I2C 模式：0x3C<<1；模块焊盘在 0x7A 侧则改 0x7A */

#if(OLED_IF_SPI)
/* SPI 模式引脚（官方 69 号例程 SPI 版：SPI1 引脚组 1） */
sbit OLED_SCLK = P2^5;          /* 时钟 D0/SCLK */
sbit OLED_SDIN = P2^3;          /* 数据 D1/MOSI */
sbit OLED_RST  = P2^4;          /* 复位 RES（GPIO） */
sbit OLED_DC   = P3^4;          /* 数据/命令 DC（GPIO） */
sbit OLED_CS   = P1^1;          /* 片选 CS（GPIO） */
/* SPI 引脚推挽输出 */
#define OLED_IO_MODE()  do {     P2M1 &= ~0x38; P2M0 |= 0x38;   /* P2.3/P2.4/P2.5 推挽 */     P3M1 &= ~0x10; P3M0 |= 0x10;   /* P3.4 推挽 */     P1M1 &= ~0x02; P1M0 |= 0x02;   /* P1.1 推挽 */ } while(0)
#else
/* I2C 模式引脚（P2.2~P2.5 开漏 + 实验箱 10K 上拉） */
sbit OLED_SCL = P2^5;           /* 时钟 */
sbit OLED_SDA = P2^4;           /* 数据 */
#define OLED_IO_MODE()  do { P2M1 |= 0x3c; P2M0 |= 0x3c; } while(0)
#endif

/* ================= 16 键 ADC 键盘 =================
 * 占位键值，按实验箱 16 键实际布局调整（见上机实测清单）
 * 应用态（计算器/游戏）直读原始键值 1~16，与菜单态 4 键不冲突
 */
#define KEY_ADC_UP      1
#define KEY_ADC_DOWN    2
#define KEY_ADC_ENTER   3
#define KEY_ADC_BACK    4

#endif
