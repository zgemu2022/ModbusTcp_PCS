#include "client.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "sys.h"
#include "crc.h"
#include "modbus_tcp_main.h"
#include "modbus.h"
#include "my_socket.h"
#include <sys/socket.h>
#include <sys/msg.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "importBams.h"
#include "logicAndControl.h"
#include "output.h"
#include "mytimer.h"
// 当使用Modbus/TCP时，modbus poll一般模拟客户端，modbus slave一般模拟服务端
char modbus_sockt_state[MAX_LCD_NUM];
int modbus_client_sockptr[MAX_LCD_NUM];

MyData clent_data_temp[MAX_LCD_NUM];
int g_comm_qmegid[MAX_LCD_NUM];

unsigned int g_num_frame[] = {1, 1, 1, 1, 1, 1};
int send_heat_beat(int id_thread)
{
	printf("心跳帧lcdid=%d发送 pcsid=%d lcd_state[id_thread]=%d\n", id_thread, curPcsId[id_thread], lcd_state[id_thread]);
	static unsigned short beatnum[] = {0, 0, 0, 0, 0, 0};
	unsigned short regaddr = 0x3056;
	unsigned short val = beatnum[id_thread];
	int ret;
	modbus_sockt_timer[id_thread] = 0x7fffffff;
	ret = SetLcdFun06(id_thread, regaddr, val);
	usleep(10);
	beatnum[id_thread]++;
	if (beatnum[id_thread] >= 1000)
		beatnum[id_thread] = 0;
	return ret;
}

