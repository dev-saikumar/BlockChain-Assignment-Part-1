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

    Block *lastBlock;

    BlockChain() {}

    BlockChain(int n)
    {
        Block *genesis = new Block("0", "0", -1, 1);
        genesis->balance = vector<int>(n, 0);
        allBlocks.insert({genesis->blockID, genesis});
        lastBlock = genesis;
    }

    void updateLongestChain(Block *block)
    {
        // Case 1: Chain Extension
        if (block->prevBlockID == lastBlock->blockID)
        {
            // remove transactions from the pending set
            for (Transaction *txn : block->transactions)
            {

                if (pendingTxns.count(txn->txnID))
                    pendingTxns.erase(txn->txnID);
            }
        }
        else
        {
            // New Longest Chain : recompute pending txn set

            // set of all transactions
            for (auto &it : allTxnRcvd)
                pendingTxns.insert(it.first);

            // filter the set, by traversing the chain
            Block *temp = block;
            while (temp->blockID != "0")
            {
                for (Transaction *txn : block->transactions)
                {
                    if (pendingTxns.count(txn->txnID))
                        pendingTxns.erase(txn->txnID);
                }

                temp = allBlocks[temp->prevBlockID];
            }
        }

        lastBlock = block;
    }

    void addBlock(Block *block)
    {
        int minerID = block->minerID;

        if (!allBlocks.count(block->prevBlockID))
        {
            cout << "Parent Block Absent. Chain length: " << block->chainLen << endl;
            exit(1);
        }

        vector<int> prevBalance = vector<int>(allBlocks[block->prevBlockID]->balance);
        vector<int> nextBalance = block->balance;

        // simulating all transactions so that prevBalance becomes current block's balance
        prevBalance[minerID] += 50; // adding coinbase

        for (Transaction *txn : block->transactions)
        {
            prevBalance[txn->from_id] -= txn->amount;
            prevBalance[txn->to_id] += txn->amount;
        }

        // validating
        for (int i = 0; i < prevBalance.size(); i++)
            if (prevBalance[i] != nextBalance[i])
            {
                cout << "Invalid Block\n";
                return;
            }

        // validation successful
        allBlocks.insert({block->blockID, block});

        // update longest chain
        if (lastBlock->chainLen < block->chainLen)
            updateLongestChain(block);
        else
            cout << "FORK\n";
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

    BlockChain blockchain;

    Node(int id, bool fastLink, bool fastCPU, int n) : id(id), fastLink(fastLink), fastCPU(fastCPU)
    {
        blockchain = BlockChain(n);
    }
};

class Event
{
public:
    int type;         // 0 -> txn generated, 1 -> txn received, 2 -> block generated, 3 -> block recived
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