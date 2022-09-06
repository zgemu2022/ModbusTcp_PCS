#ifndef _MODBUS_TCP_MAIN_H_
#define _MODBUS_TCP_MAIN_H_
#define MAX_PCS_NUM   6


#define MASTER   1
#define SLAVE    2
typedef struct
{
	char type; //1 Master 2 Slave
	unsigned char lcdnum;
	unsigned char pcsnum[MAX_PCS_NUM];
	unsigned char  devNo[MAX_PCS_NUM];
	char  server_ip[MAX_PCS_NUM][64];
	unsigned short server_port[MAX_PCS_NUM];
} PARA_MODTCP; //从主控传到Modbus-Tcp模块的结构1



#endif