void *Modbus_SendMulti_thread(void *arg) // 25
{
	int id_thread = (int)arg;
	int i;
	unsigned short reg_addr;
	unsigned short val;
	while (1)
	{
		if (lcd_state[id_thread] == lcd_state_last[id_thread])
		{

			if (lcd_state[id_thread] == LCD_PCS_START_ALL && flag_SendMult[id_thread] == 1)
			{
				flag_SendMult[id_thread] = 2;
				g_lcd_need_status[id_thread] = 0;
				for (i = 0; i < pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					g_lcd_need_status[id_thread] |= (1 << i);
				}
				printf("ssssdddd11  g_lcd_need_status[id_thread]=%x", g_lcd_need_status[id_thread]);
				for (i = 0; i < pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					printf("启动 lcdid=%d  pcsid=%d Modbus_SendMulti_thread \n", id_thread, i);

					reg_addr = pcs_on_off_set[i];
					val = 0xff00;
					SetLcdFun06(id_thread, reg_addr, val);
					usleep(10);
				}

				printf("ssssdddd  g_lcd_need_status[id_thread]=%x", g_lcd_need_status[id_thread]);
			}
			else if (lcd_state[id_thread] == LCD_PCS_STOP_ALL && flag_SendMult[id_thread] == 1)
			{

				flag_SendMult[id_thread] = 2;
				g_lcd_need_status[id_thread] = 0;
				for (i = 0; i < pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					g_lcd_need_status[id_thread] |= (1 << i);
				}
				for (i = 0; i < pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					printf("停止 lcdid=%d  pcsid=%d Modbus_SendMulti_thread \n", id_thread, i);

					reg_addr = pcs_on_off_set[i];
					val = 0x00ff;
					SetLcdFun06(id_thread, reg_addr, val);
					usleep(10);
				}
			}

			else if (lcd_state[id_thread] == LCD_PQ_STP_PWVAL_ALL && flag_SendMult[id_thread] == 1)
			{
				short val1;
				if (g_emu_op_para.err_num < total_pcsnum)
					val1 = g_emu_op_para.pq_pw_total / (total_pcsnum - g_emu_op_para.err_num);
				else
					val1 = 0;
				if (val1 > pPara_Modtcp->sys_max_pw)
					val1 = pPara_Modtcp->sys_max_pw;
				if (val1 < -pPara_Modtcp->sys_max_pw)
					val1 = -pPara_Modtcp->sys_max_pw;

				flag_SendMult[id_thread] = 2;
				g_lcd_need_status[id_thread] = 0;
				for (i = 0; i < pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					g_lcd_need_status[id_thread] |= (1 << i);
				}
				for (i = 0; i < pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					printf("下发功率 lcdid=%d  pcsid=%d Modbus_SendMulti_thread \n", id_thread, i);
					reg_addr = pqpcs_pw_set[i];

					SetLcdFun06(id_thread, reg_addr, val1);
					usleep(10);
				}
			}
			else if (lcd_state[id_thread] == LCD_PF_SETTING_ALL && flag_SendMult[id_thread] == 1)
			{
				short val1;

				val1 = g_emu_op_para.power_factor;
				if (g_emu_op_para.power_factor > 1000)
					val1 = 1000;

				flag_SendMult[id_thread] = 2;
				g_lcd_need_status[id_thread] = 0;
				for (i = 0; i < pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					g_lcd_need_status[id_thread] |= (1 << i);
				}
				for (i = 0; i < pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					printf("下发功率 lcdid=%d  pcsid=%d Modbus_SendMulti_thread \n", id_thread, i);
					reg_addr = pcs_power_factor[i];

					SetLcdFun06(id_thread, reg_addr, val1);
					usleep(10);
				}
			}

			else if (lcd_state[id_thread] == LCD_PCS_STOP_YXERR && flag_SendMult[id_thread] == 1)
			{
				int i = 0;
				unsigned char temp = g_lcdyx_err_status[id_thread];
				//	g_lcd_need_status[id_thread] = g_lcdyx_err_status[id_thread];
				printf("bbbb temp:%d \n", temp);
				for (i = 0; i <= pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					if ((temp & 1 << i) > 0)
					{
						reg_addr = pcs_on_off_set[i];
						sys_debug("LCD:%d err关机 ...\n", id_thread);
						val = 0x00ff;

						SetLcdFun06(id_thread, reg_addr, val);
						usleep(10);
					}
				}
			}
			else if (lcd_state[id_thread] == LCD_PCS_STOP_BMS_ERR && flag_SendMult[id_thread] == 1)
			{
				int i = 0;
				unsigned char temp = bms_err_status[id_thread];
				//	g_lcd_need_status[id_thread] = g_lcdyx_err_status[id_thread];
				printf("bbbb temp:%d \n", temp);
				for (i = 0; i <= pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					if ((temp & 1 << i) > 0)
					{
						reg_addr = pcs_on_off_set[i];
						sys_debug("LCD:%d err关机 ...\n", id_thread);
						val = 0x00ff;

						SetLcdFun06(id_thread, reg_addr, val);
						usleep(10);
					}
				}
			}
			else if (lcd_state[id_thread] == LCD_PCS_BMAS_OV && flag_SendMult[id_thread] == 1)
			{
				int i = 0;
				unsigned char temp = bms_ov_status[id_thread];
				//	g_lcd_need_status[id_thread] = g_lcdyx_err_status[id_thread];
				printf("bbbb temp:%d \n", temp);
				for (i = 0; i <= pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					if ((temp & 1 << i) > 0)
					{
						reg_addr = pqpcs_pw_set[i];
						sys_debug("LCD:%d 设置0功率 ...\n", id_thread);
						val = 0;

						SetLcdFun06(id_thread, reg_addr, val);
						usleep(10);
					}
				}
			}
			else if (lcd_state[id_thread] == LCD_ADJUST_PCS_PW && flag_SendMult[id_thread] == 1)
			{
				printf("LCD:%d 按策略要求调节有功功率 ...\n", id_thread);
				int i = 0;
				unsigned char temp; // = g_lcdyx_err_status[id_thread];
				for (i = 0; i < pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					if (g_emu_adj_lcd.adj_pcs[id_thread].flag_adj_pw[i] == 1)
					{
						flag_adj_pw[id_thread] |= (1 << i);
					}
				}
				temp = flag_adj_pw[id_thread];
				for (i = 0; i < pPara_Modtcp->pcsnum[id_thread]; i++)
				{
					if ((temp & 1 << i) > 0)
					{

						reg_addr = pqpcs_pw_set[i];
						val = g_emu_adj_lcd.adj_pcs[id_thread].val_pw[curPcsId[id_thread]] / 10;

						if (val > pPara_Modtcp->sys_max_pw)
							val = pPara_Modtcp->sys_max_pw;
						if (val < -pPara_Modtcp->sys_max_pw)
							val = -pPara_Modtcp->sys_max_pw;

						SetLcdFun06(id_thread, reg_addr, val);
					}
				}
			}
		}
		// if (modbus_sockt_timer[id_thread] == 0) // && lcd_state[id_thread] == LCD_RUNNING)
		// {
		// 	send_heat_beat(id_thread);
		// }
		usleep(10);
	}
}

