/*==============================================================================
 * OLED_UI_MenuData.c —— 本项目菜单数据（C51 位置初始化，GBK 编码）
 * 磁贴主屏：设置 / 关于 / 测温 / 串口 / 计算器 / 游戏
 * 设置页：深浅色 / 帧率 / 语言占位 / 提示音占位 / 返回
 * 关于页：项目信息
 * 注：中文按 GB2312 查字库，本文件必须保持 GBK 编码
 *============================================================================*/
#include "OLED_UI_MenuData.h"
#include "OLED_Fonts.h"
#include "RTC.h"
#include "EEPROM.h"
#include "timeedit.h"
#include "calendar.h"   /* Cal_Weekday（显示模式日期行星期） */
#include "OLED_UI_Driver.h"   /* Buzzer_Beep / Delay_ms（保存失败提示） */

extern bool ColorMode;
extern MenuPage *CurrentMenuPage;   /* 保存失败时屏蔽/恢复框架返回 */
extern bool OLED_UI_FpsShow;
extern bool SoundEnable;

#define SPEED 8

/* ================= 辅助函数（框架回调，暂为空） ================= */
static void MainAuxFunc(void){}
static void SettingAuxFunc(void){}

/* ================= 磁贴主屏菜单项 ================= */
MenuItem MainMenuItems[] = {
    {"设置",   NULL, &SettingsMenuPage,        NULL, NULL, NULL, Image_gear,      NULL, 0, 0},
    {"关于",   NULL, &AboutThisDeviceMenuPage, NULL, NULL, NULL, Image_more,         NULL, 0, 0},
    {"时间",   NULL, &ClockMenuPage,         NULL, NULL, NULL, Image_clock,       NULL, 0, 0},
    {"测温",   NULL, NULL,                     NULL, NULL, NULL, Image_thermo,       NULL, 0, 0},
    {"串口",   NULL, NULL,                     NULL, NULL, NULL, Image_serial,       NULL, 0, 0},
    {"计算器", NULL, NULL,                     NULL, NULL, NULL, Image_calc2,        NULL, 0, 0},
    {"游戏",   NULL, NULL,                     NULL, NULL, NULL, Image_gamepad,      NULL, 0, 0},
    {NULL}
};

/* ================= 设置页 ================= */
MenuItem SettingsMenuItems[] = {
    {"深浅色", NULL,              NULL, &ColorMode,         NULL, NULL, NULL, NULL, 0, 0},
    {"显示帧率", NULL,            NULL, &OLED_UI_FpsShow,   NULL, NULL, NULL, NULL, 0, 0},
    {"语言",   NULL,              NULL, NULL, NULL, NULL, NULL, NULL, 0, 0},
    {"提示音", NULL,              NULL, &SoundEnable, NULL, NULL, NULL, NULL, 0, 0},
    {"[返回]", OLED_UI_Back,      NULL, NULL, NULL, NULL, NULL, NULL, 0, 0},
    {NULL}
};

/* ================= 关于页 ================= */
MenuItem AboutThisDeviceMenuItems[] = {
    {" 电子手表式UI菜单", NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0},
    {" MCU: STC8H8K64U", NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0},
    {" 屏: 0.96 SSD1306",  NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0},
    {" 框架: OLED_UI",    NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0},
    {" 许可: Apache 2.0",       NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0},
    {"[返回]", OLED_UI_Back,    NULL, NULL, NULL, NULL, NULL, NULL, 0, 0},
    {NULL}
};



/* ================= 时间页（0 菜单项：显示模式 + 编辑模式） ================= */
static MenuItem TimeMenuItems[] = {
    {NULL}
};

static const char *ClockWeekStr[7] = {"一","二","三","四","五","六","日"};   /* 日期行星期（C51 %s 需通用指针） */

