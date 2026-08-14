/*==============================================================================
 * stdbool.h —— C51 shim（Keil C51 无标准 stdbool）
 * 说明：C51 的 bit 类型限制较多（仅 IRAM/数组指针等受限），
 *       统一用 unsigned char 表示布尔，语义等价且无限制
 *============================================================================*/
#ifndef STDBOOL_H
#define STDBOOL_H

#define bool    unsigned char
#define true    1
#define false   0

#endif
