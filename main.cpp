#include <bits/stdc++.h>
#include "data_structures.h"
#include "utility.cpp" // declare header and include
using namespace std;

class P2P
{
private:
    int n, z0, z1;
    double slowCPU, fastCPU;
    vector<Node *> miners;
    priority_queue<Event *, vector<Event *>, eventCompare> q;

public:
    P2P(int n, int z0, int z1) : n(n), z0(z0), z1(z1)
    {
        for (int i = 0; i < n; i++)
        {
            Node *newNode = new Node(i, randomUniform(0, 99) < z0, randomUniform(0, 99) < z1, n);
            miners.push_back(newNode);
        }

        slowCPU = 1.0 / (n * ((100 - z1) / 100.0 + 10 * (z1 / 100.0)));
        fastCPU = 10 * slowCPU;
        // cout << "Slow CPU Hashing power: " << slowCPU << endl;

        generateGraph(miners, n);

        // for (auto it : miners)
        // {
        //     for (auto it2 : it->edges)
        //         cout << it2->id << " ";
        //     cout << endl;
        // }

        for (Node *node : miners)
        {
            // transaction generation
            double timestamp = randomExponential(10); // mean = 10 seconds
            TxnEvent *txnGen = new TxnEvent(0, node, timestamp, NULL);
            q.push(txnGen);

            // block generation
            timestamp = randomExponential(60.0 / (node->fastCPU ? fastCPU : slowCPU)); // mean = 60/hashing power
            BlockEvent *blockGen = new BlockEvent(2, node, timestamp, createBlock(node));
            q.push(blockGen);
        }
    }

    void start()
    {
        while (!q.empty())
        {
            Event *e = q.top();
            q.pop();

            switch (e->type)
            {
            case 0:
                handleTxnGen((TxnEvent *)e);
                break;
            case 1:
                handleTxnRcv((TxnEvent *)e);
                break;
            case 2:
                handleBlockGen((BlockEvent *)e);
                break;
            case 3:
                handleBlockRcv((BlockEvent *)e);
                break;
            default:
                cout << "Unexpected Error" << endl; // should never come here
                exit(1);
            }

            delete e;
        }

        for (Node *node : miners)
        {
            cout << node->id << " received " << node->blockchain.allTxnRcvd.size() << " transactions. Chain length: " << node->blockchain.lastBlock->chainLen << " created by " << node->blockchain.lastBlock->minerID << endl;
        }
    }

    void handleTxnGen(TxnEvent *e)
    {
        Node *node = e->node;
        // no balance or prev txn still pending
        // if (node->current->balance[node->id] == 0 || node->pendingTxns.count(node->prevTxnID))
        //     return;

        int to;
        do
        {
            to = randomUniform(0, n - 1);
        } while (node->id == to);

        Transaction *txn = new Transaction(generateUID(), node->id, to);
        // txn->amount = randomUniform(1, node->current->balance[node->id]);

        vector<Node *> &edges = miners[e->node->id]->edges;
        vector<int> &propDelay = miners[e->node->id]->propDelay;

        // flood this transaction
        for (int i = 0; i < edges.size(); i++)
        {
            Node *destNode = edges[i];
            int pDelay = propDelay[i];

            // add transmission time (m/cij) and queueing delay (dij)
            TxnEvent *newEvent = new TxnEvent(1, destNode, e->timestamp + pDelay, txn);
            q.push(newEvent);
        }

        node->prevTxnID = txn->txnID;
        node->blockchain.allTxnRcvd.insert({txn->txnID, txn});

        // TODO: new event for next transaction generation
    }

