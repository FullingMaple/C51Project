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

/* 测试图案：左上角 32×32 白色方块，其余全黑（验证显示通路与方向）
 * 大块图案在 0.96 寸屏上清晰可辨：
 *   白块在左上 = 方向正常；右上 = 左右镜像；左下 = 上下颠倒；右下 = 180° 旋转
 *   白块缺角/不完整 = 显示通路或 COM 配置问题 */
static void OLED_TestPattern(void)
{
    uint8_t x, y;

    OLED_Clear();

    /* 左上 32×32 白块（页0-3 × 列0-31） */
    for(y = 0; y < 4; y++)
        for(x = 0; x < 32; x++)
            OLED_DisplayBuf[y][x] = 0xFF;
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
