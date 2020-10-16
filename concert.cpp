
#include <iostream>

#include "concert.h"
#include <vector>
#include <regex>
#include <sstream>
#include <cstring>
#include <string>

#define MaxStrLen 256; //最长一次可以输出128个字符

using std::ostringstream;
using std::regex;
using std::regex_match;
using std::istringstream;
using std::smatch;
using std::regex_search;
using std::vector;
using std::string;

using std::cout;
/* 几种地址解析的正则表达式 */

regex oneRow("[A-Z] ");
regex oneRowOnePos("[A-Z] \\d+ ");
regex oneRowServerlPos("[A-Z] \\d+ +\\d ");
regex oneRowRandPos("[A-Z] \\d+c ");
regex serverlRow("[A-Z] \\d+r ");
regex serverlPos("\\d+ ");

regex number("\\d+");



using namespace MY_IPC;

char TxArr[4][10]={
    "reserve",
    "buy",
    "cancel",
    "show"
    };

char StatArr[4][64]={
    "reserved",
    "locked",
    "available",
    "invaild"
};



bool Seat::avail(double limit,string &customer){
    if(status == Avail) return true;
    if(status == Lock) return false;
    string tmp_customer = get_customer();
    double diff_s = difftime(time(NULL),op_time_stamp);
    if(status ==  Book && ( diff_s> limit*60 || tmp_customer == customer)){
        if(diff_s > limit*60){
            // 重置
            // this->customer.clear();
            set_customer("");
            set(Avail);
        }
        return true;
    } 
    else if(status == Book) return false;
    if(status == Avail) return true;
}

time_t Seat::get_time_stamp(){
    return this->op_time_stamp;
}

string Seat::get_customer(){
    string tmp = this->customer;
    return tmp;
}

Stat Seat::get_stat(){
    return this->status;
}

Concert::Concert(int n, int m, double rvt){
    this->n = n;
    this->m = m;
    this->rvt = rvt; //预约最长时间
    enable=false;
    // 设置当前时间戳tick
    this->start_tick = time(NULL);
}

void Concert::initConcert(int agent_id,int sid, Seat *seats,double reserve_time,double  ticket_time,double cancel_time,double check_time)
{
    enable=true;
    this->agent_id = agent_id;
    this->sid = sid;
    this->seats = seats;
    this->reserve_time = reserve_time;
    this->ticket_time =  ticket_time;
    this->cancel_time = cancel_time;
    this->check_time = check_time;
}
bool Concert::book(int i , int j, string & customer,Stat &stat){
    assert(enable);
    if(i >= n || j >= m || i <0 || j < 0) return false;
    bool result = false;
    sem_wait(sid,i*m+j);
    // 更新
    (seats+i*m+j)->avail(rvt,customer);
    stat = (seats+i*m+j)->get_stat();
    if(stat != Book && stat != Lock){
        (seats+i*m+j)->set(Book);
        (seats+i*m+j)->set_customer(customer);
        result = true;
    }
    sem_signal(sid,i*m+j);
    return result;
}

bool Concert::cancel(int i, int j, string &customer,Stat &stat){
    assert(enable);
    if(i >= n || j >= m || i <0 || j < 0) return false;
    bool result = false;
    sem_wait(sid,i*m+j);
    // 更新
    (seats+i*m+j)->avail(rvt,customer);
    stat = (seats+i*m+j)->get_stat();
    if(customer == (seats+i*m+j)->get_customer()&&stat == Book){
        (seats+i*m+j)->set(Avail);
        (seats+i*m+j)->set_customer("");
        result = true;
    }
    sem_signal(sid,i*m+j);
    return result;
}

bool Concert::lock(int i, int j, string &customer,Stat &stat){
    assert(enable);
    if(i >= n || j >= m || i <0 || j < 0) return false;

    sem_wait(sid,i*m+j);
    bool result = false;
    stat = (seats+i*m+j)->get_stat();

    if((seats+i*m+j)->avail(rvt,customer)){
        (seats+i*m+j)->set(Lock);
        (seats+i*m+j)->set_customer(customer);
        result = true;
    }
    sem_signal(sid,i*m+j);
    return result;
}

bool Concert::check_customer(int i,int j,string &customer,Stat &stat){
    assert(enable); 
    bool result;
    sem_wait(sid,i*m+j);
    stat = (seats+i*m+j)->get_stat();
    // 更新
    (seats+i*m+j)->avail(rvt,customer);
    result = ((seats+i*m+j)->get_customer()==customer);
    sem_signal(sid,i*m+j);
    return result;
}

