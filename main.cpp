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
#include <ctime>
using namespace std;
 
#define MAPWIDTH 10
#define MAPHEIGHT 20
 
// 我所在队伍的颜色（0为红，1为蓝，仅表示队伍，不分先后）
int currBotColor;
int enemyColor;
 
// 先y后x，记录地图状态，0为空，1为以前放置，2为刚刚放置，负数为越界
// （2用于在清行后将最后一步撤销再送给对方）
int gridInfo[2][MAPHEIGHT + 2][MAPWIDTH + 2] = { 0 };
 
// 代表分别向对方转移的行
int trans[2][6][MAPWIDTH + 2] = { 0 };
 
// 转移行数
int transCount[2] = { 0 };
 
// 运行eliminate后的当前高度
int maxHeight[2] = { 0 };
 
// 总消去行数的分数之和
int elimTotal[2] = { 0 };

// 连续几回合发生过消去了
int elimCombo[2] = { 0 };
 
// 一次性消去行数对应分数
const int elimBonus[] = { 0, 1, 3, 5, 7 };
 
// 给对应玩家的各类块的数目总计
int typeCountForColor[2][7] = { 0 };
 
const int blockShape[7][4][8] = {
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
 
class Tetris
{
public:
	const int blockType;   // 标记方块类型的序号 0~6
	int blockX;            // 旋转中心的x轴坐标
	int blockY;            // 旋转中心的y轴坐标
	int orientation;       // 标记方块的朝向 0~3
	const int(*shape)[8]; // 当前类型方块的形状定义
 
	int color;
 
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

	inline bool place2()
	{
		int i, tmpX, tmpY;
		for (i = 0; i < 4; i++)
		{
			tmpX = blockX + shape[orientation][2 * i];
			tmpY = blockY + shape[orientation][2 * i + 1];
			gridInfo[color][tmpY][tmpX] = 0;
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

int GetLandingHeight(Tetris now) 
{
  return now.blockY;
}

int GetRowsRemoved(int player)
{
	int ret = 0;
	for (int i = 1; i <= MAPHEIGHT; i++)
	{
		int emptyFlag = 1;
		int fullFlag = 1;
		for (int j = 1; j <= MAPWIDTH; j++)
		{
			if (gridInfo[player][i][j] == 0)
				fullFlag = 0;
			else
				emptyFlag = 0;
		}
		if (fullFlag)
		{
			ret ++;
		}
	}
	return ret;
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

double calc (Tetris &block, int &player)
{
	return GetLandingHeight(block) * -4.500158825082766 +
		GetRowsRemoved(player) * 3.4181268101392694 +
		GetRowTransitions(player) * -3.2178882868487753 +
		GetColumnTransitions(player) * -9.348695305445199 +
		GetNumberOfHoles(player) * -7.899265427351652 +
		GetWellSums(player) * -3.3855972247263626;
};

void Print (Tetris &block, int &player)
{
	printf("%.2f %.2f %.2f %.2f %.2f %.2f\n", GetLandingHeight(block) * -4.500158825082766,
		GetRowsRemoved(player) * 3.4181268101392694,
		GetRowTransitions(player) * -3.2178882868487753,
		GetColumnTransitions(player) * -9.348695305445199,
		GetNumberOfHoles(player) * -7.899265427351652,
		GetWellSums(player) * -3.3855972247263626);
};

int alphabeta (int dep, int alpha, int beta, int player)
{
	int cnt = 0;
	if (judge (player, Block[dep - dep_pos[player]]))
	{
		if (dep == MAXDEP) bo = 1;
		if (player == currBotColor) 
		{
			if (judge (player ^ 1, Block[dep - dep_pos[player ^ 1]])) return 0;
			return - 15000 - dep;
		}
		else return 15000 + dep;
	}

	if (dep == MAXDEP)
		return bo = 1, calc(num);

	int ret;
	if (player == currBotColor)
		ret = -INF;
	else
		ret = INF;

	Tetris block = Block[dep - dep_pos[player]];
	for (int y = 1; y <= MAPHEIGHT; y++)
		for (int x = 1; x <= MAPWIDTH; x++)
			for (int o = 0; o < 4; o++)
			{
				if (block.set(x, y, o).onGround() &&
					Util::checkDirectDropTo(player, block.blockType, x, y, o))
				{
					block.set(x, y, o).place();
					if (player == currBotColor)
						ret = max(ret, alphabeta(dep + 1, alpha, beta, player ^ 1));
					else
						ret = min(ret, alphabeta(dep + 1, alpha, beta, player ^ 1));
					block.set(x, y, o).place2();
					if (beta <= alpha) goto goodbye;
				}
			}
goodbye:
	return ret;
/*
	if (player == 0)
	{
		int Ret = - INF;
		for (int k = 0; k < cnt; k ++)
		{
			int i = id[k];
			int px = x+dx[i], py = y+dy[i];
			if (px > n || py > m || px < 1 || py < 1) continue;
			if (can[px][py] > num) continue;
			int tmpo = can[px][py];
			can[px][py] = INF;
			snake[player].push_front (point (px, py));
			Ret = max (Ret, alphabeta (dep - 1, alpha, beta, 1, num));
			if (Ret > alpha && Ret < 20000)
			{
				alpha = Ret;
				if (dep == ID * 2) 
				{
					tmp = i;
					if (alpha >= 15000)
					{
						ans = i;
						PRINT (alpha);
					} 
				}
			}
			can[px][py] = tmpo;
			snake[player].pop_front ();
			if (beta <= alpha) break;
		}
		return Ret;
	}
	else
	{
		int ret = 2147483647;
		for (int k = 0; k < cnt; k ++)
		{
			int i = id[k];
			int px = x + dx[i], py = y + dy[i];
			if (px > n || py > m || px < 1 || py < 1) continue;
			if (can[px][py] > num) continue;
			int tmpo = can[px][py];
			can[px][py] = 2147483647;
			snake[player].push_front (point (px, py));
			ret = min (ret, alphabeta (dep-1, alpha, beta, 0, num+1));
			if (ret < beta) beta = ret;
			can[px][py] = tmpo;
			snake[player].pop_front ();
			if (beta <= alpha) break;
		}
		return ret;
	}
*/
}

int main()
{
#ifdef MEKTPOY
	freopen("in.txt", "r", stdin);
#endif
	// 加速输入
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
 
 
	int blockForEnemy, finalX, finalY, finalO;
 
	// 做出决策（你只需修改以下部分）
 
	// 遇事不决先输出（平台上编译不会输出）
	Util::printField();
 
 	double MAX = -1e30;
	// 贪心决策
	// 从下往上以各种姿态找到第一个位置，要求能够直着落下
	Tetris block(nextTypeForColor[currBotColor], currBotColor);
	for (int y = 1; y <= MAPHEIGHT; y++)
		for (int x = 1; x <= MAPWIDTH; x++)
			for (int o = 0; o < 4; o++)
			{
				if (block.set(x, y, o).onGround() &&
					Util::checkDirectDropTo(currBotColor, block.blockType, x, y, o))
				{
					block.set(x, y, o).place();
					double val = calc(block, currBotColor);
					//printf("%d %d %d %.2f\n", x, y, o, val);
					if (val > MAX)
					{
						MAX = val;
						finalX = x;
						finalY = y;
						finalO = o;
					}
					block.set(x, y, o).place2();
				}
			}
	//block.set(finalX, finalY, finalO).place();
	//calc2(block, currBotColor);
	//block.set(finalX, finalY, finalO).place2();
	// 再看看给对方什么好
	vector <int> enemyBlocks;
	int maxCount = 0, minCount = 99;
	for (int i = 0; i < 7; i++)
	{
		if (typeCountForColor[enemyColor][i] > maxCount)
			maxCount = typeCountForColor[enemyColor][i];
		if (typeCountForColor[enemyColor][i] < minCount)
			minCount = typeCountForColor[enemyColor][i];
	}
	if (maxCount - minCount == 2)
	{
		// 危险，找一个不是最大的块给对方吧
		for (blockForEnemy = 0; blockForEnemy < 7; blockForEnemy++)
			if (typeCountForColor[enemyColor][blockForEnemy] != maxCount)
				enemyBlocks.push_back(blockForEnemy);
	}
	else
	{
		for (int i = 0; i < 7; i ++)
			enemyBlocks.push_back(i);
	}
	double MIN = 1e30;
	for (int k = 0; k < enemyBlocks.size(); k ++)
	{
		double MAX = -1e30;
		Tetris enemyBlock(enemyBlocks[k], enemyColor);
		for (int y = 1; y <= MAPHEIGHT; y++)
			for (int x = 1; x <= MAPWIDTH; x++)
				for (int o = 0; o < 4; o++)
				{
					if (block.set(x, y, o).onGround() &&
						Util::checkDirectDropTo(enemyColor, block.blockType, x, y, o))
					{
						block.set(x, y, o).place();
						double val = calc(block, enemyColor);
						if (val > MAX)
						{
							MAX = val;
						}
						block.set(x, y, o).place2();
					}
				}
		//printf("%d %.2f\n", enemyBlocks[k], MAX);
		if (MAX < MIN)
		{
			MIN = MAX;
			blockForEnemy = enemyBlocks[k];
		}
	}
	// 决策结束，输出结果（你只需修改以上部分）
 
	cout << blockForEnemy << " " << finalX << " " << finalY << " " << finalO << endl;
 
	return 0;
}
