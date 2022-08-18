/*
 * crc.h
 *
 *  Created on: 2022-7-29
 *      Author: root
 */

#ifndef CRC_H_
#define CRC_H_
//#define CAL_BY_BIT
#define CAL_BY_BYTE
//#define CAL_BY_HALFBYTE

unsigned short crc(unsigned char * ptr, unsigned int  len);
#endif /* CRC_H_ */