void Concert::update(int i, int j,double limit){
    assert(enable); 
    sem_wait(sid,i*m+j);
    double diff_s = difftime(time(NULL),(seats+i*m+j)->get_time_stamp());
    Stat stat = (seats+i*m+j)->get_stat();
    // 更新
    if(stat ==  Book && ( diff_s> limit*60)){
        if(diff_s > limit*60){
            // 重置
            (seats+i*m+j)->set_customer("");
            (seats+i*m+j)->set(Avail);
        }
    } 
    sem_signal(sid,i*m+j);
}
// 处理消息后回显，使用write函数在linux内核版本3.14后都是原子操作
void Concert::sendMsg(const string msg){
    // 传入多一把锁作为互斥锁
    sem_wait(sid,m*n);

    printf("=================agent: %d Tick: %ld===============\n",this->agent_id,long(difftime(time(NULL),this->start_tick)));
    printf("%s",msg.c_str());

    sem_signal(sid,m*n);
    printf("=====================good bye=========================\n\n");

}

AddrType Concert::solveAddr(string seats_str, vector<Pos> &result_pos){
    // istringstream iss(seats_str);
    assert(enable);
    if(regex_match(seats_str,serverlPos)){
        string tmp_str;
        int i = 0;
        int pos = atoi(seats_str.c_str());
        result_pos.push_back(Pos(pos,0));
        return ServerlPos;
    }
    if(regex_match(seats_str,oneRow)){
        int row = seats_str[0]-'A';
        result_pos.push_back(Pos(row,0));
        return OneRow;
    }
    if(regex_match(seats_str,oneRowOnePos)){
        string tmp_str;
        int i = 0;
        while(i < seats_str.length()){
            if(isdigit(seats_str[i])){
                tmp_str.push_back(seats_str[i]);
            }
            i++;
        }
        int pos = atoi(tmp_str.c_str());
        result_pos.push_back(Pos(seats_str[0]-'A',pos-1)); 
        return OneRowOnePos;       
    }
    if(regex_match(seats_str,oneRowRandPos)){
        string tmp_str;
        int i = 2;
        while(i < seats_str.length()){
            if(isdigit(seats_str[i])){
                tmp_str.push_back(seats_str[i]);
            }
            i++;
        }
        int pos = atoi(tmp_str.c_str());
        result_pos.push_back(Pos(seats_str[0]-'A',pos));  
        return OneRowRandPos;      
    }
    if(regex_match(seats_str,serverlRow)){
        string tmp_str;
        int i = 0;
        while(i < seats_str.length()){
            if(isdigit(seats_str[i])){
                tmp_str.push_back(seats_str[i]);
            }
            i++;
        }
        int pos = atoi(tmp_str.c_str());
        result_pos.push_back(Pos(seats_str[0]-'A',pos));
        return ServerlRow;        
    }
    if(regex_match(seats_str,oneRowServerlPos)){
        
        int row = seats_str[0]-'A';
        int i = 2; // 行以后进行计算
        string tmp_str;
        int l = -1;
        int r = 0;
        while(i < seats_str.length()){
            if(isdigit(seats_str[i])){
                tmp_str.push_back(seats_str[i]);
            }
            else if(seats_str[i]==' '){
                // result_pos.push_back(Pos(seats_str[0]-'A',atoi(tmp_str.c_str())-1));
                if(l==-1) l=atoi(tmp_str.c_str())-1;
                else r=atoi(tmp_str.c_str())-1;
                tmp_str = "";
            }
            i++;
        }
        for(int i = l; i <= r; i++ ){
            result_pos.push_back(Pos(seats_str[0]-'A',i));
        }
        return OneRowServerlPos;
    }
    return Error;

}


