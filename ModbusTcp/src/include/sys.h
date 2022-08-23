/*
 * sys.h
 *
 *  Created on: 2018-1-8
 *      Author: root
 */

#ifndef SYS_H_
#define SYS_H_
#include <bits/pthreadtypes.h>
#include <sys/ipc.h>
#include <stddef.h>

//#include "share_type.h"
#ifdef __cplusplus
extern "C"{
#endif
#define SUCCESS			1
#define FAIL			0
#define ERROR		    -1

#ifndef NULL
	#define NULL  (void*)0
#endif
	

typedef void* OS_TaskRet;
#define DEFAULT_PRIO        0
typedef struct
{
    int TaskID;
    unsigned char Prio;
    unsigned int *Stk;
    unsigned int Stk_Size;
}TTaskInfo;
//取数组的元素个数
#define ARRAY_LEN(x) (sizeof((x))/sizeof((x)[0]))
int os_create_msgqueue(key_t *key, unsigned char flag);
//int os_create_msgqueue(key_t key,u8 flag);
int os_rev_msgqueue(int __qid, void *__msgp, size_t __msgsz,long int __msgtyp,unsigned int timeout);
unsigned int os_create_task(OS_TaskRet(*ptask)(void *arg), void* parg, TTaskInfo* psys_arg);
unsigned char CreateSettingThread(pthread_t *threadID ,
										pthread_attr_t *attr ,
										void * (*function) ,
										void *arg,
										unsigned char BindType,//线程绑定类型：0---系统默认（非绑定）,1---绑定,2---不绑定
										unsigned char SeparateType//线程分离状态：0---系统默认（非分离）,1---分离,2---非分离
										);
#ifdef __cplusplus

}

#endif
#endif /* SYS_H_ */
