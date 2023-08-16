/*
 * tools.h
 *
 *  Created on: 2017-12-19
 *      Author: root
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#include "com_include.h"
#ifdef __cplusplus

extern "C"{

#endif
typedef char* lib_va_list;
#define NaN (0.0 / 0.0)
#define IsNaN(x) ((x) != (x))
#define INF (1.0 / 0.0)
#define IsINF(x) ((x > FP_MAX) || (x < -FP_MAX))

#define ASSERT(expr) ((expr) ? (void)0 : (void)assert_failed())
#define MAX_OUTPUT_BUFFERSIZE  512 //测试时不能小于17
#define MAX_OUTPUT_PRINT_SIZE  (MAX_OUTPUT_BUFFERSIZE*3)
extern char OutBuffer[];//3个* 1个换行 1个\0 ..表示未打印完全
extern char Str_Buffer[];//打印输出缓冲，超过最大忽略后面的内容
extern char OutBuffer1[];//3个* 1个换行 1个\0 ..表示未打印完全
extern char Str_Buffer1[];//打印输出缓冲，超过最大忽略后面的内容

extern u16 loadFormatUnit(TFormatUnit *format_unit, const char *format, lib_va_list *parg);
extern u16 formatOutputString(TFormatUnit *format_unit, char *str_data);
extern void assert_failed(void);
extern u8 hex2char(u8 hex);
extern char *lib_strcpy(char *dest, const char *src);
extern void ConsolePortPrint(char *buf);
extern void ConsolePortPrint1(char *buf);
extern char *lib_gcvt(fp64 value, u16 ndigit, char *buf);
extern void *lib_memcpy(void *dest, const void *src, u32 count);
extern char *lib_itoa(s32 num, char *str, u16 radix);
extern char *lib_uitoa(u32 num, char *str, u16 radix);
extern u32 lib_strlen(const char *str);
int strBcd2BcdArray(const char* str,char *buf);
u8 byte2bcd(u8 num);
u8 bcd2byte(u8 bcd);
u8* lib_reverse(u8* data, int len);
u8 CountCS(u8* pData,u16 usLen);
u32 bcdtouint(const u8* psrc_buf, u32 data_len);
s32 bcdtosint(const u8* psrc_buf, u32 data_len);
#ifdef __cplusplus

}
#endif
#endif /* TOOLS_H_ */