void RunAccordingtoStatus(int id_thread)
{
	// printf("\n\nLCD:%d lcd_state[id_thread]=%d  modbus_sockt_timer=%x %d\n", id_thread, lcd_state[id_thread],modbus_sockt_timer[id_thread],modbus_sockt_timer[id_thread]);
	int ret = 1;
	ret = ret;
	switch (lcd_state[id_thread])
	{

		// case LCD_PCS_YX:
		// {
		// 	printf("LCD:%d  doFun03Tasks!!!!YX\n", id_thread);
		// 	lcd_state_last[id_thread] = lcd_state[id_thread];
		// 	flag_SendMult[id_thread] = 1;
		// }
		// break;

		// case LCD_PCS_YC:
		// {
		// 	printf("LCD:%d  doFun03Tasks!!!!YC\n", id_thread);
		// 	lcd_state_last[id_thread] = lcd_state[id_thread];
		// 	flag_SendMult[id_thread] = 1;
		// }
		// break;
	case LCD_RUNNING:
	{
		printf("doFun03Tasks!!!!LCD:%d pcsid=%d\n", id_thread, curPcsId[id_thread]);
		ret = doFun03Tasks(id_thread, &curPcsId[id_thread]);
	}
	break;
	case LCD_SET_TIME:
	{
		printf("lcd_state[%d]:%d \n", id_thread, lcd_state[id_thread]);
		printf("初始化时间...\n");
		ret = setTime(id_thread);
		lcd_state[id_thread] = LCD_DO_NOTHING;
	}
	break;
	case LCD_INIT:
	{
		printf("LCD:%d 初始化...\n", id_thread);
		ret = ReadNumPCS(id_thread);
	}
	break;
	case LCD_SET_MODE:
	{
		// 0x3046	产品运行模式设置	uint16	整机	1	5	"需在启机前设置，模块运行后无法进行设置"
		// 1：PQ模式（高低穿功能，需选择1）；
		// 5：VSG模式（并离网功能，需选择5）；
		printf("LCD:%d 设置运行模式...\n", id_thread);
		ret = SetLcdFun06(id_thread, 0x3046, g_emu_op_para.OperatingMode);
	}
	break;
	case LCD_PQ_PCS_MODE:
	{
		printf("LCD:%d PCS：%d 设置成PQ模式...\n", id_thread, curPcsId[id_thread]);
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val;
		// if (curTaskId == 0)
		// {
		regaddr = pqpcs_mode_set[curPcsId[id_thread]];
		val = g_emu_op_para.pq_mode_set; //[id_thread][curPcsId[id_thread]];

		ret = SetLcdFun06(id_thread, regaddr, val);
	}
	break;
	case LCD_PQ_STP_PWVAL:
	{
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		short val;
		int flag = 0;
		regaddr = pqpcs_pw_set[curPcsId[id_thread]];
		if (g_emu_op_para.err_num < total_pcsnum)
			val = g_emu_op_para.pq_pw_total / (total_pcsnum - g_emu_op_para.err_num);
		else
			val = 0;
		if (val > pPara_Modtcp->sys_max_pw)
			val = pPara_Modtcp->sys_max_pw;
		if (val < -pPara_Modtcp->sys_max_pw)
			val = -pPara_Modtcp->sys_max_pw;

		printf("LCD:%d PCS:%d 设置成PQ 恒功率有功参数设置...val:%d\n", id_thread, curPcsId[id_thread], val);

		ret = SetLcdFun06(id_thread, regaddr, val);
	}
	break;
	case LCD_PQ_STP_PWVAL_ALL:
	{
		// 整机设置LCD下所有pcs有功功率
		if (lcd_state[id_thread] == lcd_state_last[id_thread])
			return;

		printf(" 整机（所有pcs）下发功率 LCD:%d \n", id_thread);
		lcd_state_last[id_thread] = lcd_state[id_thread];
		flag_SendMult[id_thread] = 1;

		//	ret = SetLcdPWFun10(id_thread);
	}
	break;

	case LCD_PCS_START_ALL:
	{
		if (lcd_state[id_thread] == lcd_state_last[id_thread])
			return;
		// 整机（所有pcs）启停
		printf(" 整机（所有pcs）启动 LCD:%d \n", id_thread);
		lcd_state_last[id_thread] = lcd_state[id_thread];
		flag_SendMult[id_thread] = 1;

		//	ret = StartPcsFun10(id_thread);
	}
	break;
	case LCD_PCS_STOP_ALL:
	{
		if (lcd_state[id_thread] == lcd_state_last[id_thread])
			return;
		// 整机（所有pcs）启停
		printf(" 整机（所有pcs）停止 LCD:%d \n", id_thread);
		lcd_state_last[id_thread] = lcd_state[id_thread];
		flag_SendMult[id_thread] = 1;

		//	ret = StartPcsFun10(id_thread);
	}
	break;
	case LCD_PF_SETTING_ALL:
	{
		if (lcd_state[id_thread] == lcd_state_last[id_thread])
			return;
		// 整机（所有pcs）启停
		printf(" 整机（所有pcs）功率因数下发 LCD:%d \n", id_thread);
		lcd_state_last[id_thread] = lcd_state[id_thread];
		flag_SendMult[id_thread] = 1;

		//	ret = StartPcsFun10(id_thread);
	}
	break;

	case LCD_PQ_STP_QWVAL:
	{

		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		short val;
		int flag = 0;

		regaddr = pq_vsg_pcs_qw_set[curPcsId[id_thread]];
		if (g_emu_op_para.err_num < total_pcsnum)
			val = g_emu_op_para.pq_qw_total / (total_pcsnum - g_emu_op_para.err_num);
		else
			val = 0;

		if (val > pPara_Modtcp->sys_max_pw)
			val = pPara_Modtcp->sys_max_pw;
		if (val < -pPara_Modtcp->sys_max_pw)
			val = -pPara_Modtcp->sys_max_pw;
		printf("LCD:%d PCS:%d 设置成PQ 恒功率无功参数设置...val:%d \n", id_thread, curPcsId[id_thread], val);

		printf("LCD:%d PQ 无功参数设置  PCS:%d ...val:%d\n", id_thread, curPcsId[id_thread], val);
		ret = SetLcdFun06(id_thread, regaddr, val);
	}

	break;
	case LCD_PQ_STA_CURVAL:
	{
		printf("LCD:%d PCS:%d 设置成PQ 恒流电流设置...\n", id_thread, curPcsId[id_thread]);
		unsigned short regaddr;
		unsigned short val;
		regaddr = pqpcs_cur_set[curPcsId[id_thread]];
		if (g_emu_op_para.err_num < total_pcsnum)
			val = g_emu_op_para.pq_cur_total / (total_pcsnum - g_emu_op_para.err_num);
		else
			val = 0;
		ret = SetLcdFun06(id_thread, regaddr, val);
	}
	break;

	case LCD_VSG_MODE:
	{
		printf("LCD:%d PCS 设置成VSG模式...\n", id_thread);
		unsigned short regaddr = 0x3047; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val = g_emu_op_para.vsg_mode_set;
		ret = SetLcdFun06(id_thread, regaddr, val);
	}
	break;

	case LCD_VSG_PW_VAL:
	{
		printf("LCD:%d  VSG 有功参数设置 ...\n", id_thread);
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val;

		regaddr = vsgpcs_pw_set[curPcsId[id_thread]];
		if (g_emu_op_para.err_num < total_pcsnum)
		{
			val = g_emu_op_para.vsg_pw_total / (total_pcsnum - g_emu_op_para.err_num);
			printf("LCD:%d PCSID:%d 调节有功功率 ...val=%d\n", id_thread, curPcsId[id_thread], val);
		}
		else
			val = 0;
		ret = SetLcdFun06(id_thread, regaddr, val);
	}
	break;

	case LCD_VSG_QW_VAL:
	{
		printf("LCD:%d VSG 无功参数设置 ...\n", id_thread);
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val;

		regaddr = pq_vsg_pcs_qw_set[curPcsId[id_thread]];
		if (g_emu_op_para.err_num < total_pcsnum)
			val = g_emu_op_para.vsg_qw_total / (total_pcsnum - g_emu_op_para.err_num);
		else
			val = 0;
		ret = SetLcdFun06(id_thread, regaddr, val);
	}
	break;

	case LCD_PCS_START:
	{
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val;
		if (g_emu_op_para.flag_start == 0)
			g_emu_op_para.flag_start = 1;
		printf("000 LCD_PCS_START LCD_PCS_START curPcsId[%d]=%d\n", id_thread, curPcsId[id_thread]);
		if (findCurPcsForStart(id_thread, curPcsId[id_thread]) == 1)
		{
			printf("111 LCD_PCS_START LCD_PCS_START curPcsId[%d]=%d\n", id_thread, curPcsId[id_thread]);
			regaddr = pcs_on_off_set[curPcsId[id_thread]];
			printf("LCD:%d 开机 ...regaddr=0x%x\n", id_thread, regaddr);
			val = 0xff00;
			ret = SetLcdFun06(id_thread, regaddr, val);
		}
	}
	break;
	case LCD_PCS_STOP:
	{
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val;
#ifndef ifDebug
//		findCurPcsForStop(id_thread, curPcsId[id_thread]);
#endif
		// if (curPcsId[id_thread] >= pPara_Modtcp->pcsnum[id_thread])
		// {
		// 	curPcsId[id_thread] = 0;
		// 	curTaskId[id_thread] = 0;
		// 	lcd_state[id_thread] = LCD_RUNNING;
		// }
		printf("000 LCD_PCS_STOP LCD_PCS_STOP curPcsId[%d]=%d\n", id_thread, curPcsId[id_thread]);
		if (findCurPcsForStop(id_thread, curPcsId[id_thread]) == 1)
		{
			printf("111 LCD_PCS_STOP LCD_PCS_STOP curPcsId[%d]=%d\n", id_thread, curPcsId[id_thread]);

			regaddr = pcs_on_off_set[curPcsId[id_thread]];
			printf("LCD:%d pcsid:%d = 关机 ...regaddr=0x%x\n", id_thread, curPcsId[id_thread], regaddr);
			val = 0x00ff;

			ret = SetLcdFun06(id_thread, regaddr, val);
		}
	}
	break;
	case LCD_PCS_STOP_BMS_ERR:
	{
		if (lcd_state[id_thread] == bms_err_status[id_thread])
			return;
		printf(" 停止BMS出现故障的 pcs LCD:%d \n", id_thread);
		lcd_state_last[id_thread] = lcd_state[id_thread];
		flag_SendMult[id_thread] = 1;

		// unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		// unsigned short val;
		// int i = 0;
		// unsigned char temp = bms_err_status[id_thread];

		// printf("aaaa temp:%d \n", temp);
		// for (i = curPcsId[id_thread]; i <= pPara_Modtcp->pcsnum[id_thread]; i++)
		// {
		// 	if ((temp & 1 << i) > 0)
		// 	{
		// 		regaddr = pcs_on_off_set[i];
		// 		sys_debug("LCD:%d ov关机 ...\n", id_thread);
		// 		val = 0x00ff;
		// 		curPcsId[id_thread] = i;
		// 		ret = SetLcdFun06(id_thread, regaddr, val);
		// 		break;
		// 	}
		// }

		// if (i >= pPara_Modtcp->pcsnum[id_thread])
		// {
		// 	curPcsId[id_thread] = 0;
		// 	curTaskId[id_thread] = 0;
		// 	lcd_state[id_thread] = LCD_RUNNING;
		// }
	}
	break;
	case LCD_PCS_STOP_YXERR:
	{

		if (lcd_state[id_thread] == lcd_state_last[id_thread])
			return;
		printf(" 停止出故障的所有pcs LCD:%d \n", id_thread);
		lcd_state_last[id_thread] = lcd_state[id_thread];
		flag_SendMult[id_thread] = 1;
	}
	break;

	case LCD_PCS_START_STOP_ONE:
	{
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val;
		printf("PCS单机启动 LCD_PCS_START_ONE id_thread:%d , curPcsId[%d]:%d \n", id_thread, id_thread, curPcsId[id_thread]);

		findCurPcsidForStart_Stop(id_thread);

		regaddr = pcs_on_off_set[curPcsId[id_thread]];

		if (g_emu_action_lcd.action_pcs[id_thread].flag_start_stop_pcs[curPcsId[id_thread]] == 0x55)
		{
			if (g_emu_op_para.flag_start == 0)
				g_emu_op_para.flag_start = 1;
			printf("LCD:%d 开机 ...\n", id_thread);
			val = 0xff00;
		}
		else if (g_emu_action_lcd.action_pcs[id_thread].flag_start_stop_pcs[curPcsId[id_thread]] == 0xaa)
		{
			printf("LCD:%d 关机 ...\n", id_thread);
			val = 0x00ff;
		}
		else
		{
			printf("启停数据出现错误！！！！\n");
			return;
		}
		ret = SetLcdFun06(id_thread, regaddr, val);
	}
	break;

	case LCD_PARALLEL_AWAY_EN:
	{
		printf("LCD:%d 并转离切换使能 ...\n", id_thread);
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val;

		regaddr = 0x3044;
		val = 0x00AA;
		ret = SetLcdFun06(id_thread, regaddr, val);
	}
	case LCD_PARALLEL_AWAY_DN:
	{
		printf("LCD:%d 并转离切换失能 ...\n", id_thread);
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val;

		regaddr = 0x3044;
		val = 0;
		ret = SetLcdFun06(id_thread, regaddr, val);
	}
	break;
	case LCD_AWAY_PARALLEL_EN:
	{
		printf("LCD:%d 离转并切换使能 ...\n", id_thread);
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val;

		regaddr = 0x3045;
		val = 0x00AA;
		ret = SetLcdFun06(id_thread, regaddr, val);
	}
	case LCD_AWAY_PARALLEL_DN:
	{
		printf("LCD:%d 离转并切换失能 ...\n", id_thread);
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val;

		regaddr = 0x3045;
		val = 0;
		ret = SetLcdFun06(id_thread, regaddr, val);
	}
	break;
	case LCD_PCS_BMAS_OV: // 充电电压超过阈值进入待机--下发0功率
	{

		if (lcd_state[id_thread] == lcd_state_last[id_thread])
			return;
		printf(" 充电电压超过阈值进入待机--下发0功率 LCD:%d \n", id_thread);
		lcd_state_last[id_thread] = lcd_state[id_thread];
		flag_SendMult[id_thread] = 1;
		// unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		// unsigned short val;
		// int i = 0;
		// unsigned char temp = bms_ov_status[id_thread];

		// printf("bbbb temp:%d \n", temp);
		// for (i = curPcsId[id_thread]; i <= pPara_Modtcp->pcsnum[id_thread]; i++)
		// {
		// 	if ((temp & 1 << i) > 0)
		// 	{
		// 		regaddr = pqpcs_pw_set[i];
		// 		sys_debug("LCD:%d pcsid=%d 过压待机 ...\n", id_thread, i);
		// 		val = 0;
		// 		curPcsId[id_thread] = i;
		// 		ret = SetLcdFun06(id_thread, regaddr, val);
		// 		break;
		// 	}
		// }

		// if (i >= pPara_Modtcp->pcsnum[id_thread])
		// {
		// 	curPcsId[id_thread] = 0;
		// 	curTaskId[id_thread] = 0;
		// 	lcd_state[id_thread] = LCD_RUNNING;
		// }
	}
	break;

	case LCD_ADJUST_PCS_PW: // 按策略要求调节有功功率
	{
		if (lcd_state[id_thread] == lcd_state_last[id_thread])
			return;
		printf(" 按策略要求调节有功功率 LCD:%d \n", id_thread);
		lcd_state_last[id_thread] = lcd_state[id_thread];
		flag_SendMult[id_thread] = 1;

		// unsigned short regaddr;
		// short val;
		// int flag = 0;
		// printf("LCD:%d 按策略要求调节有功功率 ...\n", id_thread);
		// if (findCurPcsidForAdjPw(id_thread) == 1)
		// {
		// 	if (g_emu_op_para.OperatingMode == PQ)
		// 		regaddr = pqpcs_pw_set[curPcsId[id_thread]];
		// 	else if (g_emu_op_para.OperatingMode == VSG)
		// 		regaddr = vsgpcs_pw_set[curPcsId[id_thread]];
		// 	val = g_emu_adj_lcd.adj_pcs[id_thread].val_pw[curPcsId[id_thread]] / 10;

		// 	if (val > pPara_Modtcp->sys_max_pw)
		// 		val = pPara_Modtcp->sys_max_pw;
		// 	if (val < -pPara_Modtcp->sys_max_pw)
		// 		val = -pPara_Modtcp->sys_max_pw;

		// 	printf("LCD:%d PCSID:%d 按策略要求调节有功功率 ...val=%d\n", id_thread, curPcsId[id_thread], val);
		// 	ret = SetLcdFun06(id_thread, regaddr, val);
		// }
	}
	break;

	case LCD_ADJUST_PCS_QW: // 按策略要求调节无功功率
	{
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		short val;
		int flag = 0;
		printf("LCD:%d 按策略要求调节无功功率 ...\n", id_thread);
		if (findCurPcsidForAdjQw(id_thread) == 1)
		{
			regaddr = pq_vsg_pcs_qw_set[curPcsId[id_thread]];
			val = g_emu_adj_lcd.adj_pcs[id_thread].val_qw[curPcsId[id_thread]] / 10;
			printf("LCD:%d PCSID:%d 按策略要求调节无功功率 ...val=%d\n", id_thread, curPcsId[id_thread], val);
			if (val > pPara_Modtcp->sys_max_pw)
				val = pPara_Modtcp->sys_max_pw;
			if (val < -pPara_Modtcp->sys_max_pw)
				val = -pPara_Modtcp->sys_max_pw;
			ret = SetLcdFun06(id_thread, regaddr, val);
		}
	}
	break;
	// case LCD_SEND_BEAT:
	// 	send_heat_beat(id_thread);
	// 	break;
	case LCD_DO_NOTHING:
		break;
	default:
		printf("注意：出现未经定义的状态！！！\n");
		break;
	}
}

