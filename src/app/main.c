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

void main(void)
{
    System_Init();

    OLED_Clear();
    OLED_Update();              /* 清屏刷新（验证显示通路） */

    while(1)
    {
        /* 待接入 OLED_UI_MainLoop */
    }
}
