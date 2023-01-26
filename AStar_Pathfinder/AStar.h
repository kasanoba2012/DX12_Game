#pragma once
#include <windows.h>
#include <vector>
#include <set>
#include <iostream>

struct Index
{
	// 2D ÁÂÇ¥
	int x, y;

	bool operator == (const Index& p)
	{
		return (x == p.x && y == p.y);
	}

	Index operator + (const Index& p)
	{
		return { x + p.x, y + p.y };
	}

	Index() : x(0), y(0) {}
	Index(int fx, int fy) : x(fx), y(fy) {}
};

namespace AStar
{
	using uint = unsigned int;
	using pathlist = std::vector<Index>;

	struct Node
	{
		uint G, H;
		Index index;
		Node* parent;
		
		uint GetScore()
		{
			return G + H;
		}

		Node(Index i, Node* parent = nullptr)
		{
			index = i;
			this->parent = parent;
			G = H = 0;
		}
	}; // End Node

	using NodeSet = std::set<Node*>;

	class MapGenerator
	{
	private:
		Index m_WorldSize;
		uint m_Direction;
		pathlist m_Direction8;
		pathlist m_Walls;
	public:
		void SetWorldSize(Index ws);
		void AddColision(Index ws);
		pathlist findpath(Index start, Index end);
		bool DetectCollison(Index coord);
		Node* findNodeList(NodeSet& list, Index newIndex);
		uint Distance(Index src, Index target);
		void DeleteNodes(NodeSet& nodes);
		MapGenerator();
	};
} // End AStar
