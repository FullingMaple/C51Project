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
#include "calendar.h"

extern bool ColorMode;
extern bool OLED_UI_FpsShow;
extern bool SoundEnable;

#define SPEED 8

/* ================= 辅助函数（框架回调，暂为空） ================= */
static void MainAuxFunc(void){}
static void SettingAuxFunc(void){}

/* ================= 磁贴主屏菜单项 ================= */
MenuItem MainMenuItems[] = {
    {"时钟",   NULL, &ClockMenuPage,         NULL, NULL, NULL, Image_clock,     NULL, 0, 0},
    {"设置",   NULL, &SettingsMenuPage,        NULL, NULL, NULL, Image_gear,      NULL, 0, 0},
    {"关于",   NULL, &AboutThisDeviceMenuPage, NULL, NULL, NULL, Image_more,         NULL, 0, 0},
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


/* ================= 万年历 ================= */
static MenuItem ClockMenuItems[] = {
    {NULL}
};

static const char code *ClockWeekStr[7] = {"一","二","三","四","五","六","日"};

/* 每帧绘制：日期行 + 表头 + 日历格（今天反色） */
static void ClockAuxFunc(void)
{
    RTC_Time t;
    uint8_t dim, w1, idx, i, row, col;
    int16_t x, y, wpx;

    RTC_GetTime(&t);

    /* 日期行：2026年08月18日 周二（12x12 中文 + 6x8 数字，居中） */
    wpx = CalcStringWidth(OLED_12X12_FULL, OLED_6X8_HALF, "%04d年%02d月%02d日 周");
    OLED_PrintfMix((128 - wpx - 12) / 2, 0, OLED_12X12_FULL, OLED_6X8_HALF,
        "%04d年%02d月%02d日 周%s", t.year, t.month, t.day,
        ClockWeekStr[Cal_Weekday(t.year, t.month, t.day) - 1]);

    /* 表头：日 一 二 三 四 五 六 */
    wpx = CalcStringWidth(OLED_12X12_FULL, OLED_6X8_HALF, "日 一 二 三 四 五 六");
    OLED_ShowMixString((128 - wpx) / 2, 12, "日 一 二 三 四 五 六", OLED_12X12_FULL, OLED_6X8_HALF);

    /* 日历格（6x8，今天反色；第 6 行溢出画右侧） */
    dim = Cal_DaysInMonth(t.year, t.month);
    w1 = Cal_Weekday(t.year, t.month, 1);
    for(i = 1; i <= dim; i++) {
        idx = (uint8_t)((w1 - 1) + (i - 1));
        row = idx / 7;
        col = idx % 7;
        x = 22 + (int16_t)col * 12;
        y = 24 + (int16_t)row * 8;
        if(row >= 5) { x = 100 + (int16_t)(i - 29) * 8; y = 56; }
        OLED_ShowNum(x, y, i, (i >= 10) ? 2 : 1, OLED_6X8_HALF);
        if(i == t.day) OLED_ReverseArea(x, y, 8, 8);
    }
}

MenuPage ClockMenuPage = {
    MENU_TYPE_LIST, SPEED, NOT_SHOW, UNLINEAR, OLED_UI_FONT_12,
    &MainMenuPage, ClockMenuItems, 0, ClockAuxFunc,
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
