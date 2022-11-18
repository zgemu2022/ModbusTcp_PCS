#include "logicAndControl.h"
#include "output.h"
#include "YX_define.h"
#include "importBams.h"
#include <stdio.h>
#include "client.h"
#include "modbus.h"
#include <sys/mman.h>
#include <string.h>
int total_pcsnum = 28;
int g_flag_RecvNeed = 0;
int g_flag_RecvNeed_LCD = 0;

unsigned char flag_RecvNeed_PCS[MAX_PCS_NUM];
EMU_ADJ_LCD g_emu_adj_lcd;

EMU_STATUS_LCD g_emu_status_lcd;

EMU_ACTION_LCD g_emu_action_lcd;
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
int countRecvPcsFlagAry(void)
{
	// unsigned int flag = 0;
	int i;

	for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
	{

		if (modbus_sockt_state[i] == STATUS_OFF)
		{
			flag_RecvNeed_PCS[i] = 0;
		}
		else
			flag_RecvNeed_PCS[i] = countRecvFlag(pPara_Modtcp->pcsnum[i]);
	}
	return 0;
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
		printf("LCD[%d] 收到启动 startAllPcs lcd_state[i]=0x%x\n", i, lcd_state[i]);
		if (lcd_state[i] == LCD_RUNNING)
		{
			flag = 1;
			printf("LCD[%d] 立即启动 startAllPcs lcd_state[i]=%d\n", i, lcd_state[i]);
			lcd_state[i] = LCD_PCS_START;
			curTaskId[i] = 0;
			curPcsId[i] = 0;
		}
		else
		{
			flag = 1;
			printf("LCD[%d] 稍后启动 startAllPcs lcd_state[i]=%d\n", i, lcd_state[i]);
			g_emu_action_lcd.flag_start_stop_lcd[i] = 1;
		}
	}

	//		pbackBmsFun(_BMS_YK_, (void *)flag);
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
		else
		{
			flag = 1;
			g_emu_action_lcd.flag_start_stop_lcd[i] = 2;
		}
	}
