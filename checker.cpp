/**
 * Tetris2 样例程序
 * https://wiki.botzone.org/index.php?title=Tetris2
 */
// 注意：x的范围是1~MAPWIDTH，y的范围是1~MAPHEIGHT
// 数组是先行（y）后列（c）
// 坐标系：原点在左下角
 
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <queue>
#include <cstdlib>
using namespace std;
 
#define MAPWIDTH 10
#define MAPHEIGHT 20
 
// 我所在队伍的颜色（0为红，1为蓝，仅表示队伍，不分先后）
int currBotColor;
int enemyColor;

double tim;
int MAXDEP, turnID;
 
// 先y后x，记录地图状态，0为空，1为以前放置，2为刚刚放置，负数为越界
// （2用于在清行后将最后一步撤销再送给对方）
int tmpInfo[2][MAPHEIGHT + 2] = { 0 };
int temp_gridInfo[110][2][MAPHEIGHT + 2] = { 0 };
int gridInfo[2][MAPHEIGHT + 2] = { 0 };
 
// 代表分别向对方转移的行
int trans[2][6] = { 0 };
 
// 转移行数
int transCount[2] = { 0 };
 
// 运行eliminate后的当前高度
int temp_maxHeight[110][2] = { 0 };
int maxHeight[2] = { 0 };

// 总消去行数的分数之和
int temp_elimTotal[110][2] = { 0 }; 
int elimTotal[2] = { 0 };

// 连续几回合发生过消去了
int temp_elimCombo[110][2] = { 0 }; 
int elimCombo[2] = { 0 };
 
// 一次性消去行数对应分数
const int elimBonus[] = { 0, 1, 2, 3, 4, 5};
double TIME_LIMIT;
 
// 给对应玩家的各类块的数目总计
int typeCountForColor[2][7] = { 0 };
 
int blockShape[7][4][8] = {
	{ { 0,0,1,0,-1,0,-1,-1 },{ 0,0,0,1,0,-1,1,-1 },{ 0,0,-1,0,1,0,1,1 },{ 0,0,0,-1,0,1,-1,1 } },
	{ { 0,0,-1,0,1,0,1,-1 },{ 0,0,0,-1,0,1,1,1 },{ 0,0,1,0,-1,0,-1,1 },{ 0,0,0,1,0,-1,-1,-1 } },
	{ { 0,0,1,0,0,-1,-1,-1 },{ 0,0,0,1,1,0,1,-1 },{ 0,0,-1,0,0,1,1,1 },{ 0,0,0,-1,-1,0,-1,1 } },
	{ { 0,0,-1,0,0,-1,1,-1 },{ 0,0,0,-1,1,0,1,1 },{ 0,0,1,0,0,1,-1,1 },{ 0,0,0,1,-1,0,-1,-1 } },
	{ { 0,0,-1,0,0,1,1,0 },{ 0,0,0,-1,-1,0,0,1 },{ 0,0,1,0,0,-1,-1,0 },{ 0,0,0,1,1,0,0,-1 } },
	{ { 0,0,0,-1,0,1,0,2 },{ 0,0,1,0,-1,0,-2,0 },{ 0,0,0,1,0,-1,0,-2 },{ 0,0,-1,0,1,0,2,0 } },
	{ { 0,0,0,1,-1,0,-1,1 },{ 0,0,-1,0,0,-1,-1,-1 },{ 0,0,0,-1,1,-0,1,-1 },{ 0,0,1,0,0,1,1,1 } }
}; // 7种形状(长L| 短L| 反z| 正z| T| 直一| 田格)，4种朝向(上左下右)，8:每相邻的两个分别为x，y

