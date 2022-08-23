#include "client.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "sys.h"
#include "crc.h"
#include "my_socket.h"
#include "modbus_tcp_main.h"
//当使用Modbus/TCP时，modbus poll一般模拟客户端，modbus slave一般模拟服务端
char modbus_sockt_state[MAX_PCS_NUM];
int modbus_client_sockptr[MAX_PCS_NUM];
unsigned int modbus_sockt_timer[MAX_PCS_NUM];
MyData clent_data_temp[MAX_PCS_NUM];
int g_comm_qmegid[MAX_PCS_NUM];

PARA_MODTCP Para_Modtcp;
PARA_MODTCP *pPara_Modtcp = (PARA_MODTCP *)&Para_Modtcp;

Pcs_Fun03_Struct pcsYc[] = {
	//遥测
	{0x1103, 0x0A, 0x1D}, //模块1
	{0x1120, 0x0A, 0x1D}, //模块2
	{0x113D, 0x0A, 0x1D}, //模块3
	{0x115A, 0x0A, 0x1D}, //模块4
	{0x1193, 0x0A, 0x1D}, //模块5
	{0x11B0, 0x0A, 0x1D}, //模块6

	//遥信
	{0x1200, 0x0F, 0x0F}, //模块1
	{0x1210, 0x0F, 0x0F}, //模块2
	{0x1220, 0x0F, 0x0F}, //模块3
	{0x1230, 0x0F, 0x0F}, //模块4
	{0x1240, 0x0F, 0x0F}, //整机
	{0x1250, 0x0F, 0x0F}, //模块5
	{0x1260, 0x0F, 0x0F}, //模块6

};

int myprintbuf(int len, unsigned char *buf)
{
	int i = 0;
	printf("\nbuflen=%d\n", len);
	for (i = 0; i < len; i++)
		printf("0x%x ", buf[i]);
	printf("\n");
}

#if 0
int AnalysModbus(unsigned char *datain, unsigned int len, int id_client)
{
	PcsData pcs;
	int index = 0;
	int flag_comm_succ = 0;
	unsigned short addr;
	unsigned short crccode = 0;
	unsigned short lendata;
	pcs.errflag = 0;
	pcs.dev_id = datain[index++];
	pcs.code_fun = datain[index++];

	switch (pcs.code_fun)
	{
	case 0x90:
	case 0x83:
	case 0x86:
		pcs.errflag = datain[index++];
		crccode = crc(datain, 3);
		if (datain[index] == crccode / 256 && datain[index + 1] == crccode % 256)
		{
			pcs.code_fun -= 0x80;
			flag_comm_succ = 1;
		}
		break;
	case 0x03:
		pcs.addr = datain[index] * 256 + datain[index + 1];
		index += 2;
		lendata = datain[index] * 256 + datain[index];
		//	if(lendata+)

		break;
	default:
		break;
	}
	if (flag_comm_succ == 1)
	{
		// if(msgsnd(g_comm_qmegid, &pcs, sizeof(PcsData), IPC_NOWAIT) != -1)
		// {
		// 	printf("接收到完整modbus-tcp协议包，通过队列发送给发送任务处理！！")
		// }
	}

	// unsigned char *dataout
	return 0;
}
#endif
//数据解析
int AnalysModbus(void) // unsigned char *datain, unsigned short len, unsigned char *dataout
{
	printf("解析收到的PCS数据！！！！！");
	return 0;
}

int curTaskId = 0; //当前任务ID
int curPcsId = 0;  //当前PcsID
int wait_flag = 0;
unsigned short g_num_frame = 1;
static int createFun03Frame(int id_thread, int *taskid, int *pcsid,int *lenframe, unsigned char *framebuf)
{
	
	int numTask = ARRAY_LEN(pcsYc);
	int _taskid = *taskid;
	int _pcsid = *pcsid;
	printf("_taskid:%d\n", _taskid);
	printf("_pcsid:%d\n", _pcsid);
	Pcs_Fun03_Struct pcs = pcsYc[_taskid];

	int pos = 0;

	framebuf[pos++] = g_num_frame / 256;
	framebuf[pos++] = g_num_frame % 256;
	framebuf[pos++] = 0;
	framebuf[pos++] = 0;
	framebuf[pos++] = 0;
	framebuf[pos++] = 6;
	framebuf[pos++] = Para_Modtcp.devNo[id_thread];
	framebuf[pos++] = 3;	
	framebuf[pos++] = (pcs.RegStart+ _pcsid*29)/ 256;
	framebuf[pos++] = (pcs.RegStart+ _pcsid*29) % 256;

	framebuf[pos++] = pcs.numData / 256;
	framebuf[pos++] = pcs.numData % 256;
	// framebuf[pos++]=0;
	// framebuf[pos++]=0;
	*lenframe = pos;
	_taskid++;
	if (_taskid >= numTask)
		_taskid = 0;
	_pcsid++;
	if (_pcsid >= Para_Modtcp.pcsnum[id_thread])
		_pcsid = 0;
	g_num_frame++;
	if (g_num_frame == 0x10000)
		g_num_frame = 1;
    *taskid=_taskid;
	*pcsid=_pcsid;
	return 0;

}

