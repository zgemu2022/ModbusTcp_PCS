
#include "mytimer.h"
#include "sys.h"
#include <sys/time.h>
#include <unistd.h>
int modbus_sockt_timer[6];
void *TimerThread(void *arg)
{
	unsigned int second;
	unsigned int min;
	unsigned int hour;
	// time_t timep;

	int i;
	// int grp_no;

	second = 100;
	min = 6000;
	hour = 360000;

	// timep = time((time_t*)NULL);

	while (1)
	{
		usleep(10000); // 10ms

		if (second > 0)
		{
			second--;

			if (second == 0)
			{
				second = 100;

				for (i = 0; i < MX_HEART_BEAT; i++)
				{
					if (modbus_sockt_timer[i] > 0) 
					{
						modbus_sockt_timer[i]--;
					}
				}
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