#ifndef _CLIENT_H_
#define _CLIENT_H_
#include "modbus_tcp_main.h"
#define MAX_MODBUS_FLAME 1024

#define STATUS_ON		1
#define STATUS_OFF		0


typedef struct
 {
	  int msgtype;
	  char data[MAX_MODBUS_FLAME];
}msgClient;
typedef struct
{
	unsigned char  buf[MAX_MODBUS_FLAME];	//数据
	int len;	//buf的长度
}MyData;

typedef struct
{
	unsigned short frameId;//帧序号
	unsigned char recvbuf[MAX_MODBUS_FLAME];//收到的数据部分
	unsigned short lenrecv;//收到的数据长度

}ModTcp_Frame;
typedef struct
{
	unsigned short RegStart;//寄存器开始地址
	unsigned short numData;//抄取数据个数
	unsigned short totalData; //总数据个数

}Pcs_Fun03_Struct;
typedef struct
{
	unsigned char dev_id;//从设备地址
	unsigned char code_fun;//功能码
	unsigned char errflag;//功能码
	unsigned short addr;
	unsigned char recvdata[255];//收到的数据部分
	unsigned int lenrecv;//收到的数据长度



}PcsData;


extern char modbus_sockt_state[];
extern int modbus_client_sockptr[];
extern unsigned int modbus_sockt_timer[];

extern PARA_MODTCP *pPara_Modtcp;
void CreateThreads(void);

#endif//_CLIENT_H_