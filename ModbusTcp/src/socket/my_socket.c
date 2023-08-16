
#include "my_socket.h"
#include <sys/socket.h> // for socket
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <string.h>

int _socket_client_connect(int sockfd, struct sockaddr *serv_addr, int timeout)
{
	unsigned long ul = 1;
	int ret = -1;
	int error = -1, len;
	struct timeval tm;
	fd_set set;
	len = sizeof(int);
	printf("lcd模块 使用客户端连接服务器  sockfd=%d", sockfd);
	if (sockfd < 0 || timeout < 0)
		return -1;
	ioctl(sockfd, FIONBIO, &ul); /*  设置为非阻塞模式*/
	printf("lcd模块 设置为非阻塞模式");
	if (connect(sockfd, /*(struct sockaddr *)&*/ serv_addr, sizeof(struct sockaddr_in)) == -1)
	{
		tm.tv_sec = timeout;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(sockfd, &set);
		printf("lcd模块 等待  等待 select"); // comport
		if (select(sockfd + 1, NULL, &set, NULL, &tm) > 0)
		{
			getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
			if (error == 0)
			{
				printf("lcd模块 连接成功！！！！！");
				ret = 0;
			}
			else
			{
				close(sockfd);
				printf("lcd模块 对方拒绝  %s:%d socket connect fail error:%s! \n", __FILE__, __LINE__, strerror(error));
				if (error != ETIMEDOUT)
				{
					sleep(timeout); /* 防止远方拒绝时直接返回, 未等待30miao就跳出了*/
				}
				ret = -3;
			}
		}
		else
		{
			close(sockfd);
			printf("%s:%d socket connect fail, select error! \n", __FILE__, __LINE__);
			ret = -4;
		}
	}
	else
	{
		close(sockfd);
		printf("lcd模块 连接套接字失败！！！！！！！！！！！\r\n");
		ret = 0;
	}
	ul = 0;
	printf("lcd模块 查看连接结果   ret=%d\r\n", ret);
	ioctl(sockfd, FIONBIO, &ul); /* 设置为阻塞模式*/
	return ret;
}
int _socket_client_init(_SERVER_SOCKET *sock)
{

	int iSock;
	int iType, ret;
	struct sockaddr_in ServerAddr;

	if (sock == NULL)
	{
		printf("lcd模块 socket 结构为空！！！！");
		return -1;
	}

	printf("lcd模块 建立Socket第一步 获取套接字");
	iType = (sock->protocol == UDP) ? SOCK_DGRAM : SOCK_STREAM;
	printf("lcd模块 建立Socket第一步 获取套接字：选择TCU或UDP  iType=%d\r\n", iType);
	/*   一、获取套接字 */
	iSock = socket(AF_INET, iType, 0);
	if (iSock < 0)
	{
		printf("%s:%d socket create fail! \n", __FILE__, __LINE__);
		return -2;
	}
	/* 二、准备通信地址 */
	printf("lcd模块 建立Socket第二步 准备通信地址");
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = sock->port;		 //监视的端口号
	ServerAddr.sin_addr.s_addr = sock->addr; //本地IP
	memset(&(ServerAddr.sin_zero), 0, sizeof(ServerAddr.sin_zero));

	ret = _socket_client_connect(iSock, (struct sockaddr *)&ServerAddr, 1); // 20秒超时
	if (ret < 0)
	{
		printf("%s:%d socket connect fail ret < 0 ! \n", __FILE__, __LINE__);
		close(iSock);
		return -3;
	}
	printf("lcd模块 套接字成功连接\r\n");
	sock->fd = iSock;
	return 0;
}