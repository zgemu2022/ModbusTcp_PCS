#include "importBams.h"

#include <stdio.h>
#include "output.h"
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "logicAndControl.h"
#include "YC_Define.h"
#define LIB_MODBMS_PATH "/usr/local/lib/libbams_rtu.so"
PARA_BAMS para_bams;
BmsData_Newest bmsdata_cur[PORTNUM_MAX][18];
BmsData_Newest bmsdata_bak[PORTNUM_MAX][18];
unsigned char bms_ov_status[6] = {0, 0, 0, 0, 0, 0};
unsigned char bms_err_status[6] = {0, 0, 0, 0, 0, 0};
int checkBmsForStart(int sn)
{
	unsigned short status_bms;
	float soc_ave, soc_ave_p20, soc_ave_n20; // = g_emu_op_para.soc_ave;
	float soc;

	status_bms = bmsdata_cur[0][sn].sys_status;

	if (status_bms & ((1 << BMS_ST_WORKING) == 0))
		return 1;
	soc_ave = (float)g_emu_op_para.soc_ave;

	if (sn <= g_emu_op_para.num_pcs_bms[0])
	{
		soc = (float)bmsdata_cur[0][sn].soc;
	}
	else
	{
		soc = (float)bmsdata_cur[1][sn - g_emu_op_para.num_pcs_bms[0] + 1].soc;
	}
	soc_ave_p20 = (soc_ave * 6) / 5;
	soc_ave_n20 = (soc_ave * 4) / 5;

	printf("checkBmsForStart soc_ave=%f soc_ave_p20=%f soc_ave_n20=%f\n ", soc_ave, soc_ave_p20, soc_ave_n20);
	if ((soc >= soc_ave_p20) || (soc <= soc_ave_n20))
	{
		return 2;
	}
	return 0;
}
int countPcsNum_Bms(unsigned int flag_recv)
{
	int i;
	int num_pcs = 0;

	for (i = 0; i < 18; i++)
	{
		if ((flag_recv & (1 << i)) != 0)
			num_pcs++;
	}
	return num_pcs;
}

int lcdPcsCount(unsigned char bmsid, unsigned char pcsid_bms, unsigned char *pLcdid, unsigned char *pLcd_pcs_id, unsigned char *psn)
{
	int i;
	unsigned char lcdid = 0, lcd_pcs_id = 0;
	int temp = 0, pcsid_bms_temp = pcsid_bms;
	for (i = 0; i < bmsid; i++)
		temp += g_emu_op_para.num_pcs_bms[i];

	pcsid_bms_temp += temp;
	*psn = pcsid_bms_temp;

	printf("BAMS 的数据 bmsid=%d pcsid_bms=%d 总id =%d \n", bmsid, pcsid_bms, pcsid_bms_temp);

	while (pcsid_bms_temp >= pPara_Modtcp->pcsnum[lcdid])
	{
		//	printf("while BAMS 的数据 bmsid=%d pcsid_bms=%d 总id =%d \n", bmsid, pcsid_bms, pcsid_bms_temp);
		pcsid_bms_temp -= pPara_Modtcp->pcsnum[lcdid];
		lcdid++;
	}
	lcd_pcs_id = pcsid_bms_temp;
	*pLcdid = lcdid;
	*pLcd_pcs_id = lcd_pcs_id;
	// for (i = 0; i < MAX_PCS_NUM; i++)
	// {
	// 	lcd_pcs_id += pPara_Modtcp->pcsnum[i];
	// 	if ((pcsid_bms_temp + 1) > lcd_pcs_id)
	// 	{
	// 		lcdid++;
	// 	}
	// 	else
	// 	{
	// 		lcd_pcs_id = (pcsid_bms_temp + 1) - (lcd_pcs_id - pPara_Modtcp->pcsnum[i]);
	// 		break;
	// 	}
	// }
}

