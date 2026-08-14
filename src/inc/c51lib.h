/*==============================================================================
 * c51lib.h —— C51 标准库函数原型 shim
 * Keil 的 math.h/string.h/stdio.h 被 STC 增强版覆盖（含 C99/C++ 扩展，
 * C51 编译器无法解析），这里手动声明本项目实际使用的库函数原型
 *============================================================================*/
#ifndef C51LIB_H
#define C51LIB_H

#include "stdint.h"

/* ---- string ---- */
extern int          strcmp(const char *s1, const char *s2);
extern unsigned int strlen(const char *s);

/* ---- stdio ---- */
extern int          sprintf(char *buf, const char *fmt, ...);
extern int          vsprintf(char *buf, const char *fmt, char *argptr);

/* ---- math ---- */
extern float atan2(float, float);
extern float sin(float);
extern float cos(float);
extern float fabs(float);
extern float ceil(float);

#endif
