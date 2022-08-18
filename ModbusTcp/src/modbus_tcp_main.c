#include "modbus_tcp_main.h"

#include <stdio.h>
#include <unistd.h>
#include "sys.h"
#include "client.h"
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