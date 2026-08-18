/* test_timeedit.c —— 时间编辑状态机主机单测（gcc）
 * 语义：字段调整采用循环回绕（电子表式）——
 *   到边界后按同方向继续即回绕：hour 0 按"下"→23，23 按"上"→0；分/秒 0↔59；
 *   月 1↔12、日 1↔31、年 2000↔2099。
 * 编译：gcc tests/test_timeedit.c src/ui/Driver/Hardware_Driver/timeedit.c -I src/ui/Driver/Hardware_Driver -o /tmp/tte && /tmp/tte
 */
#include <stdio.h>
#include <assert.h>
#include "timeedit.h"

int main(void)
{
    RTC_Time t;

    /* ---- 年 2000~2099 循环回绕 ---- */
    t = (RTC_Time){2099, 8, 18, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 0, 1) == 1 && t.year == 2000);   /* 2099 按"上"回绕到 2000 */
    assert(TimeEdit_Inc(&t, 0, -1) == 1 && t.year == 2099);  /* 2000 按"下"回绕到 2099 */
    t = (RTC_Time){2000, 1, 1, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 0, -1) == 1 && t.year == 2099);
    assert(TimeEdit_Inc(&t, 0, 1) == 1 && t.year == 2000);

    /* ---- 月 1~12 循环回绕 ---- */
    t = (RTC_Time){2026, 12, 18, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 1, 1) == 1 && t.month == 1);     /* 12 按"上"回绕到 1 */
    assert(TimeEdit_Inc(&t, 1, -1) == 1 && t.month == 12);   /* 1 按"下"回绕到 12 */
    t = (RTC_Time){2026, 1, 18, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 1, -1) == 1 && t.month == 12);

    /* ---- 日 1~31 循环回绕（简单回绕，不联动月） ---- */
    t = (RTC_Time){2026, 2, 31, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 2, 1) == 1 && t.day == 1);       /* 31 按"上"回绕到 1 */
    assert(TimeEdit_Inc(&t, 2, -1) == 1 && t.day == 31);     /* 1 按"下"回绕到 31 */
    t = (RTC_Time){2026, 8, 1, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 2, -1) == 1 && t.day == 31);

    /* ---- 时 0~23 / 分秒 0~59 循环回绕 ---- */
    t = (RTC_Time){2026, 8, 18, 23, 59, 59};
    assert(TimeEdit_Inc(&t, 3, 1) == 1 && t.hour == 0);      /* 23 按"上"回绕到 0 */
    assert(TimeEdit_Inc(&t, 4, 1) == 1 && t.minute == 0);    /* 59 按"上"回绕到 0 */
    assert(TimeEdit_Inc(&t, 5, 1) == 1 && t.second == 0);    /* 59 按"上"回绕到 0 */
    t = (RTC_Time){2026, 8, 18, 0, 0, 0};
    assert(TimeEdit_Inc(&t, 3, -1) == 1 && t.hour == 23);    /* 0 按"下"回绕到 23（用户期望） */
    assert(TimeEdit_Inc(&t, 4, -1) == 1 && t.minute == 59);  /* 0 按"下"回绕到 59 */
    assert(TimeEdit_Inc(&t, 5, -1) == 1 && t.second == 59);  /* 0 按"下"回绕到 59 */
    assert(TimeEdit_Inc(&t, 3, 1) == 1 && t.hour == 0);      /* 23 按"上"回绕到 0 */

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
