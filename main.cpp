#include <bits/stdc++.h>
#include "data_structures.cpp"
#include "utility.cpp"

#define TXN_GEN 0
#define TXN_RCV 1
#define BLK_GEN 2
#define BLK_RCV 3
#define TXN_SIZE 8192 // 1KB in bits

using namespace std;

class P2P
{
private:
    int n, z0, z1, t_blk, t_txn, fastLinkSpeed = 100, slowLinkSpeed = 5;
    double slowCPU, fastCPU;
    vector<Node *> miners;
    priority_queue<Event *, vector<Event *>, eventCompare> eventQueue;

public:
    P2P(int n, int z0, int z1, int t_blk, int t_txn) : n(n), z0(z0), z1(z1), t_blk(t_blk), t_txn(t_txn)
    {
        int fastCpuCount = 0;
        for (int i = 0; i < n; i++)
        {
            Node *newNode = new Node(i, randomUniform(0, 99) < z0, randomUniform(0, 99) < z1, n);
            miners.push_back(newNode);
            fastCpuCount += newNode->fastCPU;
        }

        slowCPU = 1.0 / (n * ((100 - fastCpuCount) / 100.0 + 10 * (fastCpuCount / 100.0)));
        fastCPU = 10 * slowCPU;
        cout << "Fast CPU Count: " << fastCpuCount << "  Slow CPU Hash Power: " << slowCPU << endl;

        generateGraph(miners, n);

        for (Node *node : miners)
        {
            // transaction generation event
            double timestamp = randomExponential(t_txn);
            TxnEvent *txnGen = new TxnEvent(TXN_GEN, node, timestamp, NULL);
            eventQueue.push(txnGen);

            // block generation event
            timestamp = randomExponential(t_blk / (node->fastCPU ? fastCPU : slowCPU)); // mean = t_blk/hashing power
            BlockEvent *blockGen = new BlockEvent(BLK_GEN, node, timestamp, createBlock(node));
            eventQueue.push(blockGen);
        }
    }

    void simulate(int eventCount)
    {
        cout << "Simulation Started." << endl;
        while (!eventQueue.empty() && eventCount--)
        {
            Event *e = eventQueue.top();

            if (e->type == TXN_GEN)
                handleTxnGen((TxnEvent *)e);
            else if (e->type == TXN_RCV)
                handleTxnRcv((TxnEvent *)e);
            else if (e->type == BLK_GEN)
                handleBlkGen((BlockEvent *)e);
            else if (e->type == BLK_RCV)
                handleBlkRcv((BlockEvent *)e);
            else
                exit(1); // should be unreachable

            eventQueue.pop();
            delete e;
        }

        cout << "Simulation Complete. Generating Graphs and Summary Data." << endl;

        // Simulation Output
        system("mkdir Output && cd Output && mkdir Blockchain Summary");
        for (Node *node : miners)
        {
            outputGraph(node);
            minerSummary(node);
        }
    }

    void floodTxn(Node *node, Transaction *txn, double timestamp)
    {
        vector<Node *> &edges = miners[node->id]->edges;
        vector<double> &propDelay = miners[node->id]->propDelay;

        for (int i = 0; i < edges.size(); i++)
        {
            Node *destNode = edges[i];

            /* cij = linkSpeed (in bits/sec), pij = pDelay(propagation delay), tDelay(transmission delay) = msg size/cij
               queueing delay (at source node) = dij = chosen from exponential dist with mean 96kbits/cij */
            double linkSpeed = (node->fastLink && destNode->fastLink ? fastLinkSpeed : slowLinkSpeed) * 1000000; // in bits
            double pDelay = propDelay[i], tDelay = TXN_SIZE / linkSpeed;                                         // in seconds
            double qDelay = randomExponential(96000 / linkSpeed);

            double newTimestamp = timestamp + qDelay + tDelay + pDelay;
            TxnEvent *newEvent = new TxnEvent(TXN_RCV, destNode, newTimestamp, txn);
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

            double linkSpeed = (node->fastLink && destNode->fastLink ? fastLinkSpeed : slowLinkSpeed) * 1000000;
            double pDelay = propDelay[i], tDelay = (1 + block->transactions.size()) * TXN_SIZE / linkSpeed; // block size = (1 + # of txns) KB
            double qDelay = randomExponential(96000 / linkSpeed);

            double newTimestamp = timestamp + qDelay + tDelay + pDelay;
            BlockEvent *newEvent = new BlockEvent(BLK_RCV, destNode, newTimestamp, block);
            eventQueue.push(newEvent);
        }
    }

