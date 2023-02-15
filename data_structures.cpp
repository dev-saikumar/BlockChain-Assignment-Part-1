#include <bits/stdc++.h>
using namespace std;

// Coinbase transaction
class Coinbase
{
public:
    int minerID;
    int amount;

    Coinbase() {}
    Coinbase(int minerID, int amount) : minerID(minerID), amount(amount) {}
};

// Normal peer to peer transaction
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

    Coinbase coinbase; // coinbase

    vector<Transaction *> transactions; // transactions in this block
    vector<int> balance;                // Each miner's balance after this block

    Block(string prevBlockID, string blockID, int minerID, int chainLen, int amount) : prevBlockID(prevBlockID), blockID(blockID), minerID(minerID), chainLen(chainLen), coinbase(Coinbase(minerID, amount)) {}
};

class BlockChain
{
public:
    int minerID;
    unordered_map<string, Block *> allBlocks;              // set of all valid blocks received
    unordered_set<string> pendingTxns;                     // txns not in blockchain
    unordered_map<string, Transaction *> allTxnRcvd;       // set of all txns received
    unordered_set<string> parentLessBlocks, invalidBlocks; // blocks with no parent, invalid blocks received

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
    int id, balanceLeft;        // balanceLeft =balance as per the lastBlock in the chain - transactions create
    bool isFastLink, isFastCPU; // node has fast links or fast CPU

    vector<Node *> edges;     // neighbouring peers
    vector<double> propDelay; // propagation delay to neighbouring peers

    unordered_map<string, double> blockArrivalTime;

    BlockChain *blockchain; // each node will have it own blockchain

    Node(int id, bool isFastCPU, bool isFastLink, int n) : id(id), isFastCPU(isFastCPU), isFastLink(isFastLink), balanceLeft(0), blockchain(new BlockChain(n, id)) {}
};

class Event
{
public:
    int type;         // TXN_GEN or TXN_RCV or BLK_GEN or BLK_RCV
    double timestamp; // in seconds
    Node *node;       // event for this node

    Event(int type, Node *node, double timestamp) : type(type), node(node), timestamp(timestamp) {}
};

// comparator for the event queue, decreasing order of timestamp
struct eventCompare
{
    bool operator()(Event *a, Event *b)
    {
        return a->timestamp > b->timestamp;
    }
};

// transaction related events
class TxnEvent : public Event
{
public:
    Transaction *txn;

    TxnEvent(int type, Node *node, double timestamp, Transaction *txn) : Event(type, node, timestamp), txn(txn) {}
};

// block related events
class BlockEvent : public Event
{
public:
    Block *block;

    BlockEvent(int type, Node *node, double timestamp, Block *block) : Event(type, node, timestamp), block(block) {}
};