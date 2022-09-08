
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
0x3005	恒功率模式 功率给定设置	int16	模块1	0.1kW	W/R	"需在启机前设置，整机运行后可进行功率切换仅适用于PQ模式，正为放电，负为充电"
0x3006	EMS并网封波使能	uint16	模块1	1	W/R	0：封波失能；1：封波使能；
0x3008	PQ工作模式设置	uint16	模块1	1	W/R	"需在启机前设置，整机运行后可进行模式切换0：恒功率模式；3：恒流模式；"
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
post_list_t *post_list_l=NULL;

int  curTaskId = 0;
int curPcsId = 0;
EMU_OP_PARA g_emu_op_para;
PARA_MODTCP Para_Modtcp;
PARA_MODTCP *pPara_Modtcp = (PARA_MODTCP *)&Para_Modtcp;
PcsData_send g_send_data[MAX_LCD_NUM];
// int lcd_state[] = {LCD_INIT, LCD_INIT, LCD_INIT, LCD_INIT, LCD_INIT, LCD_INIT};
int lcd_state[] = {LCD_SET_TIME, LCD_SET_TIME, LCD_SET_TIME, LCD_SET_TIME, LCD_SET_TIME, LCD_SET_TIME};

// unsigned short pq_pcspw_set[6][2] = {
// 	{0x3008, 0x3005}, {0x3018, 0x3015}, {0x3028, 0x3025}, {0x3038, 0x3035}, {0x3068, 0x3065}, {0x3078, 0x3075}}; //整机设置为PQ后、设置pcs为恒功率模式，再设置功率值

//  unsigned short pq_pcscur_set[6][2]={
// 	{0x3008,0x3004},{0x3018,0x3014},{0x3028,0x3024},{0x3038,0x3034},{0x3068,0x3064},{0x3078,0x3074}
//  };//整机设置为PQ后、设置pcs为恒流模式，再设置电流值

unsigned short pqpcs_mode_set[]={0x3008,0x3018,0x3028,0x3038,0x3068,0x3078};//整机设置为PQ模式后，设置pcs模块模式
unsigned short pqpcs_cur_set[]={0x3004,0x3014,0x3024,0x3034,0x3064,0x3074};//PQ恒流模式 电流给定设置0.1A正为放电，负为充电
unsigned short pqpcs_pw_set[]={0x3005,0x3015,0x3025,0x3035,0x3065,0x3075};//恒功率模式  仅适用于PQ模式 功率给定设置0.1kW正为放电，负为充电

unsigned short vsgpcs_pw_set[]={0x3001, 0x3011, 0x3021, 0x3031, 0x3061, 0x3071}; //整机设置为VSG模式后，设置有功率
unsigned short vsgpcs_qw_set[]={0x3002, 0x3012, 0x3022, 0x3032, 0x3062, 0x3072}; //整机设置为VSG模式后，设置无功率

