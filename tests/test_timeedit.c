/* test_timeedit.c —— 时间编辑状态机主机单测（gcc）
 * 编译：gcc tests/test_timeedit.c src/ui/Driver/Hardware_Driver/timeedit.c src/ui/Driver/Hardware_Driver/RTC.c -I src/ui/Driver/Hardware_Driver -o /tmp/tte
 * 注：RTC.c 引用 STC8h.h 无法在 gcc 编译 → 用桩代替（见下）
 */
#include <stdio.h>
#include <assert.h>
#include "timeedit.h"

int main(void)
{
    RTC_Time t;

    /* ---- 边界钳制：年 2000~2099 ---- */
    t = (RTC_Time){2099, 8, 18, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 0, 1) == 0);   /* 2099 不能再加 */
    assert(t.year == 2099);
    assert(TimeEdit_Inc(&t, 0, -1) == 1);
    assert(t.year == 2098);
    t = (RTC_Time){2000, 1, 1, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 0, -1) == 0);  /* 2000 不能再减 */
    assert(t.year == 2000);

    /* ---- 月 1~12 ---- */
    t = (RTC_Time){2026, 12, 18, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 1, 1) == 0);
    assert(t.month == 12);
    assert(TimeEdit_Inc(&t, 1, -1) == 1 && t.month == 11);
    t = (RTC_Time){2026, 1, 18, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 1, -1) == 0 && t.month == 1);

    /* ---- 日 1~31（简单钳制，不联动月） ---- */
    t = (RTC_Time){2026, 2, 31, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 2, 1) == 0 && t.day == 31);
    assert(TimeEdit_Inc(&t, 2, -1) == 1 && t.day == 30);
    t = (RTC_Time){2026, 8, 1, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 2, -1) == 0 && t.day == 1);

    /* ---- 时 0~23 / 分秒 0~59 ---- */
    t = (RTC_Time){2026, 8, 18, 23, 59, 59};
    assert(TimeEdit_Inc(&t, 3, 1) == 0 && t.hour == 23);
    assert(TimeEdit_Inc(&t, 4, 1) == 0 && t.minute == 59);
    assert(TimeEdit_Inc(&t, 5, 1) == 0 && t.second == 59);
    t = (RTC_Time){2026, 8, 18, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 3, -1) == 0 && t.hour == 0);
    assert(TimeEdit_Inc(&t, 4, -1) == 0 && t.minute == 0);
    assert(TimeEdit_Inc(&t, 5, -1) == 0 && t.second == 0);
    assert(TimeEdit_Inc(&t, 3, 1) == 1 && t.hour == 1);

    /* ---- delta=0 不变化 ---- */
    t = (RTC_Time){2026, 8, 18, 12, 30, 30};
    assert(TimeEdit_Inc(&t, 2, 0) == 0 && t.day == 18);

    /* ---- 字段循环 0→1→...→5→0 ---- */
    assert(TimeEdit_NextField(0) == 1);
    assert(TimeEdit_NextField(1) == 2);
    assert(TimeEdit_NextField(2) == 3);
    assert(TimeEdit_NextField(3) == 4);
    assert(TimeEdit_NextField(4) == 5);
    assert(TimeEdit_NextField(5) == 0);   /* 第 6 次切换回到年 → 调用方提交并退出 */

    printf("ALL TIMEEDIT TESTS PASSED\n");
    return 0;
}
