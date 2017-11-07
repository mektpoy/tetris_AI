#include <iostream>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <ctime>

using namespace std;

const double blo = -10.0;
const double bup = 10.0;
const double c1 = 2.0;
const double c2 = 2.0;

struct Particle{
	double d[6]; Particle(){}
	Particle(double A, double B, double C, double D, double E, double F)
	{
		d[0] = A; d[1] = B; d[2] = C;
		d[3] = D; d[4] = E; d[5] = F;
	}
	Particle operator + (const Particle &b)
	{
		Particle c;
		for (int i = 0; i < 6; i++)
			c.d[i] = d[i] + b.d[i];
		return c;
	}
}g, x[233], p[233], v[233];

inline double getrand(double lo, double up)
{
	double tmp = (double)(rand()) / 32767.0;
	return lo + (up - lo) * tmp;
}

inline void Init(Particle &k, double lo, double up)
{
	for (int i = 0; i < 6; i++)
		k.d[i] = getrand(lo, up);
}

/*inline bool update(Particle A, Particle &B)
{
	if (A > B)
	{
		B = A; return true;
	}
	return false;
}*/

int main()
{
	int G = 50, S = 30; double w0 = 0.9, wt = 0.4; srand(time(NULL));
	g = Particle(-4.500158825082766, 3.4181268101392694, -3.2178882868487753
		, -9.348695305445199, -7.899265427351652, -3.3855972247263626);
	
	for (int i = 0; i < S; i++)
	{
		Init(x[i], blo, bup); p[i] = x[i];
		Init(v[i], blo - bup, bup - blo); update(p[i], g);
	}

	for (int total = 0; total < G; total++)
	{
		double w = (w0 - wt) * (G - total) / G + wt;
		for (int i = 0; i < S; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				double r = getrand(0.0, 1.0);
				v[i].d[j] = w * v[i].d[j] + c1 * r * (p[i].d[j] - x[i].d[j]) + c2 * r * (g.d[j] - x[i].d[j]);
			}
			x[i] = x[i] + v[i];
			for (int j = 0; j < 6; j++) x[i].d[j] = max(x[i].d[j], blo), x[i].d[j] = min(x[i].d[j], bup);
			if (update(x[i], p[i])) update(p[i], g);
		}
	}

	for (int i = 0; i < 6; i++) printf("%.15lf\n",g.d[i]);
}
