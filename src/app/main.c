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

/* 测试图案：全屏横条纹（偶数页白、奇数页黑，8 条 8px 高条纹）
 * 验证显示完整性：4 条白条纹应均匀横贯全屏（顶部第 1 条为白）
 *   某条白纹缺/短 = 对应页显示问题（查 0xDA COM 配置或接线） */
static void OLED_TestPattern(void)
{
    uint8_t x, y;

    OLED_Clear();

    for(y = 0; y < OLED_PAGES; y++)
    {
        if((y & 1) == 0)
            for(x = 0; x < OLED_WIDTH; x++)
                OLED_DisplayBuf[y][x] = 0xFF;   /* 偶数页白条纹 */
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
