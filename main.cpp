#include <bits/stdc++.h>
#include "data_structures.h"
#include "utility.cpp" // declare header and include

#define TXN_SIZE 8192            // 1KB in bits
#define BLOCK_SIZE 8388608       // 1MB in bits
#define INTER_ARRIVAL_TIME 120.0 // in seconds
#define TXN_GEN_RATE 60.0        // in seconds

using namespace std;

class P2P
{
private:
    int n, z0, z1;
    double slowCPU, fastCPU;
    vector<Node *> miners;
    priority_queue<Event *, vector<Event *>, eventCompare> eventQueue;

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

        generateGraph(miners, n);

        for (Node *node : miners)
        {
            // transaction generation
            double timestamp = randomExponential(TXN_GEN_RATE);
            TxnEvent *txnGen = new TxnEvent(0, node, timestamp, NULL);
            eventQueue.push(txnGen);

            // block generation
            timestamp = randomExponential(INTER_ARRIVAL_TIME / (node->fastCPU ? fastCPU : slowCPU)); // mean = INTER_ARRIVAL_TIME/hashing power
            BlockEvent *blockGen = new BlockEvent(2, node, timestamp, createBlock(node));
            eventQueue.push(blockGen);
        }
    }

    void start()
    {
        int loop = 100000, txnGen = 0, txnRcv = 0, blkGen = 0, blkRcv = 0;
        while (!eventQueue.empty())
        {
            if (!loop)
                break;
            loop--;
            Event *e = eventQueue.top();
            eventQueue.pop();

            switch (e->type)
            {
            case 0:
                handleTxnGen((TxnEvent *)e);
                txnGen++;
                break;
            case 1:
                handleTxnRcv((TxnEvent *)e);
                txnRcv++;
                break;
            case 2:
                handleBlockGen((BlockEvent *)e);
                blkGen++;
                break;
            case 3:
                handleBlockRcv((BlockEvent *)e);
                blkRcv++;
                break;
            default:
                cout << "Unexpected Error" << endl; // should never come here
                exit(1);
            }

            delete e;
        }

        cout << txnGen << " " << txnRcv << " " << blkGen << " " << blkRcv << endl;

        for (Node *node : miners)
        {

            Block *block = node->blockchain.lastBlock;
            cout << "Miner "
                 << "(" << node->blockchain.allBlocks.size() << "): ";
            int len = 0;
            while (block->blockID != "0")
            {
                len++;
                // cout << block->minerID << "(" << block->transactions.size() << ") ";
                block = node->blockchain.allBlocks[block->prevBlockID];
            }

            cout << " Chain Length: " << len << " Balance: " << node->blockchain.lastBlock->balance[node->id] << endl;
        }
    }

    void floodTxn(Node *node, Transaction *txn, double timestamp)
    {
        vector<Node *> &edges = miners[node->id]->edges;
        vector<double> &propDelay = miners[node->id]->propDelay;

        // flood this transaction to all neighbours
        for (int i = 0; i < edges.size(); i++)
        {
            Node *destNode = edges[i];

            // cij = linkSpeed (in bits/sec), pij = pDelay(propagation delay), tDelay(transmission delay) = msg size/cij
            // queueing delay (at source node) = dij = chosen from exponential dist with mean 96kbits/cij
            double linkSpeed = (node->fastLink && destNode->fastLink ? 100 : 5) * 1000000;
            double pDelay = propDelay[i], tDelay = TXN_SIZE / linkSpeed;
            double qDelay = randomExponential(96000 / linkSpeed);

            double newTimestamp = timestamp + qDelay + tDelay + pDelay;
            TxnEvent *newEvent = new TxnEvent(1, destNode, newTimestamp, txn);
            eventQueue.push(newEvent);
        }
    }

    void floodBlock(Node *node, Block *block, double timestamp)
    {
        vector<Node *> &edges = miners[node->id]->edges;
        vector<double> &propDelay = miners[node->id]->propDelay;

        for (int i = 0; i < edges.size(); i++)
        {
            Node *destNode = edges[i];

            double linkSpeed = (node->fastLink && destNode->fastLink ? 100 : 5) * 1000000;
            double pDelay = propDelay[i], tDelay = BLOCK_SIZE / linkSpeed;
            double qDelay = randomExponential(96000 / linkSpeed);

            double newTimestamp = timestamp + qDelay + tDelay + pDelay;
            BlockEvent *newEvent = new BlockEvent(3, destNode, newTimestamp, block);
            eventQueue.push(newEvent);
        }
    }

    void handleTxnGen(TxnEvent *e)
    {
        Node *node = e->node;
        // no balance or prev txn still pending
        if (node->blockchain.lastBlock->balance[node->id] == 0 || node->blockchain.pendingTxns.count(node->prevTxnID))
        {
            TxnEvent *txnGen = new TxnEvent(0, node, e->timestamp + randomExponential(TXN_GEN_RATE), NULL);
            eventQueue.push(txnGen);
            return;
        }

        int to;
        do
        {
            to = randomUniform(0, n - 1);
        } while (node->id == to);

        int amount = randomUniform(1, node->blockchain.lastBlock->balance[node->id]);
        Transaction *txn = new Transaction(get_uuid(), node->id, to, amount);

        node->blockchain.pendingTxns.insert(txn->txnID);
        node->blockchain.allTxnRcvd.insert({txn->txnID, txn});
        floodTxn(node, txn, e->timestamp);

        node->prevTxnID = txn->txnID;

        // new event for next transaction generation
        TxnEvent *txnGen = new TxnEvent(0, node, e->timestamp + randomExponential(TXN_GEN_RATE), NULL);
        eventQueue.push(txnGen);
    }

    void handleTxnRcv(TxnEvent *e)
    {
        Transaction *txn = e->txn;
        Node *node = e->node;
        // already received, don't flood - prevents looping
        if (node->blockchain.allTxnRcvd.count(txn->txnID))
            return;

        // TODO:  validate the transaction
        if (node->blockchain.lastBlock->balance[txn->from_id] < txn->amount)
        {
            // cout << "Invalid\n";
            return;
        }

        node->blockchain.pendingTxns.insert(txn->txnID);
        node->blockchain.allTxnRcvd.insert({txn->txnID, txn});

        floodTxn(node, txn, e->timestamp);
    }

    Block *createBlock(Node *node)
    {
        Block *lastBlock = node->blockchain.lastBlock;
        Block *newBlock = new Block(lastBlock->blockID, get_uuid(), node->id, lastBlock->chainLen + 1);
        newBlock->balance = vector<int>(lastBlock->balance);

        // add transactions to the block
        for (string txnID : node->blockchain.pendingTxns)
        {
            if (newBlock->transactions.size() >= 1000)
                break;
            Transaction *txn = node->blockchain.allTxnRcvd[txnID];
            newBlock->transactions.push_back(txn);

            // update balance vector
            newBlock->balance[txn->from_id] -= txn->amount;
            newBlock->balance[txn->to_id] += txn->amount;
        }

        // adding mining fees (coinbase)
        newBlock->balance[node->id] += 50;

        return newBlock;
    }

    void handleBlockGen(BlockEvent *e)
    {
        Node *node = e->node;
        Block *newBlock = e->block;

        // longest chain changed
        if (node->blockchain.lastBlock->blockID != newBlock->prevBlockID)
            delete newBlock;
        else
        {
            // else, add this block to blockchain and flood
            node->blockchain.addBlock(newBlock);
            floodBlock(node, newBlock, e->timestamp);
        }

        // create new block
        double timestamp = e->timestamp + randomExponential(INTER_ARRIVAL_TIME / (node->fastCPU ? fastCPU : slowCPU));
        BlockEvent *blockGen = new BlockEvent(2, node, timestamp, createBlock(node));
        eventQueue.push(blockGen);
    }

    void handleBlockRcv(BlockEvent *e)
    {
        Node *node = e->node;
        Block *block = e->block;

        if (node->blockchain.allBlocks.count(block->blockID)) // duplicate block received
            return;

        node->blockchain.addBlock(block);
        floodBlock(node, block, e->timestamp);
    }
};

int main(int argc, char **argv)
{
    srand(time(NULL));

    if (argc < 4)
    {
        cout << "Invalid Arguments. Usage: ./main n z0 z1" << endl;
        exit(1);
    }

    int n = stoi(argv[1]), z0 = stoi(argv[2]), z1 = stoi(argv[3]);

    P2P p2pSim(n, z0, z1);
    p2pSim.start();

    return 0;
}