#ifndef _MODBUS_TCP_MAIN_H_
#define _MODBUS_TCP_MAIN_H_
#define MAX_PCS_NUM   6

#define _ZJYX_  0
#define _YX_  1
#define _YC_  2
#define _ZJYC_  3

#define MASTER   1
#define SLAVE    2
#define MAX_YCDATA_NUM   32

#include "share_type.h"
#include "lib_time.h"
enum LIB_NAME							   // 模块名称编号
{

	LCD = 1,		 //LCD模块
    BAMS =2,
	PLC =3


};

// typedef struct
// {
// 	char type; //1 Master 2 Slave
// 	unsigned char lcdnum;
// 	unsigned char pcsnum;
// 	char  server_ip[64];
// 	unsigned short server_port;
// } PARA_FROM_EMU_APP; //从主控传到Modbus-Tcp模块的结构

typedef struct
{
	int firm_id; //公司编号
				 // LCD参数
	char lcd_server_ip[64];
	u32 lcd_server_port;
	int lcd_num;
	int pcs_num;
	int balance_rate;

	//系统参数
	char iflog;		   //是否将协议日志记录到flash中
	char hardware_ver; //硬件版本
	TDateTime StartLogDay;
	int logdays;
	char main_ver[6];

} pconf;


typedef struct
{
	int sn;
	int lcdid;
	int pcsid;
	unsigned short pcs_data[MAX_YCDATA_NUM];
	unsigned char data_len;
}LCD_YC_YX_DATA;//

//回调
typedef int			(*outData2Other)(unsigned char ,void *pdata);	//输出数据

typedef struct _post_list_t
{
	outData2Other  pfun;
	unsigned char type;
    struct _post_list_t *next;
}post_list_t;
int modbus_tcp_main(void *para_app);

int SubscribeLcdData(unsigned char type,outData2Other pfun);
#endif