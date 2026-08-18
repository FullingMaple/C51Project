/*==============================================================================
 * calendar.h —— 日历算法纯函数（无硬件依赖，C51/gcc 双编译）
 * 供万年历页（日历表 + 星期）与 RTC 时间转换使用
 *============================================================================*/
#ifndef __CALENDAR_H
#define __CALENDAR_H

#include "stdint.h"

/* 判断闰年（1=闰年） */
uint8_t Cal_IsLeap(uint16_t year);

/* 某月天数（1~12 月，处理闰年 2 月） */
uint8_t Cal_DaysInMonth(uint16_t year, uint8_t month);

/* 蔡勒公式：年月日 → 星期（周一=1 ... 周日=7）
 * year 为完整年份（如 2026），month 1~12，day 1~31 */
uint8_t Cal_Weekday(uint16_t year, uint8_t month, uint8_t day);

#endif
