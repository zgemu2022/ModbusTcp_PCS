#ifndef _IMPORT_PLC_
#define _IMPORT_PLC_
#include "logicAndControl.h"
#define PORTNUM_MAX 2
#define MAX_CONN_NUM 2

typedef int (*orderToLcd)(int); //输出指令



typedef struct
{
	char server_ip[64];
	unsigned short server_port;
	unsigned char lcdnum;
	unsigned char pcsnum[MAX_PCS_NUM];
	orderToLcd funOrder;
} PARA_PLC; //从主控传到plc模块的结构

void Plc_Init(void);
#endif