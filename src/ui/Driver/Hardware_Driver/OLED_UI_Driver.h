/*==============================================================================
 * OLED_UI_Driver.h —— 平台层驱动接口（C51 版）
 * Timer0 20ms + 16 键 ADC 键盘（实现待接入，见 OLED_UI_Driver.c）
 *============================================================================*/
#ifndef __OLED_UI_DRIVER_H
#define __OLED_UI_DRIVER_H

#include "config.h"

void Timer_Init(void);          /* Timer0 20ms 中断（OLED_UI_InterruptHandler 节拍） */
void Key_Init(void);            /* 16 键 ADC 键盘初始化 */
void Delay_ms(uint16_t xms);
void Delay_s(uint16_t xs);

#endif
