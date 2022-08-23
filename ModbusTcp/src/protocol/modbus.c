#include "crc.h"
#include "modbus.h"
unsigned short PcsYxRegbas[]={0x1100,0x111d,0x113a,0x1157,0x1174,0x1190};
int readYx(unsigned char devid,  unsigned char modid,unsigned char* pframe,unsigned int len)
{
	
	unsigned short reg_start = PcsYxRegbas[modid];
	unsigned int regnum = 19;
	unsigned int lendata;
	unsigned short crccode=0;
	len=0;
    pframe[len++]=devid;
	pframe[len++]=0x03;
	pframe[len++]=reg_start/256;
	pframe[len++]=reg_start%256;
	lendata=regnum*2;
	pframe[len++]=lendata/256;
	pframe[len++]=lendata%256;
    crccode = crc(pframe,len);
    pframe[len++]=crccode/256;
	pframe[len++]=crccode%256;
	return 0;


}