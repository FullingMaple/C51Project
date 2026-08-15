/*==============================================================================
 * OLED_driver.c —— SSD1306 驱动：STC8H 硬件 I2C + diff 刷新 + 虚拟/实体双后端
 *
 * 参考：69 号例程（STC8H8K64U-DEMO-CODE-V9.6，硬件 I2C + VirtualDevice 双后端）
 *       2026-08-14 实体屏已实测点亮；0xC0 上下颠倒 → 已修正为 0xC8
 *
 * 刷新策略：驱动内维护 1KB 影子缓冲，OLED_Update 逐页(8行)对比，
 *           只把变化的页写入 OLED（菜单高亮移动 <0.5ms）
 *============================================================================*/
#include "config.h"
#include "stc8h.h"
#include "OLED_driver.h"
#include "stc32_stc8_usb.h"     /* 虚拟 OLED 调试接口（USB-CDC 库，自带 BYTE/BOOL 定义） */
#include <string.h>

/* ================= 影子缓冲（diff 对比用，xdata） ================= */
static uint8_t xdata OLED_ShadowBuf[OLED_PAGES][OLED_WIDTH];

/* ================= 硬件 I2C 底层（STC8H I2C 模块，P2.5/P2.4） ================= */
static uint8_t I2C_FailCount;    /* 连续失败计数（超阈值触发总线恢复） */
static uint8_t I2C_Recovering;   /* 总线恢复重入保护 */
static void I2C_BusRecover(void);    /* 前向声明（I2C_Wait 调用） */

static void I2C_Wait(void)
{
    uint16_t timeout = 0;

    while(!(I2CMSST & 0x40))    /* 等待操作完成标志（MSIF） */
    {
        /* 超时保护：总线挂死时复位 I2C 模块。超时短（~2ms），
         * 偶发失败自愈，避免每个事务 13ms 超时导致画面冻结 */
        if(++timeout > 6000)
        {
            I2C_FailCount++;
            if(I2C_FailCount > 5 && !I2C_Recovering)
            {
                /* 连续失败：GPIO 9 脉冲释放总线 + 重新初始化 SSD1306 */
                I2C_Recovering = 1;
                I2C_FailCount = 0;
                I2C_BusRecover();
                I2C_Recovering = 0;
            }
            else
            {
                I2CCFG = 0x00;      /* 复位 I2C 模块 */
                I2CCFG = 0xCD;
                I2CMSST = 0x00;
            }
            return;
        }
    }
    I2C_FailCount = 0;
    I2CMSST &= ~0x40;           /* 清标志 */
}

/* 总线恢复：I2C 引脚切回 GPIO，SCL 拉 9 个脉冲释放卡住的从机，重新初始化 */
static void I2C_BusRecover(void)
{
    uint8_t i, j;

    P_SW2 &= ~0x10;             /* I2C 功能脚断开，P2.5/P2.4 切回 GPIO */
    P2M1 |= 0x30;               /* P2.4/P2.5 准双向/开漏 */
    P2M0 |= 0x30;
    OLED_SCL = 1;
    OLED_SDA = 1;
    for(i = 0; i < 9; i++)      /* 9 个时钟脉冲释放从机 */
    {
        OLED_SCL = 0;
        for(j = 0; j < 100; j++);   /* ~4us */
        OLED_SCL = 1;
        for(j = 0; j < 100; j++);
    }
    OLED_SDA = 1;               /* 释放 SDA */
    OLED_SCL = 1;
    P_SW2 |= 0x10;              /* 恢复 I2C 功能脚 */
    OLED_Init();                /* 重新初始化 SSD1306（内部 I2C_Wait 超时由 Recovering 保护） */
}

static void I2C_Start(void)
{
    I2CMSCR = 0x01;             /* 发送 START 命令 */
    I2C_Wait();
}

static void I2C_Stop(void)
{
    I2CMSCR = 0x06;             /* 发送 STOP 命令 */
    I2C_Wait();
}

static void I2C_SendData(uint8_t dat)
{
    I2CTXD = dat;               /* 写数据到发送缓冲 */
    I2CMSCR = 0x02;             /* 发送 SEND 命令 */
    I2C_Wait();
}

static void I2C_RecvACK(void)
{
    I2CMSCR = 0x03;             /* 发送读 ACK 命令 */
    I2C_Wait();
}

static void OLED_Write_Command(uint8_t dat)
{
    I2C_Start();
    I2C_SendData(OLED_ADDR);    /* 从机地址，RW=0 */
    I2C_RecvACK();
    I2C_SendData(0x00);         /* 控制字节，Co=0, D/C#=0（命令） */
    I2C_RecvACK();
    I2C_SendData(dat);
    I2C_RecvACK();
    I2C_Stop();
}