static RTC_Time Ts_Edit;              /* 编辑缓冲 */
static uint8_t  Ts_Field = 0;         /* 0=年 1=月 2=日 3=时 4=分 5=秒 */
static uint8_t  Ts_Ready = 0;         /* 进入后首帧装载当前时间 */
static uint8_t  Ts_LastKey = 0;       /* 键边沿去重 */
static uint8_t  Ts_Editing = 0;       /* 0=显示模式 1=编辑模式 */
static uint8_t  Ts_NeedExit = 0;      /* 保存失败：诊断展示后退出编辑 */
static uint32_t Ts_ExitTick = 0;      /* 延迟退出时刻（GetTick 毫秒） */

/* 保存 RTC + EEPROM：成功退回显示模式并恢复框架返回；失败三声+诊断停留 */
static void TimeSet_Commit(void)
{
    uint8_t i;

    RTC_SetTime(&Ts_Edit);
    if(EEPROM_SaveTime(&Ts_Edit) != 0){
        for(i = 0; i < 3; i++){ Buzzer_Beep(); Delay_ms(150); }   /* 保存失败：三声（成功为单声） */
        Ts_NeedExit = 1;
        Ts_ExitTick = GetTick() + 3000;
    }else{
        Ts_NeedExit = 0;
        Ts_Editing = 0;                                             /* 保存成功：退回显示模式 */
        CurrentMenuPage->General_ParentMenuPage = &MainMenuPage;    /* 恢复框架返回 */
    }
    Ts_Ready = 0;
}

/* 每帧：按键状态机（显示/编辑切换）+ 绘制 */
static void TimeAuxFunc(void)
{
    uint8_t k;
    int16_t x, wpx;
    RTC_Time now;

    if(!Ts_Ready){ RTC_GetTime(&Ts_Edit); Ts_Ready = 1; }

    /* 键盘：显示模式 3=进入编辑；编辑模式 1/2=调值 3=切字段(满6次保存) 4=保存 */
    k = Key_GetRawKey();
    if(k != 0 && Ts_LastKey == 0){
        if(Ts_Editing == 0){
            if(k == 3){
                Ts_Editing = 1;
                Ts_Field = 0;
                RTC_GetTime(&Ts_Edit);                            /* 编辑基准 = 当前真实时间 */
                CurrentMenuPage->General_ParentMenuPage = NULL;   /* 编辑中屏蔽框架返回（返回键=保存） */
            }
        }else{
            if(k == 1)      TimeEdit_Inc(&Ts_Edit, Ts_Field, 1);
            else if(k == 2) TimeEdit_Inc(&Ts_Edit, Ts_Field, -1);
            else if(k == 3){ Ts_Field = TimeEdit_NextField(Ts_Field);
                             if(Ts_Field == TE_FIELD_YEAR){ TimeSet_Commit(); Ts_Field = 0; } }
            else if(k == 4){ TimeSet_Commit(); Ts_Field = 0; }
        }
    }
    Ts_LastKey = k;

    if(Ts_Editing == 0){
        /* 显示模式：日期行（12x12 汉字 + 6x8 数字）+ 8x16 大时钟 + 底部提示 */
        RTC_GetTime(&now);
        wpx = CalcStringWidth(OLED_12X12_FULL, OLED_6X8_HALF, "%04d年%02d月%02d日 周",
            (int)now.year, (int)now.month, (int)now.day);
        OLED_PrintfMix((128 - wpx - 12) / 2, 4, OLED_12X12_FULL, OLED_6X8_HALF,
            "%04d年%02d月%02d日 周%s", (int)now.year, (int)now.month, (int)now.day,
            ClockWeekStr[Cal_Weekday(now.year, now.month, now.day) - 1]);
        OLED_Printf(32, 22, OLED_8X16_HALF, "%02d:%02d:%02d",
            (int)now.hour, (int)now.minute, (int)now.second);
        OLED_ShowMixString(37, 50, "确定=编辑", OLED_12X12_FULL, OLED_6X8_HALF);
        return;
    }

    /* 编辑模式：标题 + 日期行（12x12 汉字 + 7x12 数字）与时间行（7x12）统一字号、水平居中
     * 注意：变参 printf 中 uint8_t 按 1 字节压栈而 %d 读 2 字节会粘连，
     *       必须 (int) 强转（C51 经典坑） */
    OLED_ShowMixString(40, 2, "时间设置", OLED_12X12_FULL, OLED_6X8_HALF);
    OLED_PrintfMix(18, 18, OLED_12X12_FULL, OLED_7X12_HALF, "%04d年%02d月%02d日",
        (int)Ts_Edit.year, (int)Ts_Edit.month, (int)Ts_Edit.day);
    OLED_Printf(36, 36, OLED_7X12_HALF, "%02d:%02d:%02d",
        (int)Ts_Edit.hour, (int)Ts_Edit.minute, (int)Ts_Edit.second);

    /* 选中字段反色（与两行格子精确对齐） */
    switch(Ts_Field){
        case 0: x = 18;  OLED_ReverseArea(x, 18, 28, 12); break;   /* 年 "2026" 28px */
        case 1: x = 58;  OLED_ReverseArea(x, 18, 14, 12); break;   /* 月 */
        case 2: x = 84;  OLED_ReverseArea(x, 18, 14, 12); break;   /* 日 */
        case 3: x = 36;  OLED_ReverseArea(x, 36, 14, 12); break;   /* 时 */
        case 4: x = 57;  OLED_ReverseArea(x, 36, 14, 12); break;   /* 分 */
        case 5: x = 78;  OLED_ReverseArea(x, 36, 14, 12); break;   /* 秒 */
    }

    /* 底部操作提示（纯 ASCII 6x8；保存失败时被诊断行覆盖） */
    OLED_ShowString(22, 52, "1/2=CHG 3=NEXT 4=SAVE", OLED_6X8_HALF);

    /* 保存失败诊断：底部 6x8 "EEP S1 B3 05>FF"（阶段/字节/期望>读回），3 秒后退出编辑 */
    if(Ts_NeedExit){
        OLED_Printf(0, 52, OLED_6X8_HALF, "EEP S%1d B%1d %02X>%02X",
            (int)EEP_Diag_Stage, (int)EEP_Diag_Idx,
            (int)EEP_Diag_Expect, (int)EEP_Diag_Got);
        if((int32_t)(GetTick() - Ts_ExitTick) >= 0){
            Ts_NeedExit = 0;
            Ts_Editing = 0;
            CurrentMenuPage->General_ParentMenuPage = &MainMenuPage;   /* 恢复框架返回 */
        }
    }
}

