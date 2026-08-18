/* test_calendar.c —— 日历算法主机单测（gcc）
 * 编译：gcc tests/test_calendar.c src/ui/Driver/Hardware_Driver/calendar.c -I src/ui/Driver/Hardware_Driver -o /tmp/tc && /tmp/tc */
#include <stdio.h>
#include <assert.h>
#include "calendar.h"

int main(void)
{
    /* 每月天数（闰年） */
    assert(Cal_DaysInMonth(2024, 2) == 29);   /* 闰年 */
    assert(Cal_DaysInMonth(2025, 2) == 28);
    assert(Cal_DaysInMonth(2026, 2) == 28);
    assert(Cal_DaysInMonth(2024, 12) == 31);
    assert(Cal_DaysInMonth(2025, 4) == 30);
    assert(Cal_DaysInMonth(2026, 8) == 31);   /* 当前月 */

    /* 蔡勒公式星期（周一=1 ... 周日=7） */
    assert(Cal_Weekday(2026, 8, 18) == 2);    /* 2026-08-18 是周二 */
    assert(Cal_Weekday(2026, 8, 17) == 1);    /* 周一 */
    assert(Cal_Weekday(2026, 8, 23) == 7);    /* 周日 */
    assert(Cal_Weekday(2026, 8, 1)  == 6);    /* 2026-08-01 周六 */
    assert(Cal_Weekday(2026, 1, 1)  == 4);    /* 2026-01-01 周四 */
    assert(Cal_Weekday(2024, 2, 29) == 4);    /* 闰日 */
    assert(Cal_Weekday(2000, 1, 1)  == 6);    /* 2000-01-01 周六 */
    assert(Cal_Weekday(2026, 12, 31) == 4);   /* 年末 */

    /* 日历表需要：某月 1 号星期 → 排格子 */
    assert(Cal_Weekday(2026, 5, 1) == 5);     /* 2026-05-01 周五（31 天跨 6 行场景） */

    printf("ALL CALENDAR TESTS PASSED\n");
    return 0;
}
