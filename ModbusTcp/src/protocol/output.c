#include "output.h"
#include <stdio.h>
#include "modbus_tcp_main.h"
#include "modbus.h"
#include <sys/mman.h>
#include <string.h>
//所有的LCD整机信息数据数据标识1都用2来表示，
//数据标识2编号从1-6，
//每个LCD下模块信息，数据标识1都3来表示，
//数据标识2编号从1-36，每个LCD模块信息占用6个编号，LCD1模块信息数据标识2从1-6，LCD2模块信息数据标识2从7-12，
//LCD3模块信息数据标识2从13-18，LCD4模块信息数据标识2从19-24，LCD5模块信息数据标识2从25-30，LCD6模块信息数据标识2从31-36.


LCD_YC_YX_DATA g_YcData[MAX_PCS_NUM*MAX_LCD_NUM];
//libName:订阅数据的模块； type：数据类型 ；ifcomp：是否与上次数据比较
LCD_YC_YX_DATA g_YxData[MAX_PCS_NUM*MAX_LCD_NUM];
//static void outputdata(unsigned char libName,unsigned char type,unsigned char ifcomp)
static void outputdata(unsigned char type,int id)
{
	post_list_t *pnote=post_list_l;

	while(pnote!=NULL)
	{
            if(pnote->type!=type)
            {

            	if(pnote->next!=NULL)
            	{
                pnote=pnote->next;
            	continue;
            	}
            	else
            		return ;
            }
            switch(type)
            {
            case _YC_:
			   {
               pnote->pfun(type,(void*) &g_YcData[id].pcs_data);
			   }

         	   break;
            case _YX_:
			   {
               pnote->pfun(type,(void*) &g_YxData[id].pcs_data);
			   }

         	   break;

            default:
         	   break;
            }

            pnote=pnote->next;
	}

}

int SaveYcData(int id_thread,int pcsid,unsigned short *pyc,unsigned char len)
{

	int id = 0;
	int i;

	for(i=0;i<id_thread;i++)
	{
		id+= pPara_Modtcp->pcsnum[i];
	}
    id+=pcsid;


    if(memcmp((char*)g_YcData[id].pcs_data,(char*)pyc,len))
	{
		g_YcData[id].sn = id;
        g_YcData[id].lcdid=id_thread;
		g_YcData[id].pcsid = pcsid;
		g_YcData[id].data_len = len;
		memcpy((char*)g_YcData[id].pcs_data,(char*)pyc,len);
        outputdata(_YC_,id);

	}

	return 0;
}

int SaveYxData(int id_thread,int pcsid,unsigned short *pyx,unsigned char len)
{

	int id = 0;
	int i;

	for(i=0;i<id_thread;i++)
	{
		id+= pPara_Modtcp->pcsnum[i];
	}
    id+=pcsid;


    if(memcmp((char*)g_YxData[id].pcs_data,(char*)pyx,len))
	{
		g_YxData[id].sn = id;
        g_YxData[id].lcdid=id_thread;
		g_YxData[id].pcsid = pcsid;
		g_YxData[id].data_len = len;
		memcpy((char*)g_YxData[id].pcs_data,(char*)pyx,len);
        outputdata(_YX_,id);

	}

	return 0;


}
// int output_chargePara(u8 gunid,void* p_data)
// {
// 	*(chargePara*)p_data=g_chargepara[gunid];
// 	return 0;

// }