// 
bool Concert::solveCmd(Transaction tx){
    assert(enable);
    vector<Pos> result;
    ostringstream msg;
    msg << "---------------" << tx.customer <<" " << TxArr[tx.tx] << "-------------------\n";
    msg << "request position: " << tx.pos << '\n';
    msg << "---------------result---------------\n";
    string fail_msg = "Error:\n";
    switch (tx.tx)
    {
        case Reserve:
            fail_msg = "Fail: these seats are invaild or locked or reserved";                
            break;
        case Ticket:
            fail_msg = "Fail: these seats are invaild or locked or reserved";                
            break;
        case Cancel:
            fail_msg = "Fail: these seats are not belong to you";                

            break;
        case Error_tx:
            msg << "Fail: your operation is invalid:(\n";
            return false;
            break;
    }  
    if(tx.tx == Show){
        int success_count = 0;
        msg << "reserved or buyed tickets:\n" ;
        for(int i = 0; i < n ; i++){
            for(int j = 0; j < m; j++){
                Stat stat = Invaild;
                if(check_customer(i,j,tx.customer,stat)){
                    msg << '(' <<char(i+'A') << ',' <<j+1 << ',' << StatArr[stat]<<')';
                    msg << "\n";
                    success_count++;
                }
            }
        } 
        if(success_count==0){
            msg << "NULL\n";
        }
    }


    else{
        AddrType addr_type = solveAddr(tx.pos,result);
        vector<Pos> fail_pos;
        vector<Stat> fail_stat;
        msg << "Notice: all reserved only continue " << double(rvt*60) << " seconds\n";
        if(addr_type == OneRow){
            msg << "success " << " seats:\n";

            int row = result[0].x;
            int success_count = 0;
            if(row < n && row >= 0){
                for(int i = 0; i < m; i++){
                    Stat stat=Invaild;
                    bool check_ret = false;
                    switch (tx.tx)
                    {
                    case Reserve:
                        check_ret = book(row,i,tx.customer,stat);              
                        break;
                    case Ticket:
                        check_ret = lock(row,i,tx.customer,stat); 
                        break;
                    case Cancel:
                        check_ret = cancel(row,i,tx.customer,stat);   
                        break;
                    default:
                        break;
                    } 
                    if(check_ret){
                        msg << "(" << char(row+'A') << "," << i+1 << ") " << '\n';
                        success_count++;
                    }
                    else {
                        fail_pos.push_back(Pos(row,i));  
                        fail_stat.push_back(stat); 
                    }   
                }
                if(success_count == 0){
                    msg << "NULL\n";
                }
                if(!fail_pos.empty()){
                    msg << fail_msg <<":\n";
                    for(int i = 0 ;i < fail_pos.size(); i++){
                        // update(fail_pos[i].x,fail_pos[i].y,rvt);
                        msg << "(" << char('A'+fail_pos[i].x) << "," << fail_pos[i].y+1
                            << "," << StatArr[fail_stat[0]];
                        if(fail_stat[0] == Book || fail_stat[0]==Lock) msg << " 座位被占用";
                        msg << ") " << '\n';
                    }
                }
            }
            else{
                msg << "NULL\n";
                msg << "\n";
                msg << "Error: Invaild Position\n";
            }
        }
        if(addr_type == OneRowOnePos){
            msg << "success " << " seats:\n";
            int row = result[0].x;
            int i = result[0].y;
            if(i < m && i>=0 && row<n && row >=0){
                Stat stat=Invaild;
                bool check_ret = false;
                switch (tx.tx)
                {
                    case Reserve:
                        check_ret = book(row,i,tx.customer,stat);
                        break;
                    case Ticket:
                        check_ret = lock(row,i,tx.customer,stat); 
                        break;
                    case Cancel:
                        check_ret = cancel(row,i,tx.customer,stat); 

                        break;
                    case Error_tx:
                        msg << "sorry your operation is invalid:(\n";
                        sendMsg(msg.str());
                        return false;
                        break;
                }  
                if(check_ret){
                    msg << "(" << char(row+'A') << "," << i+1 << ") ";
                }
                else {
                    fail_pos.push_back(Pos(row,i));
                    fail_stat.push_back(stat);
                }   
                // }
                msg << "\n";
                if(!fail_pos.empty()){
                    msg << fail_msg <<":\n";
                    for(int i = 0 ;i < fail_pos.size(); i++){
                        // update(fail_pos[i].x,fail_pos[i].y,rvt);
                        msg << "(" << char('A'+fail_pos[i].x) << "," << fail_pos[i].y+1
                            << "," << StatArr[fail_stat[0]];
                        if(fail_stat[0] == Book || fail_stat[0]==Lock) msg << " 座位被占用";
                        msg << ") ";
                    }
                    msg << "\n";
                }
            }
            else{
                msg << "\n";
                msg << "Error: Invaild Position\n";
            }
        }
        if(addr_type == OneRowServerlPos){
            msg << "success " << " seats:\n";
            int success_count = 0;
            for(int i = 0; i < result.size(); i++){
                Stat stat=Invaild;
                bool check_ret = false;
                int row = result[i].x;
                int col = result[i].y;
                switch (tx.tx)
                {
                case Reserve:
                    check_ret = book(row,col,tx.customer,stat);  
                    break;
                case Ticket:
                    check_ret = lock(row,col,tx.customer,stat); 
                    break;
                case Cancel:
                    check_ret = cancel(row,col,tx.customer,stat);  
                    break;
                default:
                    break;
                }  
                if(check_ret){
                    msg << "(" << char(row+'A') << "," << col+1 << ")\n";
                    success_count++;
                }
                else {
                    fail_pos.push_back(Pos(row,col));
                    fail_stat.push_back(stat);
                }                 
            }
            if(success_count == 0){
                msg << "NULL\n";
            }
            if(!fail_pos.empty()){
                msg << fail_msg <<'\n';
                for(int i = 0 ;i < fail_pos.size(); i++){
                    update(fail_pos[i].x,fail_pos[i].y,rvt);
                    msg << "(" << char('A'+fail_pos[i].x) << "," << fail_pos[i].y+1
                        << "," << StatArr[fail_stat[i]];
                    if(fail_stat[0] == Book || fail_stat[0]==Lock) msg << " 座位被占用";
                    msg << ")\n";
                }
            }
        }
        if(addr_type == OneRowRandPos){
            msg << "success "  << " seats:\n";
            int row = result[0].x;
            int count = result[0].y;
            int success_count = 0;
            for(int col = 0; col < m&&count>0; col++){
                Stat stat=Invaild;
                bool check_ret = false;

                switch (tx.tx)
                {
                case Reserve:
                    check_ret = book(row,col,tx.customer,stat);                
                    break;
                case Ticket:
                    check_ret = lock(row,col,tx.customer,stat); 
                    break;
                case Cancel:
                    check_ret = cancel(row,col,tx.customer,stat); 
                    break;
                default:
                    break;
                }  
                if(check_ret){
                    msg << "(" << char(row+'A') << "," << col+1 << ")\n";
                    count--;
                    success_count++;
                }
                else {
                    fail_pos.push_back(Pos(row,col));
                    fail_stat.push_back(stat);
                    }  
            }
            if(success_count == 0){
                msg << "NULL\n";
            }
            if(count > 0){
                msg << "Fail: Not enough seats\n";

            }
        }
        if(addr_type == ServerlRow){
            int min_row = result[0].x;
            int count = result[0].y;
            int success_count = 0;
            if(min_row+count >= n){
                msg << "Notice: over range! We will ignore the invaild part\n";
            }
            msg << "success " << " seats:\n";
            for(int row = min_row; row < n&&count>0; row++){
                for(int col = 0; col < m; col++){
                Stat stat=Invaild;
                bool check_ret = false;
                switch (tx.tx)
                {
                    case Reserve:
                        check_ret = book(row,col,tx.customer,stat);             
                        break;
                    case Ticket:
                        check_ret = lock(row,col,tx.customer,stat); 
                        break;
                    case Cancel:
                        check_ret = cancel(row,col,tx.customer,stat); 
                        break;
                    default:
                        break;
                }  
                if(check_ret){
                    msg << "(" << char(row+'A') << "," << col+1 << ")\n";
                    success_count++;
                }
                else {
                    fail_pos.push_back(Pos(row,col));
                    fail_stat.push_back(stat);
                }     
            }
            count--;
            }

            if(success_count == 0){
                msg << "NULL\n";
            }
            if(!fail_pos.empty()){
                msg << fail_msg << '\n';
                for(int i = 0 ;i < fail_pos.size(); i++){
                    update(fail_pos[i].x,fail_pos[i].y,rvt);
                    msg << "(" << char('A'+fail_pos[i].x) << "," << fail_pos[i].y+1
                        << "," << StatArr[fail_stat[i]];
                    if(fail_stat[0] == Book || fail_stat[0]==Lock) msg << " 座位被占用";
                    msg << ")\n";
                }
            }            
        }
        if(addr_type == ServerlPos){
            msg << "success " << " seats:\n";
            int count = result[0].x;
            int tmp_count = count;
            for(int row = 0; row < n&&count > 0; row++){
                for(int col = 0; col < m&&count > 0; col++){
                Stat stat=Invaild;
                bool check_ret = false;
                switch (tx.tx)
                {
                    case Reserve:
                        check_ret = book(row,col,tx.customer,stat);                  
                        break;
                    case Ticket:
                        check_ret = lock(row,col,tx.customer,stat); 
                        break;
                    case Cancel:
                        check_ret = cancel(row,col,tx.customer,stat);   
                        break;
                    default:
                        break;
                } 
                if(check_ret){
                    msg << "(" << char(row+'A') << "," << col+1 << ")\n";
                    count--; 
                }
                // else {
                //     fail_stat.push_back(stat);
                // }
            }
            }
            if(tmp_count<=count){
                msg << "NULL\n";
            }
            if(count>0){
                msg << fail_msg;
                msg << "\n";
            }             
        }
        if(addr_type == Error){
            msg << "NULL\n";
            msg << "Error: Invaild Position\n";
        }

    }

    // 睡眠
    double sleep_time = 0;
    switch (tx.tx)
    {
    case Reserve:
        sleep_time=reserve_time;
        
        break;
    
    case Cancel:
        sleep_time=cancel_time;
        break;
    
    case Ticket:
        sleep_time=ticket_time;
        break;
    
    case Show:
        sleep_time=check_time;
        break;
    default:
        break;
    }

    msg << "agent " << agent_id << " sleep for " << sleep_time << " seconds\n";
    sendMsg(msg.str());

    msg.clear();
    sleep(sleep_time);    
}

