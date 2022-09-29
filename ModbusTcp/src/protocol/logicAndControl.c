#include "logicAndControl.h"

int total_pcsnum = 0;
int g_flag_RecvNeed = 0;
int g_flag_RecvNeed_LCD = 0;
unsigned int countRecvFlag(int num_read)
{
	unsigned int flag=0;
	int i;
	for(i=0;i<num_read;i++)
	{
		flag |= 1<<i;
	}
	return flag;
}


