#include <bits/stdc++.h>

using namespace std;

// --------------------- Unique ID Generator ---------------------

string get_uuid()
{
    static random_device dev;
    static mt19937 rng(dev());
    const char *v = "0123456789abcdef";
    uniform_int_distribution<int> dist(0, 15);

    string res;
    for (int i = 0; i < 16; i++)
    {
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}

// --------------------- Random Generators ---------------------

int randomUniform(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

double randomExponential(double mean)
{
    double uniform = double(rand()) / RAND_MAX;
    double r = log(1 - uniform) * (-mean);
    return r;
}

// --------------------- Simulation Output ---------------------

void minerSummary(Node *node)
{
    BlockChain *blockchain = node->blockchain;
    Block *block = blockchain->lastBlock;
    int chainLen = 1, myBlocks = 0, myID = node->id, totalBlocks = blockchain->allBlocks.size();

    string filepath = "Miner Summary/miner" + to_string(node->id) + ".txt";
    ofstream file(filepath);

    while (block->blockID != "0")
    {
        chainLen++;
        myBlocks += block->minerID == myID;
        // cout << block->transactions.size() << " "; // # of transaction in the block
        block = blockchain->allBlocks[block->prevBlockID];
    }

    file << "Fast CPU: " << node->fastCPU << " Fast Link: " << node->fastLink
         << " Blocks Received: " << totalBlocks << " Chain Length: " << chainLen << " Contribution: " << myBlocks / double(chainLen)
         << " Balance: " << node->blockchain->lastBlock->balance[node->id] << endl;

    // Branch/Fork Summary

    file << "\nBranches/Fork Summary: " << endl;

    unordered_map<string, Block *> allBlocks = blockchain->allBlocks, leaves = allBlocks;

    // leaves of the blockchain
    for (auto &it : blockchain->allBlocks)
        leaves.erase(it.second->prevBlockID);

    int longest = 1;
    Block *temp = blockchain->lastBlock;
    leaves.erase(temp->blockID);
    while (temp->blockID != "0")
    {
        longest++;
        temp = allBlocks[temp->prevBlockID];
        allBlocks.erase(temp->blockID);
    }

    unordered_map<int, int> chains;
    chains.insert({longest, 1});

    for (auto &it : leaves)
    {
        int forkLen = 0;
        Block *temp = it.second;

        while (true)
        {
            forkLen++;
            if (!allBlocks.count(temp->prevBlockID))
                break;
            temp = allBlocks[temp->prevBlockID];
        }

        chains[forkLen]++;
    }

    for (auto &it : chains)
        file << "Branch Length: " << it.first << " Count: " << it.second << endl;

    file.close();
}

void outputGraph(Node *node)
{
    int blockID = 1, nodeID = node->id;
    BlockChain *blockchain = node->blockchain;
    unordered_map<string, int> map;

    string filepath = "Graph Data/graph" + to_string(node->id) + ".gh";
    ofstream file(filepath);
    file << "digraph G{\nrankdir=\"LR\";";

    // giving every block a integer id
    map.insert({"0", 0});
    for (auto &it : blockchain->allBlocks)
        map.insert({it.first, blockID++});

    for (auto it : blockchain->allBlocks)
        file << map[it.first] << " [xlabel=\"" << node->blockArrivalTime[it.first] << "\"]" << endl;

    for (auto &it : blockchain->allBlocks)
    {
        Block *block = it.second;
        if (block->blockID == block->prevBlockID)
            continue;
        file << "\t" << map[block->prevBlockID] << " -> " << map[block->blockID] << endl;
    }

    file << "}";
    file.close();

    string graphPath = "dot -Tpng -Nshape=rect \"Graph Data\"/graph" + to_string(node->id) + ".gh -o Graph/graph" + to_string(node->id) + ".png";
    system(graphPath.c_str());
}

// --------------------- Graph Algo ---------------------

bool checkForConnectivity(vector<Node *> &miners)
{
    bool visited[miners.size()];
    fill_n(visited, miners.size(), false);
    stack<Node *> st;
    st.push(miners[0]);
    while (!st.empty())
    {
        Node *curr = st.top();
        if (visited[curr->id])
        {
            st.pop();
            continue;
        }
        visited[curr->id] = true;
        for (int i = 0; i < curr->edges.size(); i++)
        {
            st.push(curr->edges[i]);
        }
    }
    for (int i = 0; i < miners.size(); i++)
    {
        if (!visited[i])
            return false;
    }
    return true;
}

void generateGraph(vector<Node *> &miners, int n)
{
    bool connected = false;

    while (!connected)
    {
        for (int i = 0; i < n; i++)
        {
            int no_of_connections = randomUniform(4, 8);

            unordered_set<int> random_miners;
            for (auto it : miners[i]->edges)
            {
                random_miners.insert(it->id);
            }
            int counter = 0;
            while (counter < n && random_miners.size() < no_of_connections)
            {
                int r = randomUniform(0, n - 1);
                if (r != miners[i]->id && miners[r]->edges.size() < 8 && random_miners.find(r) == random_miners.end())
                {
                    random_miners.insert(r);
                    miners[i]->edges.push_back(miners[r]);
                    miners[r]->edges.push_back(miners[i]);

                    // setting pij
                    double propDelay = randomUniform(10, 500) / 1000.0; // converting ms -> s
                    miners[i]->propDelay.push_back(propDelay);
                    miners[r]->propDelay.push_back(propDelay);
                }
                else
                    counter++;
            }
        }

        if (!checkForConnectivity(miners))
        {
            for (auto it : miners)
                it->edges.clear();
        }
        else
            connected = true;
    }
}
