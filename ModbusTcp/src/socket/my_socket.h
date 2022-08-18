#ifndef _SOCKET_H_
#define _SOCKET_H_
#include<netinet/in.h>
/* 类型定义 */
//typedef int (*accept_fun)(int, struct sockaddr_in);

enum 
{
    UDP=0,
    TCP=1,
};
typedef struct
{
	char protocol;	
	int fd;
	unsigned short int port; /* Port number; port = htons(PORT) */
    in_addr_t addr; /* Internet address; addr = inet_addr("192.168.0.2"); */
}_SERVER_SOCKET;

int _socket_client_connect(int sockfd, struct sockaddr* serv_addr, int timeout);
int _socket_client_init(_SERVER_SOCKET* sock);
#endif