static void OLED_Write_Data(uint8_t dat)
{
    I2C_Start();
    I2C_SendData(OLED_ADDR);
    I2C_RecvACK();
    I2C_SendData(0x40);         /* 控制字节，D/C#=1（数据） */
    I2C_RecvACK();
    I2C_SendData(dat);
    I2C_RecvACK();
    I2C_Stop();
}

/* 批量写数据：一次 Start 连续发送 len 字节（SSD1306 水平寻址自动递增列地址）
 * 性能关键：全屏 8 页从 ~23ms 降到 ~4ms（原来每字节一次 start/stop/addr 事务） */
static void OLED_Write_Data_Bulk(const uint8_t *buf, uint16_t len)
{
    I2C_Start();
    I2C_SendData(OLED_ADDR);
    I2C_RecvACK();
    I2C_SendData(0x40);         /* 控制字节，D/C#=1（数据） */
    I2C_RecvACK();
    while(len--)
    {
        I2C_SendData(*buf++);
        I2C_RecvACK();
    }
    I2C_Stop();
}

/* 设置页地址（0~7），列从 0 开始 */
static void OLED_SetPos(uint8_t page)
{
    OLED_Write_Command(0xB0 | (page & 0x07));   /* 页地址 */
    OLED_Write_Command(0x00);                   /* 列地址低 4 位 */
    OLED_Write_Command(0x10);                   /* 列地址高 4 位 */
}

/* 设置页地址 + 列范围（水平寻址 0x21 命令）——局部刷新只发变化列段 */
static void OLED_SetPosRange(uint8_t page, uint8_t col_start, uint8_t col_end)
{
    OLED_Write_Command(0xB0 | (page & 0x07));   /* 页地址 */
    OLED_Write_Command(0x21);                   /* 设置列地址范围 */
    OLED_Write_Command(col_start);
    OLED_Write_Command(col_end);
}

/* ================= 初始化 ================= */
void OLED_Init(void)
{
#if(VIRTUAL_OLED)
    /* ---- 虚拟 OLED：USB-CDC 调试接口 ---- */
    P22 = 0;                    /* 打开调试接口通道（官方例程约定） */
    usb_init();                 /* 初始化 USB-CDC */
    OLED12864_ScrollStop();
    OLED12864_DisplayOn();      /* 打开虚拟 OLED 显示 */
    OLED12864_DisplayContent(); /* 显示屏幕内容 */
#else
    /* ---- 实体 OLED：SSD1306 硬件 I2C ---- */
    P_SW2 |= 0x10;              /* I2C 功能脚选择 P2.5/P2.4 */
    I2CCFG = 0xCD;              /* 24MHz @400kHz：MSSPEED=13（FOSC/2/(MSSPEED*2+4)=400k）
     * 总线异常由 I2C_BusRecover 自动恢复（9 脉冲+重初始化） */
    I2CMSST = 0x00;

    OLED_Write_Command(0xAE);   /* 关闭显示 */
    OLED_Write_Command(0x20);   /* 寻址模式：水平 */
    OLED_Write_Command(0x00);
    OLED_Write_Command(0x21);   /* 列地址范围 0~127 */
    OLED_Write_Command(0x00);
    OLED_Write_Command(0x7F);
    OLED_Write_Command(0x22);   /* 页地址范围 0~7 */
    OLED_Write_Command(0x00);
    OLED_Write_Command(0x07);
    OLED_Write_Command(0x40);   /* 显示起始行 0 */
    OLED_Write_Command(0x81);   /* 对比度（亮度） */
    OLED_Write_Command(0xCF);
    OLED_Write_Command(0xA1);   /* 段重映射（左右镜像改 0xA0；实测本模块需 0xA1） */
    OLED_Write_Command(0xC8);   /* COM 扫描方向（上下颠倒改 0xC0） */
    OLED_Write_Command(0xA6);   /* 正常显示（反色改 0xA7） */
    OLED_Write_Command(0xA8);   /* 多路复用比 64 */
    OLED_Write_Command(0x3F);
    OLED_Write_Command(0xD3);   /* 显示偏移 0 */
    OLED_Write_Command(0x00);
    OLED_Write_Command(0xD5);   /* 时钟分频 0x80 */
    OLED_Write_Command(0x80);
    OLED_Write_Command(0xD9);   /* 预充电 0xF1 */
    OLED_Write_Command(0xF1);
    OLED_Write_Command(0xDA);   /* COM 引脚配置（半边亮改 0x02） */
    OLED_Write_Command(0x12);
    OLED_Write_Command(0xDB);   /* VCOMH 0x40 */
    OLED_Write_Command(0x40);
    OLED_Write_Command(0x8D);   /* 电荷泵开 */
    OLED_Write_Command(0x14);
    OLED_Write_Command(0xAF);   /* 打开显示 */
#endif

    memset(OLED_ShadowBuf, 0, sizeof(OLED_ShadowBuf));
}

