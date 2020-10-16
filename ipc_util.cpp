#include "ipc_util.h"
namespace MY_IPC
{


// 所有自定义异常
SEM_EXCEPTION sem_exception;
SHM_EXCEPTION shm_exception;

int create_sem(const char * path, int id, int sem_num, int val=1)
{
	key_t ipc_key;
	int sem_id;
	union semun tmp;

	ipc_key = ftok(path, id);
	if (ipc_key == -1)
	{
        sem_exception.set("ftok fail");
		throw sem_exception;
	}

	sem_id = semget(ipc_key, sem_num, IPC_CREAT | 0666); //所有用户进程可读可写
	if (sem_id == -1)
	{
		sem_exception.set("create shared Memory error");
		throw sem_exception;
	}

    // 批量设置信号量初始值
	tmp.val = val;
	for (int i = 0; i < sem_num; i++) 
	{
		if (semctl(sem_id, i, SETVAL, tmp) == -1)
		{
			sem_exception.set("semctl set val error");
			throw sem_exception;
		}
	}

	return sem_id;
}

void sem_wait(int sid, int sn){
    struct sembuf op; 
    op.sem_num=sn; 
    op.sem_op=-1;
    op.sem_flg=0; 
    if(semop(sid, &op, 1)==-1){
        sem_exception.set("sem_wait fail");
        throw sem_exception;
    }
}

void sem_signal(int sid, int sn)
{
	struct sembuf op;
	op.sem_num = sn;
	op.sem_op = 1;
	op.sem_flg = 0;
	if (semop(sid, &op, 1) == -1)
	{
        sem_exception.set("sem_signal fail");
        throw sem_exception;
	}
}

void remove_sem(int sid)
{
	if (semctl(sid, 0, IPC_RMID) == -1)
	{
		sem_exception.set("clear sem fail");
        throw sem_exception;
	}
}

int create_shm(const char *path, int id, size_t shmsize)
{
	int shm_id;
	char *shm;
	key_t ipc_key = ftok(path, id);
	
    if (ipc_key == -1)
	{
		sem_exception.set("create ipc_key fail");
		throw sem_exception;
	}

	if ((shm_id = shmget(ipc_key, shmsize, IPC_CREAT | 0666)) == -1)//所有用户进程可读可写
	{
		sem_exception.set("Create shm Error");
		throw sem_exception;
	}

	shm = (char*)connect_shm(shm_id);

	shm[0] = '\0';

	cut_shm((const void *)shm);

	return shm_id;

}

void * connect_shm(int shm_id)
{
	void *p;
	char *q;
	p = shmat(shm_id, NULL, 0); // shmat 第二个参数设为NULL，即让系统自动选择地址映射，第三个参数是标志，一般为0
	q = (char*)p;
	if ((long)q == -1)
	{
		shm_exception.set("connet shm fail");
		throw shm_exception;
	}
	return p;
}

void * cut_shm(const void * addr)
{
	if (shmdt(addr) == -1)
	{
		shm_exception.set("cut shm relation fail");
		throw shm_exception;
	}
}

void remove_shm(int shm_id)
{
	
	if (shmctl(shm_id, IPC_RMID, NULL) == -1)
	{
		shm_exception.set("shmctl IPC_RMID Error");
		throw shm_exception;
	}
}

}// MY_IPC
