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

void outputGraph(Node *node)
{
    int blockID = 1, nodeID = node->id;
    BlockChain *blockchain = node->blockchain;
    unordered_map<string, int> map;

    string filepath = "Output/Blockchain/tree" + to_string(node->id) + ".gh";
    ofstream file(filepath);
    file << "digraph G{\nrankdir=\"LR\";";

    // giving every block a integer id
    map.insert({"0", 0});
    for (auto &it : blockchain->allBlocks)
        map.insert({it.first, blockID++});

    for (auto it : blockchain->allBlocks)
        file << map[it.first] << " [xlabel=\"" << node->blockArrivalTime[it.first] << "\"]" << '\n';

    for (auto &it : blockchain->allBlocks)
    {
        Block *block = it.second;
        if (block->blockID == block->prevBlockID)
            continue;
        file << "\t" << map[block->prevBlockID] << " -> " << map[block->blockID] << '\n';
    }

    file << "}";
    file.close();

    string graphPath = "dot -Tpdf -Nshape=rect Output/Blockchain/tree" + to_string(node->id) + ".gh -o Output/Blockchain/tree" + to_string(node->id) + ".pdf";
    system(graphPath.c_str());
}

void minerSummary(Node *node)
{
    int chainLen = 1, myBlocks = 0, myID = node->id;
    BlockChain *blockchain = node->blockchain;
    Block *block = blockchain->lastBlock;

    string filepath = "Output/Summary/miner" + to_string(node->id) + ".csv";
    ofstream file(filepath);

    while (block->blockID != "0")
    {
        chainLen++;
        myBlocks += block->minerID == myID;
        block = blockchain->allBlocks[block->prevBlockID];
    }

    // Miner's Info
    file << "Miner ID, " << node->id << '\n';
    file << "CPU Type, " << (node->fastCPU ? "Fast" : "Slow") << '\n';
    file << "Link Type, " << (node->fastLink ? "Fast" : "Slow") << '\n';
    file << "Blocks Received, " << blockchain->allBlocks.size() << '\n';
    file << "Longest Chain Length, " << chainLen << '\n';
    file << "Miner's Block in the Longest Chain, " << to_string(myBlocks) + " (" + to_string(myBlocks * 100.0 / chainLen) + ")" << '\n';
    file << "Balance, " << blockchain->lastBlock->balance[node->id] << '\n';

    // Fork Summary
    unordered_map<string, Block *> allBlocks = blockchain->allBlocks, leaves = allBlocks;

    // leaves of the blockchain
    for (auto &it : blockchain->allBlocks)
        leaves.erase(it.second->prevBlockID);

    Block *temp = blockchain->lastBlock;
    leaves.erase(temp->blockID);
    while (temp->blockID != "0")
    {
        temp = allBlocks[temp->prevBlockID];
        allBlocks.erase(temp->blockID);
    }
    unordered_map<int, int> forks;

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

        forks[forkLen]++;
    }

    file << '\n';
    for (auto &it : forks)
        file << "Fork Length, " << it.first << " (" << it.second << ")\n";

    file << "\nBlock ID, "
         << "Previous Block ID, "
         << "Arrival Timestamp, "
         << "# of Transactions in the Block" << endl;

    for (auto &it : node->blockArrivalTime)
    {
        if (blockchain->invalidBlocks.count(it.first)) // skipping invalid blocks
            continue;
        Block *block = blockchain->allBlocks[it.first];
        file << it.first << ", " << block->prevBlockID << ", " << it.second << ", " << block->transactions.size() << '\n';
    }

    file.close();
}

void overallSummary(vector<Node *> &miners)
{
    int chainLen = 1;
    Node *node = miners[0];
    BlockChain *blockchain = node->blockchain;
    Block *block = blockchain->lastBlock;
    vector<int> blocksInChain(miners.size());

    while (block->blockID != "0")
    {
        chainLen++;
        // cout << block->transactions.size() << " ";
        block = node->blockchain->allBlocks[block->prevBlockID];
        blocksInChain[block->minerID]++;
    }

    // Miner Summary
    for (Node *node : miners)
    {
        int totalBlocks = node->blockchain->allBlocks.size();
        Block *lastBlock = node->blockchain->lastBlock;

        cout << "Miner " << node->id << (node->fastCPU ? ", Fast CPU" : ", Slow CPU") << (node->fastLink ? ", Fast Link" : ", Slow Link");
        cout << ", Total Blocks: " << totalBlocks << ", Chain Length: " << lastBlock->chainLen;
        cout << fixed << setprecision(2) << ", Contribution: " << blocksInChain[node->id] << " (" << (blocksInChain[node->id] * 100.0 / totalBlocks) << ")";
        cout << ", Balance: " << lastBlock->balance[node->id] << endl;
    }
    cout << endl;

    // Fork Summary
    unordered_map<int, int> forks;
    unordered_map<string, Block *> allBlocks = blockchain->allBlocks, leaves = allBlocks;
    Block *temp = blockchain->lastBlock;

    // leaves of the blockchain
    for (auto &it : blockchain->allBlocks)
        leaves.erase(it.second->prevBlockID);

    leaves.erase(temp->blockID);
    while (temp->blockID != "0")
    {
        temp = allBlocks[temp->prevBlockID];
        allBlocks.erase(temp->blockID);
    }

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

        forks[forkLen]++;
    }

    cout << "Fork Summary: " << (forks.size() == 0 ? "No Forks" : "") << endl;
    for (auto &it : forks)
        cout << "Fork Length, " << it.first << ", Count: " << it.second << endl;
    cout << endl;
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
