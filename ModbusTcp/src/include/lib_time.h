/*
 * lib_time.h
 *
 *  Created on: 2017-12-19
 *      Author: root
 */

#ifndef LIB_TIME_H_
#define LIB_TIME_H_
#include "share_type.h"
#ifdef __cplusplus
extern "C"{
#endif
#define YY 0x01
#define MM 0x02
#define DD 0x04
#define WW 0x08
#define HH 0x10
#define MIN 0x20
#define SEC 0x40

#define MIN_YEAR 2000
#define MAX_MONTH 12
#define MIN_MONTH 1
#define MAX_DAY 31
#define MIN_DAY 1
#define MAX_HOUR 23
#define MIN_HOUR 0
#define MAX_MIN 60
#define MIN_MIN 0
#define MAX_SEC 60
#define MIN_SEC 0

#define MAX_SEC_PER_HOUR 60
#define MAX_MIN_PER_DAY 1440

typedef struct {
	u8 Month;
	u8 Day;
	u16 Year;//外部全部统一用4位年份
	u8 Week;
	u8 Sec;
	u8 Min;
	u8 Hour;
	u16 Msec;//毫秒，0.001
} TDateTime; //10个字节

void read_current_datetime(TDateTime *datetime);
s32 diff_DateTime_Min(TDateTime *datetime1, TDateTime *datetime2);
s32 diff_DateTime_Sec(TDateTime *datetime1, TDateTime *datetime2);
s32 cmp_DateTime(TDateTime *datetime1, TDateTime *datetime2);
u8 countWeek(u8 *datetime);
u16 getCurDayOfYear(u16 year, u8 month, u8 day);
void add_DateTime_Day(TDateTime *datetime, u32 day);

unsigned int date2timestamp(TDateTime *datetime_s);
#ifdef __cplusplus

}

#endif
#endif /* LIB_TIME_H_ */
