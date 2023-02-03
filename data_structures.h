using namespace std;

class Transaction
{
public:
    int from_id, to_id;
    double amount, remaining;
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
    bool cpu, link_speed;
    vector<Node *> edges;
    vector<Block *> blocks;
    Node(int id)
    {
        this->id = id;
    }
};

class Event
{
public:
    // 0 -> trans generated, 1 -> trans received, 2 -> block generated, 3 -> block recived
    int type, node;
    double time;
    Transaction *t;
    Block *b;

    Event(int type, int node, double time)
    {
        this->type = type;
        this->node = node;
        this->time = time;
    }
};

struct eventCompare
{
    bool operator()(Event &a, Event &b)
    {
        return a.time > b.time;
    }
};