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
using namespace std;
 
#define MAPWIDTH 10
#define MAPHEIGHT 20
 
// 我所在队伍的颜色（0为红，1为蓝，仅表示队伍，不分先后）
int currBotColor;
int enemyColor;

double tim;
int MAXDEP;
 
// 先y后x，记录地图状态，0为空，1为以前放置，2为刚刚放置，负数为越界
// （2用于在清行后将最后一步撤销再送给对方）
int temp_girdInfo[110][2][MAPHEIGHT + 2][MAPWIDTH + 2] = { 0 };
int gridInfo[2][MAPHEIGHT + 2][MAPWIDTH + 2] = { 0 };
 
// 代表分别向对方转移的行
int trans[2][6][MAPWIDTH + 2] = { 0 };
 
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
				gridInfo[color][tmpY][tmpX] != 0)
				return false;
		}
		return true;
	}
 
	// 判断是否落地
	inline bool onGround()
	{
		if (isValid() && !isValid(-1, blockY - 1))
			return true;
		return false;
	}
 
	// 将方块放置在场地上
	inline bool place()
	{
		if (!onGround())
			return false;
 
		int i, tmpX, tmpY;
		for (i = 0; i < 4; i++)
		{
			tmpX = blockX + shape[orientation][2 * i];
			tmpY = blockY + shape[orientation][2 * i + 1];
			gridInfo[color][tmpY][tmpX] = 2;
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
				if (gridInfo[color][blankY][blankX] != 0)
					return false;
			}
 
			fromO = (fromO + 1) % 4;
		}
		return true;
	}
};
 
// 围一圈护城河
void init()
{
	int i;
	for (i = 0; i < MAPHEIGHT + 2; i++)
	{
		gridInfo[1][i][0] = gridInfo[1][i][MAPWIDTH + 1] = -2;
		gridInfo[0][i][0] = gridInfo[0][i][MAPWIDTH + 1] = -2;
	}
	for (i = 0; i < MAPWIDTH + 2; i++)
	{
		gridInfo[1][0][i] = gridInfo[1][MAPHEIGHT + 1][i] = -2;
		gridInfo[0][0][i] = gridInfo[0][MAPHEIGHT + 1][i] = -2;
	}
}
 
namespace Util
{
 
	// 检查能否从场地顶端直接落到当前位置
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
 
