/*==============================================================================
 * RTC.h —— STC8H 内部 RTC 驱动（外部 32.768K 晶振，官方 50 号例程移植）
 * 寄存器为 xdata 扩展寄存器（0xFE60~0xFE76），需 P_SW2 |= 0x80 访问
 *============================================================================*/
#ifndef __RTC_H
#define __RTC_H

#include "stdint.h"

/* 时间结构体（十进制） */
typedef struct {
    uint16_t year;    /* 完整年份 2000~2099 */
    uint8_t  month;   /* 1~12 */
    uint8_t  day;     /* 1~31 */
    uint8_t  hour;    /* 0~23 */
    uint8_t  minute;  /* 0~59 */
    uint8_t  second;  /* 0~59 */
} RTC_Time;

/* 初始化 RTC：启动外部 32K 晶振 + 使能 RTC（需在 P_SW2|=0x80 之后调用） */
void RTC_Init(void);

/* 写入时间（十进制 → BCD 写 INI 寄存器 + 触发初始化） */
void RTC_SetTime(const RTC_Time *t);

/* 读取当前时间（BCD 寄存器 → 十进制） */
void RTC_GetTime(RTC_Time *t);

#endif