void *Modbus_clientSend_thread(void *arg) // 25
{

	int id_thread = (int)arg;

	int ret_value = 0;
	msgClient pmsg;
	MyData pcsdata;
	unsigned short id_frame;

	printf("PCS[%d] Modbus_clientSend_thread  is Starting!\n", id_thread);
	key_t key = 0;
	// unsigned char code_fun[] = {0x03, 0x06, 0x10};
	// unsigned char errid_fun[] = {0x83, 0x86, 0x90};

	while (modbus_sockt_state[id_thread] == STATUS_OFF)
	{
		usleep(10000);
	}

	g_comm_qmegid[id_thread] = os_create_msgqueue(&key, 1);
	// wait_flag[id_thread] = 0;
	sleep(2);

write_loop:

	while (modbus_sockt_state[id_thread] == STATUS_ON) //
	{
		// printf("wait_flag:%d\n", wait_flag);
		// ret_value = os_rev_msgqueue(g_comm_qmegid[id_thread], &pmsg, sizeof(msgClient), 0, 10);
		ret_value = os_rev_msgqueue(g_comm_qmegid[id_thread], &pmsg, sizeof(msgClient), 0, 10);
		if (ret_value >= 0)
		{
			memcpy((char *)&pcsdata, pmsg.data, sizeof(MyData));

			//	id_frame = pcsdata.buf[0] * 256 + pcsdata.buf[1];
			//	if ((id_frame != 0xffff && (g_num_frame[id_thread] - 1) == id_frame) || (id_frame == 0xffff && g_num_frame[id_thread] == 1))
			//{
			//	printf("recv form pcs!!!!!g_num_frame=%d  id_frame=%d\n", g_num_frame[id_thread], id_frame);
			lcd_debug("111打印从LCD接收的信息 lcdid = %d\n", id_thread);
			myprintbuf_pcs(pcsdata.len, pcsdata.buf);

			int res = AnalysModbus(id_thread, pcsdata.buf, pcsdata.len, 0);
			if (0 == res)
			{
				printf("数据解析成功！！！\n");
			}
			continue;
		}
		//	RunAccordingtoStatus(id_thread);

		// 	if (modbus_sockt_timer[id_thread] == 0)
		// {
		// 	lcd_state[id_thread] = LCD_SEND_BEAT;
		// 	goto myloop;
		// }
		if (modbus_sockt_timer[id_thread] == 0 && lcd_state[id_thread] == LCD_RUNNING)
		{
			send_heat_beat(id_thread);
		}
		else
		{
			if (setStatusPw(id_thread) == 1)
				goto myloop;

			if (bms_err_status[id_thread] > 0 && lcd_state[id_thread] == LCD_RUNNING)
			{
				lcd_debug("aaaa停机ov bms_err_status[%d]:%x \n", id_thread, bms_err_status[id_thread]);
				curPcsId[id_thread] = 0;
				lcd_state[id_thread] = LCD_PCS_STOP_BMS_ERR;
			}

			else if (bms_ov_status[id_thread] > 0 && lcd_state[id_thread] == LCD_RUNNING)
			{
				lcd_debug("电压越限 bms_ov_status[%d]:%x \n", id_thread, bms_ov_status[id_thread]);
				lcd_state[id_thread] = LCD_PCS_BMAS_OV;
				curPcsId[id_thread] = 0;
			}
			// else if (g_lcdyx_err_status[id_thread] > 0 && lcd_state[id_thread] == LCD_RUNNING)//为现场pcs当前情况增加，谨慎使用
			// {
			// 	lcd_debug("bbbb故障停机err g_lcdyx_err_status[%d]:%x \n", id_thread, g_lcdyx_err_status[id_thread]);
			// 	lcd_state[id_thread] = LCD_PCS_STOP_YXERR;
			// 	curPcsId[id_thread] = 0;
			// }
			// else if (modbus_sockt_timer[id_thread] == 0) // && lcd_state[id_thread] == LCD_RUNNING)
			// {
			// 	send_heat_beat(id_thread);
			// 	continue;
			// }

		myloop:
			RunAccordingtoStatus(id_thread);
		}
	}

	while (1)
	{
		if (modbus_sockt_state[id_thread] == STATUS_ON)
		{
			goto write_loop;
		}

		else
		{
			printf(" write_loop 连接aaa\n");
			continue;
		}
		sleep(3);
	}

	return NULL;
}

