/*==============================================================================
 * timeedit.h —— 时间编辑纯函数（无硬件依赖，C51/gcc 双编译）
 * 供"时间设置"页编辑状态机使用（调整字段值 / 字段循环）
 *============================================================================*/
#ifndef __TIMEEDIT_H
#define __TIMEEDIT_H

#include "stdint.h"
#include "RTC.h"

/* 字段编号：0=年 1=月 2=日 3=时 4=分 5=秒 */
#define TE_FIELD_YEAR   0
#define TE_FIELD_MONTH  1
#define TE_FIELD_DAY    2
#define TE_FIELD_HOUR   3
#define TE_FIELD_MIN    4
#define TE_FIELD_SEC    5

/* 对指定字段做 ±1 调整（钳制合法范围：年 2000~2099，月 1~12，日 1~31，时分秒按进制）
 * 返回 1=值有变化，0=已到边界不变 */
uint8_t TimeEdit_Inc(RTC_Time *t, uint8_t field, int8_t delta);

/* 下一个字段（0→1→...→5→0 循环） */
uint8_t TimeEdit_NextField(uint8_t field);

#endif
