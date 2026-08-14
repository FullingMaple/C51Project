/*==============================================================================
 * main.c —— 系统入口
 * v0.3 阶段：系统初始化 + OLED 点亮验证（虚拟/实体双后端）
 * 后续接入 OLED_UI 框架：System_Init → OLED_UI_init → OLED_UI_start
 *============================================================================*/
#include "config.h"
#include "stc8h.h"
#include "OLED_driver.h"
#include "OLED.h"

/* USB-CDC 库要求实现的 STC-ISP 用户命令接口（弱符号） */
char *USER_STCISPCMD = "@STCISP#";

void System_Init(void)
{
    P_SW2 |= 0x80;              /* 扩展寄存器(XFR)访问使能 */

    OLED_IO_MODE();             /* P2.2~P2.5 开漏 + 实验箱外部上拉 */

    OLED_Init();                /* 双后端：虚拟 USB-CDC / 实体硬件 I2C */
}

/* 测试图案：16×16 黑白棋盘格（8 列 × 4 行 = 32 格），左上角白格
 * 验证显示完整性：数格子数 + 看四角颜色
 *   正常：32 格完整，四角 = 左上白 / 右上黑 / 左下黑 / 右下白
 *   缺格/错位/颜色乱 = 对应区域显示问题（查 0xDA COM 配置或接线） */
static void OLED_TestPattern(void)
{
    uint8_t x, y, gx, gy, col;

    OLED_Clear();

    for(gy = 0; gy < 4; gy++)               /* 4 行格 */
    {
        for(gx = 0; gx < 8; gx++)           /* 8 列格 */
        {
            col = ((gx + gy) & 1) ? 0x00 : 0xFF;    /* (gx+gy) 偶=白 */
            for(y = 0; y < 2; y++)          /* 每格 2 页 */
                for(x = 0; x < 16; x++)     /* 每格 16 列 */
                    OLED_DisplayBuf[gy*2 + y][gx*16 + x] = col;
        }
    }
}

void main(void)
{
    System_Init();

    OLED_TestPattern();
    OLED_Update();              /* 刷新显示测试图案 */

    while(1)
    {
        /* 待接入 OLED_UI_MainLoop */
    }
}
