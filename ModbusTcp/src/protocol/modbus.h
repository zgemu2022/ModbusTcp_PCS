#ifndef _MODBUS_H
#define _MODBUS_H

#include "modbus_tcp_main.h"

#define _NO_RESET 0
#define _NEED_RESET 1
#define PQ 1
#define VSG 5

// VSG工作模式
#define VSG_SCF_SCV 3 // 3：一次调频、一次调压 ；
#define VSG_SCF_PQ 6  // 6：一次调频、并网无功 ；
#define VSG_PP_SCV 9  // 9：并网有功、一次调压；
#define VSG_PQ_PP 12  // 12：并网无功、并网有功；

// PQ工作模式设置
#define PQ_STP 0 // 0：恒功率模式；
#define PQ_STA 3 // 3：恒流模式；

#define NUM_READ_YC 29 // 0x1d
#define NUM_READ_YX 15
#define NUM_READ_ZJYX 7
#define NUM_READ_ZJYC 15

#define ON_LINE 1  // 在线
#define OFF_LINE 0 // 离线

typedef struct
{
	char type; // 1 Master 2 Slave
	unsigned char lcdnum_cfg;
	unsigned char lcdnum_real;
	unsigned char lcdnum_err;	
	unsigned char pcsnum[MAX_PCS_NUM];
	unsigned char devNo[MAX_PCS_NUM];
	char server_ip[6][64];
	unsigned short server_port[6];
	unsigned short balance_rate;
} PARA_MODTCP; //系统运行参数  从主控传到Modbus-Tcp模块的结构

typedef struct
{
	unsigned short RegStart;  //寄存器开始地址
	unsigned short numData;	  //抄取数据个数
	unsigned short totalData; //总数据个数

} Pcs_Fun03_Struct;

typedef struct
{
	unsigned char dev_id;	//从设备地址
	unsigned char code_fun; //功能码
	unsigned char errflag;	//错误码
	unsigned short addr;
	unsigned char buf_data[255]; //收到的数据部分
	unsigned int lendata;		 //收到的数据长度
} PcsData;
typedef struct
{
	unsigned char flag_waiting; //等待接收 0 不等待 1等待
	unsigned short num_frame;	//帧序号
	unsigned char dev_id;		//从设备地址
	unsigned char code_fun;		//功能码
	unsigned short regaddr;		//寄存器地址
	unsigned short numdata;		//数据个数

} PcsData_send; //当前发送到led数据，发送前记录，作为接收对比依据
extern PcsData_send g_send_data[];
// extern unsigned short pq_pcspw_set[6][2];//整机设置为PQ后、设置pcs为恒功率模式，再设置功率值
// extern unsigned short pq_pcscur_set[6][2];//整机设置为PQ后、设置pcs为恒流模式，再设置电流值

extern unsigned short pqpcs_mode_set[]; //整机设置为PQ模式后，设置pcs模块模式
extern unsigned short pqpcs_pw_set[];	//恒功率模式 功率给定设置0.1kW正为放电，负为充电
extern unsigned short pqpcs_cur_set[];	//恒流模式 电流给定设置0.1A正为放电，负为充电

extern unsigned short vsgpcs_pw_set[];	   // VSG模式 有功给定设置
extern unsigned short pq_vsg_pcs_qw_set[]; // PQ或VSG模式 无功
extern unsigned short pcs_on_off_set[];	   //开机关机
extern post_list_t *post_list_l;
// extern unsigned short lcd_pcs_remote_switch[]; // 各模块 开/关机
enum LCD_WORK_STATE // LCD当前工作状态
{

	LCD_SET_TIME = 0,	   //开机对时
	LCD_INIT = 1,		   //首先读取PCS个数(功能码03)
	LCD_SET_MODE = 2,	   //开机前整机模式参数设置(功能码06)
	LCD_PQ_PCS_MODE = 3,   //整机设置为PQ后、将pcs设置为恒功率模式或恒流模式
	LCD_PQ_STP_PWVAL = 4,  //整机设置为PQ恒功率后,有功值
	LCD_PQ_STP_QWVAL = 5,  //整机设置为PQ恒功率后,无功值
	LCD_PQ_STA_CURVAL = 6, //整机设置为PQ恒流后，电流值
	LCD_VSG_MODE = 7,	   //整机设置为VSG后、设置工作模式
	LCD_VSG_PW_VAL = 8,	   //整机设置为VSG后,有功值
	LCD_VSG_QW_VAL = 9,	   //整机设置为VSG后，无功值

	LCD_PCS_START = 10, //启动本LCD下所有的PCS
	LCD_PCS_STOP = 11,	//停止本LCD下所有的PCS

	LCD_PCS_START_STOP_ONE = 12, //启动本LCD下一个PCS

	LCD_PARALLEL_AWAY_EN = 14, //并转离切换使能
	LCD_PARALLEL_AWAY_DN = 15, //并转离切换失能

