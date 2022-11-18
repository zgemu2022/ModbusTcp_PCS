#ifndef _IMPORT_PLC_
#define _IMPORT_PLC_

#define PORTNUM_MAX 2
#define MAX_CONN_NUM 2

typedef struct
{
	char server_ip[64];
	unsigned short server_port;
} PARA_PLC; //从主控传到plc模块的结构

void Plc_Init(void);
#endif