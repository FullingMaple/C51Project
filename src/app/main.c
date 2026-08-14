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
    /* 注意：不要手动改 SP！C51 运行库 ?C_C51STARTUP 已设 SP=0x22（栈 221B），
     * 手动设 0x80 会把栈砍到 128B，深调用链+中断嵌套+printf 浮点易溢出跑飞 */
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
