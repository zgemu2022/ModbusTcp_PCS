#include "logicAndControl.h"
#include "modbus.h"

int total_pcsnum = 0;
int g_flag_RecvNeed = 0;
int g_flag_RecvNeed_LCD = 0;
unsigned int countRecvFlag(int num_read)
{
	unsigned int flag = 0;
	int i;
	for (i = 0; i < num_read; i++)
	{
		flag |= 1 << i;
	}
	return flag;
}

int handleYxFromEms(int item, unsigned char data)
{
	switch (item)
	{
	case EMS_communication_status:
		g_emu_op_para.ems_commnication_status = data;
		break;

	case one_FM_GOOSE_link_status_A:
		break;

	case one_FM_GOOSE_link_status_B:
		break;

	case one_FM_Enable:
		break;

	case one_FM_Disable:
		break;
	}
	return 0;
}

void startAllPcs(void)
{
	int i;
	int flag = 0;
	for(i=0;i<pPara_Modtcp->lcdnum;i++)
	{
		if(lcd_state[i]==LCD_RUNNING)
		{
			flag=1;
            lcd_state[i]=LCD_RUNNING
		}
	}
}
int handleYkFromEms(int item, unsigned char data)
{
	switch (item)
	{
	case Emu_Startup:
		g_emu_op_para.ems_commnication_status = data;
		break;
	default:
		break;
	}
	return 0;
}

