
#include "modbus.h"

#include "client.h"
#include <stdio.h>
#include <string.h>
#include "crc.h"
#include "my_socket.h"
#include "sys.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include "output.h"
#include "unistd.h"
#include "stdlib.h"
#include "lib_time.h"
#include "importBams.h"
#include "logicAndControl.h"
#include "importPlc.h"
/*

EMU 参数
第一部分 运行期间控制参数
//0x3000	开/关机命令	uint16	模块1	1	W/R	0xFF00：远程开机；0x00FF：远程关机
//0x3044	并转离切换使能	uint16	整机	1	W/R	"仅适用于VSG模式，切换成功后需恢复成0 0x00AA：切换使能；0：切换失能"
//0x3045	离转并切换使能	uint16	整机	1	W/R	"仅适用于VSG模式，切换成功后需恢复成0 0x00AA：切换使能；0：切换失能"
//0x3056	心跳信号	uint16	整机	1	W/R	用于触摸屏判断自身与EMS的通信状态，EMS需1s发送一次数据，该数据由0递增至1000再清0，步长为1




第二部分，开机前模块参数
0x3046	产品运行模式设置	uint16	整机	1	W/R
"需在启机前设置，模块运行后无法进行设置 1：PQ模式（高低穿功能，需选择1）；5：VSG模式（并离网功能，需选择5）；"
0x3047	VSG工作模式设置	uint16	整机	1	W/R
"需在启机前设置，整机运行后可进行模式切换
3：一次调频、一次调压 ；
6：一次调频、并网无功 ；
9：并网有功、一次调压；
12：并网无功、并网有功；"


第三部分：开机前整机参数
0x3001	VSG模式 有功给定设置	int16	模块1	1kW	W/R	"需在启机前设置，整机运行后可进行功率切换仅适用于VSG模式，正为放电，负为充电"
0x3002	无功功率值	int16	模块1	1kVar	W/R	/
0x3004	恒流模式 电流给定设置	int16	模块1	0.1A	W/R	"需在启机前设置，整机运行后可进行电流切换仅适用于PQ模式，正为放电，负为充电"
0x	恒功率模式 功率给定设置	int16	模块1	0.1kW	W/R	"需在启机前设置，整机运行后可进行功率切换仅适用于PQ模式，正为放电，负为充电"
0x3006	EMS并网封波使能	uint16	模块1	1	W/R	0：封波失能；1：封波使能；
0x3008	PQ工作模式设置	uint16	模块1	1	W/R	"需在启机前设置，整机运行后可进行模式切换0：恒功率模式；3：恒流模式；"
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
post_list_t *post_list_l = NULL;

int curTaskId[6] = {0};
int curPcsId[6] = {0};
EMU_OP_PARA g_emu_op_para;
PARA_MODTCP Para_Modtcp;
PARA_MODTCP *pPara_Modtcp = (PARA_MODTCP *)&Para_Modtcp;
PcsData_send g_send_data[MAX_LCD_NUM];
pconf conf;
pconf *pconfig = &conf;

// int lcd_state[] = {LCD_INIT, LCD_INIT, LCD_INIT, LCD_INIT, LCD_INIT, LCD_INIT};
int lcd_state[] = {LCD_SET_TIME, LCD_SET_TIME, LCD_SET_TIME, LCD_SET_TIME, LCD_SET_TIME, LCD_SET_TIME};

// unsigned short pq_pcspw_set[6][2] = {
// 	{0x3008, 0x3005}, {0x3018, 0x3015}, {0x3028, 0x3025}, {0x3038, 0x3035}, {0x3068, 0x3065}, {0x3078, 0x3075}}; //整机设置为PQ后、设置pcs为恒功率模式，再设置功率值

//  unsigned short pq_pcscur_set[6][2]={
// 	{0x3008,0x3004},{0x3018,0x3014},{0x3028,0x3024},{0x3038,0x3034},{0x3068,0x3064},{0x3078,0x3074}
//  };//整机设置为PQ后、设置pcs为恒流模式，再设置电流值

unsigned short pqpcs_mode_set[] = {0x3008, 0x3018, 0x3028, 0x3038, 0x3068, 0x3078}; //整机设置为PQ模式后，设置pcs模块模式
unsigned short pqpcs_cur_set[] = {0x3004, 0x3014, 0x3024, 0x3034, 0x3064, 0x3074};	// PQ恒流模式 电流给定设置0.1A正为放电，负为充电
unsigned short pqpcs_pw_set[] = {0x3005, 0x3015, 0x3025, 0x3035, 0x3065, 0x3075};	//恒功率模式  仅适用于PQ模式 功率给定设置0.1kW正为放电，负为充电

unsigned short vsgpcs_pw_set[] = {0x3001, 0x3011, 0x3021, 0x3031, 0x3061, 0x3071};	   //整机设置为VSG模式后，设置有功率
unsigned short pq_vsg_pcs_qw_set[] = {0x3002, 0x3012, 0x3022, 0x3032, 0x3062, 0x3072}; //整机设置为PG或VSG模式后，设置无功功率
unsigned short pcs_on_off_set[] = {0x3000, 0x3010, 0x3020, 0x3030, 0x3060, 0x3070};	   //整机开机或关机

unsigned short pcsId_pq_vsg[] = {0, 0, 0, 0, 0, 0};
unsigned short YX_YC_tab[][2] = {
	{0x1240, 0x1174},
	{0x1200, 0x1100},
	{0x1210, 0x111D},
	{0x1220, 0x113A},
	{0x1230, 0x1157},
	{0x1250, 0x1190},
	{0x1260, 0x11AD},

};

int myprintbuf(int len, unsigned char *buf)
{
	int i = 0;
	printf("\nbuflen=%d\n", len);
	for (i = 0; i < len; i++)
		printf("0x%x ", buf[i]);
	printf("\n");
	return 0;
}

// int getTime(void *ptime)
// {
// 	//获取系统时间
// 	//struct rtc_time *ptemp = (struct rtc_time *)ptime;
// 	struct rtc_time ptemp ;
// 	int timeFd = open("/dev/rtc", O_RDWR);
// 	usleep(1000000);
// 	int res = ioctl(timeFd, RTC_RD_TIME, &ptemp);
// 	printf("res : %d\n",res);
// 	if(res >= 0){
// 		close(timeFd);
// 		return 0;
// 	}else{
// 		close(timeFd);
// 		return(-1);
// 	}
// }

// int getTime(void *ptime)
// {
// 	struct rtc_time ptemp;
// 	int timeFd = open("/dev/rtc", O_RDWR);
// 	ioctl(timeFd, RTC_RD_TIME, &ptemp);
// 	printf("从系统里读取的时间为: %d-%d-%d %d:%d:%d\n",
// 		   ptemp.tm_year + 1900,
// 		   ptemp.tm_mon + 1,
// 		   ptemp.tm_mday,
// 		   ptemp.tm_hour,
// 		   ptemp.tm_min,
// 		   ptemp.tm_sec);
// 	}
// 	return 0;
// }
int setTime(int id_thread)
{
	//获取系统时间
	// struct rtc_time time;
	// int timeFd = open("/dev/rtc", O_RDWR);
	// // usleep(100000);
	// int res = ioctl(timeFd, RTC_RD_TIME, &time);

	// printf("res : %d\n", res);
	// if (res >= 0)
	// {
	// 	printf("线程:%d 获取时间成功  res:%d\n", id_thread,res);
	// 	close(timeFd);
	// 	// return 0;
	// }
	// else
	// {
	// 	printf("线程:%d 获取时间失败  res:%d\n", id_thread,res);
	// 	close(timeFd);
	// 	// return (-1);
	// }

	TDateTime tm_now; //,EndLogDay;
	read_current_datetime(&tm_now);
	printf("从系统里读取的时间为: %d-%d-%d %d:%d:%d\n",
		   tm_now.Year,
		   tm_now.Month,
		   tm_now.Day,
		   tm_now.Hour,
		   tm_now.Min,
		   tm_now.Sec);
	unsigned char sendbuf[256];
	unsigned short reg_start = 0x3050;
	int pos = 0;
	sendbuf[pos++] = (unsigned char)(g_num_frame[id_thread] / 256);
	sendbuf[pos++] = g_num_frame[id_thread] % 256;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 19;
	sendbuf[pos++] = (unsigned char)pPara_Modtcp->devNo[id_thread];
	sendbuf[pos++] = 0x10;
	sendbuf[pos++] = reg_start / 256;
	sendbuf[pos++] = reg_start % 256;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 6;
	sendbuf[pos++] = 12;
	sendbuf[pos++] = tm_now.Year / 256;
	sendbuf[pos++] = tm_now.Year % 256;
	sendbuf[pos++] = tm_now.Month / 256;
	sendbuf[pos++] = tm_now.Month % 256;
	sendbuf[pos++] = tm_now.Day / 256;
	sendbuf[pos++] = tm_now.Day % 256;
	sendbuf[pos++] = tm_now.Hour / 256;
	sendbuf[pos++] = tm_now.Hour % 256;
	sendbuf[pos++] = tm_now.Min / 256;
	sendbuf[pos++] = tm_now.Min % 256;
	sendbuf[pos++] = tm_now.Sec / 256;
	sendbuf[pos++] = tm_now.Sec % 256;

	if (send(modbus_client_sockptr[id_thread], sendbuf, pos, 0) < 0)
	{
		printf("发送失败！！！！ id_thread=%d\n", id_thread);
		return 0xffff;
	}
	else
	{
		// printf("时间已同步\n");
		printf("任务包发送成功！！！！\n");
		printf("发送数据:");
		int i;
		for (i = 0; i < pos; i++)
		{
			printf("%x ", sendbuf[i]);
		}
		printf("\n");

		g_send_data[id_thread].num_frame = g_num_frame[id_thread];
		g_send_data[id_thread].flag_waiting = 1;
		g_send_data[id_thread].code_fun = 0x10;
		g_send_data[id_thread].dev_id = pPara_Modtcp->devNo[id_thread];
		g_send_data[id_thread].numdata = 6;
		g_send_data[id_thread].regaddr = reg_start;
		g_num_frame[id_thread]++;
		if (g_num_frame[id_thread] == 0x10000)
			g_num_frame[id_thread] = 1;
	}
	// usleep(100000);
	return 0;
}

int ReadNumPCS(int id_thread)
{
	unsigned char sendbuf[256];
	unsigned short reg_start = 0x1246;
	int pos = 0;
	sendbuf[pos++] = (unsigned char)(g_num_frame[id_thread] / 256);
	sendbuf[pos++] = g_num_frame[id_thread] % 256;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 6;
	sendbuf[pos++] = (unsigned char)pPara_Modtcp->devNo[id_thread];
	sendbuf[pos++] = 0x03;
	sendbuf[pos++] = reg_start / 256;
	sendbuf[pos++] = reg_start % 256;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 1;
	// sendbuf[pos++] = 0;
	// sendbuf[pos++] = 2;
	// crccode = crc16_check(sendbuf, pos);
	// sendbuf[pos++] = crccode / 256;
	// sendbuf[pos++] = crccode % 256;
	int i;
	printf("发送数据:");
	for (i = 0; i < pos; i++)
	{
		printf("%#x ", sendbuf[i]);
	}
	printf("\n");

	if (send(modbus_client_sockptr[id_thread], sendbuf, pos, 0) < 0)
	{
		printf("发送失败！！！！id_thread=%d\n", id_thread);
		return 0xffff;
	}
	else
	{
		printf("读取LCD下PCS的数量\n");
		printf("任务包发送成功！！！！");
		g_send_data[id_thread].num_frame = g_num_frame[id_thread];
		g_send_data[id_thread].flag_waiting = 1;
		g_send_data[id_thread].code_fun = 3;
		g_send_data[id_thread].dev_id = pPara_Modtcp->devNo[id_thread];
		g_send_data[id_thread].numdata = 1;
		g_send_data[id_thread].regaddr = reg_start;
		g_num_frame[id_thread]++;
		if (g_num_frame[id_thread] == 0x10000)
			g_num_frame[id_thread] = 1;
	}
	return 0;
}

int SetLcdFun06(int id_thread, unsigned short reg_addr, unsigned short val)
{
	// printf(" id_thread:%d  reg_addr:%#x  val:%#x\n",id_thread,reg_addr,val);

	// printf("ssssssss\n");
	unsigned char sendbuf[256];
	int pos = 0;
	sendbuf[pos++] = (unsigned char)(g_num_frame[id_thread] / 256);
	sendbuf[pos++] = g_num_frame[id_thread] % 256;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 6;
	sendbuf[pos++] = (unsigned char)pPara_Modtcp->devNo[id_thread];
	sendbuf[pos++] = 0x06;
	sendbuf[pos++] = reg_addr / 256;
	sendbuf[pos++] = reg_addr % 256;
	// sendbuf[pos++] = 0;
	// sendbuf[pos++] = 1;
	sendbuf[pos++] = val / 256;
	sendbuf[pos++] = val % 256;

	// crccode = crc16_check(sendbuf, pos);
	// sendbuf[pos++] = crccode / 256;
	// sendbuf[pos++] = crccode % 256;

	int i;
	printf("发送数据:");
	for (i = 0; i < pos; i++)
	{
		printf("%#x ", sendbuf[i]);
	}
	printf("\n");

	int res = send(modbus_client_sockptr[id_thread], sendbuf, pos, 0);

	printf("res:%d\n", res);
	if (res < 0)
	{
		printf("发送失败！！！！id_thread=%d\n", id_thread);
		return 0xffff;
	}
	else
	{
		printf("任务包发送成功！！！！");
		g_send_data[id_thread].num_frame = g_num_frame[id_thread];
		g_send_data[id_thread].flag_waiting = 1;
		g_send_data[id_thread].code_fun = 6;
		g_send_data[id_thread].dev_id = pPara_Modtcp->devNo[id_thread];
		g_send_data[id_thread].numdata = 1;
		g_send_data[id_thread].regaddr = reg_addr;

		g_num_frame[id_thread]++;
		if (g_num_frame[id_thread] == 0x10000)
			g_num_frame[id_thread] = 1;

		printf("g_send_data[%d]flag_waiting:%x, code_fun:%x,dev_id:%x,numdata:%x,regaddr:%x\n", id_thread, g_send_data[id_thread].flag_waiting,
			   g_send_data[id_thread].code_fun, g_send_data[id_thread].dev_id, g_send_data[id_thread].numdata, g_send_data[id_thread].regaddr);
	}
	return 0;
}

#if 0
int AnalysModbus(unsigned char *datain, unsigned int len, int id_client)
{
	PcsData pcs;
	int index = 0;
	int flag_comm_succ = 0;
	unsigned short addr;
	unsigned short crccode = 0;
	unsigned short lendata;
	pcs.errflag = 0;
	pcs.dev_id = datain[index++];
	pcs.code_fun = datain[index++];

	switch (pcs.code_fun)
	{
	case 0x90:
	case 0x83:
	case 0x86:
		pcs.errflag = datain[index++];
		crccode = crc(datain, 3);
		if (datain[index] == crccode / 256 && datain[index + 1] == crccode % 256)
		{
			pcs.code_fun -= 0x80;
			flag_comm_succ = 1;
		}
		break;
	case 0x03:
		pcs.addr = datain[index] * 256 + datain[index + 1];
		index += 2;
		lendata = datain[index] * 256 + datain[index];
		//	if(lendata+)

		break;
	default:
		break;
	}
	if (flag_comm_succ == 1)
	{
		// if(msgsnd(g_comm_qmegid, &pcs, sizeof(PcsData), IPC_NOWAIT) != -1)
		// {
		// 	printf("接收到完整modbus-tcp协议包，通过队列发送给发送任务处理！！")
		// }
	}

	// unsigned char *dataout
	return 0;
}
#endif

int AnalysModbus_fun03(int id_thread, unsigned short regAddr, unsigned char *pdata, int len) // unsigned char *datain, unsigned short len, unsigned char *dataout
{
	int pcsid;
	unsigned short num;

	printf("AnalysModbus_fun03 regAddr=%x \n", regAddr);
	switch (regAddr)
	{
	case 0x1100:
		pcsid = 1;
		goto save_yc;
	case 0x111D:
		pcsid = 2;
		goto save_yc;
	case 0x113A:
		pcsid = 3;
		goto save_yc;
	case 0x1157:
		pcsid = 4;
		goto save_yc;
	case 0x1190:
		pcsid = 5;
		goto save_yc;
	case 0x11AD:
		pcsid = 6;
	save_yc:
		num = pdata[2];
		SaveYcData(id_thread, pcsid, (unsigned short *)&pdata[3], num);
		break;
	case 0x1200:
		pcsid = 1;
		goto save_yx;
	case 0x1210:
		pcsid = 2;
		goto save_yx;
	case 0x1220:
		pcsid = 3;
		goto save_yx;
	case 0x1230:
		pcsid = 4;
		goto save_yx;
	case 0x1250:
		pcsid = 5;
		goto save_yx;
	case 0x1260:
		pcsid = 6;
	save_yx:
		num = pdata[2];
		printf("YXAnalysModbus_fun03 id_thread=%d pcsid=%d num=%d regAddr=0x%x \n", id_thread, pcsid, num, regAddr);
		SaveYxData(id_thread, pcsid, (unsigned short *)&pdata[3], num);
		break;
	case 0x1174:
	{
		num = pdata[2];

		SaveZjycData(id_thread, (unsigned short *)&pdata[3], num);
	}

	break;

	default:
		break;
	}
	return 0;
}
//数据解析
//参数初始化
static void init_emu_op_para(int id_thread)
{
	// int i;
	//  LCD
	g_emu_op_para.flag_start = 0;
	g_emu_op_para.ems_commnication_status = OFF_LINE;
	// g_emu_op_para.ifNeedResetLcdOp[id_thread] = _NEED_RESET;
	g_emu_op_para.OperatingMode = PQ;
	g_emu_op_para.pq_mode_set = PQ_STP;
	g_emu_op_para.LcdStatus[id_thread] = STATUS_OFF;
	g_send_data[id_thread].flag_waiting = 0;
	g_emu_op_para.vsg_mode_set = VSG_PQ_PP;
	g_emu_op_para.pq_pw_total = 0;				   // 180 * total_pcsnum;  // 180.0kW*28
	g_emu_op_para.pq_cur_total = 0;				   // 140 * 28; // 140.0A*28
	g_emu_op_para.vsg_pw_total = 0;				   // 50 * 28;  // 180.0kW*28
	g_emu_op_para.pq_qw_total = 5 * total_pcsnum;  // pq模式下无功
	g_emu_op_para.vsg_qw_total = 5 * total_pcsnum; // -180~180 vsg模式下无功kVar

	// PCS
	// for (i = 0; i < MAX_PCS_NUM; i++)
	// {
	// PQ
	// g_emu_op_para.pq_mode[id_thread][i] = PQ_STP;
	// g_emu_op_para.pq_pw[id_thread][i] = 500;  // 50.0kW
	// g_emu_op_para.pq_cur[id_thread][i] = 500; // 50.0A

	// VSG
	// g_emu_op_para.vsg_pw[id_thread][i] = 50; // 50.0kW
	// g_emu_op_para.vsg_qw[id_thread][i] = 0;	 // kVar
	//}
	g_emu_op_para.err_num = 0;
	g_emu_op_para.num_pcs_bms[0] = 0;
	g_emu_op_para.num_pcs_bms[1] = 0;
	//	g_flag_RecvNeed_LCD = countRecvLcdFlag();
}

int AnalysModbus(int id_thread, unsigned char *pdata, int len) // unsigned char *datain, unsigned short len, unsigned char *dataout
{

	unsigned char emudata[256];
	unsigned char funid;
	unsigned short regAddr;
	// unsigned short numdata;
	unsigned short val;

	printf("解析收到的LCD数据！！！！！\n");

	// int i;
	// printf("=====================pdata : ");
	// for (i = 0; i < len; i++)
	// {
	// 	printf("%#x ", pdata[i]);
	// }
	// printf("\n");

	printf("---------pdata[0] * 256 + pdata[1]:%#x   g_send_data[id_thread].num_frame:%#x--------\n", pdata[0] * 256 + pdata[1], g_send_data[id_thread].num_frame);
	if (g_send_data[id_thread].flag_waiting == 0)
		return 1;
	if ((pdata[0] * 256 + pdata[1]) != g_send_data[id_thread].num_frame)
	{
		return 2;
	}

	g_send_data[id_thread].flag_waiting = 0;

	memcpy(emudata, &pdata[6], len - 6);
	funid = emudata[1];
	printf("id_thread:%d   ", id_thread);
	printf("   funid:%#x    ", funid);

	// int i;
	// printf("emudata    :");
	// for (i = 0; i < (len-6); i++)
	// {
	// 	printf("%d:%#x ", i,emudata[i]);
	// }
	// printf("\n");

	if (funid != g_send_data[id_thread].code_fun)
		return 3;

	if (funid == 6)
	{
		regAddr = emudata[2] * 256 + emudata[3];
		val = emudata[4] * 256 + emudata[5];
		if (regAddr != g_send_data[id_thread].regaddr)
		{
			return 4;
		}
	}
	else // if(unid==3)
	{
		regAddr = g_send_data[id_thread].regaddr;
	}

	printf("   regAddr:%#x   ", regAddr);
	if (funid == 3)
	{
		static int flag_get_pcsnum = 0;
		if (regAddr == 0x1246)
		{

			//放在现场时，用以下获取LCD下PCS数量
			flag_get_pcsnum |= (1 << id_thread);

			Para_Modtcp.pcsnum[id_thread] = regAddr = emudata[3] * 256 + emudata[4];

#ifdef ifDebug
			printf("000LCD[%d]的PCS数量=%d\n", id_thread, Para_Modtcp.pcsnum[id_thread]);
			if (id_thread == 0)
				Para_Modtcp.pcsnum[id_thread] = 6; //测试时获取PCS数量
#endif
			printf("LCD[%d]的PCS数量=%d\n", id_thread, Para_Modtcp.pcsnum[id_thread]);
			lcd_state[id_thread] = LCD_SET_MODE;
			if (flag_get_pcsnum == g_flag_RecvNeed_LCD)
			{
				int i;
				total_pcsnum = 0;
				for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
				{
					if (modbus_sockt_state[i] == STATUS_ON)
						total_pcsnum += pPara_Modtcp->pcsnum[i];
				}

				init_emu_op_para(id_thread);
				g_flag_RecvNeed_PCS = countRecvFlag(total_pcsnum); // countRecvPcsFlag(); //

				initInterface61850();
				// bams_Init();
				// Plc_Init();
			}
		}
		else
		{
			AnalysModbus_fun03(id_thread, regAddr, emudata, len - 6);
		}
	}
	else if (funid == 0x10 && regAddr == 0x3050)
	{ //功能码0x10，设置时间
		lcd_state[id_thread] = LCD_INIT;
		printf("功能码0x10，设置时间返回！！！！\n");
	}
	else if (funid == 6 && regAddr == 0x3046) // LCD_SET_MODE
	{

		if (val == PQ)
		{
			lcd_state[id_thread] = LCD_PQ_PCS_MODE;
			// sleep(1);
			curTaskId[id_thread] = 0;
			curPcsId[id_thread] = 0;
			printf("EMU将系统设置为PQ\n");
		}
		else
		{
			lcd_state[id_thread] = LCD_VSG_MODE;
			printf("EMU将系统设置为VSG\n");
		}
	}
	else if (funid == 6 && regAddr == 0x3047)
	{
		// val = emudata[3] * 256 + emudata[4];
		lcd_state[id_thread] = LCD_VSG_PW_VAL;
		curTaskId[id_thread] = 0;
		curPcsId[id_thread] = 0;
	}
	else if (funid == 6 && lcd_state[id_thread] == LCD_PQ_PCS_MODE)
	{
		printf("收到PQ模式设置后设置恒流或恒功率 pcsid=%d regAddr %x  %x\n", curPcsId[id_thread], pqpcs_mode_set[curPcsId[id_thread]], regAddr);
		if (regAddr == pqpcs_mode_set[curPcsId[id_thread]]) //模式设置
		{
			curTaskId[id_thread] = 0;
			curPcsId[id_thread]++;

			if (curPcsId[id_thread] >= Para_Modtcp.pcsnum[id_thread])
			{
				printf("111收到PQ模式设置后设置恒流或恒功率 pcsid=%d regAddr %x  %x\n", curPcsId[id_thread], pqpcs_mode_set[curPcsId[id_thread]], regAddr);
				curPcsId[id_thread] = 0;
				if (g_emu_op_para.pq_mode_set == PQ_STP) //设置恒功率值
				{
					lcd_state[id_thread] = LCD_PQ_STP_PWVAL;
				}
				else // PQ_STA 设置恒流
				{
					lcd_state[id_thread] = LCD_PQ_STA_CURVAL;
				}
			}
		}
		else
			printf("注意：收到PQ模式设置后设置恒流或恒功率,程序出错！！！！\n");
	}

	else if (funid == 6 && lcd_state[id_thread] == LCD_PQ_STP_PWVAL)
	{
		if (regAddr == pqpcs_pw_set[curPcsId[id_thread]]) // PQ恒功率模式下有功参数设置返回
		{
			curTaskId[id_thread] = 0;
			curPcsId[id_thread]++;
			if (curPcsId[id_thread] >= Para_Modtcp.pcsnum[id_thread])
			{
				curTaskId[id_thread] = 0;
				curPcsId[id_thread] = 0;
				lcd_state[id_thread] = LCD_PQ_STP_QWVAL; // 切换状态为PQ恒功率模式下无功参数设置
			}
		}
		else
			printf("注意：PQ恒功率模式下有功参数设置返回程序出错！！！！\n");
	}
	else if (funid == 6 && lcd_state[id_thread] == LCD_PQ_STP_QWVAL)
	{
		if (regAddr == pq_vsg_pcs_qw_set[curPcsId[id_thread]]) // PQ恒功率模式下无功参数设置返回
		{
			curTaskId[id_thread] = 0;
			curPcsId[id_thread]++;
			if (curPcsId[id_thread] >= Para_Modtcp.pcsnum[id_thread])
			{
				curTaskId[id_thread] = 0;
				curPcsId[id_thread] = 0;
				lcd_state[id_thread] = LCD_RUNNING;
			}
		}
		else
			printf("注意：PQ恒功率模式下无功参数设置返回程序出错！！！！\n");
	}
	else if (funid == 6 && lcd_state[id_thread] == LCD_PQ_STA_CURVAL)
	{
		if (regAddr == pqpcs_cur_set[curPcsId[id_thread]]) // PQ恒流模式下电流参数设置返回
		{
			curTaskId[id_thread] = 0;
			curPcsId[id_thread]++;
			if (curPcsId[id_thread] >= Para_Modtcp.pcsnum[id_thread])
			{
				curTaskId[id_thread] = 0;
				curPcsId[id_thread] = 0;
				lcd_state[id_thread] = LCD_RUNNING;
			}
		}
		else
			printf("注意：PQ恒流模式下电流参数设置返回程序出错！！！！\n");
	}

	else if (funid == 6 && lcd_state[id_thread] == LCD_VSG_PW_VAL)
	{
		if (regAddr == vsgpcs_pw_set[curPcsId[id_thread]]) // VSG模式下有功功率数值设置
		{
			curTaskId[id_thread] = 0;
			curPcsId[id_thread]++;
			if (curPcsId[id_thread] >= Para_Modtcp.pcsnum[id_thread])
			{
				curPcsId[id_thread] = 0;
				lcd_state[id_thread] = LCD_VSG_QW_VAL;
			}
		}
		else
			printf("注意：VSG模式下有功功率数值设置程序出错！！！！\n");
	}
	else if (funid == 6 && lcd_state[id_thread] == LCD_VSG_QW_VAL)
	{
		if (regAddr == pq_vsg_pcs_qw_set[curPcsId[id_thread]]) // VSG模式下无功功率数值设置
		{
			curTaskId[id_thread] = 0;
			curPcsId[id_thread]++;
			if (curPcsId[id_thread] >= Para_Modtcp.pcsnum[id_thread])
			{
				curTaskId[id_thread] = 0;
				curPcsId[id_thread] = 0;
				lcd_state[id_thread] = LCD_RUNNING;
			}
		}
		else
			printf("注意：VSG模式下无功功率数值设置程序出错！！！！\n");
	}
	else if (funid == 6 && (lcd_state[id_thread] == LCD_PCS_START || lcd_state[id_thread] == LCD_PCS_STOP))
	{
		if (regAddr == pcs_on_off_set[curPcsId[id_thread]]) //启动或停止
		{
			curTaskId[id_thread] = 0;
			curPcsId[id_thread]++;
			if (curPcsId[id_thread] >= Para_Modtcp.pcsnum[id_thread])
			{
				curTaskId[id_thread] = 0;
				curPcsId[id_thread] = 0;
				lcd_state[id_thread] = LCD_RUNNING;
			}
		}
		else
			printf("注意：整机启动或停止程序出错！！！！\n");
	}
	else if (funid == 6 && (lcd_state[id_thread] == LCD_PCS_START_ONE || lcd_state[id_thread] == LCD_PCS_STOP_ONE))
	{
		if (regAddr == pcs_on_off_set[curPcsId[id_thread]]) //单pcs启动或停止
		{

			curTaskId[id_thread] = 0;
			curPcsId[id_thread] = 0;
			lcd_state[id_thread] = LCD_RUNNING;

			printf("pcs单机启动或停止成功！！！\n");
		}
		else
			printf("注意：pcs单机启动或停止程序出错！！！！\n");
	}
	else if (funid == 6 && (lcd_state[id_thread] == LCD_PARALLEL_AWAY_EN || lcd_state[id_thread] == LCD_PARALLEL_AWAY_DN))
	{
		if (regAddr == 0x3044) //并转离切换
		{

			curTaskId[id_thread] = 0;
			curPcsId[id_thread] = 0;
			lcd_state[id_thread] = LCD_RUNNING;

			printf("lcd:%d 并转离切换成功！！！\n",id_thread);
		}
		else
			printf("注意：lcd:%d 并转离切换 程序出错！！！！\n");
	}
	else if (funid == 6 && (lcd_state[id_thread] == LCD_AWAY_PARALLEL_EN || lcd_state[id_thread] == LCD_AWAY_PARALLEL_DN))
	{
		if (regAddr == 0x3045) //离转并切换
		{
			curTaskId[id_thread] = 0;
			curPcsId[id_thread] = 0;
			lcd_state[id_thread] = LCD_RUNNING;

			printf("lcd:%d 离转并切换成功！！！\n", id_thread);
		}
		else
			printf("注意：lcd:%d 离转并切换 程序出错！！！！\n");
	}

	return 0;
}
static int createFun03Frame(int id_thread, int *p_pcsid, int *lenframe, unsigned char *framebuf)
{
	static int status = _YX_;
	unsigned short regStart;
	int pcsid = *p_pcsid;
	int pos = 0;
	unsigned short numData;

	if (status == _YX_)
	{
		regStart = YX_YC_tab[pcsid][0];
		if (pcsid == 0)
		{
			numData = NUM_READ_ZJYX;
		}
		else
		{
			numData = NUM_READ_YX;
		}
	}
	else if (status == _YC_)
	{
		regStart = YX_YC_tab[pcsid][1];
		if (pcsid == 0)
		{
			numData = NUM_READ_ZJYC;
		}
		else
		{
			numData = NUM_READ_YC;
		}
	}
	else
		printf("注意：程序出现错误！！！\n");
	printf("本次createFun03Frame status=%d  regStart=%x numdata=%d pcsid=%d g_num_frame[id_thread]=%d \n", status, regStart, numData, pcsid, g_num_frame[id_thread]);
	framebuf[pos++] = (unsigned char)(g_num_frame[id_thread] / 256);
	framebuf[pos++] = g_num_frame[id_thread] % 256;
	framebuf[pos++] = 0;
	framebuf[pos++] = 0;
	framebuf[pos++] = 0;
	framebuf[pos++] = 6;
	framebuf[pos++] = Para_Modtcp.devNo[id_thread];
	framebuf[pos++] = 3;
	framebuf[pos++] = regStart / 256;
	framebuf[pos++] = regStart % 256;

	framebuf[pos++] = numData / 256;
	framebuf[pos++] = numData % 256;
	*lenframe = pos;

	if (status == _YX_)
	{
		status = _YC_;
	}
	else if (status == _YC_)
	{
		status = _YX_;
		pcsid++;

		if (pcsid >= (Para_Modtcp.pcsnum[id_thread] + 1))
			pcsid = 0;
	}

	g_send_data[id_thread].num_frame = g_num_frame[id_thread];
	g_send_data[id_thread].flag_waiting = 1;
	g_send_data[id_thread].code_fun = 0x03;
	g_send_data[id_thread].dev_id = pPara_Modtcp->devNo[id_thread];
	g_send_data[id_thread].numdata = numData;
	g_send_data[id_thread].regaddr = regStart;

	g_num_frame[id_thread]++;
	if (g_num_frame[id_thread] == 0x10000)
		g_num_frame[id_thread] = 1;

	*p_pcsid = pcsid;
	return 0;
}

int doFun03Tasks(int id_thread, int *p_pcsid)
{
	int numfail = 0;
	unsigned char sendbuf[256];
	int lensend = 0;

	createFun03Frame(id_thread, p_pcsid, &lensend, sendbuf);

	myprintbuf(lensend, sendbuf);

	if (send(modbus_client_sockptr[id_thread], sendbuf, lensend, 0) < 0)
	{
		numfail++;
		printf("发送失败！！！！id_thread=%d\n", id_thread);
		return 0xffff;
	}
	else
	{
		g_send_data[id_thread].flag_waiting = 1;
		// printf("任务包发送成功！！！！");
	}
	return 0;

	// if(funid>=numTask)
	// {

	// }
}
