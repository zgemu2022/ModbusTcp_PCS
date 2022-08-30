#ifndef _MODBUS_H
#define _MODBUS_H

#include "modbus_tcp_main.h"
#define MAX_LCD_NUM   6



#define _NO_RESET     0
#define _NEED_RESET   1
#define PQ         1 
#define VSG        5

//VSG工作模式
#define  VSG_SCF_SCV  3//3：一次调频、一次调压 ；
#define  VSG_SCF_PQ   6//6：一次调频、并网无功 ；
#define  VSG_PP_SCV   9//9：并网有功、一次调压；
#define  VSG_PQ_PP   12//12：并网无功、并网有功；

//PQ工作模式设置
#define PQ_STP    0//0：恒功率模式；
#define PQ_STA    3 //3：恒流模式；


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
	unsigned char errflag;//错误码
	unsigned short addr;
	unsigned char buf_data[255];//收到的数据部分
	unsigned int lendata;//收到的数据长度
}PcsData;
typedef struct
{
	unsigned char flag_waiting;//等待接收 0 不等待 1等待
	unsigned short num_frame; //帧序号
	unsigned char dev_id;//从设备地址
	unsigned char code_fun;//功能码
	unsigned short regaddr;//寄存器地址
	unsigned short numdata;//数据个数

}PcsData_send;//当前发送到led数据，发送前记录，作为接收对比依据
extern PcsData_send g_send_data[];
enum LCD_WORK_STATE //LCD当前工作状态
{
		LCD_INIT = 0,//首先读取PCS个数(功能码03)
		LCD_SET_LCDMODE =1,//开机前整机模式参数设置(功能码06)
		LCD_SET_LCDPARA_VSG =2,//开机前整机为VSG模式下整机参数设置(功能码06)	
		
		LCD_SET_PCSMODE_PQ =3,//开机前整机为PQ模式下模块模式参数设置(功能码06)
		LCD_SET_PCSMODE_VSG =4,//开机前整机为VSG模式下模块模式参数设置(功能码06)


		LCD_RUNNING=0xff,//正常工作中，循环抄取遥信遥测

};
typedef struct
{
	unsigned char mode[MAX_LCD_NUM];//PQ模式下模块工作模式 0
	unsigned short val[MAX_LCD_NUM];
	

   	unsigned char LcdStatus[MAX_LCD_NUM];//当前LCD状态，整机运行前0 整机运行后 1
	unsigned char LcdOperatingMode[MAX_LCD_NUM];//当前LCD工作模式，PQ=1 VSG=5
	unsigned char ifNeedResetLcdOp[MAX_LCD_NUM];


}PQ_PARA;//装置运行参数

typedef struct
{

   	unsigned char LcdStatus[MAX_LCD_NUM];//当前LCD状态，整机运行前0 整机运行后 1
	unsigned char LcdOperatingMode[MAX_LCD_NUM];//当前LCD工作模式，PQ=1 VSG=5
	unsigned char ifNeedResetLcdOp[MAX_LCD_NUM];


}EMU_OP_PARA;//装置运行参数
extern EMU_OP_PARA g_emu_op_para;
extern PARA_MODTCP *pPara_Modtcp;
typedef struct
{
	unsigned short Reg;//寄存器开始地址


}Pcs_Fun06_Struct;
extern unsigned short g_num_frame;
extern int lcd_state[] ;
int AnalysModbus(int id_thread,unsigned char *pdata,int len);
int myprintbuf(int len, unsigned char *buf);
int ReadNumPCS(int id_thread);
int doFun03Tasks(int id_thread, int *taskid,int *pcsid);
int SetLcdFun06(int id_thread,unsigned short reg_addr,unsigned short val);
#endif