#include <iostream>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <ctime>

using namespace std;

const int N = 7;
const double blo = -10.0;
const double bup = 10.0;
const double c1 = 2.0;
const double c2 = 2.0;

int G = 10, S = 5, pos = 0;

double A, B, C, D, E, F, H;

struct Particle{
	double d[N]; Particle(){}
	Particle(double A, double B, double C, double D, double E, double F,double G)
	{
		d[0] = A; d[1] = B; d[2] = C;
		d[3] = D; d[4] = E; d[5] = F; d[6] = G;
	}
	Particle operator + (const Particle &b)
	{
		Particle c;
		for (int i = 0; i < N; i++)
			c.d[i] = d[i] + b.d[i];
		return c;
	}
}g, x[233], p[233], v[233],q[233];

vector <Particle> best;

int cg, cp[233];

inline double getrand(double lo, double up)
{
	double tmp = (double)(rand()) / 32767.0;
	return lo + (up - lo) * tmp;
}

inline void Init(Particle &k, double lo, double up)
{
	for (int i = 0; i < N; i++)
		k.d[i] = getrand(lo, up);
}

inline int battle(Particle k)
{
	int ret = 0, tmp;
	auto redp = fopen("redparameter.txt", "w");
	for (int i = 0; i < N; i++) fprintf(redp, "%.15f\n", k.d[i]);
	fclose(redp);
	for (int i = 0; i < S; i++)
	{
		auto bluep = fopen("blueparameter.txt", "w");
		for (int j = 0; j < N; j++) 
			fprintf(bluep, "%.15f\n", q[i].d[j]);
		fclose(bluep);
		for (int j = 0; j < 7; j++)
		{
			auto first = fopen("firstblock.txt", "w");
			fprintf(first, "%d\n", j);
			fclose(first);
			
			system("checker");
			
			auto f = fopen("result.txt", "r");
			fscanf(f, "%d", &tmp);
			fclose(f);
			
			auto con = fopen("CON", "w");
			fprintf(con, "%d\n", tmp);
			fclose(con);
		}
	}
	return ret;
}

void randparameter()
{
	q[0] = Particle(-4.5, 3.4, 1.5, -3.2, -9.3, -7.8, -3.3);
	q[1] = Particle(-5.5, 3.4, 1.5, -3.2, -9.3, -9.8, -4.4);
	q[2] = Particle(-1, 8, 9, -4, -10, -8, -4);
	q[3] = Particle(-1, 4, 2.3, -2, -3, -5, -2);
	q[4] = Particle(-10, 1, 2.33, -5, -2, -4, -8);
	q[5] = Particle(-4, 6, 7, -5, -2, -4, -8);
	q[6] = Particle(-5, 8, 5, -3, -2, -8, -6);
	q[7] = Particle(-5, 8, 4, -6, -8, -5, -4);
	q[8] = Particle(-5, 8, 10, -6, -8, -5, -10);
	q[9] = Particle(-2, 4, 5, -4, -6, -5, -3);
	q[10] = Particle(-5.6, 3, 4.4, -1.5, -7.2, -4.5, -6.8);
	q[11] = Particle(-5.6, 2, 4.4, -1.5, -7.2, -4.5, -3.0);
	q[12] = Particle(-5.6, 1, 4.4, -1, -4, -7, -5);
	q[13] = Particle(-7, 4, 3.68, -5, -6, -5, -6);
	q[14] = Particle(-9, 3, 3.14, -5, -8, -9, -7);
	q[15] = Particle(-1, 1, 1.66, -1, -1, -1, -1);
	q[16] = Particle(-1, 2, 0.78, -1, -1, -1, -1);
	q[17] = Particle(-1, 2, 0.75, -1, -2, -2, -1);
	q[18] = Particle(-1, 4, 4.35, -1, -2, -2, -1);
	q[19] = Particle(-5.4, 4.4, 5.68, -4, -3, -6, -8);
	q[20] = Particle(-5, 10, 4.2, -4, -8, -4, -8);
	q[21] = Particle(-5, 10, 6.9, -4, -8, -8, -4);
	q[22] = Particle(-5, 3, 2, -2, -2, -2, -2);
	q[23] = Particle(-5, 4, 1, -1, -1, -1, -1);
	q[24] = Particle(-3, 5, 3, -4, -4, -4, -4);
	q[25] = Particle(-2, 5, 2, -3, -4, -4, -3);
	q[26] = Particle(-2, 6, 9, -2, -3, -3, -2);
	q[27] = Particle(-1, 7, 1, -4, -5, -5, -4);
	q[28] = Particle(-2, 8, 0.56, -6, -8, -10, -3);
	q[29] = Particle(-9, 4, 3.655, -5, -8, -9, -3);
}

inline void update(Particle A,int ti)
{
	g = A; cg = ti; best.push_back(A);
	freopen("bestparameter.txt", "w", stdout);
	for (int i = 0; i < best.size(); i++)
		for (int j = 0; j < N; j++)
			printf("%.15f\n", best[i].d[j]);
	fclose(stdout);
}

int main()
{
	freopen("Mektpoy.txt", "r", stdin);
	
	double w0 = 0.9, wt = 0.4; srand(time(NULL));
	randparameter();
	cin >> A >> B >> C >> D >> E >> F >> H;
	g = Particle(A, B, C, D, E, F, H);
	int k = battle(g);
	return 0;
}