// int lcdPcsCount(unsigned char bmsid, unsigned char pcsid_bms, unsigned char *pLcdid, unsigned char *pLcd_pcs_id)
// {
// 	int i;
// 	unsigned char lcdid = 0, lcd_pcs_id = 0;
// 	if (bmsid == 0)
// 	{
// 		for (i = 0; i < MAX_PCS_NUM; i++)
// 		{
// 			lcd_pcs_id += pPara_Modtcp->pcsnum[i];
// 			if ((pcsid_bms + 1) > lcd_pcs_id)
// 			{
// 				lcdid++;
// 			}
// 			else
// 			{
// 				lcd_pcs_id = (pcsid_bms + 1) - (lcd_pcs_id - pPara_Modtcp->pcsnum[i]);
// 				break;
// 			}
// 		}
// 	}
// 	else if (bmsid == 1)
// 	{
// 		unsigned char pcsid_bms1 = (pcsid_bms + 1) + 15; // 15为第一台BAMS下的pcs数量
// 		for (i = 0; i < MAX_PCS_NUM; i++)
// 		{
// 			lcd_pcs_id += pPara_Modtcp->pcsnum[i];
// 			if (pcsid_bms1 > lcd_pcs_id)
// 			{
// 				lcdid++;
// 			}
// 			else
// 			{
// 				lcd_pcs_id = pcsid_bms1 - (lcd_pcs_id - pPara_Modtcp->pcsnum[i]);
// 				break;
// 			}
// 		}
// 	}
// 	*pLcdid = lcdid;
// 	*pLcd_pcs_id = lcd_pcs_id - 1;
// 	return 0;
// }
int check_adj_pw(unsigned char lcdid, unsigned char lcd_pcs_id, unsigned char sn, short mx_dpw, short mx_cpw, short pw)
{

	int flag = 0;
	printf("当前功率情况 lcdid=%d lcd_pcs_id=%d 最大放电：%d 最小充电：%d 遥测电压：%d \n", lcdid, lcd_pcs_id, mx_dpw, -mx_cpw, pw);

	if (g_emu_status_lcd.status_pcs[lcdid].flag_start_stop[lcd_pcs_id] == 1)
	{

		printf("充电开始 当前功率情况 lcdid=%d lcd_pcs_id=%d 最大放电：%d 最小充电：%d 遥测电压：%d \n", lcdid, lcd_pcs_id, mx_dpw, -mx_cpw, pw);
		if (pw > 0 && pw > mx_dpw)
		{
			sys_debug("1111需要调整放电 当前功率情况  最大放电：%d 最小充电：%d 遥测电压：%d\n", mx_dpw, -mx_cpw, pw);
			// bms_adj_pw_temp[lcdid] |= (1 << lcd_pcs_id);
			g_emu_adj_lcd.adj_pcs[lcdid].val_pw[lcd_pcs_id] = mx_dpw;
			g_emu_adj_lcd.flag_adj_pw_lcd[lcdid] = 1;
			g_emu_adj_lcd.adj_pcs[lcdid].flag_adj_pw[lcd_pcs_id] = 1;
			flag = 1;
		}
		else if (pw < 0 && pw < -mx_cpw)
		{
			sys_debug("2222需要调整充电 当前功率情况  最大放电：%d 最小充电：%d 遥测电压：%d lcdid=%d lcd_pcs_id=%d\n", mx_dpw, -mx_cpw, pw, lcdid, lcd_pcs_id);

			// bms_adj_pw_temp[lcdid] |= (1 << lcd_pcs_id);
			g_emu_adj_lcd.adj_pcs[lcdid].val_qw[lcd_pcs_id] = -mx_cpw;

			g_emu_adj_lcd.flag_adj_pw_lcd[lcdid] = 1;
			g_emu_adj_lcd.adj_pcs[lcdid].flag_adj_pw[lcd_pcs_id] = 1;
			flag = 1;
		}
	}

	return flag;
}

