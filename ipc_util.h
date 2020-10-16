#ifndef IPC_UTIL_H
#define IPC_UTIL_H

/* 
用到的IPC机制：
- 信号量sem:用来同步
- 共享内存shm:用来传递数据
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <stddef.h>
#include <wait.h>

#include "ipc_exception.h"

namespace MY_IPC
{

extern SEM_EXCEPTION sem_exception;
extern SHM_EXCEPTION shm_exception;

// 联合体，用于semctl初始化
union semun
{
    int              val; /*for SETVAL*/
    struct semid_ds *buf;
    unsigned short  *array;
};

/* 创建并初始化信号量 */
int create_sem(const char * path, int id, int sem_num, int val); // 多个信号量
/* 信号量p操作*/
void sem_wait(int sid, int sn);

/* 信号量v操作 */
void sem_signal(int sid,int sn);

/* 删除信号量 */
void remove_sem(int sid);

/* 创建共享内存区 */
int create_shm(const char *path, int id, size_t shmsize);

/* 删除共享内存区 */
void remove_shm(int shm_id);

/* 获取共享内存区地址 */
void * connect_shm(int shm_id);

/* 删除共享内存映射 */
void * cut_shm(const void * addr);

}



#endif