#ifndef _OUTPUT_H_
#define _OUTPUT_H_

typedef struct
{
	unsigned char lcdnum;
	unsigned char pcsnum;

	unsigned short balance_rate;
} PARA_61850; //从LCD到61850模块的结构


int SaveYcData(int id_thread,int pcsid,unsigned short *pyc,unsigned char len);
int SaveYxData(int id_thread,int pcsid,unsigned short *pyx,unsigned char len);
void initInterface61850(void);
#endif