/*==============================================================================
 * IR_Remote.c —— 红外遥控接收（NEC 协议）
 *
 * 解码原理（Coody 采样法，官方 29 号例程移植）：
 *   红外接收头 U7 解调 38kHz 载波 → P3.5 输出 TTL 方波（低=收到载波）
 *   T1 每 100us 采样一次 P3.5，检测下降沿并用采样计数测量脉冲宽度：
 *     宽 > 9.7ms           → 引导码（SYNC）
 *     宽 >= 1.687ms        → 位 "1"
 *     宽 0.6ms ~ 1.687ms   → 位 "0"
 *   NEC 帧 = SYNC + 32 位（16 位用户码 + 8 位命令 + 8 位反码校验）
 *   收满 32 位且反码校验通过 → IR_code 有效
 *
 * 采样中断：T1 1T 模式 16 位手动重载，100us（24MHz = 2400 计数）
 *   手动重载 ~1us 级误差，相对 NEC 脉宽阈值（600us 起）可忽略
 *   注意：T1 与 Timer0（UI 20ms）同级中断，采样执行极短不会饿死 UI
 *============================================================================*/
#include "IR_Remote.h"
#include "stc8h.h"
#include "intrins.h"

#define MAIN_Fosc       24000000L

/* 采样间隔 100us；1T 模式 16 位重载值 */
#define IR_SAMPLE_US    100
#define IR_RELOAD       (65536UL - MAIN_Fosc / (1000000UL / IR_SAMPLE_US))

/* 脉宽阈值（单位：100us 采样计数） */
#define D_IR_SYNC_MAX   (15000 / IR_SAMPLE_US)   /* 150 计数 */
#define D_IR_SYNC_MIN   (9700  / IR_SAMPLE_US)   /* 97 计数  */
#define D_IR_SYNC_DIV   (12375 / IR_SAMPLE_US)   /* 123 计数 */
#define D_IR_DATA_MAX   (3000  / IR_SAMPLE_US)   /* 30 计数  */
#define D_IR_DATA_DIV   (1687  / IR_SAMPLE_US)   /* 16 计数  */
#define D_IR_BIT_NUMBER 32

sbit P_IR_RX = P3^5;                /* 红外接收头输出（低有效） */

/* ================= 解码状态（中断共享） ================= */
static uint8_t IR_SampleCnt;        /* 采样计数（下降沿间隔） */
static uint8_t IR_BitCnt;           /* 剩余位数 */
static uint8_t IR_UserH, IR_UserL;  /* 用户码高低字节 */
static uint8_t IR_data;             /* 当前接收字节 */
static uint8_t IR_DataShift;        /* 位移寄存器 */
static bit P_IR_RX_prev;            /* 上次采样电平（替代官方 F0，防与主循环冲突） */
static bit B_IR_Sync;               /* 已收引导码标志 */
static bit B_IR_Press;              /* 收到有效按键标志 */
static bit B_IR_HaveFrame;          /* 收到过有效帧（重复帧连发判断依据） */
static uint8_t IR_code;             /* 命令码 */
static uint16_t UserCode;           /* 用户码 */
static uint16_t IR_EdgeCnt;         /* 诊断：下降沿计数（P3.5 信号活动指示） */

/* ================= 解码状态机（T1 中断内每 100us 调用一次） ================= */
static void IR_RX_NEC(void)
{
    uint8_t SampleTime;

    IR_SampleCnt++;                             /* 采样计数 +1 */
    if(P_IR_RX_prev && !P_IR_RX)                /* 上次高 && 当前低 = 下降沿 */
    {
        SampleTime = IR_SampleCnt;              /* 取间隔（脉宽） */
        IR_SampleCnt = 0;                       /* 清计数 */
        IR_EdgeCnt++;                           /* 诊断：下降沿 +1 */

        if(SampleTime > D_IR_SYNC_MAX)          /* 超长：无效，清同步 */
            B_IR_Sync = 0;
        else if(SampleTime >= D_IR_SYNC_MIN)    /* 引导码范围 */
        {
            if(SampleTime >= D_IR_SYNC_DIV)     /* 9ms+4.5ms 才认引导码（新帧） */
            {
                B_IR_Sync = 1;
                IR_BitCnt = D_IR_BIT_NUMBER;    /* 装载 32 位 */
            }
            else if(B_IR_HaveFrame)             /* 9ms+2.25ms = 重复帧（按住不放）：
                                                 * 重复触发上一键，实现按住连发 */
            {
                B_IR_Press = 1;                 /* IR_code 保持上一帧键码 */
            }
        }
        else if(B_IR_Sync)                      /* 已收引导码：数据位 */
        {
            if(SampleTime > D_IR_DATA_MAX)      /* 超长：数据错误 */
                B_IR_Sync = 0;
            else
            {
                IR_DataShift >>= 1;             /* 位移 */
                if(SampleTime >= D_IR_DATA_DIV) /* 宽脉冲 = 1 */
                    IR_DataShift |= 0x80;
                if(--IR_BitCnt == 0)            /* 32 位收满 */
                {
                    B_IR_Sync = 0;
                    if((~IR_DataShift) == IR_data)  /* 反码校验通过 */
                    {
                        UserCode = ((uint16_t)IR_UserH << 8) + IR_UserL;
                        IR_code = IR_data;
                        B_IR_Press = 1;         /* 按键有效 */
                        B_IR_HaveFrame = 1;     /* 记录有效帧（重复帧连发依据） */
                    }
                }
                else if((IR_BitCnt & 7) == 0)   /* 收满 1 字节 */
                {
                    IR_UserL = IR_UserH;        /* 用户码高字节暂存 */
                    IR_UserH = IR_data;
                    IR_data = IR_DataShift;
                }
            }
        }
    }
    P_IR_RX_prev = P_IR_RX;                     /* 记录本次采样 */
}

/* ================= T1 100us 采样中断 ================= */
void IR_T1Isr(void) interrupt 3
{
    TH1 = (uint8_t)(IR_RELOAD >> 8);    /* 先重载再采样，误差最小 */
    TL1 = (uint8_t)(IR_RELOAD & 0xFF);
    IR_RX_NEC();
}

/* ================= 初始化：P3.5 输入 + T1 100us 中断 ================= */
void IR_Init(void)
{
    P3M1 |= 0x20;                       /* P3.5 高阻输入（接收头输出） */
    P3M0 &= ~0x20;
    AUXR |= 0x40;                       /* T1x12 = 1T */
    TMOD &= 0xF0;
    TMOD |= 0x10;                       /* T1 模式 1（16 位） */
    TH1 = (uint8_t)(IR_RELOAD >> 8);
    TL1 = (uint8_t)(IR_RELOAD & 0xFF);
    ET1 = 1;                            /* 使能 T1 中断 */
    TR1 = 1;                            /* 启动采样 */
}

/* ================= 查询键码（读后清标志） ================= */
uint8_t IR_GetKey(void)
{
    uint8_t key = IR_KEY_NONE;
    if(B_IR_Press)
    {
        B_IR_Press = 0;
        key = IR_code;
    }
    return key;
}

/* 查询最近有效帧的用户码（配合 IR_GetKey 使用，校验遥控器身份） */
uint16_t IR_GetUser(void)
{
    return UserCode;
}

/* 诊断：下降沿计数（按遥控器时应快速增加；不增 = 信号未到达 P3.5） */
uint16_t IR_GetEdgeCnt(void)
{
    return IR_EdgeCnt;
}
