#ifndef IPC_EXCPETION_H
#define IPC_EXCPETION_H
#include <cstring>
// 定义我的IPC异常
namespace MY_IPC
{
#define MAX_MSG_LEN 50

class IPC_EXCPETION{

protected:
    char msg[MAX_MSG_LEN];
public:
    virtual char* get(){
        return msg;
    }
};

class SEM_EXCEPTION:public IPC_EXCPETION{

public:
    SEM_EXCEPTION(){
        strcpy(msg,"semaphore exception");
    }
    void set(char *msg){
        strcpy(this->msg,msg);
    }
};
class SHM_EXCEPTION:public IPC_EXCPETION{

public:
    SHM_EXCEPTION(){
        strcpy(msg,"shared memory exception");
    }
    void set(char *msg){
        strcpy(this->msg,msg);
    }
};

}


#endif