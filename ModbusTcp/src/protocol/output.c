#include "output.h"
#include <stdio.h>
#include "modbus_tcp_main.h"
#include "modbus.h"

//所有的LCD整机信息数据数据标识1都用2来表示，
//数据标识2编号从1-6，
//每个LCD下模块信息，数据标识1都3来表示，
//数据标识2编号从1-36，每个LCD模块信息占用6个编号，LCD1模块信息数据标识2从1-6，LCD2模块信息数据标识2从7-12，
//LCD3模块信息数据标识2从13-18，LCD4模块信息数据标识2从19-24，LCD5模块信息数据标识2从25-30，LCD6模块信息数据标识2从31-36.


LCD_YC_DATA g_YcData[MAX_PCS_NUM*MAX_LCD_NUM];
int outto(int id_thread,int pcsid,unsigned char *pyc,unsigned char num)
{

	int id = 0;

	for(i=0;i<id_thread;i++)
	{
		id+= pPara_Modtcp->pcsnum[i];
	}
    id+=pcsid;


    if(memcmp((char*)g_YcData[id].yc_data,(char*)pyc,num*2))
	{
        g_YcData[id].lcdid=id_thread;
		g_YcData[id].pcsid = pcsid;
		g_YcData[id].yc_num = num;
		memcpy((char*)g_YcData[id].yc_data,(char*）pyc,num*2);

	}



}