unsigned short pcsId_pq_vsg[] = {0, 0, 0, 0, 0, 0};
Pcs_Fun03_Struct pcsYc[] = {
	//遥测
	{0x1103, 0x0A, 0x1D}, //模块1

	//遥信
	{0x1200, 0x0F, 0x10}, //模块1
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

int setTime(int id_thread)
{
	//获取系统时间
	struct rtc_time time;
	int timeFd = open("/dev/rtc", O_RDWR);
	ioctl(timeFd, RTC_RD_TIME, &time);
	printf("从系统里读取的时间为: %d-%d-%d %d:%d:%d\n",
		   time.tm_year + 1900,
		   time.tm_mon + 1,
		   time.tm_mday,
		   time.tm_hour,
		   time.tm_min,
		   time.tm_sec);

	unsigned char sendbuf[256];
	unsigned short reg_start = 0x3050;
	int pos = 0,i;
	sendbuf[pos++] = g_num_frame / 256;
	sendbuf[pos++] = g_num_frame % 256;
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
	sendbuf[pos++] = ((time.tm_year+1900)-2000) / 256;
	sendbuf[pos++] = ((time.tm_year+1900)-2000) % 256;
	// sendbuf[pos++] = (time.tm_year + 1900) / 256;
	// sendbuf[pos++] = (time.tm_year + 1900)  % 256;
	sendbuf[pos++] = (time.tm_mon+1) / 256;
	sendbuf[pos++] = (time.tm_mon+1) % 256;
	sendbuf[pos++] = time.tm_mday / 256;
	sendbuf[pos++] = time.tm_mday % 256;
	sendbuf[pos++] = time.tm_hour / 256;
	sendbuf[pos++] = time.tm_hour % 256;
	sendbuf[pos++] = time.tm_min / 256;
	sendbuf[pos++] = time.tm_min % 256;
	sendbuf[pos++] = time.tm_sec / 256;
	sendbuf[pos++] = time.tm_sec % 256;

	printf("发送数据:");
	for(i = 0; i<pos;i++){
		printf("%#x ",sendbuf[i]);
	}
	printf("\n");
	if (send(modbus_client_sockptr[id_thread], sendbuf, pos, 0) < 0)
	{
		printf("发送失败！！！！ id_thread=%d\n", id_thread);
		return 0xffff;
	}
	else
	{
		// printf("时间已同步\n");
		printf("任务包发送成功！！！！");
		close(timeFd);
	
		g_send_data[id_thread].num_frame = g_num_frame;
		g_send_data[id_thread].flag_waiting = 1;
		g_send_data[id_thread].code_fun = 0x10;
		g_send_data[id_thread].dev_id = pPara_Modtcp->devNo[id_thread];
		g_send_data[id_thread].numdata = 6;
		g_send_data[id_thread].regaddr = reg_start;
		g_num_frame++;
		if (g_num_frame == 0x10000)
			g_num_frame = 1;
	}
	//usleep(100000);
	return 0;
}

int ReadNumPCS(int id_thread)
{
	unsigned char sendbuf[256];
	unsigned short reg_start = 0x1246;
	int pos = 0;
	sendbuf[pos++] = g_num_frame / 256;
	sendbuf[pos++] = g_num_frame % 256;
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
		printf("读取LCD数量\n");
		printf("任务包发送成功！！！！");
		g_send_data[id_thread].num_frame = g_num_frame;
		g_send_data[id_thread].flag_waiting = 1;
		g_send_data[id_thread].code_fun = 3;
		g_send_data[id_thread].dev_id = pPara_Modtcp->devNo[id_thread];
		g_send_data[id_thread].numdata = 1;
		g_send_data[id_thread].regaddr = reg_start;
		g_num_frame++;
		if (g_num_frame == 0x10000)
			g_num_frame = 1;
	}
	return 0;
}

