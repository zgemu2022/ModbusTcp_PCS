#ifndef _LOGIC_AND_CONTROL_H_
#define _LOGIC_AND_CONTROL_H_
#include "logicAndControl.h"

#define EMS_communication_status 0
#define one_FM_GOOSE_link_status_A 1 //  一次调频A网GOOSE链路状态
#define one_FM_GOOSE_link_status_B 2 //  一次调频B网GOOSE链路状态
#define one_FM_Enable 3				 //  一次调频使能
#define one_FM_Disable 4			 //  一次调频使能

#define Emu_Startup 1				  //【整机】启机命令
#define Emu_Stop 2					  //【整机】停机命令
#define Parallel_Away_conversion_en 3 //并转离切换使能
#define Away_Parallel_conversion_en 4 //离转并切换使能
#define EMS_SET_MODE 5				  //产品运行模式设置
#define EMS_VSG_MODE 6				  // VSG工作模式设置
#define EMS_PQ_MODE 7				  // PQ工作模式设置
#define BOX_35kV_ON 8				  // 35kV进线柜合闸
#define BOX_35kV_OFF 9,				  // 35kV进线柜分闸
#define BOX_SwitchD1_ON 10			  //开关柜D1合闸
#define BOX_SwitchD1_OFF 11			  //开关柜D1分闸
#define BOX_SwitchD2_ON 12			  //开关柜D2合闸
#define BOX_SwitchD2_OFF 13			  //开关柜D2分闸
#define EMS_PW_SETTING 14			  //有功功率
#define EMS_QW_SETTING 15			  //无功功率
#define ONE_FM_PW_SETTING 16		  //一次调频有功功率
#define ONE_FM_QW_SETTING 17		  //一次调频无功功率
typedef struct
{
	unsigned char item;	   //项目编号
	unsigned char el_tag;  //  数据类型
	unsigned char data[5]; //参数
} YK_PARA;				   //遥测、遥控参数

extern int total_pcsnum;
extern int g_flag_RecvNeed;
extern int g_flag_RecvNeed_LCD;
extern int g_flag_RecvNeed_PCS;
int handleYkFromEms(YK_PARA *pYkPara);
unsigned int countRecvFlag(int num_read);
int handleYxFromEms(int item, unsigned char data);
// int handleYkFromEms(int item, unsigned char data);
int countDP(int sn, unsigned short *pPw);
// unsigned short checkPcsStatus(int type, int lcdid, int pcsid);
#endif