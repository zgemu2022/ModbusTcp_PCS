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


LCD_YC_DATA g_YcData[MAX_PCS_NUM*MAX_LCD_NUM];
//libName:订阅数据的模块； type：数据类型 ；ifcomp：是否与上次数据比较
//static void outputdata(unsigned char libName,unsigned char type,unsigned char ifcomp)
static void outputdata(unsigned char type)
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
        	//can_pro_debug("111publishdata  publishdata gunid=%d pnote->type=%d  type=%d\n",gunid,pnote->type,type);
            switch(type)
            {
            case _YC_:
			   {

			   }
         //	   pnote->pfun(gunid,type,(void*) &pdata2main->gun[gunid].yx_data);
         	   break;
            case _YX_:
         //   	yc_data_temp=pdata2main->gun[gunid].yc_data;
         //   	pnote->pfun(gunid,type,(void*)&yc_data_temp);
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


    if(memcmp((char*)g_YcData[id].yc_data,(char*)pyc,len))
	{
        g_YcData[id].lcdid=id_thread;
		g_YcData[id].pcsid = pcsid;
		g_YcData[id].yc_len = len;
		memcpy((char*)g_YcData[id].yc_data,(char*)pyc,len);
        outputdata(_YC_);

	}

	return 0;


}


// int output_chargePara(u8 gunid,void* p_data)
// {
// 	*(chargePara*)p_data=g_chargepara[gunid];
// 	return 0;

// }