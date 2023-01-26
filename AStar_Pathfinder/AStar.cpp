#include "AStar.h"

namespace AStar
{
	MapGenerator::MapGenerator()
	{
		// �����¿�
		m_Direction8.push_back({ 0,-1 });
		m_Direction8.push_back({ 0,+1 });
		m_Direction8.push_back({ -1,0 });
		m_Direction8.push_back({ +1,0 });

		// �밢 4��
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
		// ������ ����Ұ͵� ��ǥ �ޱ�
		m_Walls.push_back(index);
	}

	pathlist MapGenerator::findpath(Index start, Index target)
	{
		// openSet : �� �� �ִ� �� closeSet : �� ���� ��
		NodeSet openSet, closedSet;
		openSet.insert(new Node(start));
		std::vector<Index> currentList;

		// current : ���� ��ġ
		Node* current = nullptr;
		while (!openSet.empty())
		{
			current = *openSet.begin();
			for (auto node : openSet)
			{
				// ��� ó��
				uint iNodeCost = node->GetScore();
				uint iCurrentCost = current->GetScore();
				// ���� ��ǥ�� ������ ������ �ϴ� ���� ��ǥ�� ���� ������ ���� ��ġ�� ������ �ϴ� ���� ��ǥ�� �ٲ��
				if (iNodeCost <= iCurrentCost)
				{
					current = node;
				}
			}
			// ������ ����
			if (current->index == target)
			{
				break;
			}
			// �湮������
			// �湮 ����Ʈ�� ���� ��ǥ �� �ֱ�
			currentList.push_back(current->index);
			// ������ �� ����Ʈ�� ���� ��ǥ �� �ֱ�
			closedSet.insert(current);
			// �� �� �ִ� ����Ʈ���� ���� ��ǥ �� ����
			openSet.erase(current);
			
			for (uint i = 0; i < 8; ++i)
			{
				// ���� ��ǥ���� �� �� �ִ� 8�� ��ǥ �� �ޱ�
				Index newIndex(current->index + m_Direction8[i]);
				// ���� �浹 üũ
				if (DetectCollison(newIndex))
				{
					continue;
				}
				// ���� �湮 ����Ʈ�� üũ
				if (findNodeList(closedSet, newIndex))
				{
					continue;
				}

				// �����¿��� ��� 10�� �밢�� ��� 14��
				uint totalCost = current->G + ((i < 4) ? 10 : 14);
				Node* node = findNodeList(openSet, newIndex);

				// ���� ��� ���� ���ٸ�
				if (node == nullptr)
				{
					node = new Node(newIndex, current);
					node->G = totalCost;
					node->H = Distance(node->index, target);
					openSet.insert(node);
				}
				else if (totalCost < node->G)
				{
					// ���¸���Ʈ�� �ִٸ�
					node->parent = current;
					node->G = totalCost;
				}
			}
		} // end while

		// ���� �� ����
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

	// �� �浹 üũ
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

	// �� �� �ִ� ��ǥ ã��
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

	// ���� ���������� �Ÿ� üũ
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