static int recvFrame(int fd, int id_thread, int qid, MyData *recvbuf)
{
	int readlen;

	// int index = 0, length = 0;
	//  int i;
	msgClient msg;
	// MyData *precv = (MyData *)&msg.data;
	readlen = recv(fd, recvbuf->buf, MAX_MODBUS_FLAME, 0);
	//		readlen = recv(fd, (recvbuf.buf + recvbuf.len),
	//				(MAX_BUF_SIZE - recvbuf.len), 0);
	//		printf("*****  F:%s L:%d recv readlen=%d\n", __FUNCTION__, __LINE__,	readlen);
	// printf("*****  recv readlen=%d\n",readlen);
	if (readlen < 0)
	{

		printf("连接断开或异常\r\n");
		return -1;
	}
	else if (readlen == 0)
	{
		return 1;
	}

	recvbuf->len = readlen;
	// myprintbuf(readlen, recvbuf->buf);
	lcd_debug("000打印从LCD接收的信息 lcdid = %d\n", id_thread);
	myprintbuf_pcs(readlen, recvbuf->buf);
	msg.msgtype = 1;
	memcpy((char *)&msg.data, recvbuf->buf, readlen);
	if (msgsnd(qid, &msg, sizeof(msgClient), IPC_NOWAIT) != -1)
	{

		printf("发送到发送线程succ succ !!!!!!!"); // 连接主站的网络参数I
		return 0;
	}
	else
	{
		return 1;
	}

	// for(i=0;i<readlen;i++)
	// 	printf("0x%2x ",recvbuf->buf[i]);
	// printf("\n");
}