static int doFun03Tasks(int id_thread, int *taskid,int *pcsid)
{
	int numfail = 0;
	unsigned char sendbuf[256];
	int lensend = 0;
	printf("fdssdf\n");

	createFun03Frame(id_thread, taskid,pcsid, &lensend, sendbuf);
	printf("fdssdf1\n");
	myprintbuf(lensend, sendbuf);
	//	={0x00,0x01,0x00,0x00,0x00,0x06,0x0A,0x03,0x11,0x00,0x00,0x02};
	

	if (send(modbus_client_sockptr[id_thread], sendbuf, lensend, 0) < 0)
	{
		numfail++;
		printf("发送失败！！！！id_thread=%d\n", id_thread);
		return 0xffff;
	}
	else
	{
		wait_flag = 1;

		printf("任务包发送成功！！！！");
	}
	return 0;

	// if(funid>=numTask)
	// {

	// }
}
void *Modbus_clientSend_thread(void *arg) // 25
{
	int id_thread = (int)arg;

	int ret_value = 0;
	msgClient pmsg;
	MyData pcsdata;
	int waittime = 0;
	int id_frame;
	printf("PCS[%d] Modbus_clientSend_thread  is Starting!\n", id_thread);

	key_t key = 0;
	g_comm_qmegid[id_thread] = os_create_msgqueue(&key, 1);


	// unsigned char code_fun[] = {0x03, 0x06, 0x10};
	// unsigned char errid_fun[] = {0x83, 0x86, 0x90};

	while (modbus_sockt_state[id_thread] == STATUS_OFF)
	{
		usleep(10000);
	}

	wait_flag = 0;

	// printf("modbus_sockt_state[id_thread] == STATUS_ON\n") ;
	while (modbus_sockt_state[id_thread] == STATUS_ON) //
	{
		ret_value = os_rev_msgqueue(g_comm_qmegid[id_thread], &pmsg, sizeof(msgClient), 0, 100);
		if (ret_value >= 0)
		{
			memcpy((char *)&pcsdata, pmsg.data, sizeof(MyData));
			id_frame = pcsdata.buf[0] * 256 + pcsdata.buf[1];

			if ((id_frame != 0xffff && (g_num_frame - 1) == id_frame) || (id_frame == 0xffff && g_num_frame == 1))
			{
				printf("recv form pcs!!!!!g_num_frame=%d  id_frame=%d\n", g_num_frame, id_frame);
				AnalysModbus();
			}
			else
				printf("检查是否发生丢包现象！！！！！g_num_frame=%d  id_frame=%d\n", g_num_frame, id_frame);
			wait_flag = 0;
			continue;
		}
		else if (wait_flag == 1)
		{
			waittime++;
			if (waittime == 1000)
			{
				wait_flag = 0;
				waittime = 0;
			}
			continue;
		}
		if (wait_flag == 0)
		{
			printf("do something!!!!\n");
			doFun03Tasks(id_thread, &curTaskId, &curPcsId);
		}
		// usleep(100);
	}
	}

	static int recvFrame(int fd, int qid, MyData *recvbuf)
	{
		int len, readlen;

		int index = 0, length = 0, offset;
		int i = 0;
		// int i;
		msgClient msg;
		MyData *precv = (MyData *)&msg.data;
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

		printf("收到一包数据 wait_flag=%d", wait_flag);
		recvbuf->len = readlen;
		myprintbuf(readlen, recvbuf->buf);
		msg.msgtype = 1;
		memcpy((char *)&msg.data, recvbuf->buf, readlen);
		sleep(1);
		wait_flag=0;
			// if (msgsnd(qid, &msg, sizeof(msgClient), IPC_NOWAIT) != -1)
			// {

			// 	printf("succ succ succ succ !!!!!!!"); //连接主站的网络参数I
			// }
			// else
			// {
			// 	return 1;
			// }

			// for(i=0;i<readlen;i++)
			// 	printf("0x%2x ",recvbuf->buf[i]);
			// printf("\n");
			return 0;
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

			printf("network parameters  connecting to server IP=%s   port=%d\n", Para_Modtcp.server_ip[id_thread], Para_Modtcp.server_port[id_thread]); //
			_SERVER_SOCKET server_sock;
			server_sock.protocol = TCP;
			server_sock.port = htons(Para_Modtcp.server_port[id_thread]);
			server_sock.addr = inet_addr(Para_Modtcp.server_ip[id_thread]);
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
			modbus_client_sockptr[id_thread] = server_sock.fd;
			modbus_sockt_state[id_thread] = STATUS_ON;
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
						printf("recvFrame返回值:%d\n", ret);
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
							printf("接收成功！！！！！！！！！！！！！！！！wait_flag=%d modbus_sockt_state[id_thread]=%d\r\n", wait_flag, modbus_sockt_state[id_thread]);
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
			printf("网络断开，重连！！！！");
			goto loop;
		}

		// void *Modbus_clientRecv_thread(void *arg) // 25
		// {
		// 	int cfd;
		// 	char *strIP = "192.168.4.24";
		// 	int port = 502;
		// 	char buf[1024];
		// 	struct sockaddr_in serv_addr;
		// 	socklen_t serv_addr_len;
		// 	memset(&serv_addr, 0, sizeof(serv_addr));
		// 	cfd = socket(AF_INET, SOCK_STREAM, 0);
		// 	serv_addr.sin_family = AF_INET;
		// 	serv_addr.sin_port = htons(port);
		// 	inet_pton(AF_INET, strIP, &serv_addr.sin_addr.s_addr);
		// 	int reval = connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
		// 	if (reval == 0)
		// 	{
		// 		printf("connect sucess\n");
		// 	}
		// }

		// 在这里插入代码 1 #include <stdio.h>
		//   2 #include <stdlib.h>
		//   3 #include <sys/socket.h>
		//   4 #include <arpa/inet.h>
		//   5 #include <string.h>
		//   6 #include <unistd.h>
		//   7
		//   8 #define SERV_PORT 6666
		//   9 #define SERV_IP  "127.0.0.1"
		//  10
		//  11 int main()
		//  12 {
		//  13         int cfd;
		//  14         int n;
		//  15         char buf[BUFSIZ];
		//  16         struct sockaddr_in serv_addr;
		//  17         socklen_t serv_addr_len;
		//  18         memset(&serv_addr,0,sizeof(serv_addr));
		//  19         cfd = socket(AF_INET,SOCK_STREAM,0);
		//  20         serv_addr.sin_family = AF_INET;
		//  21         serv_addr.sin_port = htons(SERV_PORT);
		//  22         inet_pton(AF_INET,SERV_IP,&serv_addr.sin_addr.s_addr);
		//  23         int reval = connect(cfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
		//  24         if(reval == 0)
		//  25         {
		//  26                 printf("connect sucess\n");
		//  27         }
		//  28         while(1)
		//  29         {
		//  30                 printf("enter while\n");
		//  31                 fgets(buf,sizeof(buf),stdin);//hello word----fgets--->"hello word\n\0"
		//  32                 printf(buf);
		//  33                 write(cfd,buf,strlen(buf));
		//  34                 n = read(cfd,buf,sizeof(buf));
		//  35                 printf("n = %d\n");
		//  36                 write(STDOUT_FILENO,buf,n);
		//  37                 //close(cfd);
		//  38         }
		//  39         close(cfd);
		//  40
		//  41         return 0;
		//  42 }

		void CreateThreads(void)
		{
			pthread_t ThreadID;
			pthread_attr_t Thread_attr;
			int i;
			printf("pPara_Modtcp lcd数量:%d\n", pPara_Modtcp->lcdnum);
			pPara_Modtcp->pcsnum[0] = 1;

			//>>>>>CRCTest
			// unsigned char sendMsg[] = {0x0a, 0x03, 0x11, 0x00, 0x00, 0x02};
			// unsigned short a;
			// // printf("sendMsg:%d\n",sendMsg);
			// printf("sendMsg长度:%d\n", sizeof(sendMsg));
			// a = crc16_check(sendMsg, sizeof(sendMsg));
			// printf("CRC校验为:%x\n",a);
			// exit(1);
			//<<<<<CRCTest
			

			for (i = 0; i < pPara_Modtcp->lcdnum; i++)
			{
				modbus_sockt_state[i] = STATUS_OFF;
				if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)Modbus_clientRecv_thread, (int *)i, 1, 1))
				{
					SYSERR_PRINTF("MODBUS CONNECT THTREAD CREATE ERR!\n");

					exit(1);
				}
				if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)Modbus_clientSend_thread, (int *)i, 1, 1))
				{
					printf("MODBUS THTREAD CREATE ERR!\n");
					exit(1);
				}
			}
			printf("MODBUS THTREAD CREATE success!\n");
		}
