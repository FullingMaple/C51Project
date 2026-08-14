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

/* 测试图案：边框 + 左上角方块 + 左上→右下斜线（验证显示通路与方向） */
static void OLED_TestPattern(void)
{
    uint8_t x, y, yp, bit;

    OLED_Clear();

    /* 边框 */
    for(x = 0; x < OLED_WIDTH; x++)
    {
        OLED_DisplayBuf[0][x]         |= 0x01;   /* 顶部第 0 行 */
        OLED_DisplayBuf[OLED_PAGES-1][x] |= 0x80; /* 底部第 63 行 */
    }
    for(y = 0; y < OLED_PAGES; y++)
    {
        OLED_DisplayBuf[y][0]             |= 0x01;   /* 左列第 0 像素 */
        OLED_DisplayBuf[y][OLED_WIDTH-1]  |= 0x80;   /* 右列第 63 像素 */
    }

    /* 左上角 8×8 方块（验证上下/左右方向） */
    OLED_DisplayBuf[0][0] = 0xFF;
    OLED_DisplayBuf[0][1] = 0xFF;
    OLED_DisplayBuf[1][0] = 0xFF;
    OLED_DisplayBuf[1][1] = 0xFF;

    /* 左上→右下斜线（y = x/2，镜像时可直观看出方向） */
    for(x = 0; x < OLED_WIDTH; x++)
    {
        yp  = x / 16;               /* 页 = (x/2)/8 */
        bit = 7 - ((x / 2) % 8);    /* 页内位 */
        OLED_DisplayBuf[yp][x] |= (uint8_t)(1 << bit);
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
