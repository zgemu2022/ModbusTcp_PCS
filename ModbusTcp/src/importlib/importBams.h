#ifndef _IMPORT_BAMS_
#define _IMPORT_BAMS_
#define PORTNUM_MAX    2
typedef struct
{
	unsigned char portnum;//接入数量
    unsigned short baud[PORTNUM_MAX]; //波特率
	unsigned char devid[PORTNUM_MAX];//设备地址
	unsigned char pcs_num[PORTNUM_MAX];


} PARA_BAMS; //

void bams_Init(void);
#endif