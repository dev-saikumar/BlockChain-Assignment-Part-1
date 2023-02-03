#include <bits/stdc++.h>
#include <stdio.h>
using namespace std;

int n,z0,z1;

class Transaction
{
public:
    int from_id;
    vector<int> to_ids;
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
    bool cpu,link_speed;
    vector<Node *> edges;
    vector<Block *> blocks;
    Node(int id)
    {
        this->id = id;
    }
};

class Event{
    public:
        double time; 
};

class Broadcast : public Event{
    public:
    int b;
    Broadcast(double time){
        this->time = time;
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
    bool connected = false;
    // argc=4;//argc
    if (argc < 4)
    {
        cout << "Invalid Arguments;Usage: ./main n z0 z1" << endl;
        exit(1);
    }
    else
    {
        
        n = stoi(argv[1]);
        z0 =stoi(argv[2]);
        z1 =stoi(argv[3]);
    }
    srand(time(NULL));
    vector<Node *> miners;
    for (int i = 0; i < n; i++)
        miners.push_back(new Node(i));
    cout << "11";
    while (!connected)
    {
        cout << "checkForConnectivity" << endl;
        for (int i = 0; i < n; i++)
        {
            int no_of_connections = generateRandom(4, 8);
        
            unordered_set<int> random_miners;
            for(auto it : miners[i]->edges){
                random_miners.insert(it->id);
            }
            int counter=0;
            while (counter<n && random_miners.size() < no_of_connections)
            {
                int r = generateRandom(0, n - 1);
                if (r != miners[i]->id && miners[r]->edges.size()<8 && random_miners.find(r) == random_miners.end())
                {
                    random_miners.insert(r);
                    miners[i]->edges.push_back(miners[r]);
                    miners[r]->edges.push_back(miners[i]);
                }else
                counter++;
            }
        }
        cout<<"loop";
        if(!checkForConnectivity(miners)){
            for(auto it : miners){
                it->edges.clear();
            }
        }else
        connected=true;
    }

    for(auto it : miners){
        for(auto it2: it->edges)
        cout << it2->id<<" ";
        cout << endl;
    }

    cout << endl
         << "Network is connected"<<endl;


    priority_queue<Event *> event_queue;
    event_queue.push(new Broadcast(10));
}