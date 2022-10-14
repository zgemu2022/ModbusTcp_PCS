#include "logicAndControl.h"
#include "modbus.h"
#include "output.h"
#include "YX_define.h"
#include "importBams.h"
int total_pcsnum = 28;
int g_flag_RecvNeed = 0;
int g_flag_RecvNeed_LCD = 0;
int g_flag_RecvNeed_PCS = 0;
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
	// return 0;
}

void startAllPcs(void)
{
	int i;
	int flag = 0;

	for (i = 0; i < pPara_Modtcp->lcdnum; i++)
	{
		if (lcd_state[i] == LCD_RUNNING)
		{
			flag = 1;
			lcd_state[i] = LCD_PCS_START;
			curTaskId[i] = 0;
			curPcsId[i] = 0;
		}
	}
	g_emu_op_para.flag_start = 1;
	// pbackBmsFun(_BMS_YK_, (void *)flag);
}

void stopAllPcs(void){
	int i;
	int flag = 0;

	for (i = 0; i < pPara_Modtcp->lcdnum; i++)
	{
		if (lcd_state[i] == LCD_RUNNING)
		{
			flag = 1;
			lcd_state[i] = LCD_PCS_STOP;
			curTaskId[i] = 0;
			curPcsId[i] = 0;
		}
	}
	g_emu_op_para.flag_start = 0;
	pbackBmsFun(_BMS_YK_, (void *)flag);
}

	int handleYkFromEms(YK_PARA *pYkPara)
{
	unsigned char item; //项目编号
	int i;
	item = pYkPara->item;
	printf("aaaaaaaaaa item:%d \n", (int)pYkPara->item);

	switch (item)
	{
	case Emu_Startup:
		startAllPcs();
		break;
	case EMS_PW_SETTING: //有功功率
	case ONE_FM_PW_SETTING:
	{
		float tem;
		tem = *(float *)pYkPara->data;
		if (g_emu_op_para.OperatingMode == PQ)
			g_emu_op_para.pq_pw_total = (unsigned int)tem;
		else
			g_emu_op_para.vsg_pw_total = (unsigned int)tem;
	}
	break;
	case EMS_QW_SETTING: //无功功率
	case ONE_FM_QW_SETTING:
	{
		float tem;
		tem = *(float *)pYkPara->data;
		g_emu_op_para.vsg_qw_total = (unsigned int)tem;
	}
	case EMS_SET_MODE:
	{
		int tem;
		tem = *(int *)pYkPara->data;
		if (tem != g_emu_op_para.OperatingMode && g_emu_op_para.flag_start == 0)
		{
			if (tem == VSG)
			{
				for (i = 0; i < pPara_Modtcp->lcdnum; i++)
				{
					curTaskId[i] = 0;
					curPcsId[i] = 0;
					lcd_state[i] = LCD_SET_MODE;
				}
				g_emu_op_para.OperatingMode = tem;
			}
		}
	}
	break;

	case EMS_VSG_MODE: // 6				  // VSG工作模式设置
	{
		int tem;
		tem = *(int *)pYkPara->data;
		if (g_emu_op_para.OperatingMode == VSG)
		{
			if (tem == VSG)
			{
				for (i = 0; i < pPara_Modtcp->lcdnum; i++)
				{
					curTaskId[i] = 0;
					curPcsId[i] = 0;
					lcd_state[i] = LCD_VSG_MODE;
				}
			}
			g_emu_op_para.vsg_mode_set = tem;
		}
	}

	break;
	case EMS_PQ_MODE: // 7				  // PQ工作模式设置
		break;
	default:
		break;
	}
	return 0;
}

// unsigned short checkPcsStatus(int lcdid, int pcsid)
// {
// 	int sn;
// 	int i;
// 	unsigned short status_pcs;
// 	int status;
// 	for (i = 0; i < lcdid; i++)
// 	{
// 		sn += pPara_Modtcp->pcsnum[i];
// 	}
// 	sn += pcsid;
// 	sn--;
// 	status_pcs = g_YxData[sn].pcs_data[u16_InvRunState1];
// 	return status_pcs;
// }
//
int findCurPcsForStart(int type, int lcdid, int pcsid)
{
	int sn;
	int i;
	unsigned short status_pcs;
	int status;

	for (i = 0; i < lcdid; i++)
	{
		sn += pPara_Modtcp->pcsnum[i];
	}
	sn += pcsid;
	sn--;

	status_pcs = g_YxData[sn].pcs_data[u16_InvRunState1];

	switch (type)
	{
	case Emu_Startup:
	{
		for (i = 0; i < pPara_Modtcp->pcsnum[lcdid]; i++)
		{
			if ((status_pcs & (1 < bPcsStoped)) != 0 && (status_pcs & (1 < bFaultStatus)) == 0 && (status_pcs & (1 < bPcsRunning)) == 0 && checkBmsForStart(sn) == 0)
			{
				break;
			}
		}
		curPcsId[lcdid] = i;
	}
	break;
	default:
		break;
	}
	return 0;
}

int countDP(int sn, unsigned short *pPw)
{
	int ret = 0;
	float soc_ave;
	float soc;
	float dt_pw;
	short pw = *pPw;
	float f_pw = (float)pw;

	soc_ave = (float)g_emu_op_para.soc_ave;
	if (sn <= g_emu_op_para.num_pcs_bms[0])
	{
		soc = (float)bmsdata_cur[0][sn].soc;
	}
	else
	{
		soc = (float)bmsdata_cur[1][sn - g_emu_op_para.num_pcs_bms[0] + 1].soc;
	}

	dt_pw = (float)pPara_Modtcp->balance_rate * (soc - soc_ave);
	if (dt_pw > -10 && dt_pw < 10)
	{
		ret = 0;
	}
	else
	{
		f_pw *= (1 + dt_pw / 10000);
		ret = 1;
	}
	if ((dt_pw > 0 && dt_pw > 10) || (dt_pw < 0 && dt_pw < -10))
		f_pw *= (1 + dt_pw / 10000);
	*pPw = (short)f_pw;
	return ret;
}