SeatInfo Concert::semGetSeatStat(int i, int j, double limit){
    assert(enable);
    SeatInfo si;
    if(i >= n || j >= m || i <0 || j < 0) return si;
    sem_wait(sid,i*m+j);
    
    double diff_s = difftime(time(NULL),(seats+i*m+j)->get_time_stamp());
    Stat stat = (seats+i*m+j)->get_stat();
    // 更新
    if(stat ==  Book && ( diff_s> limit*60)){
        if(diff_s > limit*60){
            // 重置
            (seats+i*m+j)->set_customer("");
            (seats+i*m+j)->set(Avail);
        }
    } 
    si.stat = (seats+i*m+j)->get_stat();
    si.customer = (seats+i*m+j)->get_customer();
    si.pos=i*m+j;
    sem_signal(sid,i*m+j);
    return si;    
}

void Concert::showAll(){

    vector<SeatInfo> lock_seats;
    vector<SeatInfo> reserve_seats;
    vector<SeatInfo> avail_seats;
// 搜集信息
    for(int i = 0; i < m*n;i++){
        SeatInfo si = semGetSeatStat(i/m,i%m,rvt);
        switch (si.stat)
        {
        case Avail:
            avail_seats.push_back(si);
            break;
        case Lock:
            lock_seats.push_back(si);
            break;
        case Book:
            reserve_seats.push_back(si);
            break;
        default:
            break;
        }
    }
    printf("================summary Tick: %ld===========================\n",long(difftime(time(NULL),this->start_tick)));
    printf("            rows:%d cols:%d\n\n",n,m);
    printf("----------------Locked seats:%d-------------------\n",lock_seats.size());
    for(int i = 0; i < lock_seats.size(); i++){
        SeatInfo &si = lock_seats[i];
        printf("(%-3c,%-3d,%s) ",char(si.pos/m+'A'),si.pos%m+1,si.customer.c_str());
        if( i% 3 == 0) printf("\n");
    }
    printf("\n");
    printf("----------------Reserved seats:%d----------------\n",reserve_seats.size());
    for(int i = 0; i < reserve_seats.size(); i++){
        SeatInfo &si = reserve_seats[i];
        printf("(%-3c,%-3d,%s) ",char(si.pos/m+'A'),si.pos%m+1,si.customer.c_str());

        if( i% 3 == 0) printf("\n");
    }
    printf("\n");
    printf("----------------Available seats:%d---------------\n",avail_seats.size());
    for(int i = 0; i < avail_seats.size(); i++){
        SeatInfo &si = avail_seats[i];
        printf("(%-3c,%-3d) ",char(si.pos/m+'A'),si.pos%m+1);

        if( i% 5 == 0) printf("\n");
    }
    printf("\n");

}
