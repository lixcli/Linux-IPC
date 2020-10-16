/* 定义一个交易的队列记录，每行是每个agent对应需要做的交易 */

#ifndef TRANSACTION_H
#define TRANSACTION_H
#include <string>
#include <queue>
#include <vector>

using std::queue;
using std::string;
using std::vector;

enum TxType
{
    Reserve,
    Ticket,
    Cancel,
    Show,
    Error_tx
};

struct Transaction{
    TxType tx; //交易类型
    string pos;
    string customer;
};

typedef vector<queue<Transaction>> TxRecord;

#endif // !TRANSACTION_H
