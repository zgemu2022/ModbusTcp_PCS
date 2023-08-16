#include "modbus_tcp_main.h"
#include "modbus.h"
#include <stdio.h>
#include <unistd.h>
#include "sys.h"
#include "client.h"
#include <string.h>
#include <malloc.h>
#include "output.h"
#include "importPlc.h"
#include "debug_lcd_pcs.h"
CallbackYK pbackBmsFun = NULL;
// int modbus_tcp_main(void *para_app)
// {
// 	*pconfig = *(pconf *)para_app;

// 	pPara_Modtcp->type = 1;
// 	strcpy(&pPara_Modtcp->server_ip[0][0], pconfig->lcd_server_ip);
// 	pPara_Modtcp->server_port[0] = pconfig->lcd_server_port;

// 	pPara_Modtcp->lcdnum = pconfig->lcd_num;

// 	printf("LCD 模块启动 ip=%s port=%d\n", pPara_Modtcp->server_ip[0], pPara_Modtcp->server_port[0]);

// 	CreateThreads();
// 	return 0;
// }

int modbus_tcp_main(void *para_app)
{
	int i = 0;
	PARA_FROM_EMU_APP *p = (PARA_FROM_EMU_APP *)para_app;
	fs_debug_lcd = p->writelog;
	pconfig = p->pconfig;

	pPara_Modtcp->type = 1;
	for (i = 0; i < pconfig->lcd_num; i++)

	{
		strcpy(&pPara_Modtcp->server_ip[i][0], &pconfig->lcd_server_ip[i][0]);
		pPara_Modtcp->server_port[i] = pconfig->lcd_server_port[i];
	}

	pPara_Modtcp->lcdnum_cfg = pconfig->lcd_num;
	pPara_Modtcp->lcdnum_real = 0;
	pPara_Modtcp->lcdnum_err = 0;
	pPara_Modtcp->balance_rate = pconfig->balance_rate;
	pPara_Modtcp->Maximum_individual_voltage = pconfig->Maximum_individual_voltage;
	pPara_Modtcp->Minimum_individual_voltage = pconfig->Minimum_individual_voltage;

	pPara_Modtcp->bams_num = pconfig->bams_num;
	lcd_debug("LCD 模块启动 系统定义最大功率111=%d\n", pconfig->sys_max_pw);
	lcd_debug("LCD 模块启动 最高单体电压=%d  最低单体电压=%d\n", pPara_Modtcp->Maximum_individual_voltage, pPara_Modtcp->Minimum_individual_voltage);
	lcd_debug("LCD 模块启动 最高单体电压=%d  最低单体电压=%d\n", pconfig->Maximum_individual_voltage, pconfig->Minimum_individual_voltage);

	pcs_debug("BAMS 的个数 pPara_Modtcp->bams_num=%d\n", pPara_Modtcp->bams_num);

	sprintf(_tmp_print_str, "BAMS 的个数 pPara_Modtcp->bams_num=%d\n", pPara_Modtcp->bams_num);
	fs_debug_lcd(_tmp_print_str);

	// bams_Init();
	Plc_Init();
	initInterface61850();
	CreateThreads();
	return 0;
}

int SubscribeLcdData(unsigned char type, outData2Other pfun) // 订阅pcs数据
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
		handleYkFromEms(pYkPara);
		// handleYkFromEms(pYkPara->item, pYkPara->data[0]);
		break;
	case _PCS_YK_:
		printf("BMS模块调用PCS_YK\n");
		handlePcsYkFromEms(pYkPara);
		break;

	default:
		break;
	}

	return 0;
}