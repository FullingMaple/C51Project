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

/* Timer0 中断回调（20ms 节拍）：tick 计数 + ADC 键盘扫描 + 按键记录/动画推进/帧率计数 */
void Timer0_Isr(void) interrupt 1
{
    Driver_TickHandler();       /* 系统 tick 递增（GetTick 毫秒源） */
    Driver_KeyScan();           /* ADC 键盘采样 + 三态滤波（20ms 一次） */
    Driver_IRScan();            /* 红外遥控轮询（NEC 帧→逻辑键，遥控为主/键盘兜底） */
    Buzzer_Tick();              /* 按键音节拍（60ms 自动关） */
    OLED_UI_InterruptHandler();
}