void setting_ov_status(unsigned char bmsid, unsigned char pcsid_bms, unsigned short single_mx_vol, unsigned short single_mi_vol, unsigned short sys_status, short mx_dpw, short mx_cpw)
{

	unsigned short temp_pw;
	short pw;

	unsigned char lcdid = 0, lcd_pcs_id = 0, sn = 0;
	int flag_temp;

	printf("uuuuiiii\n");
	temp_pw = g_YcData[sn].pcs_data[Active_power];
	pw = (temp_pw % 256) * 256 + temp_pw / 256;
	//	pw = -190;
	pw *= 10;
	static unsigned char flag_recv_pcs[] = {0, 0, 0, 0, 0, 0};
	static unsigned char bms_ov_status_temp[] = {0, 0, 0, 0, 0, 0};
	static unsigned char bms_err_status_temp[] = {0, 0, 0, 0, 0, 0};
	lcdPcsCount(bmsid, pcsid_bms, &lcdid, &lcd_pcs_id, &sn);

	flag_recv_pcs[lcdid] |= (1 << lcd_pcs_id);

	printf("setting_ov_status 33333 lcdid=%d  bmsid=%d lcd_pcs_id=%d  pcsid_bms=%d  flag_recv_pcs[lcdid]=%x single_mx_vol=%d, single_mi_vol=%d\n", lcdid, bmsid, lcd_pcs_id, pcsid_bms, flag_recv_pcs[lcdid], single_mx_vol, single_mi_vol);

	printf("pPara_Modtcp->Maximum_individual_voltage=%d  pPara_Modtcp->Minimum_individual_voltage=%d\n", pPara_Modtcp->Maximum_individual_voltage, pPara_Modtcp->Minimum_individual_voltage);
	printf("LCD 模块启动 最高单体电压=%d  最低单体电压=%d\n", pPara_Modtcp->Maximum_individual_voltage, pPara_Modtcp->Minimum_individual_voltage);
	/*
		1.单体正常充放电截止电压区间 2.90V~3.55V，PCS 检测到电池分系统单体最高电压达到3.6V，PCS 应停机或封脉冲；电池分系统单体,最高电压达到 3.63V，PCS 应关机；
		2.单体正常充放电截止电压区间 2.90V~3.55V，PCS 检测到电池分系统单体最低电压达到2.85V，PCS 应停机或封脉冲；电池分系统单体,最低电压达到 2.75V，PCS 应关机；
	*/
	if (single_mx_vol >= pPara_Modtcp->Maximum_individual_voltage || single_mi_vol <= pPara_Modtcp->Minimum_individual_voltage || sys_status == 5)
	{
		if (single_mx_vol >= pPara_Modtcp->Maximum_individual_voltage)
		{

			printf("setting_ov_status aaabbb single_mx_vol=%d  pPara_Modtcp->Maximum_individual_voltage=%d \n", single_mx_vol, pPara_Modtcp->Maximum_individual_voltage);

			if (pw != 0)
			{
				bms_ov_status_temp[lcdid] |= (1 << lcd_pcs_id);
			}

			sys_debug("setting_ov_status aaabbbcccc\n");
		}
		else if (single_mi_vol <= pPara_Modtcp->Minimum_individual_voltage)
		{
			printf("setting_ov_status single_mi_vol:%d pPara_Modtcp->Maximum_individual_voltage=%d \n", single_mi_vol, pPara_Modtcp->Maximum_individual_voltage);

			if (pw != 0)
			{
				bms_ov_status_temp[lcdid] |= (1 << lcd_pcs_id);
			}
			sys_debug("setting_ov_status aaabbbcccc\n");
		}
		else
		{
			bms_err_status_temp[lcdid] |= (1 << lcd_pcs_id);
			sys_debug("出现电池簇故障！！！\n");
		}
	}
	printf("setting_ov_status 44444 lcdid=%d  lcd_pcs_id=%d  flag_recv_pcs[lcdid]=%x flag_RecvNeed_PCS[lcdid]=%x\n", lcdid, lcd_pcs_id, flag_recv_pcs[lcdid], flag_RecvNeed_PCS[lcdid]);
	if (flag_recv_pcs[lcdid] == flag_RecvNeed_PCS[lcdid])
	{
		flag_recv_pcs[lcdid] = 0;

		if (bms_ov_status[lcdid] == 0 && bms_ov_status_temp[lcdid] != 0)
		{
			bms_ov_status[lcdid] = bms_ov_status_temp[lcdid];
			bms_ov_status_temp[lcdid] = 0;
			sys_debug("setting_ov_status 11111 lcdid=%d %x  %x  \n", lcdid, bms_ov_status[lcdid], bms_ov_status_temp[lcdid]);
		}
		else
			sys_debug("setting_ov_status 00000   lcdid=%d  %x  %x  \n", lcdid, bms_ov_status[lcdid], bms_ov_status_temp[lcdid]);

		if (bms_err_status[lcdid] == 0 && bms_err_status_temp[lcdid] != 0)
		{
			bms_err_status[lcdid] = bms_err_status_temp[lcdid];
			bms_err_status_temp[lcdid] = 0;
			sys_debug("setting_ov_status bbbbb lcdid=%d %x  %x  \n", lcdid, bms_err_status[lcdid], bms_err_status_temp[lcdid]);
		}
		else
			sys_debug("setting_ov_status aaaaa   lcdid=%d  %x  %x  \n", lcdid, bms_err_status[lcdid], bms_err_status_temp[lcdid]);
	}

	flag_temp = check_adj_pw(lcdid, lcd_pcs_id, sn, mx_dpw, mx_cpw, pw);
	if (flag_temp > 0)
	{
		printf("setting_ov_status 检测出需要调节功率lcdid=%d lcd_pcs_id=%d\n", lcdid, lcd_pcs_id);
	}

	if (flag_recv_pcs[lcdid] == flag_RecvNeed_PCS[lcdid])
	{
		flag_recv_pcs[lcdid] = 0;

		// if (flag > 0)
		// {
		// 	setStatusPw(lcdid);
		// }
		// flag = 0;
	}
}

