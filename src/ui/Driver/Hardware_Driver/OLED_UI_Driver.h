/*==============================================================================
 * OLED_UI_Driver.h —— 平台层驱动接口（C51 版）
 * Timer0 20ms 节拍 + 16 键 ADC 键盘（P1.0）+ 空编码器 + 软延时
 * 实现见 OLED_UI_Driver.c
 *============================================================================*/
#ifndef __OLED_UI_DRIVER_H
#define __OLED_UI_DRIVER_H

#include "config.h"

void     Timer_Init(void);          /* Timer0 20ms 中断初始化 */
void     Key_Init(void);            /* 16 键 ADC 键盘初始化（P1.0=ADC0） */
void     Encoder_Init(void);        /* 编码器：实验箱无，空实现 */
void     Encoder_Enable(void);
void     Encoder_Disable(void);
int16_t  Encoder_Get(void);         /* 恒返回 0 */

uint8_t  Key_GetUpStatus(void);     /* 0=按下 1=松开 */
uint8_t  Key_GetDownStatus(void);
uint8_t  Key_GetEnterStatus(void);
uint8_t  Key_GetBackStatus(void);

void     Delay_ms(uint16_t xms);    /* 软件延时（NOP 循环近似） */
void     Delay_s(uint16_t xs);

uint32_t GetTick(void);             /* 系统毫秒计数（Timer0 20ms 节拍 ×20） */

/* ---- Timer0 中断内调用（见 OLED_UI_Launcher.c 的 Timer0_Isr）---- */
void     Driver_TickHandler(void);  /* tick 计数递增 */
void     Driver_KeyScan(void);      /* ADC 键盘采样 + 三态滤波 */

#endif
