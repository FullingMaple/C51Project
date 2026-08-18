/*==============================================================================
 * RTC.c —— STC8H 内部 RTC 驱动实现（官方 50 号例程移植）
 * STC8H RTC = 32 位秒计数器 + BCD 时间寄存器：
 *   写：INIYEAR~INISSEC + RTCCFG|=0x01 触发
 *   读：RTCYEAR~RTCSEC（BCD）
 * 外部 32.768K 晶振由 X32KCR 启动（实验箱已焊接）
 *============================================================================*/
#include "STC8h.h"    /* RTC 寄存器宏（xdata 扩展寄存器）；不用 stc.h（其 uint8_t 与 stdint.h 冲突） */
#include "RTC.h"

/* 十进制 → BCD */
static uint8_t dec2bcd(uint8_t v)
{
    return (uint8_t)(((v / 10) << 4) | (v % 10));
}

/* BCD → 十进制 */
static uint8_t bcd2dec(uint8_t v)
{
    return (uint8_t)(((v >> 4) * 10) + (v & 0x0F));
}

void RTC_Init(void)
{
    uint16_t i;

    /* 启动外部 32.768K 晶振（低增益 0x80，不稳可改 0xC0 高增益） */
    X32KCR = 0x80 + 0x40;
    for(i = 0; i < 10000 && !(X32KCR & 0x01); i++);   /* 等待晶振稳定（超时保护防死等） */
    RTCCFG = 0x01;                    /* 选外部 32K 时钟源 + 触发寄存器初始化 */
    RTCCR  = 0x01;                    /* RTC 使能 */
    while(RTCCFG & 0x01);             /* 等待初始化完成（~30us） */
}

void RTC_SetTime(const RTC_Time *t)
{
    INIYEAR  = dec2bcd((uint8_t)(t->year % 100));   /* BCD 年份低两位 */
    INIMONTH = dec2bcd(t->month);
    INIDAY   = dec2bcd(t->day);
    INIHOUR  = dec2bcd(t->hour);
    INIMIN   = dec2bcd(t->minute);
    INISEC   = dec2bcd(t->second);
    INISSEC  = 0;
    RTCCFG |= 0x01;                   /* 触发 RTC 寄存器初始化 */
    while(RTCCFG & 0x01);             /* 等待完成 */
}

void RTC_GetTime(RTC_Time *t)
{
    t->year   = (uint16_t)2000 + bcd2dec(RTCYEAR);
    t->month  = bcd2dec(RTCMONTH);
    t->day    = bcd2dec(RTCDAY);
    t->hour   = bcd2dec(RTCHOUR);
    t->minute = bcd2dec(RTCMIN);
    t->second = bcd2dec(RTCSEC);
}
