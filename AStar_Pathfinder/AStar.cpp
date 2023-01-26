#include "AStar.h"

namespace AStar
{
	MapGenerator::MapGenerator()
	{
		// 상하좌우
		m_Direction8.push_back({ 0,-1 });
		m_Direction8.push_back({ 0,+1 });
		m_Direction8.push_back({ -1,0 });
		m_Direction8.push_back({ +1,0 });

		// 대각 4축
		m_Direction8.push_back({ -1,-1 });
		m_Direction8.push_back({ +1,-1 });
		m_Direction8.push_back({ -1,+1 });
		m_Direction8.push_back({ +1,+1 });
	}

	void MapGenerator::SetWorldSize(Index ws)
	{
		m_WorldSize = ws;
	}

	void MapGenerator::AddColision(Index index)
	{
		// 벽으로 취급할것들 좌표 받기
		m_Walls.push_back(index);
	}

	pathlist MapGenerator::findpath(Index start, Index target)
	{
		// openSet : 갈 수 있는 곳 closeSet : 못 가는 곳
		NodeSet openSet, closedSet;
		openSet.insert(new Node(start));
		std::vector<Index> currentList;

		// current : 현재 위치
		Node* current = nullptr;
		while (!openSet.empty())
		{
			current = *openSet.begin();
			for (auto node : openSet)
			{
				// 비용 처리
				uint iNodeCost = node->GetScore();
				uint iCurrentCost = current->GetScore();
				// 현재 좌표의 값보다 갈려고 하는 곳의 좌표의 값이 작으면 현재 위치를 갈려고 하는 곳의 좌표로 바꿔라
				if (iNodeCost <= iCurrentCost)
				{
					current = node;
				}
			}
			// 목적지 도착
			if (current->index == target)
			{
				break;
			}
			// 방문했으면
			// 방문 리스트에 현재 좌표 값 넣기
			currentList.push_back(current->index);
			// 못가는 곳 리스트에 현재 좌표 값 넣기
			closedSet.insert(current);
			// 갈 수 있는 리스트에서 현재 좌표 값 삭제
			openSet.erase(current);
			
			for (uint i = 0; i < 8; ++i)
			{
				// 현재 좌표에서 갈 수 있는 8축 좌표 값 받기
				Index newIndex(current->index + m_Direction8[i]);
				// 벽과 충돌 체크
				if (DetectCollison(newIndex))
				{
					continue;
				}
				// 이전 방문 리스트와 체크
				if (findNodeList(closedSet, newIndex))
				{
					continue;
				}

				// 상하좌우의 경우 10점 대각의 경우 14점
				uint totalCost = current->G + ((i < 4) ? 10 : 14);
				Node* node = findNodeList(openSet, newIndex);

				// 현재 노드 값이 없다면
				if (node == nullptr)
				{
					node = new Node(newIndex, current);
					node->G = totalCost;
					node->H = Distance(node->index, target);
					openSet.insert(node);
				}
				else if (totalCost < node->G)
				{
					// 오픈리스트에 있다면
					node->parent = current;
					node->G = totalCost;
				}
			}
		} // end while

		// 최종 길 저장
		pathlist path;
		while (current != nullptr)
		{
			path.push_back(current->index);
			current = current->parent;
		}
		DeleteNodes(openSet);
		DeleteNodes(closedSet);

		return path;
	}

	// 벽 충돌 체크
	bool MapGenerator::DetectCollison(Index coord)
	{
		if (coord.x < 0 || coord.x >= m_WorldSize.x ||
			coord.y < 0 || coord.y >= m_WorldSize.y)
		{
			return true;
		}

		if (std::find(m_Walls.begin(),
			m_Walls.end(), coord)
			!= m_Walls.end())
		{
			return true;
		}
		return false;
	}

	// 갈 수 있는 좌표 찾기
	Node* MapGenerator::findNodeList(NodeSet& list, Index newIndex)
	{
		for (auto node : list)
		{
			if (node->index == newIndex)
			{
				return node;
			}
		}
		return nullptr;
	}

	// 최종 목적지와의 거리 체크
	uint MapGenerator::Distance(Index src, Index target)
	{
		Index ret = { abs(src.x - target.x), abs(src.y - target.y) };
		return sqrt(pow(ret.x, 2) + pow(ret.y, 2)) * 10;
	}

	void MapGenerator::DeleteNodes(NodeSet& nodes)
	{
		for (auto it = nodes.begin();
			it != nodes.end();
			)
		{
			delete* it;
			it = nodes.erase(it);
		}
	}
};
