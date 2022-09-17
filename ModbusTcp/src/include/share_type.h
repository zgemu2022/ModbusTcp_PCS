/*
 *********************************************************************************************************
 * Filename      : share_type.h
 * Version       : $Rev: 59 $
 * Description   : �������Ͷ���
 *
 *********************************************************************************************************
 */

#ifndef SHARE_TYPE_H_
#define SHARE_TYPE_H_
#ifdef __cplusplus
extern "C"{
#endif
/*********************************************************************************************************
 *                                               MACRO'S
 *********************************************************************************************************/
typedef signed long long s64;
typedef signed long s32;
typedef signed short s16;
typedef signed char s8;

typedef signed long const sc32; /* Read Only */
typedef signed short const sc16; /* Read Only */
typedef signed char const sc8; /* Read Only */

typedef volatile signed long vs32;
typedef volatile signed short vs16;
typedef volatile signed char vs8;

typedef volatile signed long const vsc32; /* Read Only */
typedef volatile signed short const vsc16; /* Read Only */
typedef volatile signed char const vsc8; /* Read Only */

typedef unsigned long long u64;
typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef unsigned long const uc32; /* Read Only */
typedef unsigned short const uc16; /* Read Only */
typedef unsigned char const uc8; /* Read Only */

typedef volatile unsigned long vu32;
typedef volatile unsigned short vu16;
typedef volatile unsigned char vu8;

typedef volatile unsigned long const vuc32; /* Read Only */
typedef volatile unsigned short const vuc16; /* Read Only */
typedef volatile unsigned char const vuc8; /* Read Only */

typedef float fp32; /* Read Only */
typedef double fp64; /* Read Only */

typedef enum
{
    False = 0,
    True = !False
} boolean;
#define    OFF   0
#define    ON    1


typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;


#define FP_MAX     1.7976931348623158e+308 /* max value */
#define FP_MIN     2.2250738585072014e-308 /* min positive value */

#define ERROR16   0xFFFF
//取数组的元素个数
#define ARRAY_LEN(x) (sizeof((x))/sizeof((x)[0]))
//取结构体成员的偏移地址，s_name是结构体名，m_name是成员名
#define OFFSET_OF(s_name, m_name)  (u32)&(((s_name*)0)->m_name)
/*********************************************************************************************************
 *                                          LOCAL DATA TYPES
 *********************************************************************************************************/
typedef struct
{
    // 处理完一个参数 ，格式归零
    u8 format_type; //'d'整型，同实现格式
    u8 align_direct; //0 右对齐  1左对齐
    u8 zero_space; //0 填空格  1填'0'
    u8 align_len; // 字符宽度，不包括符号
    u8 size_type; //0 int型  1 整型的long 实数的double型   2 short型
    u16 digit_size; //精度，默认是6
} TFormatUnit;

#ifdef __cplusplus
}
#endif
#endif
/* End of SHARE_TYPE_H_ module include.*/
