#include <bits/stdc++.h>
#include "data_structures.h"
#include "utility.cpp" // declare header and include

using namespace std;

void startSim()
{
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
        miners.push_back(new Node(i));

    generateGraph(miners, n);
    cout << endl
         << "Network is connected" << endl;

    // for (auto it : miners)
    // {
    //     for (auto it2 : it->edges)
    //         cout << it2->id << " ";
    //     cout << endl;
    // }

    startSim();
}