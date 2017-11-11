#include <iostream>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <ctime>

using namespace std;

const int N = 7;
const double blo[7] = {-6, 3, 10, -7, -10, -10, -7};
const double bup[7] = {-1, 8, 15, -2, -5, -5, -2};
const double c1 = 2.0;
const double c2 = 2.0;
const double K = 18.0;
const int G = 60;
const int S = 20;
const int R = 5;

double cg, cp[233];

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

vector <double> win;
vector <Particle> best;

inline double getrand(double lo, double up)
{
	double tmp = (double)(rand()) / 32767.0;
	return lo + (up - lo) * tmp;
}

inline double battle(Particle k, int id)
{
	int tmp;
	double ret = 0;
	auto redp = fopen("redparameter.txt", "w");
	for (int i = 0; i < N; i++) fprintf(redp, "%.15f\n", k.d[i]);
	fclose(redp);
	double total = R * 7;
	for (int i = 0; i < R; i++)
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
			if (tmp == 2)
			{
				ret += 0.5;
			}
			else
			{
				ret += tmp;
			}
			fclose(con);
			total --;
			if (ret + total <= cp[id]) return ret;
		}
	}
	return ret;
}

void randparameter()
{
	q[0] = Particle(-4.5, 3.4, 1.4, -3.2, -9.3, -7.9, -3.4);
	q[1] = Particle(-7.5, 0.0, 7.5, -3.9, -10.0, -10.0, -7.0);
	q[2] = Particle(-3.3, 4.3, 11.8, -3.3, -8.8, -6.5, -4.0);
	q[3] = Particle(-2.7, 7.2, 14.9, -4.4, -5.5, -8.2, -2.2);
	q[4] = Particle(-3.2, 4.9, 13.9, -3.8, -6.0, -8.2, -3.9);
	q[5] = Particle(-4.7, 1.2, 3.8, -3.6, -8.5, -9.1, -5.7);
	q[6] = Particle(-5.0, 1.2, 3.8, -4.0, -9.4, -8.8, -5.4);
	q[7] = Particle(-5.0, 1.3, 3.4, -3.8, -8.8, -8.7, -5.0);
	q[8] = Particle(-7.0, 0.0, 5.6, -3.3, -10.0, -10.0, -6.6);
	q[9] = Particle(-3.7, 0.0, 2.3, -3.3, -5.1, -7.2, -2.0);
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

inline void update(Particle A, double ti)
{
	auto bestpa = fopen("bestparameter.txt", "w");
	g = A; cg = ti; best.push_back(g); win.push_back(cg);
	for (int i = 0; i < best.size(); i++)
	{
		for (int j = 0; j < N; j++)
			fprintf(bestpa, "%.15f ", best[i].d[j]);
		fprintf(bestpa, "%.1f\n", win[i]);
	}
	fclose(bestpa);
}

inline void Print(Particle A, double ti)
{
	auto bestpa = fopen("bestparameter.txt", "w");
	best.push_back(A); win.push_back(ti);
	for (int i = 0; i < best.size(); i++)
	{
		for (int j = 0; j < N; j++)
			fprintf(bestpa, "%.15f ", best[i].d[j]);
		fprintf(bestpa, "%.1f\n", win[i]);
	}
	fclose(bestpa);
}

inline void Print_p()
{
	auto part = fopen("partialparameter.txt", "w");
	for (int i = 1; i <= S; i++)
	{
		for (int j = 0; j < N; j++) fprintf(part, "%.15f ", p[i].d[j]);
		fprintf(part, "%.1f\n", cp[i]);
	}
	fclose(part);
}

int main()
{
	double w0 = 0.9, wt = 0.4; srand(time(NULL));
	double A = getrand(-6.0, -4.0);
	double B = getrand(1.0, 4.0);
	double C = getrand(3.0, 6.0);
	double D = getrand(-6.0, -3.0);
	double E = getrand(-10.0, -8.0);
	double F = getrand(-8.0, -6.0);
	double H = getrand(-8.0, -4.0);
	
	g = Particle(A, B, C, D, E, F, H);
	randparameter();
	cg = battle(g, 0);
	update(g, cg);
	
	for (int i = 0; i < S; i++)
	{
		for (int j = 0; j < N; j++)
			x[i].d[j] = getrand(blo[j], bup[j]),
			v[i].d[j] = getrand(blo[j] - bup[j], bup[j] - blo[j]);
		p[i] = x[i]; cp[i] = battle(p[i], i);
		if (cp[i] > cg) update(p[i], cp[i]);
		else if (cp[i] >= K) Print(p[i], cp[i]);
	}
	
	Print_p();
	
	for (int total = 0; total < G; total++)
	{
		auto rod = fopen("Round.txt", "w");
		fprintf(rod, "%d", total);
		fclose(rod);
		double w = (w0 - wt) * (G - total) / G + wt;
		for (int i = 0; i < S; i++)
		{
			for (int j = 0; j < N; j++)
			{
				double A = getrand(0.0, 1.0),B = getrand(0.0, 1.0);
				v[i].d[j] = w * v[i].d[j] + c1 * A * (p[i].d[j] - x[i].d[j]) + c2 * B * (g.d[j] - x[i].d[j]);
			}
			x[i] = x[i] + v[i];
			double now = battle(x[i], i);
			if (now > cp[i])
			{
				cp[i] = now; p[i] = x[i]; Print_p();
				if (cp[i] > cg) update(p[i], cp[i]);
				else if (cp[i] >= K) Print(p[i], cp[i]);
			}
		}
	}
	return 0;
}