    void handleTxnGen(TxnEvent *e)
    {
        Node *node = e->node;
        BlockChain *blockchain = node->blockchain;

        if (node->balanceLeft < 10)
        {
            TxnEvent *txnGen = new TxnEvent(TXN_GEN, node, e->timestamp + randomExponential(t_txn), NULL);
            eventQueue.push(txnGen);
            return;
        }

        int to_id;
        do
        {
            to_id = randomUniform(0, n - 1);
        } while (node->id == to_id);

        int amount = randomUniform(1, node->balanceLeft / 10);
        node->balanceLeft -= amount;
        Transaction *txn = new Transaction(get_uuid(), node->id, to_id, amount);

        blockchain->pendingTxns.insert(txn->txnID);
        blockchain->allTxnRcvd.insert({txn->txnID, txn});
        floodTxn(node, txn, e->timestamp);

        // new event for next transaction generation
        TxnEvent *txnGen = new TxnEvent(TXN_GEN, node, e->timestamp + randomExponential(t_txn), NULL);
        eventQueue.push(txnGen);
    }

    void handleTxnRcv(TxnEvent *e)
    {
        Node *node = e->node;
        Transaction *txn = e->txn;
        BlockChain *blockchain = node->blockchain;

        // already received, don't flood - prevents looping
        if (blockchain->allTxnRcvd.count(txn->txnID))
            return;

        // validate the transaction
        if (blockchain->lastBlock->balance[txn->from_id] < txn->amount)
            return;

        blockchain->pendingTxns.insert(txn->txnID);
        blockchain->allTxnRcvd.insert({txn->txnID, txn});

        floodTxn(node, txn, e->timestamp);
    }

    Block *createBlock(Node *node)
    {
        Block *lastBlock = node->blockchain->lastBlock;
        Block *newBlock = new Block(lastBlock->blockID, get_uuid(), node->id, lastBlock->chainLen + 1);
        newBlock->balance = vector<int>(lastBlock->balance);

        // add transactions to the block
        for (string txnID : node->blockchain->pendingTxns)
        {
            if (newBlock->transactions.size() >= 1000)
                break;
            Transaction *txn = node->blockchain->allTxnRcvd[txnID];
            newBlock->transactions.push_back(txn);

            // update balance vector
            newBlock->balance[txn->from_id] -= txn->amount;
            newBlock->balance[txn->to_id] += txn->amount;
        }

        // adding mining fees (coinbase)
        newBlock->balance[node->id] += 50;

        return newBlock;
    }

    void handleBlkGen(BlockEvent *e)
    {
        Node *node = e->node;
        Block *newBlock = e->block;

        // longest chain changed
        if (node->blockchain->lastBlock->blockID != newBlock->prevBlockID)
            delete newBlock;
        else
        {
            // else, add this block to blockchain and flood
            addBlock(e);
            floodBlock(node, newBlock, e->timestamp);
        }

        // new block generation event
        double timestamp = e->timestamp + randomExponential(t_blk / (node->fastCPU ? fastCPU : slowCPU));
        BlockEvent *blockGen = new BlockEvent(BLK_GEN, node, timestamp, createBlock(node));
        eventQueue.push(blockGen);
    }

    void handleBlkRcv(BlockEvent *e)
    {
        BlockChain *blockchain = e->node->blockchain;
        Block *block = e->block;

        if (blockchain->allBlocks.count(block->blockID)) // duplicate block received
            return;

        addBlock(e);
    }

