#include "importPlc.h"

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "modbus_tcp_main.h"
#include "modbus.h"
#include "logicAndControl.h"
#define LIB_PLC_PATH "/usr/local/lib/libplc.so"
PARA_PLC para_plc = {{"192.168.4.230"}, 2502, 6, {0, 0, 0, 0, 0, 0},NULL};
#if TEST_PLC_D1D2
int PLC_EMU_BOX_SwitchD1=0,PLC_EMU_BOX_SwitchD2=0;

YKOrder ykOrder_pcs_plc = NULL;
#endif

typedef int (*p_initlcd)(void *);
p_initlcd sendlcdpara_plc_func = NULL;	

static int orderfromPlc(int order)
{
	// short plcYK = *(short *)pdata;
	printf("LCD模块收到PLC传来的指令！order=%d\n", order);
	if(order==1)
        startAllPcs();
	else if(order==2)
	   stopAllPcs();
	return 0;
}

#if TEST_PLC_D1D2
int recvfromplc(unsigned char type, void *pdata){
	unsigned short temp = *(unsigned short *)pdata;
	if ((temp & (1 << PLC_EMU_BOX_SwitchD1_ON)) > 0)
		PLC_EMU_BOX_SwitchD1=1;
	else
		PLC_EMU_BOX_SwitchD1=0;

	if ((temp & (1 << PLC_EMU_BOX_SwitchD2_ON)) > 0)
		PLC_EMU_BOX_SwitchD2=1;
	else{
		PLC_EMU_BOX_SwitchD2=0;
	}	
		

	printf("PLC_EMU_BOX_Switch: D1、D2 %d %d data:%x\n",PLC_EMU_BOX_SwitchD1,PLC_EMU_BOX_SwitchD2,temp);
}
#endif


void sendtoPlc(void){
	int i;
	para_plc.lcdnum = pPara_Modtcp->lcdnum_cfg;
    para_plc.funOrder = orderfromPlc;
	for (i = 0; i < MAX_PCS_NUM; i++)
	{
		para_plc.pcsnum[i] = pPara_Modtcp->pcsnum[i];
	}
    para_plc.server_port=pconfig->plc_server_port;
	para_plc.flag_RecvNeed_LCD=g_flag_RecvNeed_LCD;
	strcpy(para_plc.server_ip,pconfig->plc_server_ip);
	sendlcdpara_plc_func((void *)&para_plc);
}

void Plc_Init(void)
{
	int i;
	typedef int (*p_initlcd)(void *);

	printf("yy LCD模块动态调用PLC模块！\n");
	void *handle;
	char *error;
	para_plc.lcdnum = pPara_Modtcp->lcdnum_cfg;
    para_plc.funOrder = orderfromPlc;
	for (i = 0; i < MAX_PCS_NUM; i++)
	{
		para_plc.pcsnum[i] = pPara_Modtcp->pcsnum[i];
	}

    para_plc.server_port=pconfig->plc_server_port;
	strcpy(para_plc.server_ip,pconfig->plc_server_ip);
	printf("plc Init\n");
	p_initlcd my_func = NULL;

	handle = dlopen(LIB_PLC_PATH, RTLD_LAZY);
	if (!handle)
	{
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror();

	printf("1LCD模块动态调用PLC模块！\n");
	*(void **)(&my_func) = dlsym(handle, "plc_main");
#if TEST_PLC_D1D2	
	*(void **)(&ykOrder_pcs_plc) = dlsym(handle, "ykOrderFromBms");
#endif
	*(void **)(&sendlcdpara_plc_func) = dlsym(handle, "recvLcdPara");

	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	// para_plc.funOrder = orderfromPlc;
	my_func((void *)&para_plc);
#if TEST_PLC_D1D2
	ykOrder_pcs_plc(_BMS_YX_, NULL, recvfromplc);
#endif
}