int SetLcdFun06(int id_thread, unsigned short reg_addr, unsigned short val)
{
	// printf("ssssssss\n");
	unsigned char sendbuf[256];
	int pos = 0;
	sendbuf[pos++] = g_num_frame / 256;
	sendbuf[pos++] = g_num_frame % 256;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 6;
	sendbuf[pos++] = (unsigned char)pPara_Modtcp->devNo[id_thread];
	sendbuf[pos++] = 0x06;
	sendbuf[pos++] = reg_addr / 256;
	sendbuf[pos++] = reg_addr % 256;
	sendbuf[pos++] = 0;
	sendbuf[pos++] = 1;

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

	printf("res:%d\n",res);
	if (res < 0)
	{
		printf("发送失败！！！！id_thread=%d\n", id_thread);
		return 0xffff;
	}
	else
	{
		printf("任务包发送成功！！！！");
		g_send_data[id_thread].num_frame = g_num_frame;
		g_send_data[id_thread].flag_waiting = 1;
		g_send_data[id_thread].code_fun = 6;
		g_send_data[id_thread].dev_id = pPara_Modtcp->devNo[id_thread];
		g_send_data[id_thread].numdata = 1;
		g_send_data[id_thread].regaddr = reg_addr;

		g_num_frame++;
		if (g_num_frame == 0x10000)
			g_num_frame = 1;

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

int AnalysModbus_fun03(int id_thread, unsigned short regAddr,unsigned char *pdata, int len) // unsigned char *datain, unsigned short len, unsigned char *dataout
{
	int pcsid;
	unsigned short num;
	switch(regAddr)
	{
		case 0x1103:
			pcsid=1;
		case 0x1120:
			pcsid=2;
		case 0x113c:
			pcsid=3;
		case 0x115a:
			pcsid=4;
		case 0x1193:
			pcsid=5;
		case 0x11b0:
			pcsid=6;
            num=pdata[2];
            SaveYcData(id_thread,pcsid,(unsigned short *)&pdata[3], num);
			break;
		default:
		break;

	}
	return 0;
}
//数据解析
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
	if ((pdata[0] * 256 + pdata[1]) != g_send_data[id_thread].num_frame){
		return 2;
	}
		
	g_send_data[id_thread].flag_waiting = 0;


	memcpy(emudata, &pdata[6], len - 6);
	funid = emudata[1];
	printf("   功能码:%#x    ", funid);

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
		if (regAddr != g_send_data[id_thread].regaddr)
		{
			return 4;
		}
	}
	else // if(unid==3)
	{
		regAddr = g_send_data[id_thread].regaddr;
	}

	printf("   寄存器起始地址:%#x   ", regAddr);
	if (funid == 3)
	{
		if(regAddr == 0x1246) 
		{
		//放在现场时，用以下获取LCD下PCS数量
	#if 0
		 Para_Modtcp.pcsnum[id_thread] = regAddr = emudata[3] * 256 + emudata[4];
	#endif
		Para_Modtcp.pcsnum[id_thread] =6;  //测试时获取PCS数量
		printf("LCD[%d]的PCS数量=%d\n", id_thread, Para_Modtcp.pcsnum[id_thread]);
		lcd_state[id_thread] = LCD_SET_MODE;
		}
		else
           AnalysModbus_fun03(id_thread,regAddr,emudata,len-6);

	}
	else if (funid == 6 && regAddr == 0x3046)
	{
		//val = emudata[3] * 256 + emudata[4];
		val = emudata[5];
		// val = 5;
		//printf(" emudata[3]:%#x  emudata[4]:%#x emudata[5]:%#x\n", emudata[3], emudata[4], emudata[5]);
		if (val == PQ)
		{
			lcd_state[id_thread] = LCD_PQ_PCS_MODE;
			// sleep(1);
			curTaskId = 0;
		    curPcsId = 0;
		}
		else
		{
			lcd_state[id_thread] = LCD_VSG_MODE;
		}

	}
	else if (funid == 6 && regAddr == 0x3047)
	{
		val = emudata[3] * 256 + emudata[4];
		lcd_state[id_thread] = LCD_VSG_PW_PCS_MODE;
		curTaskId = 0;
		curPcsId = 0;
	}

	else if (funid == 6 && lcd_state[id_thread] == LCD_PQ_PCS_MODE)
	{
		if(regAddr == pqpcs_mode_set[curPcsId])//模式设置
		{
			curTaskId = 1;
		}
		else if(regAddr == pqpcs_pw_set[curPcsId] || regAddr == pqpcs_cur_set[curPcsId] )//参数设置
		{
			curTaskId = 0;
			curPcsId++;
			if (curPcsId >= Para_Modtcp.pcsnum[id_thread])
			{
				curTaskId = 0;
				curPcsId = 0;
				lcd_state[id_thread] = LCD_RUNNING;
			}
		}
		else
			printf("注意：程序出错！！！！\n");
	}
	else if (funid == 0x10 && regAddr == 0x3050){  //功能码0x10，设置时间
			lcd_state[id_thread] = LCD_INIT;
	}
	else if (funid == 6 && lcd_state[id_thread] == LCD_VSG_PW_PCS_MODE)
	{	
		if (regAddr == vsgpcs_pw_set[curPcsId]) //参数设置
		{
			curTaskId = 0;
			curPcsId++;
			if (curPcsId >= Para_Modtcp.pcsnum[id_thread])
			{
				curTaskId = 0;
				curPcsId = 0;
				lcd_state[id_thread] = LCD_VSG_QW_PCS_MODE;
			}
		}
		else
			printf("注意：程序出错！！！！\n");
	}
	else if (funid == 6 && lcd_state[id_thread] == LCD_VSG_QW_PCS_MODE)
	{
		if (regAddr == vsgpcs_qw_set[curPcsId]) //参数设置
		{
			curTaskId = 0;
			curPcsId++;
			if (curPcsId >= Para_Modtcp.pcsnum[id_thread])
			{
				curTaskId = 0;
				curPcsId = 0;
				lcd_state[id_thread] = LCD_RUNNING;
			}
		}
		else
			printf("注意：程序出错！！！！\n");
	}
	return 0;
}