const int rotateBlank[7][4][10] = {
	{ { 1,1,0,0 },{ -1,1,0,0 },{ -1,-1,0,0 },{ 1,-1,0,0 } },
	{ { -1,-1,0,0 },{ 1,-1,0,0 },{ 1,1,0,0 },{ -1,1,0,0 } },
	{ { 1,1,0,0 },{ -1,1,0,0 },{ -1,-1,0,0 },{ 1,-1,0,0 } },
	{ { -1,-1,0,0 },{ 1,-1,0,0 },{ 1,1,0,0 },{ -1,1,0,0 } },
	{ { -1,-1,-1,1,1,1,0,0 },{ -1,-1,-1,1,1,-1,0,0 },{ -1,-1,1,1,1,-1,0,0 },{ -1,1,1,1,1,-1,0,0 } },
	{ { 1,-1,-1,1,-2,1,-1,2,-2,2 } ,{ 1,1,-1,-1,-2,-1,-1,-2,-2,-2 } ,{ -1,1,1,-1,2,-1,1,-2,2,-2 } ,{ -1,-1,1,1,2,1,1,2,2,2 } },
	{ { 0,0 },{ 0,0 } ,{ 0,0 } ,{ 0,0 } }
}; // 旋转的时候需要为空的块相对于旋转中心的坐标
 
const int dx[3] = {-1 ,1, 0};
const int dy[3] = {0, 0, -1};
int dep_pos[2] = {1, 3};

void PRINT ();

class Tetris
{
public:
	int blockType;   // 标记方块类型的序号 0~6
	int blockX;            // 旋转中心的x轴坐标
	int blockY;            // 旋转中心的y轴坐标
	int orientation;       // 标记方块的朝向 0~3
	int(*shape)[8]; // 当前类型方块的形状定义
 
	int color;

	Tetris()
	{ }
	Tetris(int t, int color) : blockType(t), shape(blockShape[t]), color(color)
	{ }
 
	inline Tetris &set(int x = -1, int y = -1, int o = -1)
	{
		blockX = x == -1 ? blockX : x;
		blockY = y == -1 ? blockY : y;
		orientation = o == -1 ? orientation : o;
		return *this;
	}
 
	// 判断当前位置是否合法
	inline bool isValid(int x = -1, int y = -1, int o = -1)
	{
		x = x == -1 ? blockX : x;
		y = y == -1 ? blockY : y;
		o = o == -1 ? orientation : o;
		if (o < 0 || o > 3)
			return false;
 
		int i, tmpX, tmpY;
		for (i = 0; i < 4; i++)
		{
			tmpX = x + shape[o][2 * i];
			tmpY = y + shape[o][2 * i + 1];
			if (tmpX < 1 || tmpX > MAPWIDTH ||
				tmpY < 1 || tmpY > MAPHEIGHT ||
				(gridInfo[color][tmpY] >> tmpX & 1) != 0)
				return false;
		}
		return true;
	}
 
	// 判断是否落地
	inline bool onGround()
	{
		if (isValid(-1, blockY - 1))
			return false;
		return true;
	}
 
	// 将方块放置在场地上
	inline bool place()
	{
		if (!onGround())
			return false;
 
		int i, tmpX, tmpY;
		for (i = 0; i < 4; i++)
		{
			tmpX = blockX + shape[orientation][i << 1];
			tmpY = blockY + shape[orientation][(i << 1) | 1];
			tmpInfo[color][tmpY] |= (1 << tmpX);
		}
		return true;
	}
 
	// 检查能否逆时针旋转自己到o
	inline bool rotation(int o)
	{
		if (o < 0 || o > 3)
			return false;
 
		if (orientation == o)
			return true;
 
		int fromO = orientation;
		int i, blankX, blankY;
		while (true)
		{
			if (!isValid(-1, -1, fromO))
				return false;
 
			if (fromO == o)
				break;
				
			// 检查旋转碰撞
			for (i = 0; i < 5; i++) {
				blankX = blockX + rotateBlank[blockType][fromO][2 * i];
				blankY = blockY + rotateBlank[blockType][fromO][2 * i + 1];
				if (blankX == blockX && blankY == blockY)
					break;
				if (gridInfo[color][blankY] >> blankX & 1)
					return false;
			}
 
			fromO = (fromO + 1) & 3;
		}
		return true;
	}
}Block[100];
 
