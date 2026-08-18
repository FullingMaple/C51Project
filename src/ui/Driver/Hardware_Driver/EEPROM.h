/*==============================================================================
 * EEPROM.h —— STC8H 内置 EEPROM（IAP 方式）读写 + RTC 时间戳持久化
 * 存储布局（EEPROM 0 地址起，共 8 字节）：
 *   [0-1] magic 0x5A 0xA5（有效标记）
 *   [2]  year-2000   [3] month   [4] day
 *   [5]  hour        [6] minute  [7] second
 *============================================================================*/
#ifndef __EEPROM_H
#define __EEPROM_H

#include "stdint.h"
#include "RTC.h"

/* 读取字节 */
uint8_t EEPROM_ReadByte(uint16_t addr);

/* 写入字节（自动擦除所在扇区——512B 一扇区，写前需擦） */
void EEPROM_WriteByte(uint16_t addr, uint8_t dat);

/* 保存时间戳（magic + 时间）——写后读回校验，返回 0 成功，1 失败（EEPROM 配置/地址异常） */
uint8_t EEPROM_SaveTime(const RTC_Time *t);

/* 保存失败诊断（EEPROM_SaveTime 返回 1 时有效）：
 * Stage=1 擦除未生效；Stage=2 写入未生效；Idx/Expect/Got 定位字节 */
extern uint8_t EEP_Diag_Stage, EEP_Diag_Idx, EEP_Diag_Expect, EEP_Diag_Got;

/* 读取时间戳——magic 有效返回 0 并填充 t；无效返回 1 */
uint8_t EEPROM_LoadTime(RTC_Time *t);

#endif
