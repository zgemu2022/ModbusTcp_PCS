
#include "modbus.h"

#include "client.h"
#include <stdio.h>
#include "crc.h"
#include "my_socket.h"
#include "sys.h"


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
EMU_OP_PARA g_emu_op_para;
PARA_MODTCP Para_Modtcp;
PARA_MODTCP *pPara_Modtcp = (PARA_MODTCP *)&Para_Modtcp;

Pcs_Fun03_Struct pcsYc[] = {
	//遥测
	{0x1103, 0x0A,0x1D}, //模块1

	//遥信
	{0x1200,0x0F,0x10}, //模块1
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

int ReadNumPCS(int id_thread)
{
	unsigned char sendbuf[256];
	unsigned short crccode=0;
	unsigned short reg_start=0x1246;
	int len = 0;
    sendbuf[len++]=(unsigned char)pPara_Modtcp->devNo[id_thread];
	sendbuf[len++]=0x03;
	sendbuf[len++]=reg_start/256;
	sendbuf[len++]=reg_start%256;
	sendbuf[len++]=0;
	sendbuf[len++]=1;
	sendbuf[len++]=0;
	sendbuf[len++]=2;
    crccode = crc16_check(sendbuf,len);
    sendbuf[len++]=crccode/256;
	sendbuf[len++]=crccode%256;

		if (send(modbus_client_sockptr[id_thread], sendbuf, len, 0) < 0)
	{
		printf("发送失败！！！！id_thread=%d\n", id_thread);
		return 0xffff;
	}
	else
	{


		printf("任务包发送成功！！！！");
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
//数据解析
int AnalysModbus(int id_thread,unsigned char *pdata,int len) // unsigned char *datain, unsigned short len, unsigned char *dataout
{
	unsigned char emudata[256];
	unsigned char funid;
	unsigned short regAddr;
	printf("解析收到的LCD数据！！！！！");


	memcpy(emudata,pdata,len);
    funid=emudata[1];
	regAddr = emudata[2]*256+emudata[3]
	if(emudata[1]==3 && regAddr==0x1246)
	{
		Para_Modtcp.pcsnum[id_thread]=
	     printf("LCD[%d]的PCS数量=%的\n",Para_Modtcp.pcsnum[id_thread]);

	}
	return 0;
}


static int createFun03Frame(int id_thread, int *taskid, int *pcsid,int *lenframe, unsigned char *framebuf)
{
	
	int numTask = ARRAY_LEN(pcsYc);
	int _taskid = *taskid;
	int _pcsid = *pcsid;
	unsigned short regStart; //寄存器起始地址
	unsigned char pcsNum; //pcs的数量
	Pcs_Fun03_Struct pcs = pcsYc[_taskid];

	//对不同的任务进行对应的调整
	if (_taskid == 0)
	{
		pcsNum = Para_Modtcp.pcsnum[id_thread];
		if (_pcsid == 4 || _pcsid == 5)
		{
			regStart = pcs.RegStart + 0x1C;
		}else{
			regStart = pcs.RegStart;
		}
	}
	else
	{
		pcsNum = Para_Modtcp.pcsnum[id_thread]+1;
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
	// framebuf[pos++]=0;
	// framebuf[pos++]=0;
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

	g_num_frame++;
	if (g_num_frame == 0x10000)
		g_num_frame = 1;
    *taskid=_taskid;
	*pcsid=_pcsid;
	return 0;
}

int doFun03Tasks(int id_thread, int *taskid,int *pcsid)
{
	int numfail = 0;
	unsigned char sendbuf[256];
	int lensend = 0;

	createFun03Frame(id_thread, taskid,pcsid, &lensend, sendbuf);
	myprintbuf(lensend, sendbuf);
	//	={0x00,0x01,0x00,0x00,0x00,0x06,0x0A,0x03,0x11,0x00,0x00,0x02};
	

	if (send(modbus_client_sockptr[id_thread], sendbuf, lensend, 0) < 0)
	{
		numfail++;
		printf("发送失败！！！！id_thread=%d\n", id_thread);
		return 0xffff;
	}
	else
	{


		printf("任务包发送成功！！！！");
	}
	return 0;

	// if(funid>=numTask)
	// {

	// }
}