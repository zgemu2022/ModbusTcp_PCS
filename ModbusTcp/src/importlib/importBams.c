#include "importBams.h"

#include <stdio.h>

#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "importBams.h"
#define LIB_MODBMS_PATH "/usr/lib/libbams_rtu.so"
PARA_BAMS para_bams = {1, {9600, 9600}, {2, 2}, {6, 0}};
BmsData_Newest bmsdata_cur[2][18];

int recvfromBams(unsigned char pcsid, unsigned char type, void *pdata)
{

	short status_bms = 0;

	switch (type)
	{
	case _ALL_:
	{
		BmsData temp = *(BmsData *)pdata;
		unsigned char bmsid = temp.bmsid;
		printf("LCD模块收到BAMS传来的所有数据！pcsid=%d bmsid=%d %x %x\n", pcsid, temp.bmsid, temp.buf_data[0], temp.buf_data[1]);
		bmsdata_cur[bmsid][pcsid].mx_cpw = temp.buf_data[BMS_MX_CPW * 2] * 256 + temp.buf_data[BMS_MX_CPW * 2 + 1];
		bmsdata_cur[bmsid][pcsid].mx_dpw = temp.buf_data[BMS_MX_DPW * 2] * 256 + temp.buf_data[BMS_MX_DPW * 2 + 1];
		bmsdata_cur[bmsid][pcsid].main_vol = temp.buf_data[BMS_MAIN_VOLTAGE * 2] * 256 + temp.buf_data[BMS_MAIN_VOLTAGE * 2 + 1];
		bmsdata_cur[bmsid][pcsid].mx_ccur = temp.buf_data[BMS_MX_CCURRENT * 2] * 256 + temp.buf_data[BMS_MX_CCURRENT * 2 + 1];
		bmsdata_cur[bmsid][pcsid].mx_dccur = temp.buf_data[BMS_MX_DCURRENT * 2] * 256 + temp.buf_data[BMS_MX_DCURRENT * 2 + 1];
		bmsdata_cur[bmsid][pcsid].sum_cur = temp.buf_data[BMS_SUM_CURRENT * 2] * 256 + temp.buf_data[BMS_SUM_CURRENT * 2 + 1];
		bmsdata_cur[bmsid][pcsid].soc = temp.buf_data[BMS_SOC * 2] * 256 + temp.buf_data[BMS_SOC * 2 + 1];
		bmsdata_cur[bmsid][pcsid].remain_ch_cap = temp.buf_data[BMS_remaining_charging_capacity * 2] * 256 + temp.buf_data[BMS_remaining_charging_capacity * 2 + 1];
		bmsdata_cur[bmsid][pcsid].remain_dch_cap = temp.buf_data[BMS_remaining_discharging_capacity * 2] * 256 + temp.buf_data[BMS_remaining_discharging_capacity * 2 + 1];

		bmsdata_cur[bmsid][pcsid].single_mx_vol = temp.buf_data[BMS_single_MX_voltage * 2] * 256 + temp.buf_data[BMS_single_MX_voltage * 2 + 1];
		bmsdata_cur[bmsid][pcsid].single_mi_vol = temp.buf_data[BMS_single_MI_voltage * 2] * 256 + temp.buf_data[BMS_single_MI_voltage * 2 + 1];
		bmsdata_cur[bmsid][pcsid].sys_status = temp.buf_data[BMS_SYS_STATUS * 2] * 256 + temp.buf_data[BMS_SYS_STATUS * 2 + 1];
		bmsdata_cur[bmsid][pcsid].sys_need = temp.buf_data[BMS_SYS_NEED * 2] * 256 + temp.buf_data[BMS_SYS_NEED * 2 + 1];
		bmsdata_cur[bmsid][pcsid].if_sys_fault = temp.buf_data[BMS_FAULT_STATUS * 2] * 256 + temp.buf_data[BMS_FAULT_STATUS * 2 + 1];

		printf("22LCD模块收到BAMS传来的所有数据！pcsid=%d bmsid=%d %d\n", pcsid, temp.bmsid, bmsdata_cur[bmsid][pcsid].mx_dpw);
	}
	break;
	case _SOC_:
	{
		short soc = *(short *)pdata;
		printf("LCD模块收到BAMS传来的soc数据！pcsid=%d soc=%d\n", pcsid, soc);
	}
	break;
	default:
		break;
	}
	return 0;
}
void bams_Init(void)
{
	void *handle;
	char *error;
	typedef int (*init_fun)(void *);
	typedef int (*outBmsData2Other)(unsigned char, unsigned char, void *); //输出数据
	typedef int (*indata_fun)(unsigned char type, outBmsData2Other pfun);  //命令处理函数指针
	printf("xxLCD模块动态调用BAMS模块！\n");
	init_fun my_func = (void *)0;
	indata_fun my_func_putin = (void *)0;

	//打开动态链接库
	handle = dlopen(LIB_MODBMS_PATH, RTLD_LAZY);
	if (!handle)
	{
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
	}
	dlerror();

	*(void **)(&my_func) = dlsym(handle, "bams_main");
	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	printf("1LCD模块动态调用BAMS模块！\n");
	*(void **)(&my_func_putin) = dlsym(handle, "SubscribeBamsData");
	if ((error = dlerror()) != NULL)
	{
		fprintf(stderr, "%s\n", error);

		exit(EXIT_FAILURE);
	}
	printf("2LCD模块动态调用BAMS模块！\n");
	my_func((void *)&para_bams);
	sleep(1);
	printf("xxxLCD模块订阅BAMS数据\n");
	my_func_putin(_ALL_, recvfromBams);
	my_func_putin(_SOC_, recvfromBams);
}