MenuPage ClockMenuPage = {
    MENU_TYPE_LIST, SPEED, NOT_SHOW, UNLINEAR, OLED_UI_FONT_12,
    &MainMenuPage, TimeMenuItems, 0, TimeAuxFunc,
    {0, 0, 128, 64}, 0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, {0, 0}
};

/* ================= 页面 ================= */
MenuPage MainMenuPage = {
    /* 通用属性 */
    MENU_TYPE_TILES, SPEED, NOT_SHOW, UNLINEAR, OLED_UI_FONT_16,
    NULL, MainMenuItems, 5, MainAuxFunc,
    /* 列表属性（磁贴不用） */
    {0, 0, 0, 0}, 0, 0, 0, 0,
    /* 磁贴属性 */
    128, 64, 32, 32,
    /* 运行态 */
    0, 0, {0, 0}
};

MenuPage SettingsMenuPage = {
    MENU_TYPE_LIST, SPEED, REVERSE_ROUNDRECTANGLE, UNLINEAR, OLED_UI_FONT_12,
    &MainMenuPage, SettingsMenuItems, 4, SettingAuxFunc,
    {0, 0, 128, 64}, 0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, {0, 0}
};

MenuPage AboutThisDeviceMenuPage = {
    MENU_TYPE_LIST, SPEED, REVERSE_ROUNDRECTANGLE, UNLINEAR, OLED_UI_FONT_12,
    &MainMenuPage, AboutThisDeviceMenuItems, 4, SettingAuxFunc,
    {0, 0, 128, 64}, 0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, {0, 0}
};
