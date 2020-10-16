#include <regex>
#include<sstream>
#include <string>
#include <iostream>
struct Pos{
    int x;int y;
    Pos(){x=0;y=0;};
    Pos(int x,int y):x(x),y(y){}
    Pos(Pos &rhs):x(rhs.x),y(rhs.y){}
};

char arr[6][]={"oneRow","oneRowOnePos","oneRowServerlPos","oneRowRandPos","serverRow","serverlPos"};
using namespace std;
regex oneRow("[A-Z]");
regex oneRowOnePos("[A-Z] \\d+");
regex oneRowServerlPos("[A-Z] (?:\\d+ )+\\d");
regex oneRowRandPos("[A-Z] \\d+c");
regex serverlRow("[A-Z] \\d+r");
regex serverlPos("\\d+");


AddrType solveAddr(string seats, vector<Pos> &result_pos){
    // istringstream iss(seats);

    if(regex_match(seats,serverlPos)){
        choose = ServerlPos;
        string tmp_str;
        int i = 0;

        int pos = atoi(seats.c_str());
        result_pos.push_back(Pos(pos,0));
        return ServerlPos;
    }
    if(regex_match(seats,oneRow)){
        int row = seats[0]-'A';
        result_pos.push_back(Pos(row,0));
        return OneRow;
    }
    if(regex_match(seats,oneRowOnePos)){
        string tmp_str;
        int i = 0;
        while(i < seats.length()){
            if(isdigit(seats[i])){
                tmp_str.push_back(seats[i]);
            }
            i++;
        }
        int pos = atoi(seats.c_str());
        result_pos.push_back(Pos(seats[0]-'A',pos)); 
        return OneRowOnePos;       
    }
    if(regex_match(seats,oneRowRandPos)){
        string tmp_str;
        int i = 0;
        while(i < seats.length()){
            if(isdigit(seats[i])){
                tmp_str.push_back(seats[i]);
            }
            i++;
        }
        int pos = atoi(seats.c_str());
        result_pos.push_back(Pos(seats[0]-'A',pos));  
        return OneRowRandPos;      
    }
    if(regex_match(seats,serverlRow){
        string tmp_str;
        int i = 0;
        while(i < seats.length()){
            if(isdigit(seats[i])){
                tmp_str.push_back(seats[i]);
            }
            i++;
        }
        int pos = atoi(seats.c_str());
        result_pos.push_back(Pos(seats[0]-'A',pos));
        return ServerlRow;        
    }
    if(regex_match(seats,oneRowServerlPos)){
        
        int row = seats[0]-'A';
        int i = 2; // 行以后进行计算
        string tmp_str;
        while(i < seats.length()){
            if(isdigit(seats[i])){
                tmp_str.push_back(seats[i]);
            }
            else if(seats[i]==' '){
                result_pos.push_back(Pos(seats[0]-'A',atoi(tmp_str.c_str())));
                tmp_str = "";
            }
            i++;
        }
        return OneRowServerlPos;
    }
    return Error;

}

int main(){

    string a="A 3";
    string b="B";
    string c="A 2r";
    string d="A 2c";
    string e="A 1 2 3";
    string f="3";
    string g="A";

    vector<Pos> result;

    cout << arr[solveAddr(a,result)] << endl;


    return 0;   
}