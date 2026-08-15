#ifndef __OLED_FONT_H
#define __OLED_FONT_H

#include <stdint.h>



/*中文字符字节宽度*/
#define OLED_CHN_CHAR_WIDTH			(2)		//UTF-8编码格式给3，GB2312编码格式给2

/*字模基本单元*/
typedef struct 
{
	char Index[OLED_CHN_CHAR_WIDTH + 1];	//汉字索引
	uint8_t Data[60];						//字模数据
} ChineseCell20x20_t;
typedef struct 
{
	char Index[OLED_CHN_CHAR_WIDTH + 1];	//汉字索引
	uint8_t Data[32];						//字模数据
} ChineseCell16x16_t;
typedef struct 
{
    char Index[OLED_CHN_CHAR_WIDTH + 1];   // 汉字索引
    uint8_t Data[24];                      // 字模数据，12*12的汉字需要24字节
} ChineseCell12x12_t;

typedef struct 
{
    char Index[OLED_CHN_CHAR_WIDTH + 1];   // 汉字索引
    uint8_t Data[8];                      // 字模数据，8*8的汉字需要24字节
} ChineseCell8x8_t;

/*ASCII字模数据声明*/
extern code uint8_t OLED_F8x16[][16];
extern code uint8_t OLED_F7x12[][14];
extern code uint8_t OLED_F6x8[][6];

/*汉字字模数据声明*/
extern code ChineseCell16x16_t OLED_CF16x16[];
extern code ChineseCell12x12_t OLED_CF12x12[];  // 声明12x12字模数组
extern code ChineseCell8x8_t OLED_CF8x8[];  // 声明12x12字模数组
/*图像数据声明*/
extern code uint8_t Arrow[];
extern code uint8_t Arrow1[];
extern code uint8_t UnKnown[];
extern code uint8_t Image_serial[];
extern code uint8_t Image_gamepad[];
extern code uint8_t Image_calc2[];
extern code uint8_t Image_thermo[];
extern code uint8_t Image_gear[];
extern code uint8_t Image_window[];
extern code uint8_t Image_cube[];
extern code uint8_t Image_more[];
extern code uint8_t Image_sleep[];


extern code uint8_t Image_settings_64[];
extern code uint8_t Image_calc_64[];
extern code uint8_t Image_wechat_64[];
extern code uint8_t Image_alipay_64[];
extern code uint8_t Image_night_64[];
extern code uint8_t Image_more_64[];

/*按照上面的格式，在这个位置加入新的图像数据声明*/
//...

#endif

