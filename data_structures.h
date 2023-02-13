#include <bits/stdc++.h>
using namespace std;

class Transaction
{
public:
    string txnID;
    int from_id, to_id, amount;

    Transaction(string txnID, int from_id, int to_id, int amount) : txnID(txnID), from_id(from_id), to_id(to_id), amount(amount) {}
};

class Block
{
public:
    string prevBlockID, blockID;
    int minerID, chainLen;
    vector<Transaction *> transactions;
    vector<int> balance;

    Block(string prevBlockID, string blockID, int minerID, int chainLen) : prevBlockID(prevBlockID), blockID(blockID), minerID(minerID), chainLen(chainLen) {}
};

class BlockChain
{
public:
    unordered_map<string, Block *> allBlocks;
    unordered_set<string> pendingTxns;
    unordered_map<string, Transaction *> allTxnRcvd;

    int minerID, balanceLeft;
    Block *lastBlock;

    BlockChain() {}

    BlockChain(int n, int minerID) : minerID(minerID), balanceLeft(0)
    {
        Block *genesis = new Block("0", "0", -1, 1);
        genesis->balance = vector<int>(n, 0);
        allBlocks.insert({genesis->blockID, genesis});
        lastBlock = genesis;
    }
};

class Node
{
public:
    int id;
    bool fastLink, fastCPU;
    string prevTxnID;

    vector<Node *> edges;
    vector<double> propDelay;

    BlockChain *blockchain;

    Node(int id, bool fastLink, bool fastCPU, int n) : id(id), fastLink(fastLink), fastCPU(fastCPU), blockchain()
    {
        blockchain = new BlockChain(n, id);
    }
};

class Event
{
public:
    int type;
    double timestamp; // in seconds
    Node *node;

    Event(int type, Node *node, double timestamp) : type(type), node(node), timestamp(timestamp) {}
};

struct eventCompare
{
    bool operator()(Event *a, Event *b)
    {
        return a->timestamp > b->timestamp;
    }
};

class TxnEvent : public Event
{
public:
    Transaction *txn;

    TxnEvent(int type, Node *node, double timestamp, Transaction *txn) : Event(type, node, timestamp), txn(txn) {}
};

class BlockEvent : public Event
{
public:
    Block *block;

    BlockEvent(int type, Node *node, double timestamp, Block *block) : Event(type, node, timestamp), block(block) {}
};