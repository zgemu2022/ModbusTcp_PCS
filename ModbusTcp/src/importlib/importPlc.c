#include "importPlc.h"

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define LIB_PLC_PATH "/usr/lib/libplc.so"
PARA_PLC para_plc = {1, {
							"192.168.3.104",
						},
					 {
						 502,
					 }};

void Plc_Init(void)
{
	typedef int (*p_initlcd)(void *);
	void *handle;
	char *error;
	printf("plc Init\n");
	p_initlcd my_func = NULL;

	handle = dlopen(LIB_PLC_PATH, RTLD_LAZY);
	if (!handle)
	{
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror();

	*(void **)(&my_func) = dlsym(handle, "plc_main");

	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	my_func((void *)&para_plc);
}
