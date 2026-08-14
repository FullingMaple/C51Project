/*==============================================================================
 * stdint.h —— C51 shim（Keil C51 无标准 stdint）
 * 说明：C51 中 unsigned int 为 16 位、unsigned long 为 32 位
 *============================================================================*/
#ifndef STDINT_H
#define STDINT_H

typedef unsigned char   uint8_t;
typedef unsigned int    uint16_t;
typedef unsigned long   uint32_t;
typedef signed char     int8_t;
typedef signed int      int16_t;
typedef signed long     int32_t;

#define UINT8_MAX   255
#define INT8_MAX    127
#define UINT16_MAX  65535
#define INT16_MAX   32767
#define UINT32_MAX  4294967295UL
#define INT32_MAX   2147483647L

#endif
