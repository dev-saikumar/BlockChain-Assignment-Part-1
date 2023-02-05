#include <bits/stdc++.h>
using namespace std;

string generateUID()
{
    const char *chars = "0123456789abcdef";
    string res;

    for (int i = 0; i < 16; i++)
        res += chars[rand() % 16];

    return res;
}

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
                    int propDelay = randomUniform(10, 500);
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