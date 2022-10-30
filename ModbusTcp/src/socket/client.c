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
//当使用Modbus/TCP时，modbus poll一般模拟客户端，modbus slave一般模拟服务端
int wait_flag[] = {0, 0, 0, 0, 0, 0};
char modbus_sockt_state[MAX_LCD_NUM];
int modbus_client_sockptr[MAX_LCD_NUM];

MyData clent_data_temp[MAX_LCD_NUM];
int g_comm_qmegid[MAX_LCD_NUM];

unsigned int g_num_frame[] = {1, 1, 1, 1, 1, 1};

void RunAccordingtoStatus(int id_thread)
{
	printf("\n\nLCD:%d lcd_state[id_thread]=%d  StateMachine...\n", id_thread, lcd_state[id_thread]);
	int ret = 1;
	switch (lcd_state[id_thread])
	{
	case LCD_RUNNING:
	{
		printf("LCD:%d  doFun03Tasks!!!!\n", id_thread);
		ret = doFun03Tasks(id_thread, &curPcsId[id_thread]);
	}
	break;
	case LCD_SET_TIME:
	{
		printf("lcd_state[%d]:%d \n", id_thread, lcd_state[id_thread]);
		printf("初始化时间...\n");
		ret = setTime(id_thread);
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
		// 0x3046	产品运行模式设置	uint16	整机	1	5	"需在启机前设置，模块运行后无法进行设置
		// 1：PQ模式（高低穿功能，需选择1）；
		// 5：VSG模式（并离网功能，需选择5）；"
		printf("LCD:%d 设置运行模式...\n", id_thread);
		ret = SetLcdFun06(id_thread, 0x3046, g_emu_op_para.OperatingMode);
	}
	break;
	case LCD_PQ_PCS_MODE:
	{
		printf("LCD:%d PCS 设置成PQ模式...\n", id_thread);
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
		unsigned short val;
		regaddr = pqpcs_pw_set[curPcsId[id_thread]];
		if (g_emu_op_para.err_num < total_pcsnum)
			val = g_emu_op_para.pq_pw_total / (total_pcsnum - g_emu_op_para.err_num);
		else
			val = 0;
		ret = SetLcdFun06(id_thread, regaddr, val);
	}
	break;
	case LCD_PQ_STP_QWVAL:
	{
		printf("LCD:%d PQ 无功参数设置 ...\n", id_thread);
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val;

		regaddr = pq_vsg_pcs_qw_set[curPcsId[id_thread]];
		if (g_emu_op_para.err_num < total_pcsnum)
			val = g_emu_op_para.pq_qw_total / (total_pcsnum - g_emu_op_para.err_num);
		else
			val = 0;
		ret = SetLcdFun06(id_thread, regaddr, val);
	}

	break;
	case LCD_PQ_STA_CURVAL:
	{
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
			val = g_emu_op_para.vsg_pw_total / (total_pcsnum - g_emu_op_para.err_num);
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
		printf("111LCD_PCS_START LCD_PCS_START curPcsId[id_thread]=%d\n", curPcsId[id_thread]);
		findCurPcsForStart(id_thread, curPcsId[id_thread]);

		printf("222LCD_PCS_START LCD_PCS_START curPcsId[id_thread]=%d\n", curPcsId[id_thread]);
		if (curPcsId[id_thread] >= pPara_Modtcp->pcsnum[id_thread])
		{
			curPcsId[id_thread] = 0;
			curTaskId[id_thread] = 0;
			lcd_state[id_thread] = LCD_RUNNING;
		}
		else
		{
			regaddr = pcs_on_off_set[curPcsId[id_thread]];
			if (lcd_state[id_thread] == LCD_PCS_START)
			{
				printf("LCD:%d 开机 ...\n", id_thread);
				val = 0xff00;
			}
			else
			{
				printf("LCD:%d 关机 ...\n", id_thread);
				val = 0x00ff;
			}

			ret = SetLcdFun06(id_thread, regaddr, val);
		}
	}
	break;
	case LCD_PCS_STOP:
	{
	}
	break;
	case LCD_PCS_START_ONE:
		if (g_emu_op_para.flag_start == 0)
			g_emu_op_para.flag_start = 1;
	case LCD_PCS_STOP_ONE:
	{
		unsigned short regaddr; // = pq_pcspw_set[curPcsId][curTaskId];
		unsigned short val;
		printf("PCS单机启动 LCD_PCS_START_ONE id_thread:%d , curPcsId[%d]:%d \n", id_thread, id_thread, curPcsId[id_thread]);

		regaddr = pcs_on_off_set[curPcsId[id_thread]];
		// regaddr = 0x3000;
		if (lcd_state[id_thread] == LCD_PCS_START_ONE)
		{
			printf("LCD:%d 开机 ...\n", id_thread);
			val = 0xff00;
		}
		else
		{
			printf("LCD:%d 关机 ...\n", id_thread);
			val = 0x00ff;
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
	default:
		break;
	}
	if (ret == 0)
		wait_flag[id_thread] = 1;
	else
		printf("注意：返回解析程序出错！！！\n");
}

void *Modbus_clientSend_thread(void *arg) // 25
{

	int id_thread = (int)arg;

	int ret_value = 0;
	msgClient pmsg;
	MyData pcsdata;
	int waittime = 0;
	unsigned short id_frame;

	printf("PCS[%d] Modbus_clientSend_thread  is Starting!\n", id_thread);
	key_t key = 0;
	g_comm_qmegid[id_thread] = os_create_msgqueue(&key, 1);

	// unsigned char code_fun[] = {0x03, 0x06, 0x10};
	// unsigned char errid_fun[] = {0x83, 0x86, 0x90};

	while (modbus_sockt_state[id_thread] == STATUS_OFF)
	{
		usleep(10000);
	}

	wait_flag[id_thread] = 0;

	// printf("modbus_sockt_state[id_thread] == STATUS_ON\n") ;
	while (modbus_sockt_state[id_thread] == STATUS_ON) //
	{
		// printf("wait_flag:%d\n", wait_flag);
		ret_value = os_rev_msgqueue(g_comm_qmegid[id_thread], &pmsg, sizeof(msgClient), 0, 100);
		if (ret_value >= 0)
		{
			waittime = 0;
			memcpy((char *)&pcsdata, pmsg.data, sizeof(MyData));

			id_frame = pcsdata.buf[0] * 256 + pcsdata.buf[1];
			if ((id_frame != 0xffff && (g_num_frame[id_thread] - 1) == id_frame) || (id_frame == 0xffff && g_num_frame[id_thread] == 1))
			{
				printf("recv form pcs!!!!!g_num_frame=%d  id_frame=%d\n", g_num_frame[id_thread], id_frame);
				int res = AnalysModbus(id_thread, pcsdata.buf, pcsdata.len);
				if (0 == res)
				{
					printf("数据解析成功！！！\n");
				}
			}
			else
				printf("111检查是否发生丢包现象！！！！！id_thread=%d g_num_frame=%d  id_frame=%d\n", id_thread, g_num_frame[id_thread], id_frame);
			wait_flag[id_thread] = 0;
			continue;
		}

		if (wait_flag[id_thread] == 1)
		{
			waittime++;
			// if (waittime == 1000)
			// {
			// 	wait_flag = 0;
			// 	waittime = 0;
			// }
			continue;
		}
		else
			RunAccordingtoStatus(id_thread);
	}
	return NULL;
}

static int recvFrame(int fd, int qid, MyData *recvbuf)
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
	if (readlen < 0)
	{
		printf("连接断开或异常\r\n");
		return -1;
	}
	else if (readlen == 0)
		return 1;
	recvbuf->len = readlen;
	myprintbuf(readlen, recvbuf->buf);
	msg.msgtype = 1;
	memcpy((char *)&msg.data, recvbuf->buf, readlen);
	sleep(1);
	if (msgsnd(qid, &msg, sizeof(msgClient), IPC_NOWAIT) != -1)
	{

		printf("succ succ succ succ !!!!!!!"); //连接主站的网络参数I
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
	int i = 0, jj = 0;
	MyData recvbuf;
	printf("PCS[%d] Modbus_clientRecv_thread is Starting!\n", id_thread);

	//	printf("network parameters  connecting to server IP=%s   port=%d\n", pPara_Modtcp->server_ip[id_thread], pPara_Modtcp->server_port[id_thread]); //
	_SERVER_SOCKET server_sock;
	server_sock.protocol = TCP;
	server_sock.port = htons(pPara_Modtcp->server_port[id_thread]);
	server_sock.addr = inet_addr(pPara_Modtcp->server_ip[id_thread]);
	server_sock.fd = -1;

	sleep(4);
loop:
	while (1)
	{
		server_sock.fd = -1;
		if (_socket_client_init(&server_sock) != 0)
		{
			sleep(10);
		}
		else
			break;
	}
	printf("连接服务器成功！！！！\n");
	pPara_Modtcp->lcdnum_real++;
	modbus_client_sockptr[id_thread] = server_sock.fd;
	modbus_sockt_state[id_thread] = STATUS_ON;
	g_flag_RecvNeed_LCD |= (1 << id_thread);
	//	init_emu_op_para(id_thread);
	// >>>>>>> db5448e3e13a7539dcb9a4a0240a049b602dcd2b

	jj = 0; //未接收到数据累计标志，大于1000清零
	i = 0;

	while (1)
	{
		fd = modbus_client_sockptr[id_thread];
		if (fd == -1)
			break;
		FD_ZERO(&maxFd);
		FD_SET(fd, &maxFd);
		tv.tv_sec = 0;
		//    tv.tv_usec = 50000;
		tv.tv_usec = 200000;
		ret = select(fd + 1, &maxFd, NULL, NULL, &tv);
		if (ret < 0)
		{

			printf("网络有问题！！！！！！！！！！！！");
			break;
		}
		else if (ret == 0)
		{
			jj++;

			if (jj > 1000)
			{
				printf("暂时没有数据传入！！！！未接收到数据次数=%d！！！！！！！！！！！！！！！！\r\n", jj);
				jj = 0;

				//				break;
			}
			continue;
		}
		else
		{

			jj = 0;

			// printf("貌似收到数据！！！！！！！！！！！！");
			if (FD_ISSET(fd, &maxFd))
			{
				ret = recvFrame(fd, g_comm_qmegid[id_thread], &recvbuf);
				if (ret == -1)
				{
					i++;

					if (i > 30)
					{
						printf("接收不成功！！！！！！！！！！！！！！！！i=%d\r\n", i);
						break;
					}
					else
						continue;
				}
				else if (ret == 1)
				{
					//                 i++;

					// if(i>30)
					// {
					// 	printf("接收数据长度为0！！！！！！！！！！！！！！！！\r\n");

					// 	i=0;

					// }
					continue;
				}
				else
				{
					i = 0;
					printf("接收成功！！！！！！！！！！！！！！！！wait_flag[id_thread]=%d modbus_sockt_state[id_thread]=%d\r\n", wait_flag[id_thread], modbus_sockt_state[id_thread]);
				}
			}
			else
			{
				printf("未知错误////////////////////////////////r/n");
				break;
			}
		}
	}
	modbus_sockt_state[id_thread] = STATUS_OFF;
	pPara_Modtcp->lcdnum_real--;
	g_flag_RecvNeed_LCD &= ~(1 << id_thread);
	printf("网络断开，重连！！！！");
	goto loop;
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
	}
	cleanYcYxData();
	// initInterface61850();
	//  bams_Init();
	printf("MODBUS THTREAD CREATE success!\n");
}
