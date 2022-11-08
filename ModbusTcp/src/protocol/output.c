#include "output.h"
#include <stdio.h>
#include "modbus_tcp_main.h"
#include "modbus.h"
#include <sys/mman.h>
#include <string.h>
#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>
#include "logicAndControl.h"
#include "YX_Define.h"
//所有的LCD整机信息数据数据标识1都用2来表示，#
//数据标识2编号从1-6，
//每个LCD下模块信息，数据标识1都3来表示，
//数据标识2编号从1-36，每个LCD模块信息占用6个编号，LCD1模块信息数据标识2从1-6，LCD2模块信息数据标识2从7-12，
// LCD3模块信息数据标识2从13-18，LCD4模块信息数据标识2从19-24，LCD5模块信息数据标识2从25-30，LCD6模块信息数据标识2从31-36.

LCD_YC_YX_DATA g_YcData[MAX_PCS_NUM * MAX_LCD_NUM];
// libName:订阅数据的模块； type：数据类型 ；ifcomp：是否与上次数据比较
LCD_YC_YX_DATA g_YxData[MAX_PCS_NUM * MAX_LCD_NUM];

LCD_YC_YX_DATA g_ZjyxData;
LCD_YC_YX_DATA g_ZjycData;

// static void outputdata(unsigned char libName,unsigned char type,unsigned char ifcomp)
static void outputdata(unsigned char type, int id)
{
	post_list_t *pnote = post_list_l;

	while (pnote != NULL)
	{
		if (pnote->type != type)
		{

			if (pnote->next != NULL)
			{
				pnote = pnote->next;
				continue;
			}
			else
				return;
		}
		switch (type)
		{
		case _YC_:
		{
			printf("g_YcData[id-1].sn=%d g_YcData[id-1].pcsid=%d\n", g_YcData[id - 1].sn, g_YcData[id - 1].pcsid);
			pnote->pfun(type, (void *)&g_YcData[id - 1]);
		}

		break;
		case _ZJYC_:
		{
			pnote->pfun(type, (void *)&g_YcData[id - 1]);
		}

		break;
		case _YX_:
		{
			pnote->pfun(type, (void *)&g_YxData[id - 1]);
		}

		break;

		default:
			break;
		}

		pnote = pnote->next;
	}
}

int SaveYcData(int id_thread, int pcsid, unsigned short *pyc, unsigned char len)
{

	int id = 0;
	int i;
	static unsigned char flag_recv_pcs[] = {0,0,0,0,0,0};
	static int flag_recv_lcd=0;
	int qw;
	for (i = 0; i < id_thread; i++)
	{
		id += pPara_Modtcp->pcsnum[i];
	}
	id += pcsid;
	// id = MAX_PCS_NUM * id_thread + pcsid;

	printf("saveYcData id_thread=%d pcsid=%d id=%d num=%d\n", id_thread, pcsid, id, len);

	//  if(memcmp((char*)g_YcData[id].pcs_data,(char*)pyc,len))
	{
		g_YcData[id - 1].sn = id - 1;
		g_YcData[id - 1].lcdid = id_thread;
		g_YcData[id - 1].pcsid = pcsid;
		g_YcData[id - 1].data_len = len;
		memcpy((char *)g_YcData[id - 1].pcs_data, (char *)pyc, len);

		outputdata(_YC_, id);
	}

	flag_recv_pcs[id_thread] |= (1 << (pcsid-1));

	if (flag_recv_pcs[id_thread] == flag_RecvNeed_PCS[id_thread])
	{
          flag_recv_lcd |= (1<<id_thread);
	}

	if(g_emu_op_para.flag_soc_bak==1)
	{
		if(g_emu_op_para.OperatingMode == PQ)
		{
			qw=g_emu_op_para.pq_qw_total;

		}
		else if(g_emu_op_para.OperatingMode == VSG)
		{
			qw=g_emu_op_para.vsg_qw_total;
			checkQw(id_thread,pcsid,&qw);
		}
		checkQw(id_thread,pcsid,&qw);

	}
	printf("YC pcsid=%d flag_recv_pcs[%d]=%x flag_RecvNeed_PCS[%d]=%x flag_recv_lcd=%x g_flag_RecvNeed_LCD=%x\n ",pcsid,id_thread,flag_recv_pcs[id_thread],id_thread,flag_RecvNeed_PCS[id_thread],flag_recv_lcd,g_flag_RecvNeed_LCD);
	if (flag_recv_lcd == g_flag_RecvNeed_LCD)
	{
		printf("888888888888 pcsid=%d\n",pcsid);
		for(i=0;i<MAX_PCS_NUM;i++)
			flag_recv_pcs[i]=0;
		flag_recv_lcd = 0;
		setStatusPw_Qw();
	}
	return 0;
}