//	pbackBmsFun(_BMS_YK_, (void *)flag);
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
		printf("handleYkFromEms startAllPcs\n");
		startAllPcs();
		break;
	case Emu_Stop:
		stopAllPcs();
		break;
	case EMS_PW_SETTING: //有功功率
	case ONE_FM_PW_SETTING:
	{

		float tem;
		tem = *(float *)pYkPara->data;
		printf("············有功功率：%f", (float)tem);

		if (g_emu_op_para.OperatingMode == PQ)
		{
			g_emu_op_para.pq_pw_total_last = g_emu_op_para.pq_pw_total;
			g_emu_op_para.pq_pw_total = (unsigned int)tem;
			printf("·········PQ···有功功率：%d\n", g_emu_op_para.pq_pw_total);
		}
		else
		{
			g_emu_op_para.vsg_pw_total_last = g_emu_op_para.vsg_pw_total;
			g_emu_op_para.vsg_pw_total = (unsigned int)tem;
			printf("·········VSG···有功功率：%d\n", g_emu_op_para.pq_pw_total);
		}
		// if (g_emu_op_para.flag_start == 0 || tem == 0)
		// {

		for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
		{
			if (modbus_sockt_state[i] == STATUS_OFF)
				continue;
			if (g_emu_op_para.flag_start == 0 || tem == 0)
			{
				curTaskId[i] = 0;
				curPcsId[i] = 0;
				if (g_emu_op_para.OperatingMode == PQ && g_emu_op_para.pq_pw_total_last != g_emu_op_para.pq_pw_total)
				{
					printf("61850 PQ模式有功功率下发 g_emu_op_para.pq_pw_total_last=%d g_emu_op_para.pq_pw_total=%d \n", g_emu_op_para.pq_pw_total_last,g_emu_op_para.pq_pw_total);
					lcd_state[i] = LCD_PQ_STP_PWVAL;
				}
				else if (g_emu_op_para.OperatingMode == VSG && g_emu_op_para.vsg_pw_total_last != g_emu_op_para.vsg_pw_total)
				{
					printf("61850 VSG模式有功功率下发\n");
					printf("61850 PQ模式有功功率下发 g_emu_op_para.vsg_pw_total_last=%d g_emu_op_para.vsg_pw_total=%d \n", g_emu_op_para.vsg_pw_total_last,g_emu_op_para.vsg_pw_total);

					lcd_state[i] = LCD_VSG_PW_VAL;
				}
				else
					printf("下发有功功率未发生变化，无需重新设置！\n");
			}
			else
			{
				printf("收到有功功率调节要求LCD[%d]（第一次启动完成已经完成） tem=%f \n",i,tem);
				g_emu_adj_lcd.flag_adj_pw_lcd_cfg[i] = 1; // Lcd收到有功功率调节要求
			}
		}
	}
	break;
	case EMS_QW_SETTING: //无功功率
	case ONE_FM_QW_SETTING:
	{
		float tem;
		tem = *(float *)pYkPara->data;
		printf("············无功功率：%f", tem);
		if (g_emu_op_para.OperatingMode == PQ)
		{

			g_emu_op_para.pq_qw_total_last = g_emu_op_para.pq_qw_total;
			g_emu_op_para.pq_qw_total = (unsigned int)tem;
			printf("-----------PQ---无功功率:%d\n", g_emu_op_para.pq_qw_total);
		}
		else
		{
			g_emu_op_para.vsg_qw_total_last = g_emu_op_para.vsg_qw_total;
			g_emu_op_para.vsg_qw_total = (unsigned int)tem;
			printf("-----------VSG---无功功率:%d\n", g_emu_op_para.vsg_qw_total);
		}

		for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
		{

			if (modbus_sockt_state[i] == STATUS_OFF)
				continue;

			if (g_emu_op_para.flag_start == 0 || tem == 0)
			{
				curTaskId[i] = 0;
				curPcsId[i] = 0;
				if (g_emu_op_para.OperatingMode == PQ && g_emu_op_para.pq_qw_total_last != g_emu_op_para.pq_qw_total)
				{

					printf("61850 PQ模式无功功率下发\n");
					lcd_state[i] = LCD_PQ_STP_QWVAL;
				}
				else if (g_emu_op_para.OperatingMode == VSG && g_emu_op_para.vsg_qw_total_last != g_emu_op_para.vsg_qw_total)
				{
					printf("61850 VSG模式无功功率下发\n");
					lcd_state[i] = LCD_VSG_QW_VAL;
				}
				else
					printf("下发无功功率未发生变化，无需重新设置！\n");
			}
			else
			{
				printf("收到无功功率调节要求LCD[%d]（第一次启动完成已经完成） tem=%f \n",i,tem);
				g_emu_adj_lcd.flag_adj_qw_lcd_cfg[i] = 1; // Lcd收到无功功率调节要求
			}
		}
	}

	break;
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

	printf("aaaaaaaaaaaa__sn:%d\n", sn);
	int lcdid = (sn - 1) / 6;
	int pcsid = (sn - 1) % 6;

	printf("bbbbbbbbbbbb__lcdid:%d\n", lcdid);
	printf("cccccccccccc__pcsid:%d\n", pcsid);
	// if (pcsid >= pPara_Modtcp->pcsnum[lcdid])
	// {
	// 	goto endPcsYk;
	// }

	// ret = ckeckCurPcsStartEn(lcdid, pcsid);

	// if (ret != 0)
	// {
	// 	printf("lcdid=%d pcsid=%d 不满足启动条件，ret=%d\n", lcdid, pcsid, ret);
	// 	goto endPcsYk;
	// }
	printf("lcdid=%d pcsid=%d 满足启动条件，等待启动\n", lcdid, pcsid);
	// if (lcd_state[lcdid] == LCD_RUNNING)
	// {
	// 	flag = 1;
	// 	if (pYkPara->data[0] == 0)
	// 		lcd_state[lcdid] = LCD_PCS_STOP_ONE;
	// 	else
	// 		lcd_state[lcdid] = LCD_PCS_START_ONE;
	// 	curTaskId[lcdid] = 0;
	// 	curPcsId[lcdid] = pcsid;
	// }
	// else
	// {
	flag = 1;
	g_emu_action_lcd.flag_start_stop_lcd[lcdid] = 3;
	if (pYkPara->data[0] == 0)
		g_emu_action_lcd.action_pcs[lcdid].flag_start_stop_pcs[pcsid] = 0xaa;
	else
		g_emu_action_lcd.action_pcs[lcdid].flag_start_stop_pcs[pcsid] = 0x55;

	// }

	// pbackBmsFun(_BMS_YK_, (void *)flag);

	return 0;
}

