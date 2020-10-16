// 定义代理和座位
#ifndef CONCERT_H
#define CONCERT_H

#include "ipc_util.h"
#include "transaction.h"

#include <assert.h>
#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <cstring>


#define USER_LEN 20

using std::string;
using std::map;
using std::vector;
using std::pair;

enum Stat{
    Book,
    Lock,
    Avail,
    Invaild
};
enum AddrType{
    OneRow,
    OneRowOnePos,
    OneRowServerlPos,
    OneRowRandPos,
    ServerlRow,
    ServerlPos,
    Error
};
struct Pos{
    int x;int y;
    Pos(){x=0;y=0;};
    Pos(int x,int y):x(x),y(y){}
    Pos(const Pos &rhs):x(rhs.x),y(rhs.y){}
};

// 存储座位信息
struct SeatInfo{
    int pos;
    Stat stat;
    string customer;
    SeatInfo(int pos,Stat stat,string customer):pos(pos),stat(stat),customer(customer){}
    SeatInfo():pos(0),stat(Invaild){}
};

class Seat{
    time_t op_time_stamp;
    Stat status;
    char customer[USER_LEN];
public:
    Seat(){
        status = Avail;
        op_time_stamp = time(NULL);
    }   
/* 设置座位状态 */
    void set(Stat stat){
        status = stat;
        if(stat == Book)
            op_time_stamp = time(NULL);
    }
/* 设置座位所属用户 */
    void set_customer(string customer){
        // this->customer = customer;
        if(customer.size() > USER_LEN){
            strncpy(this->customer,customer.c_str(),USER_LEN-1);
            this->customer[USER_LEN-1]='\0';
        }
        else{
            strcpy(this->customer,customer.c_str());
        }
    }
/*是否空闲*/
    bool avail(double limit, string &customer);
/*获取最后一次操作时间*/
    time_t get_time_stamp();
/*获取购票者信息*/
    string get_customer();
/* 获取状态*/
    Stat get_stat();
};

// 需要保证进程安全

class Concert{
    int agent_id;
    // 需要m*n+1个锁，第0个作为全局锁
    key_t sid;
    int n;
    int m;
    double rvt;
    Seat * seats; //一维替二维
    bool enable;
    time_t start_tick;
    // 处理延迟时间
    double reserve_time;double  ticket_time;double cancel_time;double check_time;
public:
    /* 需要传入音乐厅配置信息*/
    Concert(int n, int m, double rvt);
    /* 需要传入代理配置信息，绑定锁和座位(共享内存) */
    void initConcert(int agent,int sid, Seat *seats,double reserve_time,double  ticket_time,double cancel_time,double check_time);
    /* 以下函数返回操作是否有效 处理单个座位*/
    bool book(int i, int j, string &customer,Stat &stat);
    bool cancel(int i, int j, string &customer,Stat &stat);
    bool lock(int i, int j, string &customer,Stat &stat);
    bool check_customer(int i,int j,string &customer,Stat &stat);
    
/* 解析地址, 如果地址无效则返回false，地址有效则返回true
    允许以下几种地址格式，地址命令以;分隔
    - 一行
    - 一行中的某几个座位
    - 一行中特定几个位置
    - 多行
    - 任意个数座位 // 另外处理
    OneRow, 返回(k,0)
    OneRowOnePos,返回(i,j)
    OneRowServerlPos,返回完整列表
    OneRowRandPos,(k,m)k行m个
    ServerlRow,返回(k,m)，k行以后m列m只能正数
    ServerlPos,返回(k,0)
    Error
*/
    AddrType solveAddr(string seats, vector<Pos> &result_pos);

    /* 对应一条命令的处理,并且把结果输出 */
    /* 输出一条命令的提示信息，该信息将被专门用于显示的进程读取 */
    bool solveCmd(Transaction tx);

    SeatInfo semGetSeatStat(int i, int j, double rvt);
    void showAll();
    private:
    void update(int i, int j,double limit);
    void sendMsg(const string msg);
};


#endif // !CONCERT_H