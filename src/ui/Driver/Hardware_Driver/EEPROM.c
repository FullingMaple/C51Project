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

/* EEPROM 区基址：STC8H 的 IAP 地址 = EEPROM 区偏移（从 0000H 开始，
 * 与 STC15 等"加程序区偏移"不同——官方手册明确"不需要加偏移量"）
 * 注意：地址填 EEPROM 区外的值（如 0xFE00）会被 IAP 忽略，
 *       擦/写无效、读回 0x00——表现为写后读回校验失败 */
#define EEPROM_BASE     0x0000

static void IAP_Enable(void)
{
    EA = 0;   /* IAP 操作期间必须关总中断：0x5A/0xA5 触发序列被中断插入会导致触发失败 */
    IAP_CONTR = IAP_EN;                             /* 使能 IAP（STC8H 的 IAP_CONTR 无 TPS 位，只置 bit7） */
    IAP_TPS = (uint8_t)(MAIN_Fosc / 1000000);       /* 等待参数 = 主频 MHz（STC8H 独立寄存器 0xF5，必须设置！） */
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

/* 诊断变量（保存失败时填充，UI 可显示定位）：
 * Stage=1 擦除后读回非 FF；Stage=2 写入后读回不符；Idx/Expect/Got 为字节索引/期望/读回 */
uint8_t EEP_Diag_Stage  = 0;
uint8_t EEP_Diag_Idx    = 0;
uint8_t EEP_Diag_Expect = 0;
uint8_t EEP_Diag_Got    = 0;

uint8_t EEPROM_SaveTime(const RTC_Time *t)
{
    uint8_t buf[8];
    uint8_t i, got;

    buf[0] = 0x5A;                                   /* magic */
    buf[1] = 0xA5;
    buf[2] = (uint8_t)(t->year - 2000);
    buf[3] = t->month;
    buf[4] = t->day;
    buf[5] = t->hour;
    buf[6] = t->minute;
    buf[7] = t->second;

    EEP_Diag_Stage = 0;

    EEPROM_EraseSector(0);              /* 擦整个扇区（0.5K EEPROM 单扇区） */

    /* 诊断 1：擦除后读回应全 0xFF（区分擦除失败与写失败） */
    for(i = 0; i < 8; i++){
        got = EEPROM_ReadByte(i);
        if(got != 0xFF){
            EEP_Diag_Stage = 1; EEP_Diag_Idx = i;
            EEP_Diag_Expect = 0xFF; EEP_Diag_Got = got;
            return 1;
        }
    }

    for(i = 0; i < 8; i++) EEPROM_WriteByte(i, buf[i]);

    /* 诊断 2：写后读回校验，全部一致才认为保存成功 */
    for(i = 0; i < 8; i++){
        got = EEPROM_ReadByte(i);
        if(got != buf[i]){
            EEP_Diag_Stage = 2; EEP_Diag_Idx = i;
            EEP_Diag_Expect = buf[i]; EEP_Diag_Got = got;
            return 1;
        }
    }
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
