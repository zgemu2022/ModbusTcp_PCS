#ifndef _MODBUS_H
#define _MODBUS_H

#include "modbus_tcp_main.h"
#define MAX_LCD_NUM   6

    //   EMU_INIT = 0,//系统初始化，抄取开机前整机信息（功能码03）
	//   EMU_SETPARA_BEBOOT = 1,//开机前参数设置(功能码06)
	//   EMU_ON_WORKING = 2,//开机后遥信遥测（功能码03）

    //   EMU_CTRL= 3,//控制LCD(pcs) 0：停止；1：放电；2：充电；3：待机（单位为1）

    //   EMU_IDLE=0xFF,

#define _NO_RESET     0
#define _NEED_RESET   1
#define PQ         1 
#define VSG        5


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
enum LCD_WORK_STATE //LCD当前工作状态
{
		LCD_INIT = 0,//首先读取PCS个数
		LCD_SET_MODE =1,//开机前模式参数设置(功能码06)


		LCD_RUNNING=0xff,//正常工作中

};
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
int AnalysModbus(unsigned char *pdata,int len);
int myprintbuf(int len, unsigned char *buf);
int ReadNumPCS(int id_thread);
int doFun03Tasks(int id_thread, int *taskid,int *pcsid);
#endif