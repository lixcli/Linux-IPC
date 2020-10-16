/* 主函数，这里主进程会根据输入文件创建agent */

#include "concert.h"
#include "ipc_util.h"
#include "transaction.h"
#include <iostream>
#include <regex>

using namespace MY_IPC;
using std::cin;
using std::cout;
using std::smatch;
using std::regex;
using std::regex_match;
/* 锁和共享内存 */
int sem_; 
int shm_;

/* 正则表达式识别合法的用户名 */
regex c_name("[A-Za-z][A-Za-z0-9]*");

int main(int argc,char **argv){

    cout << "start\n";

    // char inputFile[]="test.txt";
    char inputFile[64]="test/all_test.txt";
    if(argc >= 2)
        strcpy(inputFile,argv[1]);
    freopen(inputFile,"r",stdin);
    pid_t pid;
    int n,m,k;
    double rvt;
    scanf("%d",&n);
    scanf("%d",&m);
    scanf("%d",&k);
    scanf("%lf",&rvt);

    // 定义共享内存和锁
    sem_ = create_sem("./", 770, n*m+1, 1);
	shm_ = create_shm("./", 111, sizeof(Seat)*n*m);

    /* 定义音乐厅 */
    Concert concert(n,m,rvt);

    /* 读取数据 */
    vector<Transaction> record;
    int agent = 0;

    int reverse_time=0,ticket_time=0,cancel_time=0,check_time=0;

    cout << "start read\n";

    for(int i = 1; i <= k; i++){
        // 先获取所有关于这个进程的配置信息传入给子进程
        // fork后子进程复制这部分信息
        string oper;
        cin >> oper >> agent;
        cin >> oper >> reverse_time;
        cin >> oper >> ticket_time;
        cin >> oper >> cancel_time;
        cin >> oper >> check_time;
        /* 读取所有代理所有交易 */
        cin >> oper;

        while(oper != "end"){
            Transaction tx;
            if(oper == "show"){
                tx.tx = Show;
                tx.pos = "";
                cin >> oper;
                cin >> tx.customer;
            }
            else{
                /* 获取座位 */
                string pos;
                char c;
                cin >> c;
                bool flag = false;
                while(!flag || c != '\"'){
                    c=getchar();
                    if(c == '\"'){flag = true;break;}
                    pos.push_back(c);
                }
                pos.push_back(' ');
                tx.pos = pos;
                cin >> tx.customer;
                if(oper == "reserve") tx.tx = Reserve;
                else if(oper == "ticket")tx.tx = Ticket;
                else if(oper == "check_customer")tx.tx = Show;
                else if(oper == "cancel")tx.tx = Cancel;
                else tx.tx = Error_tx; //TODO 堆Error_tx的检测

            }
            if(regex_match(tx.customer,c_name)) //只有符合要求的用户名才会被交易
                record.push_back(tx);
            cin >> oper;
        }
        // 读取后开启新进程
        pid = fork();
        if ( pid < 0)
        {
            perror("Fork failed");
            exit(0);
        }
        else if(pid == 0){
            agent = i;
            break;
        }
        else{
            // 清空后读取下一个agent信息
            agent = 0;
            record.clear();
        }

    }

    if(agent == 0){

        // 父进程阻塞直到所有进程结束
        for(int i = 0; i < k; i++){
            wait(NULL);
        }
        // 主进程输出最后的所有信息
        Seat * seat = (Seat*) connect_shm( shm_ );
        concert.initConcert(agent,sem_,seat,reverse_time,ticket_time,cancel_time,check_time);
        concert.showAll();

		remove_sem(sem_);
		remove_shm(shm_);		
    }
    else{

        Seat * seat = (Seat*) connect_shm( shm_ );
        concert.initConcert(agent,sem_,seat,reverse_time,ticket_time,cancel_time,check_time);

        // 处理所有交易
        for(int i = 0; i < record.size(); i++){
           

            concert.solveCmd(record[i]);
        }
        cut_shm(seat);
        exit(agent);
    }


    return 0;
}