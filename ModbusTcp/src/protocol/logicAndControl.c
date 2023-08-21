#include "logicAndControl.h"
#include "output.h"
#include "YX_define.h"
#include "importBams.h"
#include <stdio.h>
#include "client.h"
#include "modbus.h"
#include <sys/mman.h>
#include <string.h>
#include "modbus_tcp_main.h"
int total_pcsnum = 32;
int g_flag_RecvNeed = 0;
int g_flag_RecvNeed_LCD = 0;

unsigned char flag_RecvNeed_PCS[MAX_PCS_NUM];
EMU_ADJ_LCD g_emu_adj_lcd;

EMU_STATUS_LCD g_emu_status_lcd;

EMU_ACTION_LCD g_emu_action_lcd;

// int bams_heartbeat_timer_flag[2][18];

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
	YK_PARA para;
#if TEST_PLC_D1D2
	// 整机开机前检查D1、D2为分闸的情况下合闸
	if (PLC_EMU_BOX_SwitchD1 == 0)
	{
		para.item = BOX_SwitchD1_ON;
		para.data[0] = 1;
		ykOrder_pcs_plc(_BMS_PLC_YK_, &para, NULL);
	}
	if (PLC_EMU_BOX_SwitchD2 == 0)
	{
		para.item = BOX_SwitchD2_ON;
		para.data[0] = 1;
		ykOrder_pcs_plc(_BMS_PLC_YK_, &para, NULL);
	}