void *Modbus_clientRecv_thread(void *arg) // 25
{
	int id_thread = (int)arg;
	int fd = -1;
	fd_set maxFd;
	struct timeval tv;
	int ret;
	//	int i = 0; //, jj = 0;

	MyData recvbuf;
	printf("LCD[%d] Modbus_clientRecv_thread is Starting!\n", id_thread);

	//	printf("network parameters  connecting to server IP=%s   port=%d\n", pPara_Modtcp->server_ip[id_thread], pPara_Modtcp->server_port[id_thread]); //
	_SERVER_SOCKET server_sock;
	server_sock.protocol = TCP;
	server_sock.port = htons(pPara_Modtcp->server_port[id_thread]);
	server_sock.addr = inet_addr(pPara_Modtcp->server_ip[id_thread]);
	server_sock.fd = -1;

	sleep(1);
loop:
	while (1)
	{
		server_sock.fd = -1;
		ret = _socket_client_init(&server_sock);
		if (ret != 0)
		{
			// goto endconn;
			sleep(1);
			continue;
		}
		else
			break;
	}

	pPara_Modtcp->lcdnum_real++;
	modbus_client_sockptr[id_thread] = server_sock.fd;
	g_flag_RecvNeed_LCD |= (1 << id_thread);
	printf("111LCD[%d] ip=%s  port=%d g_flag_RecvNeed_LCD=%x\n", id_thread, pPara_Modtcp->server_ip[id_thread], pPara_Modtcp->server_port[id_thread], g_flag_RecvNeed_LCD);
	//	init_emu_op_para(id_thread);
	// >>>>>>> db5448e3e13a7539dcb9a4a0240a049b602dcd2b

	//	jj = 0; //未接收到数据累计标志，大于1000清零
	// while(1)
	// {
	// 	if(pPara_Modtcp->lcdnum_real==pPara_Modtcp->lcdnum_cfg)
	// 	  break;
	// 	iconn++;
	// 	if(iconn>10)
	// 		break;
	// 	sleep(1);
	// }
	while ((pPara_Modtcp->lcdnum_err + pPara_Modtcp->lcdnum_real) < pPara_Modtcp->lcdnum_cfg)
	{
		sleep(1);
	}
	modbus_sockt_state[id_thread] = STATUS_ON;
	modbus_sockt_timer[id_thread] = MX_HEART_BEAT;

	printf("lcdid=%d 连接服务器成功！！！！modbus_sockt_timer=%d\n", id_thread, modbus_sockt_timer[id_thread]);
	while (1)
	{
		fd = modbus_client_sockptr[id_thread];
		if (fd == -1)
			break;
		FD_ZERO(&maxFd);
		FD_SET(fd, &maxFd);
		tv.tv_sec = 0;
		//    tv.tv_usec = 50000;
		tv.tv_usec = 20000;
		ret = select(fd + 1, &maxFd, NULL, NULL, &tv);
		// printf("ret:%d \n",ret);
		if (ret < 0)
		{

			printf("网络有问题！！！！！！！！！！！！");
			sleep(1);
			break;
		}
		else if (ret == 0)
		{
			// jj++;

			// if (jj > 1000)
			// {
			// 	printf("暂时没有数据传入！！！！未接收到数据次数=%d！！！！！！！！！！！！！！！！\r\n", jj);
			// 	jj = 0;

			// 	//				break;
			// }
			continue;
		}
		else
		{

			// jj = 0;

			// printf("貌似收到数据！！！！！！！！！！！！");
			if (FD_ISSET(fd, &maxFd))
			{
				ret = recvFrame(fd, id_thread, g_comm_qmegid[id_thread], &recvbuf);
				if (ret == -1)
				{
					printf("客户端连接异常！！！！！！！！！！！！！！！！\r\n");
					break;
					// i++;

					// if (i > 30)
					// {
					// 	printf("接收不成功！！！！！！！！！！！！！！！！i=%d\r\n", i);
					// 	break;
					// }
					// else
					// 	continue;
				}
				else if (ret == 1)
				{
					//                 i++;

					// if(i>30)
					// {
					// printf("接收数据长度为0！！！！！！！！！！！！！！！！\r\n");

					// 	i=0;

					// }
					// printf("id_thread:%d 对方已断开连接 \n", id_thread);
					// break;
					continue;
				}
				else
				{

					printf("接收线程发送到发送线程成功！ modbus_sockt_state[%d]=%d\r\n", id_thread, modbus_sockt_state[id_thread]);
				}
			}
			else
			{
				printf("未知错误////////////////////////////////r/n");
				break;
			}
		}
	}
	close(fd);
	modbus_sockt_state[id_thread] = STATUS_OFF;
	pPara_Modtcp->lcdnum_real--;
	// g_flag_RecvNeed_LCD &= ~(1 << id_thread);
	printf("网络断开，重连！！！！");
	goto loop;
	// endconn:
	// 	printf("lcdid=%d 连接服务器失败！！！！\n", id_thread);
	// 	pPara_Modtcp->lcdnum_err++;
	// 	return NULL;
}