static int createFun03Frame(int id_thread, int *taskid, int *pcsid, int *lenframe, unsigned char *framebuf)
{

	int numTask = ARRAY_LEN(pcsYc);
	int _taskid = *taskid;
	int _pcsid = *pcsid;
	unsigned short regStart; //寄存器起始地址
	unsigned char pcsNum;	 // pcs的数量
	Pcs_Fun03_Struct pcs = pcsYc[_taskid];

	//对不同的任务进行对应的调整
	if (_taskid == 0)
	{
		pcsNum = Para_Modtcp.pcsnum[id_thread];
		if (_pcsid == 4 || _pcsid == 5)
		{
			regStart = pcs.RegStart + 0x1C;
		}
		else
		{
			regStart = pcs.RegStart;
		}
	}
	else
	{
		pcsNum = Para_Modtcp.pcsnum[id_thread] + 1;
		regStart = pcs.RegStart;
	}

	int pos = 0;
	printf("pos:%d\n", pos);
	framebuf[pos++] = g_num_frame / 256;
	framebuf[pos++] = g_num_frame % 256;
	framebuf[pos++] = 0;
	framebuf[pos++] = 0;
	framebuf[pos++] = 0;
	framebuf[pos++] = 6;
	framebuf[pos++] = Para_Modtcp.devNo[id_thread];
	framebuf[pos++] = 3;
	framebuf[pos++] = (regStart + _pcsid * pcs.totalData) / 256;
	framebuf[pos++] = (regStart + _pcsid * pcs.totalData) % 256;
	framebuf[pos++] = pcs.numData / 256;
	framebuf[pos++] = pcs.numData % 256;
	// g_send_data[id_thread].code_fun = 3;
	// g_send_data[id_thread].dev_id = pPara_Modtcp->devNo[id_thread];
	// g_send_data[id_thread].numdata = pcs.numData;
	// g_send_data[id_thread].regaddr = regStart + _pcsid * pcs.totalData;
	*lenframe = pos;

	_pcsid++;
	printf("_pcsid：%d\n", _pcsid);
	printf("_taskid:%d\n", _taskid);
	if (_pcsid >= pcsNum)
	{
		_taskid++;

		if (_taskid >= numTask)
			_taskid = 0;
		_pcsid = 0;
	}

	printf("任务包发送成功！！！！");
	g_send_data[id_thread].num_frame = g_num_frame;
	g_send_data[id_thread].flag_waiting = 1;
	g_send_data[id_thread].code_fun = 0x03;
	g_send_data[id_thread].dev_id = pPara_Modtcp->devNo[id_thread];
	g_send_data[id_thread].numdata = pcs.numData;
	g_send_data[id_thread].regaddr = regStart;

	g_num_frame++;
	if (g_num_frame == 0x10000)
		g_num_frame = 1;
	*taskid = _taskid;
	*pcsid = _pcsid;
	return 0;
}

int doFun03Tasks(int id_thread, int *taskid, int *pcsid)
{
	int numfail = 0;
	unsigned char sendbuf[256];
	int lensend = 0;

	createFun03Frame(id_thread, taskid, pcsid, &lensend, sendbuf);
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
		printf("任务包发送成功！！！！");
	}
	return 0;

	// if(funid>=numTask)
	// {

	// }
}