int recvfromBams(unsigned char pcsid_bms, unsigned char type, void *pdata)
{
	int i, j;
	switch (type)
	{
	case _ALL_:
	{
		static unsigned int flag_recv_bms[PORTNUM_MAX];
		static int flag_first = 1;

		int num_pcs[PORTNUM_MAX], total_temp;
		BmsData temp = *(BmsData *)pdata;
		unsigned char bmsid = temp.bmsid;

		if (flag_first == 1)
		{
			flag_first = 0;
			for (i = 0; i < PORTNUM_MAX; i++)
				flag_recv_bms[i] = 0;
		}

		printf("xxxLCD模块收到BAMS传来的所有数据！bmsid=%d pcsid=%d %d g_emu_op_para.num_pcs_bms[0]=%d\n", temp.bmsid, pcsid_bms, temp.pcsid_bms, g_emu_op_para.num_pcs_bms[0]);
		bmsdata_cur[bmsid][pcsid_bms].mx_cpw = temp.buf_data[BMS_MX_CPW * 2] * 256 + temp.buf_data[BMS_MX_CPW * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].mx_dpw = temp.buf_data[BMS_MX_DPW * 2] * 256 + temp.buf_data[BMS_MX_DPW * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].heartbeat = temp.buf_data[BMS_CONN_HEARTBEAT * 2] * 256 + temp.buf_data[BMS_CONN_HEARTBEAT * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].main_vol = temp.buf_data[BMS_MAIN_VOLTAGE * 2] * 256 + temp.buf_data[BMS_MAIN_VOLTAGE * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].mx_ccur = temp.buf_data[BMS_MX_CCURRENT * 2] * 256 + temp.buf_data[BMS_MX_CCURRENT * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].mx_dccur = temp.buf_data[BMS_MX_DCURRENT * 2] * 256 + temp.buf_data[BMS_MX_DCURRENT * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].sum_cur = temp.buf_data[BMS_SUM_CURRENT * 2] * 256 + temp.buf_data[BMS_SUM_CURRENT * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].soc = temp.buf_data[BMS_SOC * 2] * 256 + temp.buf_data[BMS_SOC * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].remain_ch_cap = temp.buf_data[BMS_remaining_charging_capacity * 2] * 256 + temp.buf_data[BMS_remaining_charging_capacity * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].remain_dch_cap = temp.buf_data[BMS_remaining_discharging_capacity * 2] * 256 + temp.buf_data[BMS_remaining_discharging_capacity * 2 + 1];

		bmsdata_cur[bmsid][pcsid_bms].single_mx_vol = temp.buf_data[BMS_single_MX_voltage * 2] * 256 + temp.buf_data[BMS_single_MX_voltage * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].single_mi_vol = temp.buf_data[BMS_single_MI_voltage * 2] * 256 + temp.buf_data[BMS_single_MI_voltage * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].sys_status = temp.buf_data[BMS_SYS_STATUS * 2] * 256 + temp.buf_data[BMS_SYS_STATUS * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].sys_need = temp.buf_data[BMS_SYS_NEED * 2] * 256 + temp.buf_data[BMS_SYS_NEED * 2 + 1];
		bmsdata_cur[bmsid][pcsid_bms].if_sys_fault = temp.buf_data[BMS_FAULT_STATUS * 2] * 256 + temp.buf_data[BMS_FAULT_STATUS * 2 + 1];
		flag_recv_bms[bmsid] |= (1 << pcsid_bms);
		printf("xxyyzz g_emu_op_para.num_pcs_bms[0]=%d  g_emu_op_para.num_pcs_bms[1] =%d \n", g_emu_op_para.num_pcs_bms[0], g_emu_op_para.num_pcs_bms[1]);

		if (g_emu_op_para.num_pcs_bms[0] > 0 && g_emu_op_para.num_pcs_bms[1] > 0)
		{
			printf("recvfromBams g_emu_op_para.num_pcs_bms[0]=%d  g_emu_op_para.num_pcs_bms[1]=%d \n", g_emu_op_para.num_pcs_bms[0], g_emu_op_para.num_pcs_bms[1]);
			// setting_ov_status(bmsid, pcsid_bms, bmsdata_cur[bmsid][pcsid_bms].single_mx_vol, bmsdata_cur[bmsid][pcsid_bms].single_mi_vol, bmsdata_cur[bmsid][pcsid_bms].sys_status);
			setting_ov_status(bmsid, pcsid_bms, bmsdata_cur[bmsid][pcsid_bms].single_mx_vol, bmsdata_cur[bmsid][pcsid_bms].single_mi_vol, bmsdata_cur[bmsid][pcsid_bms].sys_status, bmsdata_cur[bmsid][pcsid_bms].mx_dpw, bmsdata_cur[bmsid][pcsid_bms].mx_cpw);
		}
		total_temp = 0;
		for (i = 0; i < pPara_Modtcp->bams_num; i++)
		{

			num_pcs[i] = countPcsNum_Bms(flag_recv_bms[i]);

			total_temp += num_pcs[i];

			printf("countPcsNum_Bms bmsid=%d pcsid_bms=%d curpcsid=%d,flag_recv_bms[i]=%x,num_pcs[i]=%d curtotal=%d\n", bmsid, pcsid_bms, i, flag_recv_bms[i], num_pcs[i], total_temp);
		}
		printf("recvfromBams total_temp=%d total_pcsnum=%d\n", total_temp, total_pcsnum);
		if (total_temp >= total_pcsnum)
		{

			unsigned int temp = 0;
			for (j = 0; j < pPara_Modtcp->bams_num; j++)
			{
				for (i = 0; i < num_pcs[j]; i++)
				{
					temp += bmsdata_cur[j][i].soc;
				}
				g_emu_op_para.num_pcs_bms[j] = num_pcs[j];
				flag_recv_bms[j] = 0;
			}

			temp /= total_temp;
			g_emu_op_para.soc_ave = temp;

			memcpy((unsigned char *)bmsdata_bak, (unsigned char *)bmsdata_cur, sizeof(BmsData_Newest) * PORTNUM_MAX * 18);
			g_emu_op_para.flag_soc_bak = 1;

			printf_pcs_soc();
		}
	}
	break;
	case _SOC_:
	{
		short soc = *(short *)pdata;
		printf("LCD模块收到BAMS传来的soc数据！pcsid_bms=%d soc=%d\n", pcsid_bms, soc);
	}
	break;
	default:
		break;
	}
	return 0;
}

