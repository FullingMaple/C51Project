/* test_calendar_layout.c —— 日历网格定位主机单测（gcc）
 * 布局规格：
 *   表头与格子统一 13px 节距；格子原点 x=18, y=24（行距 8px）
 *   数字在 13px 格内水平居中：1 位数左移 +3px，2 位数左移 0（中心对齐表头汉字）
 *   第 6 行（row>=5）溢出画第 5 行右侧：y=56，x=110+(day-30)*6（6px 节距）
 * 编译：gcc tests/test_calendar_layout.c src/ui/Driver/Hardware_Driver/calendar.c -I src/ui/Driver/Hardware_Driver -o /tmp/tcl && /tmp/tcl
 */
#include <stdio.h>
#include <assert.h>
#include "calendar.h"

int main(void)
{
    int16_t x, y;

    /* ---- 常规格（2026-08：w1=6 周六；2026-08-18 周二） ---- */
    /* day 1  → idx=5, row=0, col=5 → x=18+5*13+3(1位居中), y=24 */
    Cal_GetCellPos(1, 31, 6, &x, &y);
    assert(x == 86 && y == 24);
    /* day 2  → idx=6, col=6 → x=18+6*13+3 */
    Cal_GetCellPos(2, 31, 6, &x, &y);
    assert(x == 99 && y == 24);
    /* day 3  → idx=7, row=1, col=0 → x=18+0+3, y=32 */
    Cal_GetCellPos(3, 31, 6, &x, &y);
    assert(x == 21 && y == 32);
    /* day 18（今天）→ idx=22, row=3, col=1 → x=18+13(2位), y=48 */
    Cal_GetCellPos(18, 31, 6, &x, &y);
    assert(x == 31 && y == 48);
    /* day 24 → idx=28, row=4, col=0 → x=18(2位), y=56 */
    Cal_GetCellPos(24, 31, 6, &x, &y);
    assert(x == 18 && y == 56);

    /* ---- 2 位数居中（中心=18+col*13+6 与表头一致） ---- */
    /* 2026-08：day 10 → idx=14, row=2, col=0 → x=18+0(2位), y=40 */
    Cal_GetCellPos(10, 31, 6, &x, &y);
    assert(x == 18 && y == 40);
    /* day 30 → idx=34, row=4, col=6 → x=18+78=96, y=56（2 位，中心 102） */
    Cal_GetCellPos(30, 31, 6, &x, &y);
    assert(x == 96 && y == 56);

    /* ---- 6 行月溢出（row>=5）---- */
    /* 2026-08：day 31 → idx=35, row=5 → 溢出区 x=110, y=56 */
    Cal_GetCellPos(31, 31, 6, &x, &y);
    assert(x == 110 && y == 56);
    /* 周日开头的 31 天月（w1=7）：day 30,31 都溢出 → x=110, 116 */
    Cal_GetCellPos(30, 31, 7, &x, &y);
    assert(x == 110 && y == 56);
    Cal_GetCellPos(31, 31, 7, &x, &y);
    assert(x == 116 && y == 56);
    /* 周六开头的 30 天月（w1=6）：day 30 → idx=34 正常格 */
    Cal_GetCellPos(30, 30, 6, &x, &y);
    assert(x == 96 && y == 56);
    /* 周一开头（w1=1）：31 天全在 5 行内，无溢出（day31 → idx=30, row=4, col=2） */
    Cal_GetCellPos(31, 31, 1, &x, &y);
    assert(x == 44 && y == 56);

    /* ---- 全月边界扫描：任何月份所有格子都不越界 ---- */
    {
        uint8_t w1s[7] = {1, 2, 3, 4, 5, 6, 7};
        uint8_t dims[3] = {28, 30, 31};   /* 闰 2 月 29 不扫，行分布同 28 */
        uint8_t w, d, i;
        for(w = 0; w < 7; w++){
            for(d = 0; d < 3; d++){
                for(i = 1; i <= dims[d]; i++){
                    Cal_GetCellPos(i, dims[d], w1s[w], &x, &y);
                    /* 数字宽度 12（2 位）为最宽 → 右缘不超 128；底部不超 64 */
                    assert(x >= 0 && y >= 0);
                    assert(x + 12 <= 128 && y + 8 <= 64);
                    /* 溢出日（idx>=35 ⟺ i>=37-w1）必在右侧区，且不与 row4 常规格重叠 */
                    if(i >= (uint8_t)(37 - w1s[w])){
                        assert(x >= 106 && y == 56);
                    }
                }
            }
        }
    }

    printf("ALL CALENDAR LAYOUT TESTS PASSED\n");
    return 0;
}
