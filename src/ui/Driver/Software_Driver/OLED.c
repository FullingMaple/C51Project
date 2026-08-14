/*==============================================================================
 * OLED.c —— 图形绘制层（最小版：帧缓冲 + 基础操作）
 * 完整绘制库（文字/图形/图标）随 OLED_UI 框架移植（v0.3 后半程）
 *============================================================================*/
#include "config.h"
#include "OLED_driver.h"
#include <string.h>

/* 帧缓冲：8 页 × 128 列（页优先，与 SSD1306 显存布局一致），xdata 1KB */
uint8_t xdata OLED_DisplayBuf[OLED_PAGES][OLED_WIDTH];

/* 清屏 */
void OLED_Clear(void)
{
    memset(OLED_DisplayBuf, 0, sizeof(OLED_DisplayBuf));
}
