#include <bits/stdc++.h>
using namespace std;

class Transaction
{
public:
    int from_id;
    vector<int> to_id;
    vector<double> amount;
};

class Block
{
public:
    Block *previous_block;
    vector<Transaction *> transactions;
};

class Node
{
public:
    int id;
    vector<Node *> edges;
    Node(int id)
    {
        this->id = id;
    }
};

struct compare
{
    bool operator()(Event &a, Event &b)
    {
        return a.timestamp > b.timestamp;
    }
};

class Event
{
public:
    // 0 -> trans generated, 1 -> trans received, 2 -> block generated, 3 -> block recived
    int type, node;
    long timestamp;
    Transaction *t;
    Block *b;

    Event(int type, int node, long timestamp)
    {
        this->type = type;
        this->node = node;
        this->timestamp = timestamp;
    }
};