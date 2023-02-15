#include <bits/stdc++.h>
using namespace std;

class Coinbase
{
public:
    int minerID;
    int amount;

    Coinbase() {}
    Coinbase(int minerID, int amount) : minerID(minerID), amount(amount) {}
};

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
    int minerID, chainLen;
    string prevBlockID, blockID;

    Coinbase coinbase;

    vector<Transaction *> transactions;
    vector<int> balance;

    Block(string prevBlockID, string blockID, int minerID, int chainLen, int amount) : prevBlockID(prevBlockID), blockID(blockID), minerID(minerID), chainLen(chainLen), coinbase(Coinbase(minerID, amount)) {}
};

class BlockChain
{
public:
    int minerID;
    unordered_map<string, Block *> allBlocks;        // set of all valid blocks received
    unordered_set<string> pendingTxns;               // txns not in blockchain
    unordered_map<string, Transaction *> allTxnRcvd; // set of all txns received
    unordered_set<string> parentLessBlocks, invalidBlocks;

    Block *lastBlock;

    BlockChain() {}

    BlockChain(int n, int minerID) : minerID(minerID)
    {
        Block *genesis = new Block("0", "0", -1, 1, 0);
        genesis->balance = vector<int>(n, 0);
        allBlocks.insert({genesis->blockID, genesis});
        lastBlock = genesis;
    }
};

class Node
{
public:
    int id, balanceLeft;
    bool fastLink, fastCPU;

    vector<Node *> edges;
    vector<double> propDelay;

    unordered_map<string, double> blockArrivalTime;

    BlockChain *blockchain;

    Node(int id, bool fastLink, bool fastCPU, int n) : id(id), fastLink(fastLink), fastCPU(fastCPU), balanceLeft(0)
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