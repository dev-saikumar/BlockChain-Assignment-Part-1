#include <bits/stdc++.h>
#include <stdio.h>
using namespace std;

int n;

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

int generateRandom(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

int main(int argc, char **argv)
{
    cout << argv[0] << endl;
    // argc=2;//argc
    if (argc < 2)
    {
        cout << "Usage: Invalid Argument" << endl;
        exit(1);
    }
    else
    {
        // n=40;
        n = stoi(argv[1]);
    }
    srand(time(NULL));
    vector<Node *> miners;

    for (int i = 0; i < n; i++)
        miners.push_back(new Node(i));
    cout << "11";
    while (!checkForConnectivity(miners))
    {
        cout << "checkForConnectivity" << endl;
        for (int i = 0; i < n; i++)
        {
            int no_of_connections = generateRandom(4, 8);
            unordered_set<int> random_miners;
            while (random_miners.size() < no_of_connections)
            {
                int r = generateRandom(0, n - 1);
                if (r != miners[i]->id && random_miners.find(r) == random_miners.end())
                {
                    random_miners.insert(r);
                }
            }
            for (auto it : random_miners)
            {
                cout << it << "-";
                miners[i]->edges.push_back(miners[it]);
            }
            cout << endl;
        }
    }
    cout << endl
         << "Network is connected"<<endl;


         
}