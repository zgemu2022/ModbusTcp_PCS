#include "logicAndControl.h"
#include "output.h"
#include "YX_define.h"
#include "importBams.h"
#include <stdio.h>
#include "client.h"
#include "modbus.h"
int total_pcsnum = 28;
int g_flag_RecvNeed = 0;
int g_flag_RecvNeed_LCD = 0;
int g_flag_RecvNeed_PCS = 0;
EMU_ADJ_LCD g_emu_adj_lcd;

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

unsigned int countRecvPcsFlag(void)
{
	unsigned int flag = 0;
	int i, j;
	for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
	{
		if (modbus_sockt_state[i] == STATUS_OFF)
			continue;
		for (j = 0; j < pPara_Modtcp->pcsnum[i]; j++)
		{
			flag |= (1 << (i * MAX_PCS_NUM + j));
		}
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

	for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
	{
		if (modbus_sockt_state[i] == STATUS_OFF)
			continue;
		if (lcd_state[i] == LCD_RUNNING)
		{
			flag = 1;
			lcd_state[i] = LCD_PCS_START;
			curTaskId[i] = 0;
			curPcsId[i] = 0;
		}
	}

	pbackBmsFun(_BMS_YK_, (void *)flag);
}

void stopAllPcs(void)
{
	int i;
	int flag = 0;

	for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
	{
		if (modbus_sockt_state[i] == STATUS_OFF)
			continue;
		if (lcd_state[i] == LCD_RUNNING)
		{
			flag = 1;
			lcd_state[i] = LCD_PCS_STOP;
			curTaskId[i] = 0;
			curPcsId[i] = 0;
		}
	}
	pbackBmsFun(_BMS_YK_, (void *)flag);
}

int handleYkFromEms(YK_PARA *pYkPara)
{
	unsigned char item; //项目编号
	int i;
	item = pYkPara->item;
	// printf("aaaaaaaaaa item:%d \n", (int)pYkPara->item);

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
		if (g_emu_op_para.OperatingMode == PQ)
			g_emu_op_para.pq_qw_total = (unsigned int)tem;
		else
			g_emu_op_para.vsg_qw_total = (unsigned int)tem;
	}
	case EMS_SET_MODE: //系统未启动下改变运行模式：从PQ-->VSG 或VSG-->PQ
	{
		int tem;
		tem = *(int *)pYkPara->data;
		if (tem != g_emu_op_para.OperatingMode && g_emu_op_para.flag_start == 0)
		{
			for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
			{
				if (modbus_sockt_state[i] == STATUS_OFF)
					continue;
				curTaskId[i] = 0;
				curPcsId[i] = 0;
				lcd_state[i] = LCD_SET_MODE;
			}
			g_emu_op_para.OperatingMode = tem;
		}
	}
	break;

	case EMS_VSG_MODE: // 6				  //系统为VSG工作模式下，设置具体工作模式
	{
		// VSG_SCF_SCV 3 // 3：一次调频、一次调压 ；
		// VSG_SCF_PQ 6  // 6：一次调频、并网无功 ；
		// VSG_PP_SCV 9  // 9：并网有功、一次调压；
		// VSG_PQ_PP 12  // 12：并网无功、并网有功；
		int tem;
		tem = *(int *)pYkPara->data;
		if (g_emu_op_para.OperatingMode == VSG && (g_emu_op_para.vsg_mode_set != tem))
		{

			for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
			{
				if (modbus_sockt_state[i] == STATUS_OFF)
					continue;
				curTaskId[i] = 0;
				curPcsId[i] = 0;
				lcd_state[i] = LCD_VSG_MODE;
			}
			g_emu_op_para.vsg_mode_set = tem;
		}
	}

	break;
	case EMS_PQ_MODE: //系统为PQ工作模式下，设置工作模式为恒功率模式或恒流模式
	{
		int tem;
		tem = *(int *)pYkPara->data;
		if (g_emu_op_para.OperatingMode == PQ && (g_emu_op_para.pq_mode_set != tem))
		{

			for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
			{
				if (modbus_sockt_state[i] == STATUS_OFF)
					continue;
				curTaskId[i] = 0;
				curPcsId[i] = 0;
				lcd_state[i] = LCD_PQ_PCS_MODE;
			}
			g_emu_op_para.pq_mode_set = tem;
		}
	}

	break;
	case Parallel_Away_conversion_en: //并转离切换使能
	{
		unsigned char tem;
		tem = pYkPara->data[0];
		if (g_emu_op_para.OperatingMode == VSG)
		{
			for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
			{
				if (modbus_sockt_state[i] == STATUS_OFF)
					continue;
				curTaskId[i] = 0;
				curPcsId[i] = 0;
				if (tem == 0)
					lcd_state[i] = LCD_PARALLEL_AWAY_DN;
				else
					lcd_state[i] = LCD_PARALLEL_AWAY_EN;
			}
		}
	}

	break;
	case Away_Parallel_conversion_en: //离转并切换使能
	{
		unsigned char tem;
		tem = pYkPara->data[0];
		if (g_emu_op_para.OperatingMode == VSG)
		{
			for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
			{
				if (modbus_sockt_state[i] == STATUS_OFF)
					continue;
				curTaskId[i] = 0;
				curPcsId[i] = 0;
				if (tem == 0)
					lcd_state[i] = LCD_AWAY_PARALLEL_DN;
				else
					lcd_state[i] = LCD_AWAY_PARALLEL_EN;
			}
		}
	}
	break;

	default:
		break;
	}
	return 0;
}

