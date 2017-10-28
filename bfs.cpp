
const int dx[] = {1,-1,0};
const int dy[] = {0,0,-1};

struct data{
	int x,y,o; data(){}
	data(int x,int y,int z): x(x),y(y),o(o){}
};

int rt,total,cloc,vis[23][23][5],blocktyp[maxn],og[maxn],fx[maxn],fy[maxn],fo[maxn];
bool Mark[maxn];
 
queue <data> Q;
vector <data> v[maxn];

	inline void BFS(int x)
	{
		int typ = blocktyp[x],o = og[x];
		vis[5][MAPHEIGHT][o] = ++cloc;
		Q.push(data(5,MAPHEIGHT,o));
		while (!Q.empty())
		{
			data k = Q.front(); Q.pop();
			for (int i = 0; i < 3; i++)
			{
				int xx = k.x + dx[i],yy = k.y + dy[i];
				if (xx < 1 || xx > MAPWIDTH || yy < 1 || yy > MAPHEIGHT) continue;
				if (vis[xx][yy][k.o] == cloc) continue;
				vis[xx][yy][k.o] cloc; Q.push(data(xx,yy,k.o));
				if (block.set(xx,yy,k.o).isValid()) v[x].push_back(data(xx,yy,k.o));
			}
			for (int i = 0; i < 4; i++)
			{
				if (i == k.o || vis[k.x][k.y][i] == cloc) continue;
				if (block.set(k.x,k.y,k.o).rotation(i))
				{
					vis[k.x][k.y][i] = cloc;
					Q.push(data(k.x,k.y,i));
					if (block.set(k.x,k.y,i).isValid()) v[x].push_back(data(k.x,k.y,i));
				}
			}
		}
	}