// 围一圈护城河
void init()
{
	int i;
	for (i = 0; i < MAPHEIGHT + 2; i++)
	{
		gridInfo[1][i] |= 1;
		gridInfo[1][i] |= (1 << MAPWIDTH + 1);
		gridInfo[0][i] |= 1;
		gridInfo[0][i] |= (1 << MAPWIDTH + 1);
	}
	gridInfo[1][0] = gridInfo[1][MAPHEIGHT + 1] = (1 << MAPWIDTH + 2) - 1;
	gridInfo[0][0] = gridInfo[0][MAPHEIGHT + 1] = (1 << MAPWIDTH + 2) - 1;
}
 
namespace Util
{
 
	// 检查能否从场地顶端直接落到当前位置
	/*
	inline bool checkDirectDropTo(int color, int blockType, int x, int y, int o)
	{
		auto &def = blockShape[blockType][o];
		for (; y <= MAPHEIGHT; y++)
			for (int i = 0; i < 4; i++)
			{
				int _x = def[i * 2] + x, _y = def[i * 2 + 1] + y;
				if (_y > MAPHEIGHT)
					continue;
				if (_y < 1 || _x < 1 || _x > MAPWIDTH || gridInfo[color][_y][_x])
					return false;
			}
		return true;
	}
 	*/

	// 消去行
	void eliminate(int color)
	{
		int &count = transCount[color] = 0;
		int i, j, firstFull = 1, hasBonus = 0;
		maxHeight[color] = MAPHEIGHT;

		int full = (1 << MAPWIDTH + 2) - 1;
		int empty = (1 << MAPWIDTH + 1) + 1;

		for (i = 1; i <= MAPHEIGHT; i++)
		{
			int now = gridInfo[color][i] | tmpInfo[color][i];
			tmpInfo[color][i] = 0;
			if (now == full)
			{
				if (firstFull && ++elimCombo[color] >= 3)
				{
					// 奖励行
					trans[color][count] = gridInfo[color][i];
					count++;
					hasBonus = 1;
				}
				firstFull = 0;
				trans[color][count] = gridInfo[color][i];
				gridInfo[color][i] = empty;
				count++;
			}
			else if (now == empty)
			{
				maxHeight[color] = i - 1;
				break;
			}
			else
			{
				gridInfo[color][i - count + hasBonus] = now;
				if (count)
				{
					gridInfo[color][i] = empty;
					tmpInfo[color][i] = 0;
				}
			}
		}
		if (count == 0)
			elimCombo[color] = 0;
		maxHeight[color] -= count - hasBonus;
		elimTotal[color] += elimBonus[count];
	}
 
	// 转移双方消去的行，返回-1表示继续，否则返回输者
	int transfer()
	{
		int color1 = 0, color2 = 1;
		if (transCount[color1] == 0 && transCount[color2] == 0)
			return -1;
		if (transCount[color1] == 0 || transCount[color2] == 0)
		{
			if (transCount[color1] == 0 && transCount[color2] > 0)
				swap(color1, color2);
			int h2;
			maxHeight[color2] = h2 = maxHeight[color2] + transCount[color1];
			if (h2 > MAPHEIGHT)
				return color2;
			int i, j;
 
			for (i = h2; i > transCount[color1]; i--)
				gridInfo[color2][i] = gridInfo[color2][i - transCount[color1]];
 
			for (i = transCount[color1]; i > 0; i--)
				gridInfo[color2][i] = trans[color1][i - 1];
			return -1;
		}
		else
		{
			int h1, h2;
			maxHeight[color1] = h1 = maxHeight[color1] + transCount[color2];//从color1处移动count1去color2
			maxHeight[color2] = h2 = maxHeight[color2] + transCount[color1];
 
			if (h1 > MAPHEIGHT) return color1;
			if (h2 > MAPHEIGHT) return color2;
 
			int i, j;
			for (i = h2; i > transCount[color1]; i--)
				gridInfo[color2][i] = gridInfo[color2][i - transCount[color1]];
 
			for (i = transCount[color1]; i > 0; i--)
				gridInfo[color2][i] = trans[color1][i - 1];
 
			for (i = h1; i > transCount[color2]; i--)
				gridInfo[color1][i] = gridInfo[color1][i - transCount[color2]];
 
			for (i = transCount[color2]; i > 0; i--)
				gridInfo[color1][i] = trans[color2][i - 1];
 
			return -1;
		}
	}
 
