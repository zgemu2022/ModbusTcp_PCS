#ifndef _LOGIC_AND_CONTROL_H_
#define _LOGIC_AND_CONTROL_H_

#define MAX_PCS_NUM 6 //每个LCD下最多包含pcs的个数
#define MAX_LCD_NUM 6
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
typedef struct
{
	unsigned char flag_adj_pw[MAX_PCS_NUM]; //有功调节标志
	unsigned char flag_adj_qw[MAX_PCS_NUM]; //无功调节标志
	unsigned short val_pw[MAX_PCS_NUM];		//有功调节功率数值
	unsigned short val_qw[MAX_PCS_NUM];		//无功调节功率数值
} EMU_ADJ_PCS;
typedef struct
{
	unsigned char flag_adj_pw_lcd_cfg[MAX_LCD_NUM]; // Lcd收到有功功率调节要求
	unsigned char flag_adj_qw_lcd_cfg[MAX_LCD_NUM]; // Lcd收到无功功率调节要求
	unsigned char flag_adj_pw_lcd[MAX_LCD_NUM];		// lcd里面是否包含需要调节有功功率的pcs
	unsigned char flag_adj_qw_lcd[MAX_LCD_NUM];		// lcd里面是否包含需要调节无功功率的pcs
	EMU_ADJ_PCS adj_pcs[MAX_LCD_NUM];
} EMU_ADJ_LCD; // LCD装置调节有功无功
typedef struct
{
	unsigned char flag_start_stop_pcs[MAX_PCS_NUM]; // pcs启动或停止 0x55：启动  0xAA：停止 0：不变；
} EMU_ACTION_PCS;

typedef struct
{
	unsigned char flag_start_stop_lcd[MAX_LCD_NUM]; // lcd 0：本lcd中没有需要启停动作的设备 1 lcd中的所有pcs启动 2 lcd中所有pcs停止
													// 3 lcd中的pcs各自判断启停
	EMU_ACTION_PCS action_pcs[MAX_LCD_NUM];
} EMU_ACTION_LCD; // LCD动作定义

typedef struct
{
	unsigned char flag_start_stop[MAX_PCS_NUM]; // pcs当前状态 0停止 1启动
	unsigned char flag_err[MAX_PCS_NUM];		// pcs当前故障 0无故障 1故障
} EMU_STATUS_PCS;								// pcs当前状态

typedef struct
{
	// unsigned char flag_start_stop_lcd[MAX_LCD_NUM]; // lcd 0：本lcd中没有需要启停动作的设备
	EMU_STATUS_PCS status_pcs[MAX_LCD_NUM];
} EMU_STATUS_LCD; // LCD当前状态

extern int total_pcsnum;
extern int g_flag_RecvNeed;
extern int g_flag_RecvNeed_LCD;
extern unsigned char flag_RecvNeed_PCS[];
extern EMU_ADJ_LCD g_emu_adj_lcd;
extern EMU_STATUS_LCD g_emu_status_lcd;
extern EMU_ACTION_LCD g_emu_action_lcd;
// int (YK_PARA *pYkPara);
int handleYkFromEms(YK_PARA *pYkPara);
int handlePcsYkFromEms(YK_PARA *pYkPara);
unsigned int countRecvFlag(int num_read);
unsigned int countRecvPcsFlag(void);
int handleYxFromEms(int item, unsigned char data);
// int (int item, unsigned char data);
// int handleYkFromEms(int item, unsigned char data);
void startAllPcs(void);
void stopAllPcs(void);
int countRecvPcsFlagAry(void);

int findCurPcsForStart(int lcdid, int pcsid);
int findCurPcsForStop(int lcdid, int pcsid);
int setStatusPw(int lcdid);
int setStatusQw(int lcdid);

void printf_pcs_soc(void);
// int checkQw(int lcdid, int pcsid, unsigned short QW);
int findCurPcsidForAdjQw(int id_thread);
int findCurPcsidForAdjPw(int id_thread);
void initEmuParaData(void); //初始化EMU参数和数据
int countQwAdj(int lcdid, int pcsid, int QW, int flag_soc);
int countPwAdj(int lcdid, int pcsid, int PW, int flag_soc);
// int setStatusStart_Stop(void);
int setStatusStart_Stop(int lcdid);
int findCurPcsidForStart_Stop(int id_thread);

#endif