#ifndef _MODBUS_TCP_MAIN_H_
#define _MODBUS_TCP_MAIN_H_
#define MAX_PCS_NUM   6
#define _YC_  1
#define _YX_  2

#define MASTER   1
#define SLAVE    2
#define MAX_YCDATA_NUM   32
enum LIB_NAME							   // 模块名称编号
{

	LCD = 1,		 //LCD模块
    BAMS =2,
	PLC =3


};
typedef struct
{
	char type; //1 Master 2 Slave
	unsigned char lcdnum;
	unsigned char pcsnum[MAX_PCS_NUM];
	unsigned char  devNo[MAX_PCS_NUM];
	char  server_ip[MAX_PCS_NUM][64];
	unsigned short server_port[MAX_PCS_NUM];
} PARA_MODTCP; //从主控传到Modbus-Tcp模块的结构1
typedef struct
{
	int lcdid;
	int pcsid;
	unsigned short yc_data[MAX_YCDATA_NUM];
	unsigned char yc_len;
}LCD_YC_DATA;//

//回调
typedef int			(*outData2Other)(unsigned char ,unsigned char ,void *pdata);	//输出数据

typedef struct _post_list_t
{
	outData2Other  pfun;
	unsigned char type;
    struct _post_list_t *next;
}post_list_t;

#endif