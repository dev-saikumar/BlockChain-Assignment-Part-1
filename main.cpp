#include <bits/stdc++.h>
#include "data_structures.h"
#include "utility.cpp" // declare header and include

using namespace std;

void startSim(vector<Node *> &miners, int n)
{
    priority_queue<Event *, vector<Event *>, eventCompare> q;

    for (Node *node : miners)
    {
        // transaction create event
        double time = randomExponential(10); // mean = 10 seconds
        Event *transactionGen = new Event(0, node, time);

        // block create event
        time = randomExponential(60); // mean should be 60/hk where hk is the hashing power
        Event *blockGen = new Event(2, node, time);
        // blockGen->block = generateBlock();

        q.push(transactionGen);
        q.push(blockGen);
    }

    while (!q.empty())
    {
        Event *e = q.top();
        q.pop();

        if (e->type == 0)
        {
            int to;
            do
            {
                to = randomUniform(0, n - 1);
            } while (e->node->id == to);

            Transaction *txn = new Transaction(generateUID(), e->node->id, to);

            // flood this transaction
            for (int i = 0; i < miners[e->node->id]->edges.size(); i++)
            {
                Node *destNode = miners[e->node->id]->edges[i];
                int propDelay = miners[e->node->id]->propDelay[i];

                // add transmission time (m/cij) and queueing delay (dij)
                Event *newEvent = new Event(1, destNode, e->time + propDelay);
                newEvent->transaction = txn;
                newEvent->rcvdFrm = e->node;
                q.push(newEvent);
            }

            e->node->txnRcvd.insert(txn->txnID);
        }
        else if (e->type == 1 && !e->node->txnRcvd.count(e->transaction->txnID))
        {
            // validate the transaction first
            e->node->txnRcvd.insert(e->transaction->txnID);

            for (int i = 0; i < miners[e->node->id]->edges.size(); i++)
            {
                Node *destNode = miners[e->node->id]->edges[i];
                if (destNode->id == e->rcvdFrm->id)
                    continue;

                int propDelay = miners[e->node->id]->propDelay[i];

                // add transmission time (m/cij) and queueing delay (dij)
                Event *newEvent = new Event(1, destNode, e->time + propDelay);
                newEvent->transaction = e->transaction;
                newEvent->rcvdFrm = e->node;
                q.push(newEvent);
            }
        }
        else if (e->type == 2)
        {
            cout << "Miner: " << e->node->id << " generated a block at time: " << e->time << endl;
        }
        else if (e->type == 4)
        {
            cout << "Miner " << e->node->id << " received a block" << endl;
        }

        delete e;
    }

    for (Node *node : miners)
    {
        cout << node->id << " received " << node->txnRcvd.size() << " transactions." << endl;
    }
}

int main(int argc, char **argv)
{
    int n, z0, z1;
    srand(time(NULL));
    // argc=4;//argc
    if (argc < 4)
    {
        cout << "Invalid Arguments;Usage: ./main n z0 z1" << endl;
        exit(1);
    }

    n = stoi(argv[1]);
    z0 = stoi(argv[2]);
    z1 = stoi(argv[3]);

    vector<Node *> miners;
    for (int i = 0; i < n; i++)
    {
        Node *newNode = new Node(i);
        newNode->fastLink = randomUniform(0, 99) < z0;
        newNode->fastCPU = randomUniform(0, 99) < z1;
        miners.push_back(newNode);
    }

    generateGraph(miners, n);
    // cout << "Network is connected" << endl;

    // int fastCPU = 0, fastLink = 0;
    // for (auto it : miners)
    // {
    //     fastCPU += it->fastCPU;
    //     fastLink += it->fastLink;

    //     for (auto it2 : it->edges)
    //         cout << it2->id << " ";
    //     cout << endl;
    // }

    // cout << "Fast CPU: " << fastCPU << " Fast Link: " << fastLink << endl;

    startSim(miners, n);
}