/* 清影子缓冲：OLED_Clear 后同步调用，diff 对比才不会把全屏当变化 */
void OLED_ShadowClear(void)
{
    memset(OLED_ShadowBuf, 0, sizeof(OLED_ShadowBuf));
}

/* ================= 刷新 ================= */
static uint8_t OLED_FrameCounter;   /* 帧计数：周期全刷自愈（I2C 偶发丢字节的残留清理） */

/* 全屏刷新：diff 逐页对比，只发变化的页（列级）；每 50 帧强制全刷一次自愈 */
void OLED_Update(void)
{
#if(VIRTUAL_OLED)
    /* 虚拟 OLED：整屏经 USB-CDC 发送（虚拟屏本身是整屏渲染）
     * USB 忙/未枚举时跳过本帧：库发送为阻塞式，渐隐期间每帧全屏发送
     * 若主机读取不及时会缓冲满阻塞 → 主循环卡死 */
    P22 = 0;
    if(!bUsbInBusy && DeviceState == DEVSTATE_CONFIGURED)
    {
        OLED12864_ShowPicture(0, 0, OLED_WIDTH, OLED_PAGES, &OLED_DisplayBuf[0][0]);
    }
    P22 = 1;
#else
    uint8_t page, c0, c1;
    uint8_t force_full;
    OLED_FrameCounter++;
    force_full = (OLED_FrameCounter >= 50);   /* 每 50 帧（~0.6s）全刷自愈 */
    for(page = 0; page < OLED_PAGES; page++)
    {
        if(force_full || memcmp(OLED_ShadowBuf[page], OLED_DisplayBuf[page], OLED_WIDTH) != 0)
        {
            if(force_full)
            {
                /* 全页发送（自愈） */
                OLED_SetPos(page);
                OLED_Write_Data_Bulk(OLED_DisplayBuf[page], OLED_WIDTH);
            }
            else
            {
                /* 列级 diff：只发变化列段 */
                c0 = 0;
                while(c0 < OLED_WIDTH && OLED_ShadowBuf[page][c0] == OLED_DisplayBuf[page][c0]) c0++;
                c1 = OLED_WIDTH - 1;
                while(c1 > c0 && OLED_ShadowBuf[page][c1] == OLED_DisplayBuf[page][c1]) c1--;
                OLED_SetPosRange(page, c0, c1);
                OLED_Write_Data_Bulk(&OLED_DisplayBuf[page][c0], c1 - c0 + 1);
            }
            memcpy(OLED_ShadowBuf[page], OLED_DisplayBuf[page], OLED_WIDTH);
        }
    }
    if(force_full) OLED_FrameCounter = 0;
#endif
}

/* 局部刷新：diff 对比指定区域覆盖的页 */
void OLED_UpdateArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
#if(!VIRTUAL_OLED)
    uint8_t page, page_start, page_end, c0, c1;
    X = X;    Width = Width;    /* 当前 diff 按整页(128列)比较，列范围参数暂用不到 */

    page_start = (Y >> 3) & 0x07;
    page_end   = ((Y + Height - 1) >> 3) & 0x07;

    for(page = page_start; page <= page_end; page++)
    {
        if(memcmp(OLED_ShadowBuf[page], OLED_DisplayBuf[page], OLED_WIDTH) != 0)
        {
            c0 = 0;
            while(c0 < OLED_WIDTH && OLED_ShadowBuf[page][c0] == OLED_DisplayBuf[page][c0]) c0++;
            c1 = OLED_WIDTH - 1;
            while(c1 > c0 && OLED_ShadowBuf[page][c1] == OLED_DisplayBuf[page][c1]) c1--;
            OLED_SetPosRange(page, c0, c1);
            OLED_Write_Data_Bulk(&OLED_DisplayBuf[page][c0], c1 - c0 + 1);
            memcpy(OLED_ShadowBuf[page], OLED_DisplayBuf[page], OLED_WIDTH);
        }
    }
#else
    OLED_Update();  /* 虚拟后端：整屏发送即可 */
#endif
}

/* ================= 主题与亮度 ================= */
/* 1=反色(0xA7) 0=正常(0xA6)；虚拟 OLED 无反色指令，忽略 */
void OLED_SetColorMode(bool colormode)
{
#if(!VIRTUAL_OLED)
    if(colormode)
        OLED_Write_Command(0xA7);
    else
        OLED_Write_Command(0xA6);
#else
    (void)colormode;
#endif
}

/* 亮度 0~255；实体 0x81 对比度寄存器，虚拟 OLED12864_SetContrast */
void OLED_Brightness(int16_t Brightness)
{
    if(Brightness < 0)   Brightness = 0;
    if(Brightness > 255) Brightness = 255;

#if(VIRTUAL_OLED)
    OLED12864_SetContrast((uint8_t)Brightness);
#else
    OLED_Write_Command(0x81);
    OLED_Write_Command((uint8_t)Brightness);
#endif
}