	// 消去行
	void eliminate(int color)
	{
		int &count = transCount[color] = 0;
		int i, j, emptyFlag, fullFlag, firstFull = 1, hasBonus = 0;
		maxHeight[color] = MAPHEIGHT;
		for (i = 1; i <= MAPHEIGHT; i++)
		{
			emptyFlag = 1;
			fullFlag = 1;
			for (j = 1; j <= MAPWIDTH; j++)
			{
				if (gridInfo[color][i][j] == 0)
					fullFlag = 0;
				else
					emptyFlag = 0;
			}
			if (fullFlag)
			{
				if (firstFull && ++elimCombo[color] >= 3)
				{
					// 奖励行
					for (j = 1; j <= MAPWIDTH; j++)
						trans[color][count][j] = gridInfo[color][i][j] == 1 ? 1 : 0;
					count++;
					hasBonus = 1;
				}
				firstFull = 0;
				for (j = 1; j <= MAPWIDTH; j++)
				{
					// 注意这里只转移以前的块，不包括最后一次落下的块（“撤销最后一步”）
					trans[color][count][j] = gridInfo[color][i][j] == 1 ? 1 : 0;
					gridInfo[color][i][j] = 0;
				}
				count++;
			}
			else if (emptyFlag)
			{
				maxHeight[color] = i - 1;
				break;
			}
			else
				for (j = 1; j <= MAPWIDTH; j++)
				{
					gridInfo[color][i - count + hasBonus][j] =
						gridInfo[color][i][j] > 0 ? 1 : gridInfo[color][i][j];
					if (count)
						gridInfo[color][i][j] = 0;
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
				for (j = 1; j <= MAPWIDTH; j++)
					gridInfo[color2][i][j] = gridInfo[color2][i - transCount[color1]][j];
 
			for (i = transCount[color1]; i > 0; i--)
				for (j = 1; j <= MAPWIDTH; j++)
					gridInfo[color2][i][j] = trans[color1][i - 1][j];
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
				for (j = 1; j <= MAPWIDTH; j++)
					gridInfo[color2][i][j] = gridInfo[color2][i - transCount[color1]][j];
 
			for (i = transCount[color1]; i > 0; i--)
				for (j = 1; j <= MAPWIDTH; j++)
					gridInfo[color2][i][j] = trans[color1][i - 1][j];
 
			for (i = h1; i > transCount[color2]; i--)
				for (j = 1; j <= MAPWIDTH; j++)
					gridInfo[color1][i][j] = gridInfo[color1][i - transCount[color2]][j];
 
			for (i = transCount[color2]; i > 0; i--)
				for (j = 1; j <= MAPWIDTH; j++)
					gridInfo[color1][i][j] = trans[color2][i - 1][j];
 
			return -1;
		}
	}
 
	// 颜色方还能否继续游戏
	inline bool canPut(int color, int blockType)
	{
		Tetris t(blockType, color);
		for (int y = MAPHEIGHT; y >= 1; y--)
			for (int x = 1; x <= MAPWIDTH; x++)
				for (int o = 0; o < 4; o++)
				{
					t.set(x, y, o);
					if (t.isValid() && checkDirectDropTo(color, blockType, x, y, o))
						return true;
				}
		return false;
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
		for (int y = MAPHEIGHT + 1; y >= 0; y--)
		{
			for (int x = 0; x <= MAPWIDTH + 1; x++)
				cout << i2s[gridInfo[0][y][x] + 2];
			for (int x = 0; x <= MAPWIDTH + 1; x++)
				cout << i2s[gridInfo[1][y][x] + 2];
			cout << endl;
		}
#endif
	}
}

struct Result
{
	int blockForEnemy, finalX, finalY, finalO;
	Result(int blockForEnemy, int finalX, int finalY, int finalO) :
		blockForEnemy(blockForEnemy), finalX(finalX), finalY(finalY), finalO(finalO) {}
	Result() {}
}ans, tmp;

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

const double Height[21] = {0.0, 0.2, 0.4, 0.7, 1.1, 1.5, 1.9, 2.5, 3.1, 3.8, 4.7, 5.9, 7.0, 8.2, 9.5, 11.2, 13.5, 16.5, 20.0, 24.5, 30.0};

int GetLandingHeight(int player) 
{
  return Height[maxHeight[player]];
}

int GetRowsRemoved(int player)
{
	return elimTotal[player];
	/*
	int ret = 0;
	for (int i = 1; i <= MAPHEIGHT; i++)
	{
		int fullFlag = 1;
		for (int j = 1; j <= MAPWIDTH; j++)
		{
			if (gridInfo[player][i][j] == 0)
				fullFlag = 0;
		}
		if (fullFlag)
		{
			ret ++;
		}
	}
	return ret;	
	*/
}

int GetRowTransitions(int player) 
{
	int ret = 0;
	int last_bit = 1;

	for (int i = 1; i <= MAPHEIGHT; i ++) 
	{
		int bit;
		for (int j = 1; j <= MAPWIDTH; j ++)
		{
			bit = (gridInfo[player][i][j] > 0);
			if (bit != last_bit)
			{
				++ ret;
			}
			last_bit = bit;
		}
		if (bit == 0) ++ ret;
		last_bit = 1;
	}
	return ret;
}

/**
 * The total number of column transitions.
 * A column transition occurs when an empty cell is adjacent to a filled cell
 * on the same row and vice versa.
 */

int GetColumnTransitions(int player) {
	int ret = 0;
	int last_bit = 1;

	for (int i = 1; i <= MAPWIDTH; i ++)
	{
		int bit;
		for (int j = 1; j <= MAPHEIGHT; j ++)
		{
			bit = (gridInfo[player][j][i] > 0);
			if (bit != last_bit)
			{
				++ ret;
			}
			last_bit = bit;
		}
		if (bit == 0) ++ ret;
		last_bit = 1;
	}
	return ret;
}

int GetNumberOfHoles(int player) 
{
	int ret = 0;
	int row_holes = 0x0000;
	int previous_row = 0;
	for (int i = 1; i <= MAPWIDTH; i ++)
	{
		previous_row = (previous_row << 1) | (gridInfo[player][MAPHEIGHT][i] > 0);
	}

	for (int i = MAPHEIGHT - 1; i >= 1; i --)
	{
		int row = 0;
		for (int j = 1; j <= MAPWIDTH; j ++)
			row = (row << 1) | (gridInfo[player][i][j] > 0);
		row_holes = ~row & (previous_row | row_holes);

		for (int j = 0; j < MAPWIDTH; j ++)
			ret += ((row_holes >> j) & 1);

		previous_row = row;
	}

	return ret;
}

int GetWellSums(int player) 
{
	int well_sums = 0;
	// Check for well cells in the "inner columns" of the board.
	// "Inner columns" are the columns that aren't touching the edge of the board.
	int board[21];
	for (int i = 1; i <= MAPHEIGHT; i ++)
	{
		board[i] = 0;
		for (int j = 1; j <= MAPWIDTH; j ++)
			board[i] = (board[i] << 1) | (gridInfo[player][i][j] > 0);
	}
	for (int i = 1; i < MAPWIDTH - 1; ++i) 
	{
		for (int j = MAPHEIGHT - 1; j >= 0; --j) 
		{
			if ((((board[j] >> i) & 1) == 0) && 
			(((board[j] >> (i - 1)) & 1) == 1) &&
			(((board[j] >> (i + 1)) & 1) == 1)) 
			{

			// Found well cell, count it + the number of empty cells below it.
				++ well_sums;

				for (int k = j - 1; k >= 0; --k) 
				{
					if (((board[k] >> i) & 1) == 0) 
					{
						++well_sums;
					} 
					else 
					{
						break;
					}
				}
			}
		}
	}

	// Check for well cells in the leftmost column of the board.
	for (int j = MAPWIDTH - 1; j >= 0; --j) 
	{
		if ((((board[j] >> 0) & 1) == 0) && 
		(((board[j] >> (0 + 1)) & 1) == 1)) 
		{

			// Found well cell, count it + the number of empty cells below it.
			++well_sums;

			for (int k = j - 1; k >= 0; --k) 
			{
				if (((board[k] >> 0) & 1) == 0) 
				{
					++well_sums;
				} 
				else 
				{
					break;
				}
			}
		}
	}

	// Check for well cells in the rightmost column of the board.
	for (int j = MAPWIDTH - 1; j >= 0; j --) 
	{
		if ((((board[j] >> (MAPHEIGHT - 1)) & 1) == 0) && 
		(((board[j] >> (MAPHEIGHT - 2)) & 1) == 1)) 
		{
			// Found well cell, count it + the number of empty cells below it.

			++well_sums;
			for (int k = j - 1; k >= 0; --k) 
			{
				if (((board[k] >> (MAPHEIGHT - 1)) & 1) == 0) 
				{
					++well_sums;
				}
				else
				{
					break;
				}
			}
		}
	}

	return well_sums;
}

Tetris Block[100];

double calc (int player)
{
	double player_score[2];
	for (int i = player; i < player + 1; i ++)
	{
		player_score[i] = 
			GetLandingHeight(i) * -4.500158825082766 +
			GetRowsRemoved(i) * 3.4181268101392694 +
			GetRowTransitions(i) * -3.2178882868487753 +
			GetColumnTransitions(i) * -9.348695305445199 +
			GetNumberOfHoles(i) * -7.899265427351652 +
			GetWellSums(i) * -3.3855972247263626;
	}
	return player_score[player];
};

const double INF = 1e30;

/*
bool judge (int player, Tetris &block)
{
	for (int y = 1; y <= MAPHEIGHT; y++)
		for (int x = 1; x <= MAPWIDTH; x++)
			for (int o = 0; o < 4; o++)
			{
				if (block.set(x, y, o).onGround() &&
					Util::checkDirectDropTo(player, block.blockType, x, y, o))
						return false;
			}
	return true;
}
*/

void copy (int depth)
{
	/*
		todo
		gridInfo需要常数优化
	*/
	memcpy(temp_maxHeight[depth], maxHeight, sizeof(maxHeight));
	memcpy(temp_elimCombo[depth], elimCombo, sizeof(elimCombo));
	memcpy(temp_elimTotal[depth], elimTotal, sizeof(elimTotal));
	memcpy(temp_girdInfo[depth], gridInfo, sizeof(gridInfo));
}

void recover (int depth)
{
	memcpy(maxHeight, temp_maxHeight[depth], sizeof(maxHeight));
	memcpy(elimCombo, temp_elimCombo[depth], sizeof(elimCombo));
	memcpy(elimTotal, temp_elimTotal[depth], sizeof(elimTotal));
	memcpy(gridInfo, temp_girdInfo[depth], sizeof(gridInfo));
}

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
					if (_y < 1 || _x < 1 || _x > MAPWIDTH || (gridInfo[t.color][_y][_x] > 0))
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

double alphabeta (int dep, double alpha, double beta, int player)
{
	// printf("%d %.2f %.2f %d\n", dep, alpha, beta, player);
	if ((clock() - tim) / CLOCKS_PER_SEC > TIME_LIMIT) return -INF;

	if (dep & 1)
	{
		double ret = -INF;
		vector <data> v;
		bfs (Block[dep >> 1], v);
		if (v.empty())
		{
			return - 15000 + dep;
		}

		for (int i = 0; i < v.size(); i ++)
		{
			copy(dep);
			Tetris block = Block[dep >> 1];
			block.set(v[i].x, v[i].y, v[i].o).place();
			Util::eliminate(player);
			v[i].val = calc(player);
			recover(dep);
		}
		sort(v.begin(), v.end());
		reverse(v.begin(), v.end());
		int sz = (int)v.size();

		for (int i = 0; i < sz; i ++)
		{
			copy(dep);
			Tetris block = Block[dep >> 1];
			block.set(v[i].x, v[i].y, v[i].o).place();
			
			Util::eliminate(player);
			ret = max(ret, alphabeta(dep + 1, alpha, beta, player ^ 1));
			if (ret > alpha) 
			{
				alpha = ret;
				if (dep == 1)
				{
					tmp = Result(ab_block, v[i].x, v[i].y, v[i].o);
				}
			}
			recover(dep);
			if (beta <= alpha) goto goodbye;
		}
	goodbye:
		return ret;
	}
	else
	{
		if (dep == MAXDEP)
			return calc(player ^ 1);
		double ret = INF;
		vector <int> enemyBlocksType;
		int maxCount = 0, minCount = 99;
		for (int i = 0; i < 7; i++)
		{
			if (typeCountForColor[player ^ 1][i] > maxCount)
				maxCount = typeCountForColor[player ^ 1][i];
			if (typeCountForColor[player ^ 1][i] < minCount)
				minCount = typeCountForColor[player ^ 1][i];
		}
		if (maxCount - minCount == 2)
		{
			// 危险，找一个不是最大的块给对方吧
			for (int i = 0; i < 7; i ++)
				if (typeCountForColor[player ^ 1][i] != maxCount)
					enemyBlocksType.push_back(i);
		}
		else
		{
			for (int i = 0; i < 7; i ++)
				enemyBlocksType.push_back(i);
		}
		for (int i = 0; i < enemyBlocksType.size(); i ++)
		{
			typeCountForColor[player ^ 1][enemyBlocksType[i]] ++;
			Block[dep >> 1] = Tetris(enemyBlocksType[i], player ^ 1);
			ret = min(ret, alphabeta(dep + 1, alpha, beta, player ^ 1));
			typeCountForColor[player ^ 1][enemyBlocksType[i]] --;
			if (ret < beta)
			{
				beta = ret;
				if (dep == 2)
				{
					ab_block = enemyBlocksType[i];
				}
			}
			if (beta <= alpha) goto goodbye2;
		}
	goodbye2:
		return ret;
	}
}

double calc2 (int player)
{
	double player_score[2];
	for (int i = 0; i < 2; i ++)
	{
		player_score[i] = 
			GetLandingHeight(i) * -4.500158825082766 +
			GetRowsRemoved(i) * 3.4181268101392694 +
			GetRowTransitions(i) * -3.2178882868487753 +
			GetColumnTransitions(i) * -9.348695305445199 +
			GetNumberOfHoles(i) * -7.899265427351652 +
			GetWellSums(i) * -3.3855972247263626;
	}
	return player_score[player] - player_score[player ^ 1];
};

double alphabeta2 (int dep, double alpha, double beta, int player)
{

// 卡tmd时
	if ((clock() - tim) / CLOCKS_PER_SEC > TIME_LIMIT) return -INF;

// 判断当前是否已经赢了 or 输了
	if (dep % 2 == 0)
	{
		Util::eliminate(player);
		Util::eliminate(player ^ 1);
		int result = Util::transfer();
		if (result != -1)
		{
			if (result == player)
			{
				return -15000 + dep;
			}
			else
			{
				return +15000 - dep;
			}
		}
		if (dep == MAXDEP)
		{
			double ret = calc2(player);
			//cout << ret << endl;
 			return ret;
		}
	}

// 判断当前是否是必胜 or 必败态
	vector <data> v;
	bfs (Block[dep - dep_pos[player]], v);
	vector <data> v2;
	bfs (Block[(dep ^ 1) - dep_pos[player ^ 1]], v2);
	if (v.empty() && v2.empty())
	{
		return 0;
	}
	if (v.empty())
	{
		return - 15000 + dep;
	}
	if (v2.empty())
	{
		return 15000 - dep;
	}

// 预处理可给对手的方块
	vector <int> enemyBlocksType;
	int maxCount = 0, minCount = 99;
	for (int i = 0; i < 7; i++)
	{
		if (typeCountForColor[player ^ 1][i] > maxCount)
			maxCount = typeCountForColor[player ^ 1][i];
		if (typeCountForColor[player ^ 1][i] < minCount)
			minCount = typeCountForColor[player ^ 1][i];
	}
	if (maxCount - minCount == 2)
	{
		for (int i = 0; i < 7; i ++)
			if (typeCountForColor[player ^ 1][i] != maxCount)
				enemyBlocksType.push_back(i);
	}
	else
	{
		for (int i = 0; i < 7; i ++)
			enemyBlocksType.push_back(i);
	}

// alpha-beta决策

	double ret;
	if (dep % 2 == 0)
		ret = -INF;
	else
		ret = INF;

	for (int k = 0; k < enemyBlocksType.size(); k ++)
	{
		for (int i = 0; i < v.size(); i ++)
		{
			copy(dep);
			Block[dep] = Tetris(enemyBlocksType[k], player ^ 1);
			Tetris block = Block[dep - dep_pos[player]];
			block.set(v[i].x, v[i].y, v[i].o).place();

			double new_alphabeta;

			new_alphabeta = alphabeta2(dep + 1, alpha, beta, player ^ 1);
			if (dep % 2 == 0)
			{
				ret = max(ret, new_alphabeta);
				if (ret > alpha) 
				{
					alpha = ret;
					if (dep == 2)
					{
						Util::printField();
						tmp = Result(enemyBlocksType[k], v[i].x, v[i].y, v[i].o);
					}
				}
			}
			else
			{
				ret = min(ret, new_alphabeta);
				if (ret < beta)
				{
					beta = ret;
				}
			}
			
			recover(dep);
			if (beta <= alpha) goto goodbye;
		}
	}
goodbye:
	return ret;
}

void PRINT ()
{
	cout << ans.blockForEnemy << " " << ans.finalX << " " << ans.finalY << " " << ans.finalO << endl;
}

int main()
{
#ifdef MEKTPOY
	freopen("in.txt", "r", stdin);
#endif
	// 加速输入
 	tim = clock();
	istream::sync_with_stdio(false);
	srand(time(NULL));
	init();

	int turnID, blockType;
	int nextTypeForColor[2];
	cin >> turnID;
 
	// 先读入第一回合，得到自己的颜色
	// 双方的第一块肯定是一样的
	cin >> blockType >> currBotColor;
	enemyColor = 1 - currBotColor;
	nextTypeForColor[0] = blockType;
	nextTypeForColor[1] = blockType;
	typeCountForColor[0][blockType]++;
	typeCountForColor[1][blockType]++;
 
	// 然后分析以前每回合的输入输出，并恢复状态
	// 循环中，color 表示当前这一行是 color 的行为
	// 平台保证所有输入都是合法输入
	for (int i = 1; i < turnID; i++)
	{
		int currTypeForColor[2] = { nextTypeForColor[0], nextTypeForColor[1] };
		int x, y, o;
		// 根据这些输入输出逐渐恢复状态到当前回合
 
		// 先读自己的输出，也就是自己的行为
		// 自己的输出是自己的最后一步
		// 然后模拟最后一步放置块
		cin >> blockType >> x >> y >> o;
 
		// 我当时把上一块落到了 x y o！
		Tetris myBlock(currTypeForColor[currBotColor], currBotColor);
		myBlock.set(x, y, o).place();
 
		// 我给对方什么块来着？
		typeCountForColor[enemyColor][blockType]++;
		nextTypeForColor[enemyColor] = blockType;
 
		// 然后读自己的输入，也就是对方的行为
		// 裁判给自己的输入是对方的最后一步
		cin >> blockType >> x >> y >> o;
 
		// 对方当时把上一块落到了 x y o！
		Tetris enemyBlock(currTypeForColor[enemyColor], enemyColor);
		enemyBlock.set(x, y, o).place();
 
		// 对方给我什么块来着？
		typeCountForColor[currBotColor][blockType]++;
		nextTypeForColor[currBotColor] = blockType;
 
		// 检查消去
		Util::eliminate(0);
		Util::eliminate(1);
 
		// 进行转移
		Util::transfer();
	}

	// 做出决策（你只需修改以下部分）
 
	// 遇事不决先输出（平台上编译不会输出）
	Util::printField();
	// 贪心决策
	// 从下往上以各种姿态找到第一个位置，要求能够直着落下


	if (maxHeight[0] < 19 && maxHeight[1] < 19, 1)
	{
		TIME_LIMIT = 0.475;
		Block[0] = Tetris(nextTypeForColor[currBotColor], currBotColor);
		for (MAXDEP = 2; MAXDEP <= 50; MAXDEP += 2)
		{
			tmp = Result(-1, -1, -1, -1);
			ab_block = -1;
			alphabeta(1, -INF, INF, currBotColor);
			if ((clock() - tim) / CLOCKS_PER_SEC < TIME_LIMIT)
			{
				ans = tmp;
			}
			else
			{
				break;
			}
		}

		int first_dep = MAXDEP;

		TIME_LIMIT = 0.95;
		Block[0] = Tetris(nextTypeForColor[enemyColor], enemyColor);
		for (MAXDEP = 2; MAXDEP <= 50; MAXDEP += 2)
		{
			tmp = Result(-1, -1, -1, -1);
			ab_block = -1;
			alphabeta(1, -INF, INF, enemyColor);
			if ((clock() - tim) / CLOCKS_PER_SEC < TIME_LIMIT)
			{
				ans.blockForEnemy = tmp.blockForEnemy;
			}
			else
			{
				break;
			}
		}

		PRINT();

		int second_dep = MAXDEP;

	#ifdef MEKTPOY
		cout << first_dep << " " << second_dep << endl;
	#endif
	}
	else
	{
 		if (currBotColor == 1) swap(dep_pos[0], dep_pos[1]);
		TIME_LIMIT = 0.95;
		Block[0] = Tetris(nextTypeForColor[enemyColor], enemyColor);
		Block[1] = Tetris(nextTypeForColor[currBotColor], currBotColor);
		for (MAXDEP = 4; MAXDEP <= 50; MAXDEP += 2)
		{
			tmp = Result(-1, -1, -1, -1);
			alphabeta2(2, -INF, INF, currBotColor);
			if ((clock() - tim) / CLOCKS_PER_SEC < TIME_LIMIT) 
				ans = tmp;
			else
				break;
		}
		PRINT();
	#ifdef MEKTPOY
		cout << MAXDEP << endl;
	#endif
	}
	//block.set(finalX, finalY, finalO).place();
	//calc2(block, currBotColor);
	//block.set(finalX, finalY, finalO).place2();
	// 再看看给对方什么好
	
	// 决策结束，输出结果（你只需修改以上部分）
 
 
	return 0;
}