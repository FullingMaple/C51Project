/*==============================================================================
 * OLED_UI_MenuData.c —— 本项目菜单数据（C51 位置初始化，GBK 编码）
 * 磁贴主屏：设置 / 关于 / 测温 / 串口 / 计算器 / 游戏
 * 设置页：亮度（窗口）/ 深浅色 / 帧率 / 语言占位 / 提示音占位 / 返回
 * 关于页：项目信息
 * 注：中文按 GB2312 查字库，本文件必须保持 GBK 编码
 *============================================================================*/
#include "OLED_UI_MenuData.h"
#include "OLED_Fonts.h"

extern bool ColorMode;
extern bool OLED_UI_FpsShow;
extern bool SoundEnable;
extern int16_t OLED_UI_Brightness;

#define SPEED 8

/* ================= 辅助函数（框架回调，暂为空） ================= */
static void MainAuxFunc(void){}
static void SettingAuxFunc(void){}

/* ================= 亮度调节窗口 ================= */
MenuWindow SetBrightnessWindow = {
    80, 28, 4.0, WINDOW_ROUNDRECTANGLE,     /* 宽/高/持续时间/类型 */
    "屏幕亮度", OLED_UI_FONT_12, 4, 3,      /* 标题/字号/边距 */
    NULL, &OLED_UI_Brightness,              /* 数据指针（float/int 二选一） */
    5.0, 5.0, 255.0,                        /* 步长/最小/最大 */
    3, 4, 8,                                /* 底部间距/边距/进度条高度 */
    0                                       /* _LineSlip */
};

void BrightnessWindow(void){
    OLED_UI_CreateWindow(&SetBrightnessWindow);
}

/* ================= 磁贴主屏菜单项 ================= */
MenuItem MainMenuItems[] = {
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
    {"亮度",   BrightnessWindow,  NULL, NULL, &OLED_UI_Brightness, NULL, NULL, NULL, 0, 0},
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
