#include "modbus_tcp_main.h"
#include "modbus.h"
#include <stdio.h>
#include <unistd.h>
#include "sys.h"
#include "client.h"
#include <string.h>
#include <malloc.h>

CallbackYK pbackBmsFun = NULL;

int modbus_tcp_main(void *para_app)
{
	*pconfig = *(pconf *)para_app;

	pPara_Modtcp->type = 1;
	strcpy(pPara_Modtcp->server_ip, pconfig->lcd_server_ip);
	pPara_Modtcp->server_port = pconfig->lcd_server_port;

	pPara_Modtcp->lcdnum = pconfig->lcd_num;

	printf("LCD 模块启动 ip=%s port=%d\n", pPara_Modtcp->server_ip, pPara_Modtcp->server_port);

	CreateThreads();
	return 0;
}

int SubscribeLcdData(unsigned char type, outData2Other pfun) //订阅pcs数据
{
	printf("正在订阅pcs数据 type=%d！！！！！\n", type);
	post_list_t *note = (post_list_t *)malloc(sizeof(post_list_t));
	note->type = type;

	note->pfun = pfun;
	note->next = post_list_l;
	post_list_l = note;

	return 0;
}

int ykOrderFromBms(unsigned char type, YK_PARA *pYkPara, CallbackYK pfun)
{
	pbackBmsFun = pfun;
	switch (type)
	{
	case _BMS_YX_:
		printf("61850模块调用YX id=%d  para=%d \n", pYkPara->item, pYkPara->data[0]);
		handleYxFromEms(pYkPara->item, pYkPara->data[0]);
		break;
	case _BMS_YK_:
		printf("BMS模块调用YK\n");
		break;
	case _PCS_YK_:
		printf("BMS模块调用PCS_YK\n");
		break;

	default:
		break;
	}

	return 0;
}