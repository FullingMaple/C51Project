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

/* 日历网格格子定位（万年历页布局）：
 * 表头/格子统一 13px 节距，格子原点 x=18, y=24，行距 8px；
 * 返回数字绘制起点（格内水平居中：1 位数左移 +3px，2 位数左移 0，中心与表头汉字对齐）；
 * 第 6 行（row>=5）溢出画第 5 行右侧：y=56，x=110+(day-(37-w1))*6（6px 节距）。
 * 输出 *x/*y 均为屏幕内坐标（保证 x+12<=128 且 y+8<=64）。
 * day 1~31，dim 当月天数，w1 = Cal_Weekday(year,month,1)（1~7） */
void Cal_GetCellPos(uint8_t day, uint8_t dim, uint8_t w1, int16_t *x, int16_t *y);

#endif
