#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#define MAX_YCDATA_NUM   32
typedef struct
{
	int lcdid;
	int pcsid;
	unsigned short yc_data[MAX_YCDATA_NUM];
	unsigned char yc_num;
}LCD_YC_DATA;//
#endif