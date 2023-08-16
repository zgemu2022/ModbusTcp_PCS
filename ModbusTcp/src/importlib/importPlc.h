#ifndef _IMPORT_PLC_
#define _IMPORT_PLC_
#include "logicAndControl.h"
#define PORTNUM_MAX 2
#define MAX_CONN_NUM 2

// 0 BIT0备用
#define PLC_EMU_BOX_35kV_ON 1// BIT135kV进线柜合闸
#define PLC_EMU_BOX_35kV_OFF 2 // BIT235kV进线柜分闸
#define PLC_EMU_BOX_SwitchD1_ON 3// BIT3开关柜D1合闸
#define PLC_EMU_BOX_SwitchD1_OFF 4// BIT4开关柜D1分闸
#define PLC_EMU_BOX_SwitchD2_ON 5// BIT5开关柜D2合闸
#define PLC_EMU_BOX_SwitchD2_OFF 6// BIT6开关柜D2分闸
#define PLC_EMU_TRANSFORMER_ROOM_OPEN 7// BIT7变压器室外门打开
#define PLC_EMU_DSTART 8  // BIT0系统一键放电
#define PLC_EMU_STOP   9  // BIT1系统一键停止
#define PLC_EMU_CSTART 10  // BIT2系统一键充电

typedef int (*orderToLcd)(int); //输出指令


// #define Emu_Startup 1				  //【整机】启机命令
// #define Emu_Stop 2					  //【整机】停机命令
// #define Parallel_Away_conversion_en 3 //并转离切换使能
// #define Away_Parallel_conversion_en 4 //离转并切换使能
// #define EMS_SET_MODE 5				  //产品运行模式设置
// #define EMS_VSG_MODE 6				  // VSG工作模式设置
// #define EMS_PQ_MODE 7				  // PQ工作模式设置
// #define BOX_35kV_ON 8				  // 35kV进线柜合闸
// #define BOX_35kV_OFF 9			  // 35kV进线柜分闸
// #define BOX_SwitchD1_ON 10			  //开关柜D1合闸
// #define BOX_SwitchD1_OFF 11			  //开关柜D1分闸
// #define BOX_SwitchD2_ON 12			  //开关柜D2合闸
// #define BOX_SwitchD2_OFF 13			  //开关柜D2分闸
// #define EMS_PW_SETTING 14			  //有功功率
// #define EMS_QW_SETTING 15			  //无功功率
// #define ONE_FM_PW_SETTING 16		  //一次调频有功功率
// #define ONE_FM_QW_SETTING 17		  //一次调频无功功率



typedef struct
{
	char server_ip[64];
	unsigned short server_port;
	unsigned char lcdnum;
	unsigned char pcsnum[MAX_PCS_NUM];
	orderToLcd funOrder;
	int flag_RecvNeed_LCD;
} PARA_PLC; //从主控传到plc模块的结构

void Plc_Init(void);
#endif