	LCD_AWAY_PARALLEL_EN = 16, //离转并切换使能
	LCD_AWAY_PARALLEL_DN = 17, //离转并切换失能
	LCD_ADJUST_PCS_PW = 29,//按策略要求调节有功功率
	LCD_ADJUST_PCS_QW = 30,//按策略要求调节无功功率


	LCD_DO_NOTHING = 0xfe, //什么都不做
	LCD_RUNNING = 0xff, //正常工作中，循环抄取遥信遥测
};
// <<<<<<< HEAD

/*enum LCD_WORK_STATE // LCD当前工作状态
{
	LCD_INIT = 0, //首先读取PCS个数(功能码03)
	LCD_SET_LCDMODE = 1,		   //开机前整机模式参数设置(功能码06)
	LCD_SET_LCDPARA_VSG = 2,	   //开机前整机为VSG模式下整机参数设置(功能码06)

	LCD_SET_PCSMODE_PQ = 3,	 //开机前整机为PQ模式下模块模式参数设置(功能码06)
	LCD_SET_PCSMODE_VSG = 4, //开机前整机为VSG模式下模块模式参数设置(功能码06)

	LCD_RUNNING = 0xff, //正常工作中，循环抄取遥信遥测

};*/

// typedef struct
// {
// 	unsigned char mode[MAX_LCD_NUM];//PQ模式下模块工作模式 0
// 	unsigned short val[MAX_LCD_NUM];

// =======
// >>>>>>> db5448e3e13a7539dcb9a4a0240a049b602dcd2b

typedef struct
{
	unsigned char flag_start;
	unsigned char ems_commnication_status; // ems通信状态 1 ON_LINE 0 OFF_LINE
	unsigned short soc_ave;
	unsigned char flag_soc_bak;
	int err_num;
	unsigned char OperatingMode;		  //当前工作模式，PQ=1 VSG=5
	// unsigned char LcdOperatingMode[MAX_LCD_NUM]; //当前LCD工作模式，PQ=1 VSG=5
	// unsigned char ifNeedResetLcdOp[MAX_LCD_NUM];
	unsigned short pq_mode_set; //[MAX_LCD_NUM][MAX_PCS_NUM]; // PQ模式下PCS模块工作模式
	int pq_pw_total_last;
	int pq_cur_total_last;
	int pq_qw_total_last;

	int pq_pw_total;
	int pq_cur_total;
	int pq_qw_total;
	// short pq_pw[MAX_LCD_NUM][MAX_PCS_NUM];	// PQ，恒功率模式下pcs模块功率值
	// short pq_cur[MAX_LCD_NUM][MAX_PCS_NUM]; // PQ、恒流模式下pcs模块电流值

	unsigned short vsg_mode_set; //[MAX_LCD_NUM]; // VSG模式下LCD工作模式
	int vsg_pw_total_last;
	int vsg_qw_total_last;

	int vsg_pw_total;
	int vsg_qw_total;
	// short vsg_pw[MAX_LCD_NUM][MAX_PCS_NUM]; // VSG模式，pcs模块有功功率
	// short vsg_qw[MAX_LCD_NUM][MAX_PCS_NUM]; // VSG模式、pcs模块无功功率

	int num_pcs_bms[2];

} EMU_OP_PARA; //装置运行参数

// typedef struct
// {
// 	short Line_AB_voltage; // 0x1103	"电网AB线电压 int16	0.1 V
// 	short Line_BC_voltage; // 0x1104	"电网BC线电压 int16	0.1 V
// 	short Line_CA_voltage; // 0x1105	"电网CA线电压 int16	0.1 V
// 	short Phase_A_current; // 0x1106	"电网A相电流 int16	0.1 A
// 	short Phase_B_current; // 0x1107	"电网B相电流 int16	0.1 A
// 	short Phase_C_current; // 0x1108	"电网C相电流 int16	0.1 A
// 	short Power_factor;	   // 0x1109	"功率因数 int16	0.001
// 	short Frequency;	   // 0x110A	"电网频率 int16	0.01 Hz
// 	short Active_power;	   // 0x110B	"交流有功功率 int16	0.1kW 正为放电，负为充电
// 	short Reactive_power;  // 0x110C	"交流无功功率 int16	0.1kVar 正为感性，负为容性
// 	short Apparent_power;  // 0x110D	"交流视在功率 int16	0.1kVA
// } PcsCountData;			   // PCS周期统计数据

extern EMU_OP_PARA g_emu_op_para;
extern PARA_MODTCP *pPara_Modtcp;
typedef struct
{
	unsigned short Reg; //寄存器开始地址

} Pcs_Fun06_Struct;
extern pconf *pconfig;
extern int curTaskId[];
extern int curPcsId[];
extern unsigned int g_num_frame[];
extern int lcd_state[];

int AnalysModbus(int id_thread, unsigned char *pdata, int len);
int myprintbuf(int len, unsigned char *buf);
int ReadNumPCS(int id_thread);
// int getTime(void *ptime);
int setTime(int id_threa);
int doFun03Tasks(int id_thread, int *p_pcsid);
int SetLcdFun06(int id_thread, unsigned short reg_addr, unsigned short val);
#endif