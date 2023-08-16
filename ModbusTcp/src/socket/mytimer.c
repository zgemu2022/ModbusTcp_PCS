
#include "mytimer.h"
#include "sys.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "logicAndControl.h"
#include "importBams.h"

int modbus_sockt_timer[MAX_LCD_NUM]; 
int bams_heartbeat_timer[2][18];
void *TimerThread(void *arg)
{
	unsigned int second;
	unsigned int min;
	unsigned int hour;
	// time_t timep;

	int i;
	// int j;
	// int grp_no;
	second = 100;
	min = 6000;
	hour = 360000;

	// timep = time((time_t*)NULL);

	while (1)
	{
		usleep(10000); // 10ms
		for (i = 0; i < MAX_LCD_NUM; i++)
		{
			if (modbus_sockt_timer[i] > 0)
			{
				modbus_sockt_timer[i]--;
			}
		}

		// for(i = 0; i < 2; i++){
		// 	for(j=0;j < 18;j++){
		// 		if (bams_heartbeat_timer[i][j] > 0)
		// 		{
		// 			bams_heartbeat_timer[i][j]--;
		// 			printf("ÐÄÌø¼ÆÊ±Æ÷  %d\n",bams_heartbeat_timer[i][j]);
		// 		}
		// 	}
		// }

		if (second > 0)
		{
			second--;

			if (second == 0)
			{
				second = 100;

				// for (i = 0; i < MX_HEART_BEAT; i++)
				// {
				// 	if (modbus_sockt_timer[i] > 0)
				// 	{
				// 		modbus_sockt_timer[i]--;
				// 	}
				// }
			}
		}

		if (min > 0)
		{
			min--;

			if (min == 0)
			{
				min = 6000; // 1m
			}
		}

		if (hour > 0)
		{
			hour--;

			if (hour == 0)
			{
				hour = 360000; // 1h
			}
		}
	}
}

void CreateTmThreads(void)
{
	pthread_t ThreadID;
	pthread_attr_t Thread_attr;

	if (FAIL == CreateSettingThread(&ThreadID, &Thread_attr, (void *)TimerThread, NULL, 1, 1))
	{
		printf("MODBUS THTREAD CREATE ERR!\n");
		exit(1);
	}
}