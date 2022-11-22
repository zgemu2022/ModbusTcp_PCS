#include "importPlc.h"

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "modbus_tcp_main.h"
#include "modbus.h"
#include "logicAndControl.h"
#define LIB_PLC_PATH "/usr/lib/libplc.so"
PARA_PLC para_plc = {{"192.168.4.230"}, 2502, 6, {0, 0, 0, 0, 0, 0},NULL};

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

	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	// para_plc.funOrder = orderfromPlc;
	my_func((void *)&para_plc);
}