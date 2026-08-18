/*==============================================================================
 * timeedit.c —— 时间编辑纯函数实现（C89，无硬件依赖）
 *============================================================================*/
#include "timeedit.h"

uint8_t TimeEdit_Inc(RTC_Time *t, uint8_t field, int8_t delta)
{
    if(delta > 0)
    {
        switch(field)
        {
            case TE_FIELD_YEAR:  if(t->year   < 2099){ t->year++;   return 1; } return 0;
            case TE_FIELD_MONTH: if(t->month  < 12)  { t->month++;  return 1; } return 0;
            case TE_FIELD_DAY:   if(t->day    < 31)  { t->day++;    return 1; } return 0;
            case TE_FIELD_HOUR:  if(t->hour   < 23)  { t->hour++;   return 1; } return 0;
            case TE_FIELD_MIN:   if(t->minute < 59)  { t->minute++; return 1; } return 0;
            case TE_FIELD_SEC:   if(t->second < 59)  { t->second++; return 1; } return 0;
        }
    }
    else if(delta < 0)
    {
        switch(field)
        {
            case TE_FIELD_YEAR:  if(t->year   > 2000){ t->year--;   return 1; } return 0;
            case TE_FIELD_MONTH: if(t->month  > 1)   { t->month--;  return 1; } return 0;
            case TE_FIELD_DAY:   if(t->day    > 1)   { t->day--;    return 1; } return 0;
            case TE_FIELD_HOUR:  if(t->hour   > 0)   { t->hour--;   return 1; } return 0;
            case TE_FIELD_MIN:   if(t->minute > 0)   { t->minute--; return 1; } return 0;
            case TE_FIELD_SEC:   if(t->second > 0)   { t->second--; return 1; } return 0;
        }
    }
    return 0;
}

uint8_t TimeEdit_NextField(uint8_t field)
{
    return (field < TE_FIELD_SEC) ? (uint8_t)(field + 1) : TE_FIELD_YEAR;
}
