#include "modbus_tcp_main.h"
#include "modbus.h"
#include <stdio.h>
#include <unistd.h>
#include "sys.h"
#include "client.h"
#include <string.h>
#include <malloc.h>

int modbus_tcp_main(void *para_app)
{
	int i;
	*pconfig = *(pconf *)para_app;

	pPara_Modtcp->type = 1;
	strcpy(pPara_Modtcp->server_ip, pconfig->lcd_server_ip);
	pPara_Modtcp->server_port = pconfig->lcd_server_port;

	pPara_Modtcp->lcdnum = pconfig->lcd_num;


	printf("LCD 模块启动 ip=%s port=%d\n",pPara_Modtcp->server_ip,pPara_Modtcp->server_port);

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
