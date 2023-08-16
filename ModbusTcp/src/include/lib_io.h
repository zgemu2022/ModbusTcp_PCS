/*
 * lib_io.h
 *
 *  Created on: 2017-12-19
 *      Author: root
 */

#ifndef LIB_IO_H_
#define LIB_IO_H_
#ifdef __cplusplus

extern "C"{

#endif
#include "com_include.h"
#include <stdio.h>

//typedef char* lib_va_list;
u16 lib_vsprintf(char* buf, char* out_buf, const char *format, char* arg, boolean float_fix);
u16 lib_sprintf(char* buf, const char *format, ...);
u16 lib_vprintf(const char *format, char* arg);

u16 lib_printf(const char *format, ...);

/********************************************************************************************************
 *                                            Type Definition
 *********************************************************************************************************/

/*********************************************************************************************************
 *                                               MACRO'S
 *********************************************************************************************************/
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define lib_va_start(ap,v)  ( ap = (lib_va_list)&v + _INTSIZEOF(v) )

#define lib_va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )

#define lib_va_end(ap)      ( ap = (lib_va_list)0 )

#ifdef __cplusplus
}
#endif
#endif /* LIB_IO_H_ */
