#include "const.hpp"

double RAND()
{
    // return rand()/(double)(RAND_MAX);
    return double(rand() * (RAND_MAX + 1.0) + rand()) / ((RAND_MAX + 1.0) * (RAND_MAX + 1.0) - 1);
}

void splitRange(int S, int T, int n, vector<int>& rangeS, vector<int>& rangeT)
{
    assert(T - S >= n);
    rangeS.clear();
    rangeT.clear();
    for(long long i = 0; i < n; i ++)
    {
        rangeS.push_back(i*(T-S)/n+S);
    }
    for(long long i = 1; i <= n; i ++)
    {
        rangeT.push_back(i*(T-S)/n+S);
    }
}