void bams_Init(void)
{
	void *handle;
	char *error;
	int i;
	typedef int (*init_fun)(void *);
	typedef int (*outBmsData2Other)(unsigned char, unsigned char, void *); // 输出数据
	typedef int (*indata_fun)(unsigned char type, outBmsData2Other pfun);  // 命令处理函数指针
	printf("xxLCD模块动态调用BAMS模块！\n");
	init_fun my_func = (void *)0;
	indata_fun my_func_putin = (void *)0;
	para_bams.portnum = pPara_Modtcp->bams_num;
	for (i = 0; i < pPara_Modtcp->bams_num; i++)
	{

		para_bams.baud[i] = 9600;
		para_bams.devid[i] = 1;
		para_bams.pcs_num[i] = 6;
	}
	// 打开动态链接库
	handle = dlopen(LIB_MODBMS_PATH, RTLD_LAZY);
	if (!handle)
	{
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror();

	*(void **)(&my_func) = dlsym(handle, "bams_main");
	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	printf("1LCD模块动态调用BAMS模块！\n");
	*(void **)(&my_func_putin) = dlsym(handle, "SubscribeBamsData");
	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);

		exit(EXIT_FAILURE);
	}
	printf("2LCD模块动态调用BAMS模块！\n");
	my_func((void *)&para_bams);
	printf("LCD模块订阅BAMS数据\n");
	my_func_putin(_ALL_, recvfromBams);
	my_func_putin(_SOC_, recvfromBams);
}
