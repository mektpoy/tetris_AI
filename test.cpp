#include<cstdio>
#include<iostream>
#include<cstdlib>
#include<cstring>
#include<algorithm>
#include<ctime>
#include<vector>
#include<queue>
#include<stack>
#include<set>
#include<map>

using namespace std;
const double Height[21] = {0.0, 0.2, 0.4, 0.7, 1.1, 1.5, 1.9, 2.5, 3.1, 3.8, 4.7, 5.9, 7.0, 8.2, 9.5, 11.2, 13.5, 16.5, 20.0, 24.5, 30.0};

int test()
{
	return Height[15];
}

int main ()
{
#ifndef ONLINE_JUDGE
	freopen("in.txt", "r", stdin);
#endif
	cout << test();
}

