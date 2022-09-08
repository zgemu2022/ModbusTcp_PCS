#include "modbus_tcp_main.h"
#include "modbus.h"
#include <stdio.h>
#include <unistd.h>
#include "sys.h"
#include "client.h"
#include <string.h>
#include <malloc.h>
int modbus_tcp_main(void* para)
{
  //PARA_MODTCP Para_Modtcp;
  memcpy((void*)pPara_Modtcp,para,sizeof(PARA_MODTCP));
 // PARA_MODTCP *pPara_Modtcp = (PARA_MODTCP *)para;

//  for (i = 0; i < MAX_CLIENT_CNT; i++)
// {
//   modbus_sockt_state[i] = STATUS_OFF;
//   modbus_sockt_timer[i]=30;
// }
printf("111modbus_tcp_main  server_ip=%s port=%d\n",pPara_Modtcp->server_ip[0],pPara_Modtcp->server_port[0]);
// while(1)			
//   {
//     usleep(100000);

//   }
CreateThreads();
return 0;
}


int SubscribePcsData(unsigned char type,outData2Other pfun)//订阅pcs数据
{
	printf("正在订阅pcs数据 type=%d！！！！！\n",type);
	post_list_t *note=(post_list_t*) malloc(sizeof(post_list_t));
	note->type=type;

   note->pfun=pfun;
   note->next=post_list_l;
   post_list_l=note;

	return 0;
}
