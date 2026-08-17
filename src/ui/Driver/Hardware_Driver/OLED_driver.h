/*==============================================================================
 * OLED_driver.h —— SSD1306 驱动接口（对齐上游 OLED_UI 框架）
 * 后端：0 = 实体 OLED（STC8H 硬件 I2C，P2.5/P2.4，0x78）
 *       1 = 虚拟 OLED（AiCube-ISP 调试仿真接口，经 USB-CDC）
 *       由 config.h 的 VIRTUAL_OLED 选择
 *============================================================================*/
#ifndef __OLED_DRIVER_H
#define __OLED_DRIVER_H

#include "config.h"

#ifndef OLED_WIDTH
#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_PAGES   8
#endif

/* 帧缓冲由图形层（OLED.c）定义，驱动层 extern 引用 */
extern uint8_t xdata OLED_DisplayBuf[OLED_PAGES][OLED_WIDTH];

void OLED_Init(void);
extern uint16_t OLED_I2CTimeout;             /* 诊断：I2C 超时次数 */
void OLED_ShadowClear(void);                    /* 清影子缓冲（供 OLED_Clear 同步，避免 diff 全屏重发） */
void OLED_Update(void);                              /* 全屏刷新（diff 只发变化页） */
void OLED_UpdateArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height); /* 局部刷新 */
void OLED_SetColorMode(bool colormode);              /* 1=反色(0xA7) 0=正常(0xA6)；虚拟端无操作 */

#endif
