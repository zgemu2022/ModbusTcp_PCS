#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#define MAX_YCDATA_NUM   32
typedef struct
{
	int lcdid;
	int pcsid;
	unsigned short yc_data[MAX_YCDATA_NUM];
	unsigned char yc_len;
}LCD_YC_DATA;//

int SaveYcData(int id_thread,int pcsid,unsigned short *pyc,unsigned char len);
#endif