void CreateThreads(void)
{
	pthread_t ThreadID;
	pthread_attr_t Thread_attr;
	int i;
	printf("pPara_Modtcp 设置lcd数量:%d\n", pPara_Modtcp->lcdnum_cfg);

	for (i = 0; i < pPara_Modtcp->lcdnum_cfg; i++)
	{
		pPara_Modtcp->pcsnum[i] = 0;
		pPara_Modtcp->devNo[i] = 0xa;
		modbus_sockt_state[i] = STATUS_OFF;
		modbus_sockt_timer[i] = 0x7fffffff;

		//"send multiple packets of data".

		sleep(1);
		if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)Modbus_clientRecv_thread, (int *)i, 1, 1))
		{
			printf("MODBUS CONNECT THTREAD CREATE ERR!\n");

			exit(1);
		}
		if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)Modbus_clientSend_thread, (int *)i, 1, 1))
		{
			printf("MODBUS THTREAD CREATE ERR!\n");
			exit(1);
		}

		//"send multiple packets of data".
		if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)Modbus_SendMulti_thread, (int *)i, 1, 1))
		{
			printf("MODBUS THTREAD CREATE ERR!\n");
			exit(1);
		}

		sleep(2);
	}
	cleanYcYxData();
	initEmuParaData();
	CreateTmThreads();
	// bams_Init();
	// initInterface61850();
	printf("MODBUS THTREAD CREATE success!\n");
}
