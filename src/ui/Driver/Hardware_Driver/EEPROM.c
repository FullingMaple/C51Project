/*==============================================================================
 * EEPROM.c —— STC8H IAP 读写实现（官方 19 号例程移植）
 * 注意：写入前必须擦除整个扇区（STC8H 扇区 = 512B）
 * 关键：IAP 触发后 CPU 硬件暂停直到操作完成，无需（也不应）查询忙标志——
 *       忙标志等待在触发失败时会死等，故按官方例程：触发后 _nop_() 即关闭
 *============================================================================*/
#include "config.h"     /* MAIN_Fosc */
#include "intrins.h"    /* _nop_() */
#include "EEPROM.h"

#define IAP_EN          (1 << 7)    /* IAP_CONTR 使能位 */

/* EEPROM 区基址（IAP 绝对地址）：
 * STC8H 的 IAP 地址 = 程序空间绝对地址（非 EEPROM 区偏移）！
 * 出厂 EEPROM 0.5K 位于 flash 末尾：0x10000 - 0x200 = 0xFE00
 * 注意：用 0x0000 会擦/写程序区（中断向量表所在）→ 程序被毁卡死+蜂鸣长鸣
 * 若 ISP 下载时将 EEPROM 大小改为 N K，基址 = 0x10000 - N*1024 */
#define EEPROM_BASE     0xFE00

static void IAP_Enable(void)
{
    EA = 0;   /* IAP 操作期间必须关总中断：0x5A/0xA5 触发序列被中断插入会导致触发失败 */
    IAP_CONTR = IAP_EN | (MAIN_Fosc / 1000000);   /* TPS = 主频 MHz */
}

static void IAP_Disable(void)
{
    IAP_CONTR = 0;
    IAP_CMD   = 0;
    IAP_TRIG  = 0;
    IAP_ADDRH = 0xFF;
    IAP_ADDRL = 0xFF;
    EA = 1;   /* 恢复总中断 */
}

static void IAP_Trigger(void)
{
    IAP_TRIG = 0x5A;
    IAP_TRIG = 0xA5;
    _nop_();  /* 触发后硬件自动暂停直到操作完成（官方例程写法，无忙等待） */
}

uint8_t EEPROM_ReadByte(uint16_t addr)
{
    uint8_t dat;

    IAP_Enable();
    IAP_CMD  = 1;                       /* 读命令 */
    IAP_ADDRH = (uint8_t)((EEPROM_BASE + addr) >> 8);
    IAP_ADDRL = (uint8_t)(EEPROM_BASE + addr);
    IAP_Trigger();
    dat = IAP_DATA;
    IAP_Disable();
    return dat;
}

void EEPROM_WriteByte(uint16_t addr, uint8_t dat)
{
    IAP_Enable();
    IAP_CMD    = 2;                     /* 写命令 */
    IAP_ADDRH  = (uint8_t)((EEPROM_BASE + addr) >> 8);
    IAP_ADDRL  = (uint8_t)(EEPROM_BASE + addr);
    IAP_DATA   = dat;
    IAP_Trigger();
    IAP_Disable();
}

/* 擦除扇区（addr 所在 512B 扇区） */
static void EEPROM_EraseSector(uint16_t addr)
{
    IAP_Enable();
    IAP_CMD    = 3;                     /* 擦除命令 */
    IAP_ADDRH  = (uint8_t)((EEPROM_BASE + addr) >> 8);
    IAP_ADDRL  = (uint8_t)(EEPROM_BASE + addr);
    IAP_Trigger();
    IAP_Disable();
}

uint8_t EEPROM_SaveTime(const RTC_Time *t)
{
    EEPROM_EraseSector(0);              /* 擦整个扇区（0.5K EEPROM 单扇区） */
    EEPROM_WriteByte(0, 0x5A);          /* magic */
    EEPROM_WriteByte(1, 0xA5);
    EEPROM_WriteByte(2, (uint8_t)(t->year - 2000));
    EEPROM_WriteByte(3, t->month);
    EEPROM_WriteByte(4, t->day);
    EEPROM_WriteByte(5, t->hour);
    EEPROM_WriteByte(6, t->minute);
    EEPROM_WriteByte(7, t->second);
    return 0;
}

uint8_t EEPROM_LoadTime(RTC_Time *t)
{
    if(EEPROM_ReadByte(0) != 0x5A) return 1;
    if(EEPROM_ReadByte(1) != 0xA5) return 1;

    t->year   = (uint16_t)2000 + EEPROM_ReadByte(2);
    t->month  = EEPROM_ReadByte(3);
    t->day    = EEPROM_ReadByte(4);
    t->hour   = EEPROM_ReadByte(5);
    t->minute = EEPROM_ReadByte(6);
    t->second = EEPROM_ReadByte(7);

    /* 合理性校验（防止 EEPROM 损坏数据） */
    if(t->month < 1 || t->month > 12) return 1;
    if(t->day   < 1 || t->day   > 31) return 1;
    if(t->hour  > 23 || t->minute > 59 || t->second > 59) return 1;
    return 0;
}
