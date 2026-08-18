/*==============================================================================
 * calendar.c —— 日历算法实现（C89 纯函数，无硬件依赖）
 *============================================================================*/
#include "calendar.h"

#ifdef __C51__
#define CAL_CODE code          /* C51：字模/常量表放 Flash */
#else
#define CAL_CODE               /* gcc：普通常量 */
#endif

/* 判断闰年：能被 4 整除且（不能被 100 整除 或 能被 400 整除） */
uint8_t Cal_IsLeap(uint16_t year)
{
    if((year % 4) != 0) return 0;
    if((year % 100) == 0 && (year % 400) != 0) return 0;
    return 1;
}

/* 每月天数（1~12 月） */
uint8_t Cal_DaysInMonth(uint16_t year, uint8_t month)
{
    static const uint8_t CAL_CODE Days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if(month < 1 || month > 12) return 0;
    if(month == 2 && Cal_IsLeap(year)) return 29;
    return Days[month - 1];
}

/* 蔡勒公式（1582-10-15 之后的格里高利历）：
 *   w = (y + y/4 - y/100 + y/400 + (13m+8)/5 + d) % 7
 * 其中 1、2 月视为上一年的 13、14 月；结果 0=周日
 * 返回：周一=1 ... 周日=7 */
uint8_t Cal_Weekday(uint16_t year, uint8_t month, uint8_t day)
{
    uint16_t y, m, d;
    uint8_t w;

    m = month;
    y = year;
    if(m < 3) { m += 12; y--; }
    d = day;

    w = (uint8_t)((y + y/4 - y/100 + y/400 + (13*m+8)/5 + d) % 7);
    if(w == 0) return 7;      /* 0=周日 */
    return w;
}

/* 日历网格格子定位（布局规格见 calendar.h） */
void Cal_GetCellPos(uint8_t day, uint8_t dim, uint8_t w1, int16_t *x, int16_t *y)
{
    uint8_t idx, row, col;

    (void)dim;   /* 仅用于边界说明；溢出判定用 row 即可 */

    idx = (uint8_t)((w1 - 1) + (day - 1));
    row = idx / 7;
    col = idx % 7;

    if(row >= 5){   /* 第 6 行溢出：画第 5 行右侧（6px 节距，第一个溢出日恒在 x=110）
                     * 首个溢出日 = 37-w1（idx=35 时 day=37-w1），最多 2 天（110/116） */
        *x = (int16_t)(110 + (day - (uint8_t)(37 - w1)) * 6);
        *y = 56;
        return;
    }
    *x = (int16_t)(18 + col * 13 + ((day >= 10) ? 0 : 3));   /* 数字格内居中 */
    *y = (int16_t)(24 + row * 8);
}