int SaveYxData(int id_thread, int pcsid, unsigned short *pyx, unsigned char len)
{

	int id = 0; //, id_z;
	int i;
	static unsigned char flag_recv_pcs[] = {0,0,0,0,0,0};
	static int flag_recv_lcd;

	unsigned short temp;
	unsigned char b1, b2;
	for (i = 0; i < id_thread; i++)
	{
		id += pPara_Modtcp->pcsnum[i];
	}
	id += pcsid;
	myprintbuf(len, (unsigned char *)pyx);
	printf("saveYxData id_thread=%d pcsid=%d id=%d num=%d\n", id_thread, pcsid, id, len);
	//  if(memcmp((char*)g_YxData[id].pcs_data,(char*)pyx,len))
	{
		g_YxData[id - 1].sn = id - 1;
		g_YxData[id - 1].lcdid = id_thread;
		g_YxData[id - 1].pcsid = pcsid;
		g_YxData[id - 1].data_len = len;

		for (i = 0; i < len / 2; i++)
		{
			b1 = pyx[i] % 256;
			b2 = pyx[i] / 256;
			g_YxData[id - 1].pcs_data[i] = b1 * 256 + b2;
		}
		// memcpy((char *)g_YxData[id - 1].pcs_data, (char *)pyx, len);
		myprintbuf(len, (unsigned char *)g_YxData[id - 1].pcs_data);
		temp = g_YxData[id - 1].pcs_data[u16_InvRunState1];

		printf("lcdid=%d pcsid=%d g_YxData[id - 1].pcs_data[u16_InvRunState1]=%x \n", id_thread, pcsid, temp);
		if ((temp && (1 << bPcsStoped)) == 0 && (temp && (1 << bPcsRunning)) != 0) //当前pcs已经启动
		{
			if (g_emu_op_para.flag_start == 0)
				g_emu_op_para.flag_start = 1;
		}
		if ((temp & (1 << bFaultStatus)) != 0)
		{
			printf("lcdid=%d pcsid=%d 有故障 temp=%x\n", id_thread, pcsid, temp);
		}
		outputdata(_YX_, id);
	}
	flag_recv_pcs[id_thread] |= (1 << (pcsid-1));

	if (flag_recv_pcs[id_thread] == flag_RecvNeed_PCS[id_thread])
	{
          flag_recv_lcd |= (1<<id_thread);
	}

	printf("pcsid=%d flag_recv_pcs[%d]=%x flag_RecvNeed_PCS[%d]=%x flag_recv_lcd=%x g_flag_RecvNeed_LCD=%x\n ",pcsid,id_thread,flag_recv_pcs[id_thread],id_thread,flag_RecvNeed_PCS[id_thread],flag_recv_lcd,g_flag_RecvNeed_LCD);
	if (flag_recv_lcd == g_flag_RecvNeed_LCD)
	{
		int err_num = 0;
		
		printf("99999999999999999999\n");
		for (i = 0; i < total_pcsnum; i++)
		{
			if ((g_YxData[id - 1].pcs_data[u16_InvRunState1] & (1 << bFaultStatus)) != 0)
			{
				err_num++;
				
			}
		}
		printf("lcdid=%d pcsid=%d 有故障 目前故障总数=%d pcs总数=%d \n", id_thread, pcsid, err_num, total_pcsnum);
		for(i=0;i<MAX_PCS_NUM;i++)
			flag_recv_pcs[i]=0;
		flag_recv_lcd = 0;
	}
	return 0;
}
int SaveZjyxData(int id_thread, unsigned short *pzjyx, unsigned char len)
{

	if (memcmp((char *)g_ZjyxData.pcs_data, (char *)pzjyx, len))
	{
		g_ZjyxData.sn = 0xff;
		g_ZjyxData.lcdid = id_thread;
		g_ZjyxData.pcsid = 0;
		g_ZjyxData.data_len = len;
		memcpy((char *)g_ZjyxData.pcs_data, (char *)pzjyx, len);
		outputdata(_ZJYX_, 0);
	}

	return 0;
}
int SaveZjycData(int id_thread, unsigned short *pzjyc, unsigned char len)
{

	//  if(memcmp((char*)g_ZjycData.pcs_data,(char*)pzjyc,len))
	{
		g_ZjycData.sn = 0xff;
		g_ZjycData.lcdid = id_thread;
		g_ZjycData.pcsid = 0;
		g_ZjycData.data_len = len;
		memcpy((char *)g_ZjycData.pcs_data, (char *)pzjyc, len);
		outputdata(_ZJYC_, 0);
	}

	return 0;
}
void cleanYcYxData(void)
{
	memset((unsigned char *)g_YcData, 0x00, sizeof(g_YcData));
	memset((unsigned char *)g_YxData, 0x00, sizeof(g_YxData));

	memset((unsigned char *)&g_ZjyxData, 0x00, sizeof(LCD_YC_YX_DATA));
	memset((unsigned char *)&g_ZjycData, 0x00, sizeof(LCD_YC_YX_DATA));

	memset((unsigned char *)&g_emu_adj_lcd, 0, sizeof(EMU_ADJ_LCD));
}

PARA_61850 para_61850;
void initInterface61850(void)
{
#define LIB_61850_PATH "/usr/lib/libiec61850_1.so"
	typedef int (*p_initlcd)(void *);
	void *handle;
	char *error;
	int i;
	printf("initInterface61850\n");
	p_initlcd my_func = NULL;
	para_61850.lcdnum = pPara_Modtcp->lcdnum_cfg;

	for (i = 0; i < MAX_PCS_NUM; i++)
	{

		para_61850.pcsnum[i] = pPara_Modtcp->pcsnum[i];
	}
	para_61850.balance_rate = pconfig->balance_rate;
	printf("传输到61850接口的参数 %d %d \n", para_61850.lcdnum, para_61850.balance_rate);
	handle = dlopen(LIB_61850_PATH, RTLD_LAZY);
	if (!handle)
	{
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror();

	*(void **)(&my_func) = dlsym(handle, "lib61850_main");

	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	my_func((void *)&para_61850);
}