	// 打印场地用于调试
	inline void printField()
	{
#ifndef _BOTZONE_ONLINE
		static const char *i2s[] = {
			"~~",
			"~~",
			"  ",
			"[]",
			"##"
		};
		cout << "~~：墙，[]：块，##：新块" << endl;
		for (int y = MAPHEIGHT; y >= 1; y--)
		{
			cout << i2s[0];
			for (int x = 1; x <= MAPWIDTH; x++)
			{
				if (tmpInfo[0][y] >> x & 1) cout << i2s[4];
				else cout << i2s[(gridInfo[0][y] >> x & 1) + 2];
			}
			cout << i2s[0];

			cout << i2s[0];
			for (int x = 1; x <= MAPWIDTH; x++)
			{
				if (tmpInfo[1][y] >> x & 1) cout << i2s[4];
				else cout << i2s[(gridInfo[1][y] >> x & 1) + 2];
			}
			cout << i2s[0];
			cout << endl;
		}
#endif
	}
}

int ab_block;

bool vis[MAPWIDTH + 3][MAPHEIGHT + 3][4];

struct data{
	int x, y, o; data(){}
	double val;
	data(int x, int y, int o): x(x), y(y), o(o){}
	void setVal(double _val)
	{
		val = _val;
	}
	bool operator < (const data &a) const
	{
		return val < a.val;
	}
};

double totalcal;

const double INF = 1e30;

double totalcp, totalrc;

inline void bfs(Tetris t, vector<data> &v)
{
	queue <data> Q;
	memset(vis, 0, sizeof(vis));

	for (int x = 1; x <= MAPWIDTH; x++)
	{
		for (int o = 0; o < 4; o++)
		{
			for (int y = MAPHEIGHT; y; y--)
			{
				auto &def = blockShape[t.blockType][o];
				for (int i = 0; i < 4; i ++)
				{
					int _x = def[i << 1] + x, _y = def[i << 1 | 1] + y;
					if (_y > MAPHEIGHT)
						continue;
					if (_y < 1 || _x < 1 || _x > MAPWIDTH || (gridInfo[t.color][_y] >> _x & 1))
						goto gg;
				}
				if (t.isValid(x, y, o))
				{
					vis[x][y][o] = 1; Q.push(data(x, y, o)); break;
				}
			}
			gg: 1 == 1;
		}
	}

	while (!Q.empty())
	{
		data k = Q.front(); Q.pop();
		for (int i = 0; i < 3; i++)
		{
			int xx = k.x + dx[i], yy = k.y + dy[i];
			if (xx < 1 || xx > MAPWIDTH || yy < 1 || yy > MAPHEIGHT) continue;
			if (vis[xx][yy][k.o] || !t.isValid(xx, yy, k.o)) continue;
			vis[xx][yy][k.o] = 1; Q.push(data(xx, yy, k.o));
		}
		if (vis[k.x][k.y][(k.o + 1) & 3]) continue;
		if (t.set(k.x, k.y, k.o).rotation((k.o + 1) & 3))
			vis[k.x][k.y][(k.o + 1) & 3] = 1, Q.push(data(k.x, k.y, (k.o + 1) & 3));
	}
	for (int x = 1; x <= MAPWIDTH; x++)
		for (int y = 1; y <= MAPHEIGHT; y++)
			for (int o = 0; o < 4; o++)
				if (vis[x][y][o] && t.set(x, y, o).onGround()) v.push_back(data(x, y, o));
}


