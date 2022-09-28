#include "importBams.h"

#include <stdio.h>


#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "importBams.h"
#define LIB_MODBMS_PATH "/usr/lib/libbams_rtu.so"
PARA_BAMS para_bams={1,{9600,9600},{2,2},{6,0}};

void bams_Init(void)
{	
	void *handle;
	char *error;
	typedef int (*init_fun)(void*);
    init_fun  my_func = (void *)0;
 	//打开动态链接库
 	 handle = dlopen(LIB_MODBMS_PATH, RTLD_LAZY);
 	if (!handle) {
 		fprintf(stderr, "%s\n", dlerror());
 		exit(EXIT_FAILURE);
 	}
	dlerror();

	*(void **) (&my_func) = dlsym(handle, "bams_main");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}
    my_func((void*)&para_bams);


}
 
