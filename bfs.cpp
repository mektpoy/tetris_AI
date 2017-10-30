

const int dx[3] = {-1 ,1, 0};
const int dy[3] = {0, 0, -1};

bool vis[MAPWIDTH + 3][MAPHEIGHT + 3][4];

struct data
{
	int x,y,o; data(){}
	data(int x, int y, int o): x(x), y(y), o(o){}
};

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

inline void bfs(Tetris t, vector<data> &v)
{
	queue <data> Q; Q.clear();
	memset(vis, 0, sizeof(vis));
	for (int x = 1; x <= MAPWIDTH; x++)
	{
		bool flag[4]; int cnt = 0;
		memset(flag, 0, sizeof (flag));
		for (int y = MAPHEIGHT; y; y--)
		{
			for (int o = 0; o < 4; o++)
				if (!flag[o] && Util::checkDirectDropTo(t.color, t.blockType, x, y, o))
				{
					vis[x][y][o] = 1; Q.push(data(x, y, o)); flag[o] = 1; ++cnt;
				}
			if (cnt == 4) break;
		}
	}

	while (!Q.empty())
	{
		data k = Q.front(); Q.pop();
		for (int i = 0; i < 3; i++)
		{
			int xx = k.x + dx[i], yy = k.y + dy[i];
			if (xx < 1 || xx > MAPWIDTH || yy < 1 || yy > MAPHEIGHT) continue;
			if (vis[xx][yy][o] || !t.isValid(xx, yy, o)) continue;
			vis[xx][yy][o] = 1; Q.push(data(xx, yy, o));
		}
		for (int o = 0; o < 4; o++)
		{
			if (vis[k.x][k.y][o]) continue;
			if (t.set(k.x, k.y, k.o).rotation(o))
				vis[k.x][k.y][o] = 1, Q.push(data(k.x, k.y, k.o));
		}
	}
	for (int x = 1; x <= MAPWIDTH; x++)
		for (int y = 1; y <= MAPHEIGHT; y++)
			for (int o = 0; o < 4; o++)
				if (vis[x][y][o] && t.set(x, y, o).onGround()) v.push_back(data(x, y, o));
}