int ckeckCurPcsStartEn(int lcdid, int pcsid)
{
	int sn = 0;
	int i;
	unsigned short status_pcs;
	int ret;

	for (i = 0; i < lcdid; i++)
	{
		sn += pPara_Modtcp->pcsnum[i];
	}
	sn += pcsid;
	sn--;

	status_pcs = g_YxData[sn].pcs_data[u16_InvRunState1];

	if ((status_pcs & (1 < bPcsStoped)) != 0)
	{
		return 1;
	}
	else if ((status_pcs & (1 < bFaultStatus)) != 0)
	{
		return 2;
	}
	ret = checkBmsForStart(sn);
	if (ret != 0)
	{
		return 2 + ret;
	}

	return 0;
}
int handlePcsYkFromEms(YK_PARA *pYkPara)
{
	unsigned char sn;
	int flag = 0;
	sn = pYkPara->item;

	int lcdid = sn / 6;
	int pcsid = sn % 6;
	int ret;

	if (pcsid >= pPara_Modtcp->pcsnum[lcdid])
	{
		goto endPcsYk;
	}

	ret = ckeckCurPcsStartEn(lcdid, pcsid);

	if (ret != 0)
	{
		printf("lcdid=%d pcsid=%d 不满足启动条件，ret=%d\n", lcdid, pcsid, ret);
		goto endPcsYk;
	}
	printf("lcdid=%d pcsid=%d 满足启动条件，等待启动\n", lcdid, pcsid);
	if (lcd_state[lcdid] == LCD_RUNNING)
	{
		flag = 1;
		if (pYkPara->data[0] == 0)
			lcd_state[lcdid] = LCD_PCS_STOP_ONE;
		else
			lcd_state[lcdid] = LCD_PCS_START_ONE;
		curTaskId[lcdid] = 0;
		curPcsId[lcdid] = pcsid;
	}
endPcsYk:
	pbackBmsFun(_BMS_YK_, (void *)flag);

	return 0;
}
int ckeckSnStartEn(int sn)
{

	unsigned short status_pcs;
	int ret;
	status_pcs = g_YxData[sn].pcs_data[u16_InvRunState1];

	if ((status_pcs & (1 < bPcsStoped)) != 0)
	{
		return 1;
	}
	else if ((status_pcs & (1 < bFaultStatus)) != 0)
	{
		return 2;
	}
	ret = checkBmsForStart(sn);
	if (ret != 0)
	{
		return 2 + ret;
	}

	return 0;
}
int findCurPcsForStart(int lcdid, int pcsid)
{
	int sn = 0;
	int i;
	int ret;

	for (i = 0; i < lcdid; i++)
	{
		sn += pPara_Modtcp->pcsnum[i];
	}
	printf("findCurPcsForStart lcdid=%d, pcsid=%d\n", lcdid, pcsid);
	for (i = pcsid; i < pPara_Modtcp->pcsnum[lcdid]; i++)
	{
		sn += i;
		printf("findCurPcsForStart lcdid=%d, pcsid=%d sn=%d\n", lcdid, pcsid, sn);
		ret = ckeckSnStartEn(sn);

		if (ret == 0)
			break;
	}
	curPcsId[lcdid] = i;

	return 0;
}
int findCurPcsForStop(int lcdid, int pcsid)
{
	int sn = 0;
	int i;
	unsigned short temp;

	for (i = 0; i < lcdid; i++)
	{
		sn += pPara_Modtcp->pcsnum[i];
	}
	printf("findCurPcsForStop lcdid=%d, pcsid=%d\n", lcdid, pcsid);
	for (i = pcsid; i < pPara_Modtcp->pcsnum[lcdid]; i++)
	{
		sn += i;
		printf("findCurPcsForStop lcdid=%d, pcsid=%d sn=%d\n", lcdid, pcsid, sn);
		temp = g_YxData[sn].pcs_data[u16_InvRunState1];
		if ((temp && (1 << bPcsStoped)) == 0 && (temp && (1 << bPcsRunning)) != 0) //当前pcs已经启动
			break;
	}
	curPcsId[lcdid] = i;

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

int countDP_test(int lcdid, int pcsid, unsigned short *pQw)
{
	unsigned short soc_ave = 30;
	unsigned short soc = 0;
	unsigned short qw1 = *pQw;
	unsigned short qw2 = *pQw;
	unsigned short dtqw;
	soc = pcsid * 10;
	if (soc > soc_ave)
		qw1 *= (1 - (soc - soc_ave));
	else if (soc < soc_ave)
		qw1 *= (1 + (soc_ave - soc));

	*pQw = qw1;

	dtqw = qw1 - qw2;

	if (dtqw > 1 || dtqw < -1)
		return 1;

	return 0;
}
int checkQw(int lcdid, int pcsid, unsigned short QW)
{
	unsigned short dtQW;
	unsigned short qw;
	unsigned char flag = 0;
	int ret;
	dtQW = QW - (g_emu_op_para.pq_qw_total * 10) / total_pcsnum;
	if (dtQW >= 10 || dtQW <= -10)
	{
		qw = g_emu_op_para.pq_qw_total / total_pcsnum;
		flag = 1;
	}
	ret = countDP_test(lcdid, pcsid, &qw);
	if (ret == 1)
		flag = 1;

	if (flag == 1)
	{
		g_emu_adj_lcd.flag_adj_qw_lcd[lcdid] = 1;
		g_emu_adj_lcd.adj_pcs[lcdid].flag_adj_qw[pcsid] = 1;
		g_emu_adj_lcd.adj_pcs[lcdid].val_qw[pcsid] = qw;
	}

	return 0;
}

// int setStatusQw(void)
// {
// 	int i;
// 	for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
// 	{
// 		if (g_emu_adj_lcd.flag_adj_qw_lcd[i] == 1)
// 		{
// 			lcd_state[i] = LCD_ADJUST_PCS_QW;
// 		}
// 	}
// }

int setStatusPw_Qw(void)
{
	int i;
	for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
	{
		if (g_emu_adj_lcd.flag_adj_pw_lcd[i] == 1)
		{
			lcd_state[i] = LCD_ADJUST_PCS_PW;
		}
		else if (g_emu_adj_lcd.flag_adj_qw_lcd[i] == 1)
		{
			lcd_state[i] = LCD_ADJUST_PCS_QW;
		}
	}
	return 0;
}