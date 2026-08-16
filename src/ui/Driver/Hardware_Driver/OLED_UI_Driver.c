/*==============================================================================
 * OLED_UI_Driver.c —— OLED_UI 框架平台层（C51 / STC8H8K64U 实验箱 9.62）
 *
 * - Timer_Init：Timer0 20ms 中断（12T 模式，24MHz，重载 0x63C0）
 *   注：1T 模式下 16 位定时器最大 2.73ms，无法直接 20ms，故用 12T。
 *   Timer0 中断由 OLED_UI_Launcher.c 的 Timer0_Isr 接管：
 *     Driver_TickHandler(); Driver_KeyScan(); OLED_UI_InterruptHandler();
 * - Key_Init：16 键 ADC 键盘（P1.0 = ADC0，256 分档阈值 + 三态滤波，
 *   算法参考官方 17 号例程 ADC_KeyScan.c 的 Coody 三态滤波）
 * - Encoder：实验箱无编码器，空实现（Get 恒返回 0）
 * - 按键状态语义：0=按下 1=松开（与框架 OLED_UI_Key 一致）
 *============================================================================*/
#include "OLED_UI_Driver.h"
#include "config.h"
#include "stc8h.h"
#include "intrins.h"
#include "IR_Remote.h"

/* ================= 系统 tick（20ms 节拍 ×20 = 1ms） ================= */
static volatile uint32_t TickCounter;      /* 20ms 计数 */

void Driver_TickHandler(void)
{
    TickCounter++;
}

uint32_t GetTick(void)
{
    return TickCounter * 20;
}

/* ================= 按键音（P5.4 有源蜂鸣器） =================
 * 电路：P5.4 → 1N5819 → BEEP1 → SYS-VCC——低电平导通响（P5.4=0 响） */
sbit Buzzer = P5^4;
static uint8_t BuzzerCnt;           /* 剩余鸣响节拍（20ms 单位） */
bool SoundEnable = true;            /* 提示音总开关（设置页绑定） */

void Buzzer_Init(void)
{
    P5M1 &= ~0x10;                  /* P5.4 推挽输出 */
    P5M0 |= 0x10;
    Buzzer = 1;                     /* 初始静音（低电平响） */
}

void Buzzer_Beep(void)
{
    if(!SoundEnable) return;        /* 开关关闭则静音 */
    Buzzer = 0;                     /* 响（低电平） */
    BuzzerCnt = 3;                  /* 鸣 60ms */
}

void Buzzer_Tick(void)              /* Timer0 20ms 中断内调用 */
{
    if(BuzzerCnt && (--BuzzerCnt == 0)) Buzzer = 1;   /* 计时到恢复静音 */
}

/* ================= 红外遥控桥接（遥控为主、键盘兜底） =================
 * NEC 无"松开"信号：收到一帧 → 逻辑键按下并保持 40ms 后自动松开；
 * 按住不放（重复帧 108ms/帧）→ 逻辑键周期刷新 → 菜单约 9 键/秒连发 */
static uint8_t IR_LogicalKey;       /* 当前遥控逻辑键（0=无） */
static uint8_t IR_HoldTick;         /* 模拟松开计时（20ms 单位） */
static uint8_t IR_LastRaw;          /* 上次键码（重复帧去重：不重复响铃） */

static uint8_t IR_KeyToLogical(uint8_t nec_key)
{
    switch(nec_key)
    {
        case IR_KEY_UP:     return KEY_ADC_UP;
        case IR_KEY_DOWN:   return KEY_ADC_DOWN;
        case IR_KEY_OK:
        case IR_KEY_RIGHT:  return KEY_ADC_ENTER;
        case IR_KEY_LEFT:   return KEY_ADC_BACK;
        default:            return 0;   /* 数字键等暂不映射（计算器/游戏预留） */
    }
}

void Driver_IRScan(void)            /* Timer0 20ms 中断内调用 */
{
    uint8_t raw, logical;

    raw = IR_GetKey();
    if(raw != IR_KEY_NONE)
    {
        logical = IR_KeyToLogical(raw);
        if(logical)
        {
            IR_LogicalKey = logical;
            IR_HoldTick = 2;            /* 模拟按下 40ms */
            if(raw != IR_LastRaw)       /* 新键码才响铃（重复帧静音） */
                Buzzer_Beep();
            IR_LastRaw = raw;
        }
    }
    else if(IR_HoldTick)
    {
        if(--IR_HoldTick == 0) IR_LogicalKey = 0;
    }
}


/* ================= Timer0 20ms 中断初始化 =================
 * 24MHz @12T：20ms = 24000000/12 * 0.02 = 40000 计数
 * 重载值 = 65536 - 40000 = 25536 (0x63C0)                       */
void Timer_Init(void)
{
    AUXR &= ~0x80;                  /* Timer0 12T 模式 */
    TMOD &= ~0x0F;
    TMOD |= 0x01;                   /* 16 位自动重载，不门控 */
    TH0 = 0x63;                     /* 重载值高字节 */
    TL0 = 0xC0;                     /* 重载值低字节 → 20ms 中断 */
    ET0 = 1;                        /* 使能 Timer0 中断 */
    TR0 = 1;                        /* 启动定时 */
}

