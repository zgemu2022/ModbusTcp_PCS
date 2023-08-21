#ifndef _DEBUG_LCD_PCS_H_
#define _DEBUG_LCD_PCS_H_
#ifdef __cplusplus

extern "C"
{

#endif
#include "log.h"

#define TITLE_MAIN "[EMU:]"

#define _LCD_MAIN_ 1 // LCD相关调试
#if (_LCD_MAIN_ == 1)
#define lcd_debug(x, ...) log_debug_new(TITLE_MAIN, "[LCD]" #x "\n", ##__VA_ARGS__)
#else
#define lcd_debug(x, ...)
#endif

#define _LCD_PCS_ 1 // PCS相关调试
#if (_LCD_PCS_ == 1)
#define pcs_debug(x, ...) log_debug_new(TITLE_MAIN, "[PCS]" #x "\n", ##__VA_ARGS__) // 12345
#else
#define pcs_debug(x, ...)
#endif

#define _LCD_PCS_SYS_ 1 // LCD模块系统相关调试
#if (_LCD_PCS_SYS_ == 1)
#define sys_debug(x, ...) log_debug_new(TITLE_MAIN, "[SYS]" #x "\n", ##__VA_ARGS__) // 12345
#else
#define sys_debug(x, ...)
#endif

#define _LCD_BAMS_ 1 // LCD模块系统相关调试
#if (_LCD_BAMS_ == 1)
#define bms_debug(x, ...) log_debug_new(TITLE_MAIN, "[BMS]" #x "\n", ##__VA_ARGS__) // 12345
#else
#define bms_debug(x, ...)
#endif
#ifdef __cplusplus
}

#endif
#endif