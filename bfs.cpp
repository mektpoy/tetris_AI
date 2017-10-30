

const int dx[3] = {-1 ,1, 0};
const int dy[3] = {0, 0, 1};

bool vis[MAPWIDTH + 3][MAPHEIGHT + 3][4];

struct data
{
	int x,y,o; data(){}
	data(int x, int y, int o): x(x), y(y), o(o){}
};


class Tetris
{
public:
	int blockType;//标记方块类型的序号 0~6
	int blockX;     //旋转中心的x轴坐标
	int blockY;     //旋转中心的y轴坐标
	int orientation; //标记方块的朝向 0~3

	Tetris(int t, int x, int y, int o)
	{
		blockType = t;
		blockX = x;
		blockY = y;
		orientation = o;
	}
	void set(int t = -1, int x = -1, int y = -1, int o = -1)
	{
		blockType = t == -1 ? blockType : t;
		blockX = x == -1 ? blockX : x;
		blockY = y == -1 ? blockY : y;
		orientation = o == -1 ? orientation : o;
	}
	bool isValid(int color, int x = -1, int y = -1, int o = -1) const
	{
		x = x == -1 ? blockX : x;
		y = y == -1 ? blockY : y;
		o = o == -1 ? orientation : o;
		if (o < 0 || o > 3 || blockType < 0 || blockType > 6)
			return false;

		int i, tmpX, tmpY;
		for (i = 0; i < 4; i++)
		{
			tmpX = x + deltaBlock[blockType][o][2 * i];
			tmpY = y + deltaBlock[blockType][o][2 * i + 1];
			if (tmpX < 1 || tmpX > MAPWIDTH || tmpY < 1 || tmpY > MAPHEIGHT)
				return false;
			if (gridInfo[color][tmpY][tmpX] != 0)
				return false;
		}
		return true;
	}
	bool checkRotationBlank(int color) const
	{
		for (int i = 0; i < 5; i++)
		{
			int blankX = blockX + rotateBlank[blockType][orientation][2 * i];
			int blankY = blockY + rotateBlank[blockType][orientation][2 * i + 1];
			if (blankX == blockX && blankY == blockY)
				break;
			if (gridInfo[color][blankY][blankX] != 0)
				return false;
		}
		return true;
	}


	// 已改
	bool isGround(int color) const
	{
		if (isValid(color) && !isValid(color, -1, blockY - 1))
			return true;
		return false;
	}

	// 已改 rotation会测量方块是否可以从befO逆时针旋转到o
	bool rotation(int color, int x = -1, int y = -1, int befO = -1, int o = -1)
	{
		x = x == -1 ? blockX : x;
		y = y == -1 ? blockY : y;
		befO = befO == -1 ? orientation : befO;
		o = o == -1 ? orientation : o;
		if (o < 0 || o > 3 || blockType < 0 || blockType > 6)
			return false;

		if (befO == o)
			return true;

		int i, blankX, blankY;
		while (true)
		{
			if (!isValid(color, x, y, befO))
				return false;

			if (befO == o)
				break;

			// 检查旋转碰撞
			for (i = 0; i < 5; i++) {
				blankX = blockX + rotateBlank[blockType][befO][2 * i];
				blankY = blockY + rotateBlank[blockType][befO][2 * i + 1];
				if (blankX == blockX && blankY == blockY)
					break;
				if (gridInfo[color][blankY][blankX] != 0)
					return false;
			}

			befO = (befO + 1) % 4;
		}
		return true;
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
}

inline void bfs(int type, int o, int player, vector<data> &v)
{
	queue <data> Q; Q.clear();
	memset(vis, 0, sizeof(vis));
	for (int x = 1; x <= MAPWIDTH; x++)
		for (int y = MAPHEIGHT; y; y--)
				if (Util::checkDirectDropTo(player, type, x, y, o))
				{
					vis[x][y][o] = 1; Q.push(data(x, y, o)); break;
				}
	while (!Q.empty())
	{
		data k = Q.front(); Q.pop();
		for (int i = 0; i < 3; i++)
		{
			int xx = k.x + dx[i], yy = k.y + dy[i];
			if (xx < 1 || xx > MAPWIDTH || yy < 1 || yy > MAPHEIGHT) continue;
			if (vis[xx][yy][o] || !Tetris2.set(type, xx, yy, o).isValid(player)) continue;
			vis[xx][yy][o] = 1; Q.push(data(xx, yy, o));
		}
		for (int o = 0; o < 4; o++)
		{
			if (vis[k.x][k.y][o]) continue;
			if (Tetris.set(type, k.x, k.y, k.o).rotation(player, -1, -1, -1,o))
				vis[k.x][k.y][o] = 1, Q.push(data(k.x, k.y, k.o));
		}
	}
	for (int x = 1; x <= MAPWIDTH; x++)
		for (int y = 1; y <= MAPHEIGHT; y++)
			for (int o = 0; o < 4; o++)
				if (vis[x][y][o] && Tetris2.set(x, y, o).isGround(player)) v.push_back(data(x, y, o));
}