#endif

	for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
	{
		if (modbus_sockt_state[i] == STATUS_OFF)
			continue;
		printf("LCD[%d] 收到启动 startAllPcs lcd_state[i]=0x%x\n", i, lcd_state[i]);
		if (lcd_state[i] == LCD_RUNNING)
		{
			flag = 1;
			printf("LCD[%d] 立即启动 startAllPcs lcd_state[i]=%d\n", i, lcd_state[i]);

			// if (g_emu_op_para.err_num != 0)
			// 	lcd_state[i] = LCD_PCS_START;
			// else
			lcd_state[i] = LCD_PCS_START_ALL;
			// lcd_state[i] = LCD_PCS_START;
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
}
//		pbackBmsFun(_BMS_YK_, (void *)flag);

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
			lcd_state[i] = LCD_PCS_STOP_ALL;
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
	unsigned char item; // 项目编号
	int i;
	item = pYkPara->item;
	printf("aaaaaaaaaa item:%d \n", (int)pYkPara->item);

	switch (item)
	{
	case Emu_Startup:
	{
		printf("handleYkFromEms startAllPcs\n");
		startAllPcs();
	}
	break;
	case Emu_Stop:
	{
		printf("stopAllPcs \n");
		stopAllPcs();
	}
	break;
	case EMS_PW_SETTING: // 有功功率
	case ONE_FM_PW_SETTING:
	{

		float tem;
		tem = *(float *)pYkPara->data;
		printf("············有功功率：%f", (float)tem);

		if (g_emu_op_para.OperatingMode == PQ)
		{
			g_emu_op_para.pq_pw_total_last = g_emu_op_para.pq_pw_total;
			g_emu_op_para.pq_pw_total = (int)tem * 10;
			printf("·········PQ···有功功率：%d\n", g_emu_op_para.pq_pw_total);
		}
		else
		{
			g_emu_op_para.vsg_pw_total_last = g_emu_op_para.vsg_pw_total;
			g_emu_op_para.vsg_pw_total = (int)tem * 10;
			printf("·········VSG···有功功率：%d\n", g_emu_op_para.pq_pw_total);
		}

		for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
		{
			if (modbus_sockt_state[i] == STATUS_OFF)
				continue;
			lcd_state[i] = LCD_PQ_STP_PWVAL_ALL;
		}
#if 0
		for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
		{
			if (modbus_sockt_state[i] == STATUS_OFF)
				continue;
			if (g_emu_op_para.flag_start == 0 || tem == 0)
			{
				curTaskId[i] = 0;
				curPcsId[i] = 0;
				// if (g_emu_op_para.OperatingMode == PQ && g_emu_op_para.pq_pw_total_last != g_emu_op_para.pq_pw_total)
				if (g_emu_op_para.OperatingMode == PQ)
				{
					printf("61850 PQ模式有功功率下发 g_emu_op_para.pq_pw_total_last=%d g_emu_op_para.pq_pw_total=%d \n", g_emu_op_para.pq_pw_total_last, g_emu_op_para.pq_pw_total);
					lcd_state[i] = LCD_PQ_STP_PWVAL;
				}
				else if (g_emu_op_para.OperatingMode == VSG && g_emu_op_para.vsg_pw_total_last != g_emu_op_para.vsg_pw_total)
				{
					printf("61850 VSG模式有功功率下发\n");
					printf("61850 PQ模式有功功率下发 g_emu_op_para.vsg_pw_total_last=%d g_emu_op_para.vsg_pw_total=%d \n", g_emu_op_para.vsg_pw_total_last, g_emu_op_para.vsg_pw_total);

					lcd_state[i] = LCD_VSG_PW_VAL;
				}
				else
					printf("下发有功功率未发生变化，无需重新设置！\n");
			}
			else
			{
				printf("收到有功功率调节要求LCD[%d]（第一次启动完成已经完成） tem=%f \n", i, tem);
				g_emu_adj_lcd.flag_adj_pw_lcd_cfg[i] = 1; // Lcd收到有功功率调节要求
			}
		}
#endif
	}
	break;
	case POWER_FACTOR_SETTING:
	{
		int tem;
		tem = *(int *)pYkPara->data;
		printf("············ems下发的功率因数：%d", tem);
		g_emu_op_para.power_factor_last = g_emu_op_para.power_factor;
		g_emu_op_para.power_factor = tem;
		printf("·········PQ···功率因数：%d\n", g_emu_op_para.power_factor);
		for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
		{
			if (modbus_sockt_state[i] == STATUS_OFF)
				continue;
			lcd_state[i] = LCD_PF_SETTING_ALL;
		}
	}
	break;

	case EMS_QW_SETTING: // 无功功率
	case ONE_FM_QW_SETTING:
	{
		float tem;
		tem = *(float *)pYkPara->data;
		printf("············无功功率：%f", tem);
		if (g_emu_op_para.OperatingMode == PQ)
		{

			g_emu_op_para.pq_qw_total_last = g_emu_op_para.pq_qw_total;
			g_emu_op_para.pq_qw_total = (int)tem;
			printf("-----------PQ---无功功率:%d\n", g_emu_op_para.pq_qw_total);
		}
		else
		{
			g_emu_op_para.vsg_qw_total_last = g_emu_op_para.vsg_qw_total;
			g_emu_op_para.vsg_qw_total = (int)tem;
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
				printf("收到无功功率调节要求LCD[%d]（第一次启动完成已经完成） tem=%f \n", i, tem);
				g_emu_adj_lcd.flag_adj_qw_lcd_cfg[i] = 1; // Lcd收到无功功率调节要求
			}
		}
	}

	break;
	case EMS_SET_MODE: // 系统未启动下改变运行模式：从PQ-->VSG 或VSG-->PQ
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
	case EMS_PQ_MODE: // 系统为PQ工作模式下，设置工作模式为恒功率模式或恒流模式
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
	case Parallel_Away_conversion_en: // 并转离切换使能
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
	case Away_Parallel_conversion_en: // 离转并切换使能
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
	int lcd_pcs_id = 0, lcdid = 0, pcsid = 0;
	int i;
	// int lcdid = (sn - 1) / 6;
	// int pcsid = (sn - 1) % 6;

	for (i = 0; i < MAX_PCS_NUM; i++)
	{
		lcd_pcs_id += pPara_Modtcp->pcsnum[i];
		if (sn > lcd_pcs_id)
		{
			lcdid++;
		}
		else
		{
			pcsid = sn - (lcd_pcs_id - pPara_Modtcp->pcsnum[i]);
			break;
		}
	}

	printf("bbbbbbbbbbbb__lcdid:%d\n", lcdid);
	printf("cccccccccccc__pcsid:%d\n", pcsid);
	// if (pcsid >= pPara_Modtcp->pcsnum[lcdid])
	// {
	// 	goto endPcsYk;
	// }

	// int ret = (lcdid, pcsid);

	// if (ret != 0)
	// {
	// 	printf("lcdid=%d pcsid=%d 不满足启动条件，ret=%d\n", lcdid, pcsid, ret);
	// 	return;
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
		g_emu_action_lcd.action_pcs[lcdid].flag_start_stop_pcs[pcsid - 1] = 0xaa;
	else
		g_emu_action_lcd.action_pcs[lcdid].flag_start_stop_pcs[pcsid - 1] = 0x55;

	// }

	// pbackBmsFun(_BMS_YK_, (void *)flag);

	return 0;
}

