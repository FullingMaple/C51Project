/*==============================================================================
 * timeedit.c —— 时间编辑纯函数实现（C89，无硬件依赖）
 * 字段调整采用循环回绕（电子表式）：到边界后同方向继续即回绕——
 *   hour 0 按"下"→23、23 按"上"→0；分/秒 0↔59；月 1↔12；日 1↔31；年 2000↔2099
 *============================================================================*/
#include "timeedit.h"

uint8_t TimeEdit_Inc(RTC_Time *t, uint8_t field, int8_t delta)
{
    if(delta > 0)
    {
        switch(field)
        {
            case TE_FIELD_YEAR:  t->year   = (t->year   < 2099) ? (uint16_t)(t->year + 1)  : 2000; return 1;
            case TE_FIELD_MONTH: t->month  = (t->month  < 12)   ? (uint8_t)(t->month + 1)  : 1;   return 1;
            case TE_FIELD_DAY:   t->day    = (t->day    < 31)   ? (uint8_t)(t->day + 1)    : 1;   return 1;
            case TE_FIELD_HOUR:  t->hour   = (t->hour   < 23)   ? (uint8_t)(t->hour + 1)   : 0;   return 1;
            case TE_FIELD_MIN:   t->minute = (t->minute < 59)   ? (uint8_t)(t->minute + 1) : 0;   return 1;
            case TE_FIELD_SEC:   t->second = (t->second < 59)   ? (uint8_t)(t->second + 1) : 0;   return 1;
        }
    }
    else if(delta < 0)
    {
        switch(field)
        {
            case TE_FIELD_YEAR:  t->year   = (t->year   > 2000) ? (uint16_t)(t->year - 1)  : 2099; return 1;
            case TE_FIELD_MONTH: t->month  = (t->month  > 1)    ? (uint8_t)(t->month - 1)  : 12;   return 1;
            case TE_FIELD_DAY:   t->day    = (t->day    > 1)    ? (uint8_t)(t->day - 1)    : 31;   return 1;
            case TE_FIELD_HOUR:  t->hour   = (t->hour   > 0)    ? (uint8_t)(t->hour - 1)   : 23;   return 1;
            case TE_FIELD_MIN:   t->minute = (t->minute > 0)    ? (uint8_t)(t->minute - 1) : 59;   return 1;
            case TE_FIELD_SEC:   t->second = (t->second > 0)    ? (uint8_t)(t->second - 1) : 59;   return 1;
        }
    }
    return 0;
}

uint8_t TimeEdit_NextField(uint8_t field)
{
    return (field < TE_FIELD_SEC) ? (uint8_t)(field + 1) : TE_FIELD_YEAR;
}
