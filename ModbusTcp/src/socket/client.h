#ifndef _CLIENT_H_
#define _CLIENT_H_
#include "modbus_tcp_main.h"
#define MAX_MODBUS_FLAME 1024

#define STATUS_ON 1
#define STATUS_OFF 0

typedef struct
{
	int msgtype;
	char data[MAX_MODBUS_FLAME];
} msgClient;
typedef struct
{
	unsigned char buf[MAX_MODBUS_FLAME]; //数据
	int len;							 // buf的长度
} MyData;

typedef struct
{
	unsigned short frameId;					 //帧序号
	unsigned char recvbuf[MAX_MODBUS_FLAME]; //收到的数据部分
	unsigned short lenrecv;					 //收到的数据长度

} ModTcp_Frame;

extern char modbus_sockt_state[];
extern int modbus_client_sockptr[];
extern unsigned int modbus_sockt_timer[];


void CreateThreads(void);

#endif //_CLIENT_H_