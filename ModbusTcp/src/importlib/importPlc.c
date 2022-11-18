#include "importPlc.h"

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define LIB_PLC_PATH "/usr/lib/libplc.so"
PARA_PLC para_plc = {1, {
							"192.168.3.230",
						},
					 {
						 502,
					 }};

int recvfromPlc(void *pdata)
{
	short plcYK = *(short *)pdata;
	printf("LCD模块收到PLC传来的数据！plcYK=%d\n", plcYK);

	return 0;
}

void Plc_Init(void)
{
	typedef int (*p_initlcd)(void *);
	typedef int (*outPlcData2Other)(void *);			  //输出数据
	typedef int (*indata_fun_plc)(outPlcData2Other pfun); //命令处理函数指针
	printf("yy LCD模块动态调用PLC模块！\n");
	void *handle;
	char *error;
	printf("plc Init\n");
	p_initlcd my_func = NULL;
	indata_fun_plc my_func_putin_plc = (void *)0;

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

	printf("2LCD模块动态调用PLC模块！\n");
	*(void **)(&my_func_putin_plc) = dlsym(handle, "SubscribePlcData");

	printf("yyy LCD模块读取PLC模块！\n");
	my_func((void *)&para_plc);
	my_func_putin_plc(recvfromPlc);
}