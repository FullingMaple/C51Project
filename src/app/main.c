/*==============================================================================
 * main.c —— 系统入口
 * v0.3 阶段：系统初始化 → OLED_UI 框架启动（磁贴主屏 + 按键导航）
 * 流程：System_Init → OLED_UI_init（内部含 OLED_Init/Timer_Init/Key_Init）
 *       → OLED_UI_start（while(1) OLED_UI_MainLoop）
 *============================================================================*/
#include "config.h"
#include "stc8h.h"
#include "OLED_driver.h"
#include "OLED_UI_Launcher.h"

/* USB-CDC 库要求实现的 STC-ISP 用户命令接口（弱符号） */
char *USER_STCISPCMD = "@STCISP#";

void System_Init(void)
{
    SP = 0x80;                  /* 无 STARTUP.A51：手动设栈顶（深调用/中断不覆盖变量） */

    P_SW2 |= 0x80;              /* 扩展寄存器(XFR)访问使能 */

    OLED_IO_MODE();             /* P2.2~P2.5 开漏 + 实验箱外部上拉 */

    OLED_Init();                /* 双后端：虚拟 USB-CDC / 实体硬件 I2C */
}

void main(void)
{
    System_Init();

    EA = 1;                     /* 开总中断（Timer0 20ms 节拍） */

    OLED_UI_init();             /* 框架初始化：OLED_Init + Timer_Init + Key_Init + Encoder_Init */
    OLED_UI_start();            /* 主循环（阻塞）：OLED_UI_MainLoop */
}
