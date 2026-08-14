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
        Key_Hold = Key_State1;
    }
}

/* 按键状态：0=按下 1=松开（KEY_ADC_* 键值见 config.h，按实验箱布局调整） */
uint8_t Key_GetUpStatus(void)    { return (Key_Hold == KEY_ADC_UP)    ? 0 : 1; }
uint8_t Key_GetDownStatus(void)  { return (Key_Hold == KEY_ADC_DOWN)  ? 0 : 1; }
uint8_t Key_GetEnterStatus(void) { return (Key_Hold == KEY_ADC_ENTER) ? 0 : 1; }
uint8_t Key_GetBackStatus(void)  { return (Key_Hold == KEY_ADC_BACK)  ? 0 : 1; }

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
