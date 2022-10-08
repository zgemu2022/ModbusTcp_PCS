#ifndef _IMPORT_BAMS_
#define _IMPORT_BAMS_
#define PORTNUM_MAX 2
#define _ALL_ 1
#define _SOC_ 2
#define BMS_MX_CPW 0						 //电池分系统 n 最大允许充电功率
#define BMS_MX_DPW 1						 //电池分系统 n 最大允许放电功率
#define BMS_CONN_HEARTBEAT 2				 //电池分系统 n 通讯心跳
#define BMS_MAIN_VOLTAGE 3					 //电池分系统 n 总电压
#define BMS_MX_CCURRENT 4					 //电池分系统 n 最大允许充电电流
#define BMS_MX_DCURRENT 5					 //电池分系统 n 最大允许放电电流
#define BMS_SUM_CURRENT 6					 //电池分系统 n 电池总电流
#define BMS_SOC 7							 //电池分系统 n 电池 SOC
#define BMS_remaining_charging_capacity 8	 //电池分系统 n 电池剩余可充电量
#define BMS_remaining_discharging_capacity 9 //电池分系统 n 电池剩余可放电量
#define BMS_single_MX_voltage 10			 //电池分系统 n 单体最高电压
#define BMS_single_MI_voltage 11			 //电池分系统 n 单体最低电压
#define BMS_SYS_STATUS 12					 //电池分系统 n 状态0-初始化 1-停机 2-启动中 3-运行 4-待机 5-故障 9-关机 255-调试
#define BMS_SYS_NEED 13						 //电池分系统 n 需求 0-禁充禁放(PCS禁止充电放电, PCS应停机或封脉冲) 1-只能充电（PCS禁止放电） 2-只能放电（PCS禁止充电） 3-可充可放（正常）
#define BMS_FAULT_STATUS 14					 //电池分系统 n 总故障状态

enum _BMS_SYS_STATUS //电池分系统状态
{
	BMS_ST_INIT = 0,	 //-初始化
	BMS_ST_PAUSE = 1,	 //-停机
	BMS_ST_START = 2,	 //-启动中
	BMS_ST_WORKING = 3,	 //-运行
	BMS_ST_WAITTING = 4, //-待机
	BMS_ST_FAULT = 5,	 //-故障
	BMS_ST_SHUTDOWN = 9, //-关机
	BMS_ST_TEST = 255	 //-调试
};

typedef struct
{
	unsigned char bmsid;
	unsigned char pcsid;
	unsigned char lendata;		 //收到的数据长度
	unsigned char buf_data[256]; //收到的数据部分

} BmsData;
typedef struct
{
	unsigned char bmsid;		   // 最大允许充电功率
	unsigned char pcsid;		   // 最大允许放电功率
	unsigned short mx_cpw;		   //最大允许充电功率
	unsigned short mx_dpw;		   //最大允许放电功率
	unsigned short main_vol;	   //总电压
	unsigned short mx_ccur;		   //最大允许充电电流
	unsigned short mx_dccur;	   //最大允许放电电流
	unsigned short sum_cur;		   //电池总电流
	unsigned short soc;			   //电池 SOC
	unsigned short remain_ch_cap;  //电池剩余可充电量
	unsigned short remain_dch_cap; //电池剩余可放电量
	unsigned short single_mx_vol;  // 单体最高电压
	unsigned short single_mi_vol;  //单体最低电压
	unsigned short sys_status;	   //当前系统状态0-初始化 1-停机 2-启动中 3-运行 4-待机 5-故障 9-关机 255-调试
	unsigned short sys_need;	   //当前系统需求 0-禁充禁放(PCS禁止充电放电, PCS应停机或封脉冲) 1-只能充电（PCS禁止放电） 2-只能放电（PCS禁止充电） 3-可充可放（正常）
	unsigned short if_sys_fault;   //总故障状态
} BmsData_Newest;				   //

typedef struct
{
	unsigned char portnum;			  //接入数量
	unsigned short baud[PORTNUM_MAX]; //波特率
	unsigned char devid[PORTNUM_MAX]; //设备地址
	unsigned char pcs_num[PORTNUM_MAX];

} PARA_BAMS; //

void bams_Init(void);
#endif