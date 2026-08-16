/*==============================================================================
 * IR_Remote.h —— 红外遥控接收（NEC 协议）
 * 硬件：接收头 U7 → P3.5；T1 100us 采样中断（1T 模式 16 位手动重载）
 * 算法：官方 29 号例程 Coody 采样法移植（F0 改用独立变量防冲突）
 * 接口：IR_GetKey() 查询，返回键码（0xFF=无按键，读后清标志）
 *============================================================================*/
#ifndef __IR_REMOTE_H
#define __IR_REMOTE_H

#include "stdint.h"

#define IR_KEY_NONE     0xFF    /* 无按键返回值 */

/* ============ 本机遥控器实测键码（NEC 命令码，User=0xFF00） ============
 * 实测日期 2026-08-16；换遥控器需改码重烧（HANDOFF 定案：硬编码） */
#define IR_KEY_UP       0x18
#define IR_KEY_DOWN     0x52
#define IR_KEY_LEFT     0x08
#define IR_KEY_RIGHT    0x5A
#define IR_KEY_OK       0x1C
#define IR_KEY_0        0x19
#define IR_KEY_1        0x45
#define IR_KEY_2        0x46
#define IR_KEY_3        0x47
#define IR_KEY_4        0x44
#define IR_KEY_5        0x40
#define IR_KEY_6        0x43
#define IR_KEY_7        0x07
#define IR_KEY_8        0x15
#define IR_KEY_9        0x09
#define IR_KEY_STAR     0x16
#define IR_KEY_POUND    0x0D

void IR_Init(void);             /* 初始化：P3.5 输入 + T1 100us 采样中断 */
uint8_t IR_GetKey(void);        /* 查询键码：有按键返回 NEC 命令码（低字节），无返回 IR_KEY_NONE */
uint16_t IR_GetUser(void);      /* 查询最近一次有效帧的用户码（16 位） */
uint16_t IR_GetEdgeCnt(void);   /* 诊断：下降沿计数（信号活动指示） */

#endif
