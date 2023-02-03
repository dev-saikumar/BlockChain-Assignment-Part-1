using namespace std;

class Transaction
{
public:
    string txnID;
    int from_id, to_id;
    double amount, remaining;

    Transaction(string id, int from, int to)
    {
        txnID = id;
        from_id = from;
        to_id = to;
    }
};

class Block
{
public:
    int prevBlockID;
    vector<Transaction *> transactions;
};

class Node
{
public:
    int id;
    bool fastCPU, fastLink;
    vector<Node *> edges;
    vector<int> propDelay;

    // vector<Transaction *> pendingTxns;
    unordered_set<string> txnRcvd;

    // TODO: BlockChain tree for each node

    Node(int id)
    {
        this->id = id;
    }
};

class Event
{
public:
    // 0 -> trans generated, 1 -> trans received, 2 -> block generated, 3 -> block recived
    int type;
    Node *node, *rcvdFrm;
    double time; // in seconds
    Transaction *transaction;
    Block *block;

    Event(int type, Node *node, double time)
    {
        this->type = type;
        this->node = node;
        this->time = time;
    }
};

struct eventCompare
{
    bool operator()(Event *a, Event *b)
    {
        return a->time > b->time;
    }
};