int findCurPcsForStart(int lcdid, int pcsid)
{
	int i;

	for (i = pcsid; i < pPara_Modtcp->pcsnum[lcdid]; i++)
	{
		if (g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[pcsid] == 0)
		{
			break;
		}
		else
		  	printf("lcdid=%d, pcsid=%d 已经启动\n", lcdid, pcsid); 
	}
	if (i == pPara_Modtcp->pcsnum[lcdid])
	{
		printf("LCD[%d]中所有pcs启动完成\n", lcdid);
		curTaskId[lcdid] = 0;
		curPcsId[lcdid] = 0;
		lcd_state[lcdid] = LCD_RUNNING;
		return 0;
	}
	else
		curPcsId[lcdid] = i;

	return 1;
}
int findCurPcsForStop(int lcdid, int pcsid)
{
	int i;
	printf("findCurPcsForStop lcdid=%d, pcsid=%d\n", lcdid, pcsid);
	for (i = pcsid; i < pPara_Modtcp->pcsnum[lcdid]; i++)
	{
		if (g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[pcsid] == 1)
		{
			break;
		}
	}
	if (i == pPara_Modtcp->pcsnum[lcdid])
	{
		printf("LCD[%d]中所有pcs停止完成\n", lcdid);
		curTaskId[lcdid] = 0;
		curPcsId[lcdid] = 0;
		lcd_state[lcdid] = LCD_RUNNING;
		return 0;
	}
	else
		curPcsId[lcdid] = i;

	return 1;
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
void printf_pcs_soc(void)
{
	int i;
	if (para_bams.portnum == 0)
		return;
	for (i = 0; i < g_emu_op_para.num_pcs_bms[0]; i++)
	{
		printf(" %d ", bmsdata_bak[0][i].soc);
	}
	printf("\n");

	if (para_bams.portnum == 1)
		return;
	for (i = 0; i < g_emu_op_para.num_pcs_bms[1]; i++)
	{
		printf(" %d ", bmsdata_bak[1][i].soc);
	}
	printf("\n");
}

int countPwAdj(int lcdid, int pcsid, int PW, int flag_soc)
{
	int i;
	unsigned short soc_ave = g_emu_op_para.soc_ave;
	int id = 0, id_bms = 0;
	unsigned short soc = 0;
	short dt_soc = 0;
	int pw, dtpw;

	if ((total_pcsnum == 0) || (total_pcsnum - g_emu_op_para.err_num) == 0)
	{
		return 1;
	}

	if (g_emu_op_para.OperatingMode == PQ)
	{
		if (g_emu_op_para.pq_pw_total == 0)
		{
			if (PW == 0)
				goto endAdjPw;
			else
			{
				pw = 0;
				goto settingAdjPw;
			}
		}
		pw = (g_emu_op_para.pq_pw_total * 10) / (total_pcsnum - g_emu_op_para.err_num);
	}
	else if (g_emu_op_para.OperatingMode == VSG)
	{
		if (g_emu_op_para.vsg_pw_total == 0)
		{
			if (PW == 0)
				goto endAdjPw;
			else
			{
				pw = 0;
				goto settingAdjPw;
			}
		}

		pw = (g_emu_op_para.vsg_pw_total * 10) / (total_pcsnum - g_emu_op_para.err_num);
	}
	if (flag_soc == 1)
	{
		for (i = 0; i < lcdid; i++)
		{
			id += pPara_Modtcp->pcsnum[i];
		}
		id += (pcsid - 1);

		printf_pcs_soc();
		if (id >= g_emu_op_para.num_pcs_bms[0])
		{
			id -= g_emu_op_para.num_pcs_bms[0];
			id_bms = 1;
		}
		else
			id_bms = 0;
		soc = bmsdata_bak[id_bms][id].soc;
		dt_soc = soc_ave - soc;
		pw *= (100000 + dt_soc * pPara_Modtcp->balance_rate);
		pw /= 100000;
	}

	dtpw = pw - PW;
	printf("计算出当前需要调节的有功lcd[%d] pcsid[%d] qw=%d 测量值PW=%d 差值=%d\n", lcdid, pcsid, pw, PW, dtpw);
	if (dtpw >= 10 || dtpw <= -10)
	{
	settingAdjPw:
		g_emu_adj_lcd.flag_adj_pw_lcd[lcdid] = 1;
		g_emu_adj_lcd.adj_pcs[lcdid].flag_adj_pw[pcsid - 1] = 1;
		g_emu_adj_lcd.adj_pcs[lcdid].val_pw[pcsid - 1] = pw;
	}

endAdjPw:
	return 0;
}
int countQwAdj(int lcdid, int pcsid, int QW, int flag_soc)
{
	int i;
	unsigned short soc_ave = g_emu_op_para.soc_ave;
	int id = 0, id_bms = 0;
	unsigned short soc = 0;
	short dt_soc = 0;
	int qw, dtqw;

	if ((total_pcsnum == 0) || (total_pcsnum - g_emu_op_para.err_num) == 0)
	{
		return 1;
	}

	if (g_emu_op_para.OperatingMode == PQ)
	{
		if (g_emu_op_para.pq_qw_total == 0)
		{
			if (QW == 0)
				goto endAdjQw;
			else
			{
				qw = 0;
				goto settingAdjQw;
			}
		}
		qw = (g_emu_op_para.pq_qw_total * 10) / (total_pcsnum - g_emu_op_para.err_num);
	}
	else if (g_emu_op_para.OperatingMode == VSG)
	{
		if (g_emu_op_para.vsg_qw_total == 0)
		{
			if (QW == 0)
				goto endAdjQw;
			else
			{
				qw = 0;
				goto settingAdjQw;
			}
		}

		qw = (g_emu_op_para.vsg_qw_total * 10) / (total_pcsnum - g_emu_op_para.err_num);
	}
	if (flag_soc == 1)
	{
		for (i = 0; i < lcdid; i++)
		{
			id += pPara_Modtcp->pcsnum[i];
		}
		id += (pcsid - 1);

		printf_pcs_soc();
		if (id >= g_emu_op_para.num_pcs_bms[0])
		{
			id -= g_emu_op_para.num_pcs_bms[0];
			id_bms = 1;
		}
		else
			id_bms = 0;
		soc = bmsdata_bak[id_bms][id].soc;
		dt_soc = soc_ave - soc;
		qw *= (100000 + dt_soc * pPara_Modtcp->balance_rate);
		qw /= 100000;
	}

	dtqw = qw - QW;
	printf("计算出当前需要调节的无功lcd[%d] pcsid[%d] qw=%d 测量值QW=%d 差值=%d\n", lcdid, pcsid, qw, QW, dtqw);
	if (dtqw > 10 || dtqw < -10)
	{
	settingAdjQw:
		g_emu_adj_lcd.flag_adj_qw_lcd[lcdid] = 1;
		g_emu_adj_lcd.adj_pcs[lcdid].flag_adj_qw[pcsid - 1] = 1;
		g_emu_adj_lcd.adj_pcs[lcdid].val_qw[pcsid - 1] = qw;
	}

endAdjQw:
	return 0;
}
/*int checkQw(int lcdid, int pcsid, unsigned short QW)
{
	unsigned short dtQW;
	int qw;
	unsigned char flag = 0;
	int ret;
	if((total_pcsnum==0) || (total_pcsnum-g_emu_op_para.err_num)==0)
	{
		return 1;
	}

		if(g_emu_op_para.OperatingMode == PQ)
		{
			qw =  (g_emu_op_para.pq_qw_total * 10) /  (total_pcsnum-g_emu_op_para.err_num);

		}
		else if(g_emu_op_para.OperatingMode == VSG)
		{
			qw = (g_emu_op_para.vsg_qw_total * 10) /  (total_pcsnum-g_emu_op_para.err_num);
		}

	dtQW = QW-qw;
	if (dtQW >= 10 || dtQW <= -10)
	{
		flag = 1;
	}

	printf("cba checkQw qw=%d dtQW=%d flag=%d\n",qw,dtQW,flag);
	#if(1)
	ret = countDP_test(lcdid, pcsid, &qw);
	if (ret == 1)
		flag = 1;
	#endif
	if (flag == 1)
	{
		g_emu_adj_lcd.flag_adj_qw_lcd[lcdid] = 1;
		g_emu_adj_lcd.adj_pcs[lcdid].flag_adj_qw[pcsid-1] = 1;
		g_emu_adj_lcd.adj_pcs[lcdid].val_qw[pcsid-1] = qw;
	}

	return 0;
}*/

void printf_adj_qw(int lcdid)
{
	int i;
	printf("abc需要调节无功功率的LCDid=%d\n", lcdid);
	for (i = 0; i < pPara_Modtcp->pcsnum[lcdid]; i++)
	{
		printf("\ng_emu_adj_lcd.adj_pcs[%d].flag_adj_qw[%d]=%d", lcdid, i, g_emu_adj_lcd.adj_pcs[lcdid].flag_adj_qw[i]);
		printf("  g_emu_adj_lcd.adj_pcs[%d].val_qw[%d]=%d\n", lcdid, i, g_emu_adj_lcd.adj_pcs[lcdid].val_qw[i]);
	}
}
int setStatusPw(int lcdid)
{
	if (g_emu_adj_lcd.flag_adj_pw_lcd[lcdid] == 1)
	{
		curPcsId[lcdid] = 0;
		curTaskId[lcdid] = 0;
		lcd_state[lcdid] = LCD_ADJUST_PCS_PW;
		return 1;
	}

	return 0;
}
int setStatusQw(int lcdid)
{
	if (g_emu_adj_lcd.flag_adj_qw_lcd[lcdid] == 1)
	{
		curPcsId[lcdid] = 0;
		curTaskId[lcdid] = 0;
		lcd_state[lcdid] = LCD_ADJUST_PCS_QW;
		printf_adj_qw(lcdid);
	}

	return 0;
}
// int setStatusPw_Qw(void)
// {
// 	int i;
// 	for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
// 	{

// 		if (modbus_sockt_state[i] == STATUS_OFF)
// 			continue;
// 		if (g_emu_adj_lcd.flag_adj_pw_lcd[i] == 1)
// 		{
// 			curPcsId[i] = 0;
// 			curTaskId[i] = 0;
// 			lcd_state[i] = LCD_ADJUST_PCS_PW;
// 		}
// 		else if (g_emu_adj_lcd.flag_adj_qw_lcd[i] == 1)
// 		{
// 			curPcsId[i] = 0;
// 			curTaskId[i] = 0;
// 			lcd_state[i] = LCD_ADJUST_PCS_QW;
// 			printf_adj_qw(i);
// 		}
// 	}
// 	return 0;
// }

int setStatusStart_Stop(int lcdid)
{
	int i;
	int flag = 0;

	if (g_emu_action_lcd.flag_start_stop_lcd[lcdid] == 1)
	{

		lcd_state[lcdid] = LCD_PCS_START;
		curPcsId[lcdid] = 0;
		curTaskId[lcdid] = 0;
		flag = 1;
	}
	else if (g_emu_action_lcd.flag_start_stop_lcd[lcdid] == 2)
	{
		lcd_state[lcdid] = LCD_PCS_STOP;
		curPcsId[lcdid] = 0;
		curTaskId[lcdid] = 0;
		flag = 2;
	}
	else if (g_emu_action_lcd.flag_start_stop_lcd[lcdid] == 3)
	{
		for (i = 0; i < pPara_Modtcp->pcsnum[lcdid]; i++)
		{
			if (g_emu_action_lcd.action_pcs[lcdid].flag_start_stop_pcs[i] == 0xaa || g_emu_action_lcd.action_pcs[lcdid].flag_start_stop_pcs[i] == 0x55)
			{
				curTaskId[lcdid] = 0;
				curPcsId[lcdid] = i;
				lcd_state[lcdid] = LCD_PCS_START_STOP_ONE;
			}
		}
		flag = 3;
	}

	return flag;
}
int setStatusStart_Stop1(void)
{
	int i, j;
	int flag = 0;
	for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
	{
		if (modbus_sockt_state[i] == STATUS_OFF)
			continue;
		curPcsId[i] = 0;
		curTaskId[i] = 0;

		if (g_emu_action_lcd.flag_start_stop_lcd[i] == 1)
		{

			lcd_state[i] = LCD_PCS_START;
			flag = 1;
		}
		else if (g_emu_action_lcd.flag_start_stop_lcd[i] == 2)
		{
			lcd_state[i] = LCD_PCS_STOP;
			flag = 2;
		}
		else if (g_emu_action_lcd.flag_start_stop_lcd[i] == 3)
		{
			for (j = 0; j < pPara_Modtcp->pcsnum[i]; j++)
			{
				if (g_emu_action_lcd.action_pcs[i].flag_start_stop_pcs[j] == 0xaa || g_emu_action_lcd.action_pcs[i].flag_start_stop_pcs[j] == 0x55)
				{
					curPcsId[i] = j;
					lcd_state[i] = LCD_PCS_START_STOP_ONE;
				}
			}
			flag = 3;
		}
	}
	return flag;
}
int findCurPcsidForStart_Stop(int id_thread)
{
	int i;
	for (i = curPcsId[id_thread]; i < pPara_Modtcp->pcsnum[id_thread]; i++)
	{
		if (g_emu_action_lcd.action_pcs[id_thread].flag_start_stop_pcs[i] == 0x55 || g_emu_action_lcd.action_pcs[id_thread].flag_start_stop_pcs[i] == 0xaa)
		{
			curPcsId[id_thread] = i;
			break;
		}
		else
			printf("LCD:%d PCSID:%d 没有启停请求 ...\n", id_thread, i);
	}
	if (i == pPara_Modtcp->pcsnum[id_thread])
	{
		printf("LCD:%d 中所有PCS完成启停\n", id_thread);
		curTaskId[id_thread] = 0;
		curPcsId[id_thread] = 0;
		g_emu_action_lcd.flag_start_stop_lcd[id_thread] = 0;
		lcd_state[id_thread] = LCD_RUNNING;
		return 0;
	}
	return 1;
}
int findCurPcsidForAdjPw(int id_thread)
{
	int i;
	for (i = curPcsId[id_thread]; i < pPara_Modtcp->pcsnum[id_thread]; i++)
	{
		if (g_emu_adj_lcd.adj_pcs[id_thread].flag_adj_pw[i] == 1)
		{
			curPcsId[id_thread] = i;
			break;
		}
		else
			printf("LCD:%d PCSID:%d 无需调节有功功率 ...\n", id_thread, i);
	}
	if (i == pPara_Modtcp->pcsnum[id_thread])
	{
		printf("按策略要求调节有功功率完成\n");
		curTaskId[id_thread] = 0;
		curPcsId[id_thread] = 0;
		g_emu_adj_lcd.flag_adj_pw_lcd[id_thread] = 0;
		lcd_state[id_thread] = LCD_RUNNING;
		return 0;
	}
	return 1;
}
int findCurPcsidForAdjQw(int id_thread)
{
	int i;
	for (i = curPcsId[id_thread]; i < pPara_Modtcp->pcsnum[id_thread]; i++)
	{
		if (g_emu_adj_lcd.adj_pcs[id_thread].flag_adj_qw[i] == 1)
		{
			curPcsId[id_thread] = i;
			break;
		}
		else
			printf("LCD:%d PCSID:%d 无需调节无功功率 ...\n", id_thread, i);
	}
	if (i == pPara_Modtcp->pcsnum[id_thread])
	{
		printf("按策略要求调节无功功率完成\n");
		curTaskId[id_thread] = 0;
		curPcsId[id_thread] = 0;
		g_emu_adj_lcd.flag_adj_qw_lcd[id_thread] = 0;
		lcd_state[id_thread] = LCD_RUNNING;
		return 0;
	}
	return 1;
}

void initEmuParaData(void) //初始化EMU参数和数据
{
	memset((unsigned char *)&g_emu_adj_lcd, 0, sizeof(EMU_ADJ_LCD));
	memset((unsigned char *)&g_emu_action_lcd, 0, sizeof(EMU_ACTION_LCD));
	memset((unsigned char *)&g_emu_status_lcd, 0, sizeof(EMU_STATUS_LCD));
}