int findCurPcsForStart(int lcdid, int pcsid)
{
	int i;

	for (i = pcsid; i < pPara_Modtcp->pcsnum[lcdid]; i++)
	{
		if (g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[i] == 0 && g_emu_status_lcd.status_pcs[lcdid].flag_err[i] == 0)
		{
			break;
		}
		else
			printf("lcdid=%d, pcsid=%d 已经启动 或发生故障\n", lcdid, pcsid);
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
		if (g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[i] == 1)
		{
			break;
		}
		else
			printf("findCurPcsForStop 没有启动 lcdid=%d, pcsid=%d  err=%d\n", lcdid, pcsid, g_emu_status_lcd.status_pcs[lcdid].flag_err[i]);
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

void printf_pcs_soc(void)
{
	int i, j;
	if (para_bams.portnum == 0)
		return;
	for (j = 0; j < pPara_Modtcp->bams_num; j++)
	{
		for (i = 0; i < g_emu_op_para.num_pcs_bms[j]; i++)
		{
			printf(" %d ", bmsdata_bak[j][i].soc);
		}
	}

	printf("\n");
}

int countPwAdj(int lcdid, int pcsid, short PW, int flag_soc)
{
	int i;
	unsigned short soc_ave = g_emu_op_para.soc_ave;
	int id = 0, id_bms = 0;
	unsigned short soc = 0;
	short dt_soc = 0;
	int pw, dtpw;

	printf("test asdfdsf  PW:%d\n", PW);
	if ((total_pcsnum == 0) || (total_pcsnum - g_emu_op_para.err_num) == 0)
	{
		return 1;
	}
	printf("test asdfdsf222\n");
	if (g_emu_op_para.OperatingMode == PQ)
	{
		if (g_emu_op_para.pq_pw_total == 0)
		{
			if (PW == 0)
				goto endAdjPw;
			else
			{
				pw = 0;
				printf("有功功率直接进行调成0\n");
				goto settingAdjPw;
			}
		}
		if ((total_pcsnum - g_emu_op_para.err_num) != 0)
		{
			pw = (g_emu_op_para.pq_pw_total * 10) / (total_pcsnum - g_emu_op_para.err_num);
			// pw = g_emu_op_para.pq_pw_total / (total_pcsnum - g_emu_op_para.err_num);
		}

		else
			pw = 0;
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
		if ((total_pcsnum - g_emu_op_para.err_num) != 0)
			pw = (g_emu_op_para.vsg_pw_total * 10) / (total_pcsnum - g_emu_op_para.err_num);
		// pw = g_emu_op_para.vsg_pw_total / (total_pcsnum - g_emu_op_para.err_num);
		else
			pw = 0;
	}
	printf("test asdfdsf11\n");

	if (flag_soc == 1)
	{
		// pw = (g_emu_op_para.pq_pw_total * 10);
		// printf("功率调节目标值 lcdid=%d  pcsid=%d  pw=%d g_emu_op_para.pq_pw_total=%d num=%d\n",lcdid,pcsid,pw,g_emu_op_para.pq_pw_total,(total_pcsnum - g_emu_op_para.err_num));

		// pw /= (total_pcsnum);
		printf("功率调节目标值 lcdid=%d  pcsid=%d  pw=%d g_emu_op_para.pq_pw_total=%d pcsnum=%d errnum=%d\n", lcdid, pcsid, pw, g_emu_op_para.pq_pw_total,
			   total_pcsnum, g_emu_op_para.err_num);

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
		if (pw < 0)
			soc = soc - soc_ave;
		else if (pw > 0)
			soc = soc_ave - soc;
		// dt_soc = soc_ave - soc;
		pw *= (100000 + dt_soc * pPara_Modtcp->balance_rate);
		pw /= 100000;
	}
	dtpw = pw - (PW * 10);
	printf("test asdfdsf22222\n");
	printf("计算出当前需要调节的有功lcd[%d] pcsid[%d] qw=%d 测量值PW=%d 差值=%d\n", lcdid, pcsid, pw, PW, dtpw);
	if (dtpw >= 30 || dtpw <= -30)
	{
	settingAdjPw:
		g_emu_adj_lcd.flag_adj_pw_lcd[lcdid] = 1;
		g_emu_adj_lcd.adj_pcs[lcdid].flag_adj_pw[pcsid - 1] = 1;
		g_emu_adj_lcd.adj_pcs[lcdid].val_pw[pcsid - 1] = pw;
	}

endAdjPw:
	return 0;
}
int countQwAdj(int lcdid, int pcsid, short QW, int flag_soc)
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
	if (dtqw > 30 || dtqw < -30)
	{
	settingAdjQw:
		g_emu_adj_lcd.flag_adj_qw_lcd[lcdid] = 1;
		g_emu_adj_lcd.adj_pcs[lcdid].flag_adj_qw[pcsid - 1] = 1;
		g_emu_adj_lcd.adj_pcs[lcdid].val_qw[pcsid - 1] = qw;
	}

endAdjQw:
	return 0;
}

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

void initEmuParaData(void) // 初始化EMU参数和数据
{
	memset((unsigned char *)&g_emu_adj_lcd, 0, sizeof(EMU_ADJ_LCD));
	memset((unsigned char *)&g_emu_action_lcd, 0, sizeof(EMU_ACTION_LCD));
	memset((unsigned char *)&g_emu_status_lcd, 0, sizeof(EMU_STATUS_LCD));

	memset((unsigned char *)&bmsdata_cur, 0, sizeof(BmsData_Newest));
	memset((unsigned char *)&bmsdata_bak, 0, sizeof(BmsData_Newest));
}
