#include <bits/stdc++.h>
using namespace std;

/*
So, generate a uniform random number, u, in [0,1), then calculate x by:

x = log(1-u)/(-λ) = log(1-u)*(-mean),

where λ is the rate parameter of the exponential distribution.
Now, x is a random number with an exponential distribution. Note that log above is ln, the natural logarithm.
*/

double random_exponential(int mean)
{
    double uniform = double(rand()) / RAND_MAX;
    double r = log(1 - uniform) * (-mean);
    return r;
}

int main()
{
    srand(time(0));
    int x = 20;
    while (x--)
    {
        cout << random_exponential(10) << endl;
    }

    return 0;
}