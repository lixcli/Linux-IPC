#include "ipc_util.h"
#include <iostream>
// #define EXCEPTION

using namespace MY_IPC;

int main(){

    // 创建信号量
    int sem_id =  create_sem("./",0,1,1);
    // 创建共享内存
    int shm_id = create_shm("./",1,sizeof(int));
    int status = -1;
    pid_t pid = fork();

    int* addr = (int*)connect_shm(shm_id);
    if(pid < 0){
        std::cout << "error\n";
    } 

#ifdef EXCEPTION
    try{
#endif

    if(pid == 0){
        std::cout << "parent" << std::endl;

        // sem_wait(sem_id,0);
        
        *addr = 1;

        std::cout << *addr << std::endl;
        // sem_signal(sem_id,0);
        // wait(&status);
        std::cout << *addr << std::endl;
        remove_sem(sem_id);
        remove_shm(shm_id);
    }
    if(pid > 0){
        // sem_wait(sem_id,0);
        std::cout << "child" << std::endl;
        std::cout << *addr << std::endl;
        *addr = 2;
        std::cout << "child" << std::endl;
        std::cout << *addr << std::endl;
        // sem_signal(sem_id,0);
    }

#ifdef EXCEPTION
    }
    catch(IPC_EXCPETION e){
        std::cout << e.get() << std::endl;
    }
#endif

    return 0;
}
