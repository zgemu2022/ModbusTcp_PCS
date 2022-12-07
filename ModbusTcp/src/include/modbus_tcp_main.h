#ifndef _MODBUS_TCP_MAIN_H_
#define _MODBUS_TCP_MAIN_H_

#include "share_type.h"
#include "lib_time.h"
#include "logicAndControl.h"

//#define ifDebug

#define _ZJYX_ 0
#define _YX_ 1
#define _YC_ 2
#define _ZJYC_ 3

#define _BMS_YX_ 0
#define _BMS_YK_ 1
#define _PCS_YK_ 2

#define MASTER 1
#define SLAVE 2
#define MAX_YCDATA_NUM 32

enum LIB_NAME // 模块名称编号
{

	LIB_LCD = 0,
	LIB_61850 = 1,
	LIB_BMS = 2,
	LIB_PLC = 3,

};
// #define LIB_61850   1
// #define LIB_BMS     2
// #define LIB_PLC     3
// typedef struct
// {
// 	char type; //1 Master 2 Slave
// 	unsigned char lcdnum;
// 	unsigned char pcsnum;
// 	char  server_ip[64];
// 	unsigned short server_port;
// } PARA_FROM_EMU_APP; //从主控传到Modbus-Tcp模块的结构


typedef struct {
	int firm_id;//公司编号
//LCD参数
	char  lcd_server_ip[6][64];
	u32 lcd_server_port[6];
    int lcd_num;
    int pcs_num;
    char plc_server_ip[64];
    u32 plc_server_port;
    int balance_rate;
    int sys_max_pw;
    int flag_init_lcd;	
////
//    char bams1_pcsid[8];
//    char bams2_pcsid[8];

//系统参数
	char iflog;//是否将协议日志记录到flash中
	char hardware_ver;//硬件版本
	TDateTime StartLogDay;
	int logdays;
	char main_ver[6];


}pconf;
typedef struct
{
	int sn;
	int lcdid;
	int pcsid;
	unsigned short pcs_data[MAX_YCDATA_NUM];
	unsigned char data_len;
} LCD_YC_YX_DATA; //

//回调

typedef int (*outData2Other)(unsigned char, void *pdata); //输出数据签名
typedef int (*CallbackYK)(unsigned char, void *pdata);	  //遥控回调函数签名
extern CallbackYK pbackBmsFun;
typedef struct _post_list_t
{
	outData2Other pfun;
	unsigned char type;
	struct _post_list_t *next;
} post_list_t;

int modbus_tcp_main(void *para_app);

int SubscribeLcdData(unsigned char type, outData2Other pfun);
int ykOrderFromBms(unsigned char type, YK_PARA *pYkPara, CallbackYK pfun);
#endif