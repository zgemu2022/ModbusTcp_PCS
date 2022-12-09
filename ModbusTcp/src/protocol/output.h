#ifndef _OUTPUT_H_
#define _OUTPUT_H_
#include "modbus_tcp_main.h"
typedef struct
{
	unsigned char lcdnum;
	unsigned char pcsnum[MAX_PCS_NUM];

	unsigned short balance_rate;
	int flag_RecvNeed_LCD
} PARA_61850; //从LCD到61850模块的结构

extern LCD_YC_YX_DATA g_YxData[];
int SaveYcData(int id_thread, int pcsid, unsigned short *pyc, unsigned char len);
int SaveYxData(int id_thread, int pcsid, unsigned short *pyx, unsigned char len);
int SaveZjyxData(int id_thread, unsigned short *pzjyx, unsigned char len);
int SaveZjycData(int id_thread, unsigned short *pzjyc, unsigned char len);
void initInterface61850(void);
void cleanYcYxData(void);
#endif