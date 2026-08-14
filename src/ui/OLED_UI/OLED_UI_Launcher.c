/*
 *  OLED_UI_Launcher.c
 *  Version: 0.2.1 (C51 移植版)
 *  Author: 5akura1 / C51 port
 */
#include "OLED_UI_Launcher.h"
#include "stc8h.h"
#include "OLED_UI_Driver.h"

void OLED_UI_init(void)
{
    /* 注意：此处会写 EEPROM，先初始化外设避免电压跌落 */
    OLED_UI_Init(&MainMenuPage);
}

void OLED_UI_start(void)
{
    while(1)
    {
        OLED_UI_MainLoop();
    }
}

/* Timer0 中断回调（20ms 节拍）：按键记录/动画推进/帧率计数 */
void Timer0_Isr(void) interrupt 1
{
    OLED_UI_InterruptHandler();
}
