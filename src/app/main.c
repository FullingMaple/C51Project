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

/* 测试图案：边框 + 四角诊断标志（16×16）+ 中部短斜线（验证显示通路与方向）
 * 四角标志形状各异且互不干扰，烧录后报告各角所见形状即可判断上下/左右镜像 */
static void OLED_TestPattern(void)
{
    uint8_t x, y, yp, bitpos;

    OLED_Clear();

    /* 边框：上下 1 像素线；左右整列 0xFF（每页 8 像素连续） */
    for(x = 0; x < OLED_WIDTH; x++)
    {
        OLED_DisplayBuf[0][x]          |= 0x01;   /* 顶边（第 0 行） */
        OLED_DisplayBuf[OLED_PAGES-1][x] |= 0x80; /* 底边（第 63 行） */
    }
    for(y = 0; y < OLED_PAGES; y++)
    {
        OLED_DisplayBuf[y][0]            |= 0xFF; /* 左边实线 */
        OLED_DisplayBuf[y][OLED_WIDTH-1] |= 0xFF; /* 右边实线 */
    }

    /* 左上角：实心块（页0-1 × 列0-15，16×16） */
    for(y = 0; y < 2; y++)
        for(x = 0; x < 16; x++)
            OLED_DisplayBuf[y][x] = 0xFF;

    /* 右上角：空心方框（页0-1 × 列112-127，16×16 边框） */
    for(x = 112; x <= 127; x++)
    {
        OLED_DisplayBuf[0][x] |= 0x01;   /* 上边 */
        OLED_DisplayBuf[1][x] |= 0x80;   /* 下边 */
    }
    for(y = 0; y < 2; y++)
    {
        OLED_DisplayBuf[y][112] = 0xFF;  /* 左边 */
        OLED_DisplayBuf[y][127] = 0xFF;  /* 右边 */
    }

    /* 左下角：对角斜线块（页6-7 × 列0-15，从(0,48)到(15,63)） */
    for(x = 0; x < 16; x++)
    {
        yp     = 6 + (x / 8);            /* 页：x<8 时页6，x>=8 时页7 */
        bitpos = x & 0x07;               /* 页内位：对角 */
        OLED_DisplayBuf[yp][x] |= (uint8_t)(1 << bitpos);
    }

    /* 右下角：双横线块（页6-7 × 列112-127，y=48 与 y=63） */
    for(x = 112; x < OLED_WIDTH; x++)
    {
        OLED_DisplayBuf[6][x] |= 0x01;   /* y=48 横线 */
        OLED_DisplayBuf[7][x] |= 0x80;   /* y=63 横线 */
    }

    /* 中部短斜线（x=40..88，y=x/2，避开四角） */
    for(x = 40; x <= 88; x++)
    {
        yp  = x / 16;               /* 页 = (x/2)/8 */
        bitpos = 7 - ((x / 2) % 8); /* 页内位 */
        OLED_DisplayBuf[yp][x] |= (uint8_t)(1 << bitpos);
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