int redinput[233][4],blueinput[233][4];

inline bool setblock(Tetris t,int finalX, int finalY,int finalO,int enemy)
{
	int Min = 233,Max = -233;
	for (int i = 0; i < 7; i++)
		Min = min(Min,typeCountForColor[enemy][i]),
		Max = max(Max,typeCountForColor[enemy][i]);
	if (Max - Min > 2) return 0;

	vector <data> v; v.clear(); bfs(t, v);
	for (int i = 0; i < v.size(); i++)
		if (v[i].x == finalX && v[i].y == finalY && v[i].o == finalO) return 1;
	return 0;
}

int main()
{
	// 加速输入
 	tim = clock();
	istream::sync_with_stdio(false);
	srand(time(NULL));
	init();

	int blockType,Firstblock,finalX,finalY,finalO;
	int nextTypeForColor[2];

	Tetris red, blue;
	vector <data> v;

	for (int turnID = 0; ; turnID++)
	{
		if (turnID == 0)
		{
			Firstblock = rand() % 7;
			freopen("redinput.txt", "w", stdout);
			printf("1\n%d %d\n", Firstblock, 0);
			red = Tetris(Firstblock, 0);
			typeCountForColor[0][Firstblock]++;
			freopen("blueinput.txt", "w", stdout);
			printf("1\n%d %d\n", Firstblock, 1);
			blue = Tetris(Firstblock, 1);
			typeCountForColor[1][Firstblock]++;
		}
		else
		{
			freopen("redinput.txt", "w", stdout);
			printf("%d\n%d %d\n", turnID + 1, Firstblock, 0);
			for (int i = 0; i < turnID * 2; i++)
				for (int j = 0; j < 4; j++)
					printf("%d%c", redinput[i][j], j == 3 ? '\n' : ' ');
			freopen("blueinput.txt", "w", stdout);
			printf("%d\n%d %d\n", turnID + 1, Firstblock, 1);
			for (int i = 0; i < turnID * 2; i++)
				for (int j = 0; j < 4; j++)
					printf("%d%c", blueinput[i][j], j == 3 ? '\n' : ' ');
		}

		system("" < "redinput.txt" > "redoutput.txt");
		system("" < "blueinput.txt" > "blueoutput.txt");
		int a = turnID * 2, b = turnID * 2 + 1;

		freopen("redoutput.txt", "r", stdin);
		cin >> nextTypeForColor[1] >> finalX >> finalY >> finalO;
		++typeCountForColor[1][nextTypeForColor[1]];
		redinput[a][0] = nextTypeForColor[1];
		redinput[a][1] = finalX; redinput[a][2] = finalY; redinput[a][3] = finalO;
		bool redflag = setblock(red, finalX, finalY, finalO, 1);

		freopen("blueoutput.txt", "r", stdin);
		cin >> nextTypeForColor[0] >> finalX >> finalY >> finalO;
		++typeCountForColor[0][nextTypeForColor[0]];
		blueinput[a][0] = nextTypeForColor[0];
		blueinput[a][1] = finalX; blueinput[a][2] = finalY; blueinput[a][3] = finalO;
		bool blueflag = setblock(blue, finalX, finalY, finalO, 0);

		if (!redflag || !blueflag)
		{
			if (!redflag && !blueflag) puts("-1");
			else if (!redflag) puts("1"); else puts("0");
			break;
		}

		red.set(finalX, finalY, finalO).place();
		blue.set(finalX, finalY, finalO).place();
		Util::eliminate(0);
		Util::eliminate(1);
		int result = Util::transfer();
		if (result != -1)
		{
			freopen("result.txt", "w", stdout);
			printf("%d\n", result ^ 1); break;
		}

		red = Tetris(nextTypeForColor[0], 0);
		blue = Tetris(nextTypeForColor[1], 1);
		for (int i = 0; i < 4; i++)
			redinput[b][i] = blueinput[a][i], blueinput[b][i] = redinput[a][i];
	}
	return 0;
}