    void addBlock(BlockEvent *e)
    {
        Node *node = e->node;
        Block *block = e->block;
        BlockChain *blockchain = node->blockchain;
        unordered_map<string, Block *> &allBlocks = blockchain->allBlocks;
        unordered_map<string, Transaction *> &allTxnRcvd = blockchain->allTxnRcvd;

        allBlocks.insert({block->blockID, block});
        node->blockArrivalTime.insert({block->blockID, e->timestamp});

        // Check if parent block is present
        if (allBlocks.find(block->prevBlockID) == allBlocks.end())
        {
            blockchain->parentLessBlocks.insert(block->blockID);
            return;
        }

        vector<int> prevBalance = vector<int>(allBlocks[block->prevBlockID]->balance);
        vector<int> &nextBalance = block->balance;

        // simulating all transactions
        prevBalance[block->minerID] += 50; // adding coinbase
        for (Transaction *txn : block->transactions)
        {
            // new transaction rcvd
            if (allTxnRcvd.find(txn->txnID) == allTxnRcvd.end())
            {
                allTxnRcvd.insert({txn->txnID, txn});
                floodTxn(miners[blockchain->minerID], txn, e->timestamp);
            }

            prevBalance[txn->from_id] -= txn->amount;
            prevBalance[txn->to_id] += txn->amount;
        }

        // validating the block
        for (int i = 0; i < prevBalance.size(); i++)
        {
            if (prevBalance[i] != nextBalance[i] || nextBalance[i] < 0)
            {
                blockchain->invalidBlocks.insert(block->blockID);
                allBlocks.erase(block->blockID);
                return; // invalid block
            }
        }

        // validation successful, update longest chain if needed
        if (blockchain->lastBlock->chainLen < block->chainLen)
            updateLongestChain(blockchain, block);
        // else cout << "FORK\n";

        floodBlock(node, block, e->timestamp);
        checkParentLessBlocks(node, e->timestamp);
    }

    void updateLongestChain(BlockChain *blockchain, Block *block)
    {
        unordered_set<string> &pendingTxns = blockchain->pendingTxns;
        unordered_map<string, Block *> &allBlocks = blockchain->allBlocks;
        unordered_map<string, Transaction *> &allTxnRcvd = blockchain->allTxnRcvd;

        // Case 1: Chain Extension
        if (block->prevBlockID == blockchain->lastBlock->blockID)
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
            // Case 2: new longest Chain: recompute pendingTxn set
            // pendingTxns will hold set of all transactions received
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

        miners[blockchain->minerID]->balanceLeft = block->balance[blockchain->minerID];
        blockchain->lastBlock = block;
    }

    void checkParentLessBlocks(Node *node, double timestamp)
    {
        BlockChain *blockchain = node->blockchain;
        unordered_map<string, Block *> &allBlocks = blockchain->allBlocks;
        unordered_set<string> &invalidBlocks = blockchain->invalidBlocks;
        vector<string> processed;

        for (auto &blockID : blockchain->parentLessBlocks)
        {
            Block *block = allBlocks[blockID];
            if (invalidBlocks.find(block->prevBlockID) != invalidBlocks.end())
            {
                // this block is also invalid
                blockchain->invalidBlocks.insert(block->blockID);
                allBlocks.erase(block->blockID);
                processed.push_back(blockID);
                continue;
            }

            if (allBlocks.find(block->prevBlockID) == allBlocks.end()) // parent block still absent
                continue;

            processed.push_back(blockID);

            vector<int> prevBalance = vector<int>(allBlocks[block->prevBlockID]->balance);
            vector<int> &nextBalance = block->balance;

            // validating the block
            for (int i = 0; i < prevBalance.size(); i++)
            {
                if (prevBalance[i] != nextBalance[i] || nextBalance[i] < 0)
                {
                    blockchain->invalidBlocks.insert(block->blockID);
                    allBlocks.erase(block->blockID);
                    continue; // invalid block
                }
            }

            // validation successful, update longest chain if needed
            if (blockchain->lastBlock->chainLen < block->chainLen)
                updateLongestChain(blockchain, block);

            floodBlock(node, block, timestamp);
        }

        for (string &blockID : processed)
            blockchain->parentLessBlocks.erase(blockID);
    }
};

int main(int argc, char **argv)
{
    srand(time(NULL));

    if (argc < 7)
    {
        cout << "Invalid Arguments. Usage: ./main n z0 z1 t_blk t_txn eventCount" << endl;
        exit(1);
    }

    int n = stoi(argv[1]), z0 = stoi(argv[2]), z1 = stoi(argv[3]);
    int t_blk = stoi(argv[4]), t_txn = stoi(argv[5]), eventCount = stoi(argv[6]);

    P2P p2pSim(n, z0, z1, t_blk, t_txn);
    p2pSim.simulate(eventCount); // simulate <eventCount> events

    return 0;
}