/* ================= 16 键 ADC 键盘（P1.0 = ADC0） ================= */
#define ADC_OFFSET  64              /* 阈值 ± 偏差（官方例程参数） */

static uint8_t Key_Hold;            /* 当前确认按住的键 1~16，0=无按键 */
static uint8_t Key_State1, Key_State2, Key_State3;   /* 三态滤波链 */

void Key_Init(void)
{
    P_SW2 |= 0x80;                  /* 扩展寄存器(XFR)访问使能 */
    P1M1 |= 0x01;                   /* P1.0 高阻输入（ADC 通道 0） */
    P1M0 &= ~0x01;
    ADCTIM = 0x3f;                  /* ADC 内部时序（官方例程参数） */
    ADCCFG = 0x2f;                  /* 时钟 /2/16/16 */
    ADC_CONTR = 0x80;               /* 使能 ADC 模块 */
}

/* 查询式单次 12 位 ADC 转换（channel = 0~15）
 * 在 20ms 中断内调用：必须带超时，否则 ADC 异常会死等卡死整个中断（屏幕黑屏） */
static uint16_t Get_ADC12bitResult(uint8_t channel)
{
    uint16_t timeout = 0;

    if(!(ADC_CONTR & 0x80)) return 0xFFFF;  /* ADC 未使能（Key_Init 尚未执行）直接返回 */

    ADC_RES = 0;
    ADC_RESL = 0;
    ADC_CONTR = (ADC_CONTR & 0xF0) | 0x40 | channel;    /* 启动转换 */
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    while((ADC_CONTR & 0x20) == 0)          /* 等待转换完成（超时保护） */
    {
        if(++timeout > 1000) return 0xFFFF; /* ~0.5ms 上限，ADC 正常 20us 内完成 */
    }
    ADC_CONTR &= ~0x20;                     /* 清完成标志 */
    return (((uint16_t)ADC_RES << 8) | ADC_RESL);
}

/* 20ms 一次：采样 ADC → 分档 → 三态滤波确认按住键 */
void Driver_KeyScan(void)
{
    uint16_t adc, j;
    uint8_t i;

    adc = Get_ADC12bitResult(0);        /* 键盘接 ADC0（P1.0） */
    if(adc >= 4096) return;             /* 转换异常 */

    if(adc < 256 - ADC_OFFSET)          /* 低于第一档阈值：无按键 */
    {
        Key_Hold = 0;
        return;
    }

    j = 256;
    for(i = 1; i <= 16; i++)
    {
        if(adc >= (j - ADC_OFFSET) && adc <= (j + ADC_OFFSET))  break;   /* 落入分档 */
        j += 256;
    }
    if(i > 16)                          /* 超出分档范围（异常） */
    {
        Key_Hold = 0;
        return;
    }

    /* 三态滤波：连续 3 次采样同一键才确认（60ms 去抖） */
    Key_State3 = Key_State2;
    Key_State2 = Key_State1;
    Key_State1 = i;
    if(Key_State3 == Key_State2 && Key_State2 == Key_State1)
    {
        if(Key_Hold == 0) Buzzer_Beep();    /* 新键按下沿响铃（长按连发不重复响） */
        Key_Hold = Key_State1;
    }
}

/* 按键状态：0=按下 1=松开（键盘 || 遥控 双输入合并；KEY_ADC_* 见 config.h） */
uint8_t Key_GetUpStatus(void)    { return ((Key_Hold == KEY_ADC_UP)    || (IR_LogicalKey == KEY_ADC_UP))    ? 0 : 1; }
uint8_t Key_GetDownStatus(void)  { return ((Key_Hold == KEY_ADC_DOWN)  || (IR_LogicalKey == KEY_ADC_DOWN))  ? 0 : 1; }
uint8_t Key_GetEnterStatus(void) { return ((Key_Hold == KEY_ADC_ENTER) || (IR_LogicalKey == KEY_ADC_ENTER)) ? 0 : 1; }
uint8_t Key_GetBackStatus(void)  { return ((Key_Hold == KEY_ADC_BACK)  || (IR_LogicalKey == KEY_ADC_BACK))  ? 0 : 1; }

/* ================= 编码器（实验箱无，空实现） ================= */
void Encoder_Init(void)     {}
void Encoder_Enable(void)   {}
void Encoder_Disable(void)  {}
int16_t Encoder_Get(void)   { return 0; }

/* ================= 软件延时（NOP 循环近似） ================= */
/* 24MHz 1T：1ms ≈ 24000 周期；内层循环约 10 周期/次 → 2400 次 */
void Delay_ms(uint16_t xms)
{
    uint16_t i;
    while(xms--)
    {
        for(i = 0; i < 2400; i++)
        {
            _nop_();
        }
    }
}

void Delay_s(uint16_t xs)
{
    while(xs--)
    {
        Delay_ms(1000);
    }
}