    void handleTxnRcv(TxnEvent *e)
    {
        if (e->node->blockchain.allTxnRcvd.count(e->txn->txnID)) // already received
            return;

        // validate the transaction

        Node *node = e->node;
        node->blockchain.allTxnRcvd.insert({e->txn->txnID, e->txn});

        vector<Node *> &edges = miners[e->node->id]->edges;
        vector<int> &propDelay = miners[e->node->id]->propDelay;

        for (int i = 0; i < edges.size(); i++)
        {
            Node *destNode = edges[i];
            int pDelay = propDelay[i];

            // add transmission time (m/cij) and queueing delay (dij)
            TxnEvent *newEvent = new TxnEvent(1, destNode, e->timestamp + pDelay, e->txn);
            q.push(newEvent);
        }
    }

    Block *createBlock(Node *node)
    {
        Block *newBlock = new Block(node->blockchain.lastBlock->blockID, generateUID(), node->id, node->blockchain.lastBlock->chainLen + 1);
        newBlock->transactions = vector<Transaction *>();
        newBlock->balance = vector<int>(node->blockchain.lastBlock->balance);

        // add transactions to the block
        for (string txnID : node->blockchain.pendingTxns)
        {
            if (newBlock->transactions.size() >= 1000)
                break;
            Transaction *txn = node->blockchain.allTxnRcvd.find(txnID)->second;
            newBlock->transactions.push_back(txn);

            // update balance vector
            newBlock->balance[txn->from_id] -= txn->amount;
            newBlock->balance[txn->to_id] += txn->amount;
        }

        // adding mining fees
        newBlock->balance[node->id] += 50;

        return newBlock;
    }

    void handleBlockGen(BlockEvent *e)
    {
        Node *node = e->node;
        Block *newBlock = e->block;

        // longest chain changed
        if (node->blockchain.lastBlock->blockID != newBlock->prevBlockID)
        {
            delete newBlock;
            double timestamp = randomExponential(60.0 / (node->fastCPU ? fastCPU : slowCPU));
            BlockEvent *blockGen = new BlockEvent(2, node, timestamp, createBlock(node));
            q.push(blockGen);
            return;
        }

        cout << newBlock->minerID << ". Longest Chain Intact" << endl;
        // else, add this block to blockchain and flood
        node->blockchain.addBlock(newBlock);

        vector<Node *> &edges = miners[e->node->id]->edges;
        vector<int> &propDelay = miners[e->node->id]->propDelay;

        for (int i = 0; i < edges.size(); i++)
        {
            Node *destNode = edges[i];
            int pDelay = propDelay[i];

            BlockEvent *newEvent = new BlockEvent(3, destNode, e->timestamp + pDelay, e->block);
            q.push(newEvent);
        }

        // create new block
        // double timestamp = randomExponential(60.0 / (node->fastCPU ? fastCPU : slowCPU));
        // BlockEvent *blockGen = new BlockEvent(2, node, timestamp, createBlock(node));
        // q.push(blockGen);
    }

    void handleBlockRcv(BlockEvent *e)
    {
        Node *node = e->node;
        Block *block = e->block;

        if (node->blockchain.allBlocks.count(block->blockID)) // duplicate block received
            return;

        node->blockchain.addBlock(block);

        // forward to neighbours
        vector<Node *> &edges = miners[e->node->id]->edges;
        vector<int> &propDelay = miners[e->node->id]->propDelay;

        for (int i = 0; i < edges.size(); i++)
        {
            Node *destNode = edges[i];
            int pDelay = propDelay[i];

            BlockEvent *newEvent = new BlockEvent(3, destNode, e->timestamp + pDelay, block);
            q.push(newEvent);
        }

        // cout << "Miner " << e->node->id << " received a block mined by " << e->block->minerID << endl;
    }
};

int main(int argc, char **argv)
{
    srand(time(NULL));

    if (argc < 4)
    {
        cout << "Invalid Arguments;Usage: ./main n z0 z1" << endl;
        exit(1);
    }

    int n = stoi(argv[1]), z0 = stoi(argv[2]), z1 = stoi(argv[3]);

    P2P p2pSim(n, z0, z1);

    p2pSim.start();

    return 0;
}