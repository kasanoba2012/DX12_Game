#include "Npc.h"
#include "Player.h"
#include <stdio.h>

// TODO : Random Int�� ��� ���� Npc ��ǥ�� +-�� ���ؼ� ���� ���� ��Ű�� 
// ���ڿ� ���ϴ� �� ��� Player ��ǥ�� ���� ó�� �� �� �ֵ��� �����ؾ��Ѵ�.

void StandState::Process(Player* player, Npc* npc)
{
	std::cout << "Stand State\n";
	// 0,1 �� 0 ������ MoveState::Process ����

	std::printf("���� �÷��̾� ��ǥ X : %d, Y :%d\n", (int)player->m_Pos[0], (int)player->m_Pos[1]);
	std::printf("Stand Npc ��ǥ X : %d Y : %d\n", (int)npc->m_NpcPos[0], (int)npc->m_NpcPos[1]);
	m_pOwner->SetTransition(EVENT_TIMEMOVE);

	// Ÿ�ϰ� �Ÿ� ����
	if ((int)npc->m_NpcPos[0] != (int)player->m_Pos[0])
	{
		if ((int)max(npc->m_NpcPos[0], player->m_Pos[0]) - (int)min(npc->m_NpcPos[0], player->m_Pos[0]) <= 10)
		{
			// Ÿ�� ã����
			printf("�Ÿ� 20�̳�\n");
			m_pOwner->SetTransition(EVENT_POINTMOVE);
		}
		else {
			// Ÿ�Ͼ��?
			printf("�Ÿ� �־��\n");
		}
	}
}

void MoveState::Process(Player* player, Npc* npc)
{
	int npcMove = (rand() % 5) + 1;
	std::printf("���Ͱ� %d��ŭ �̵��մϴ�.\n", npcMove);
	// Npc �̵�
	npc->m_NpcPos[0] += npcMove;
	npc->m_NpcPos[1] += 0;

	// npc ���� ����
	m_pOwner->SetTransition(EVENT_STOPMOVE);

}

void PointMovekState::Process(Player* player, Npc* npc)
{
	std::printf("��ǥ���� �����ߴ�\n");
	std::printf("���� �÷��̾� ��ǥ X : %d, Y :%d\n", (int)player->m_Pos[0], (int)player->m_Pos[1]);
	std::printf("Stand Npc ��ǥ X : %d Y : %d\n", (int)npc->m_NpcPos[0], (int)npc->m_NpcPos[1]);
	if ((int)npc->m_NpcPos[0] != (int)player->m_Pos[0])
	{
		if ((int)npc->m_NpcPos[0] > (int)player->m_Pos[0])
		{
			// npc ���� ������ Ÿ�� �ѱ�
			npc->m_NpcPos[0] -= 1;
			if ((int)npc->m_NpcPos[0] == (int)player->m_Pos[0])
			{
				// Ÿ�� �߰� ���ݽ���
				m_pOwner->SetTransition(EVENT_FINDTARGET);
			}
		}
		else
		{
			// npc ���� �÷��� Ÿ�� �ѱ�
			npc->m_NpcPos[0] += 1;
			if ((int)npc->m_NpcPos[0] == (int)player->m_Pos[0])
			{
				// Ÿ�� �߰� ���ݽ���
				m_pOwner->SetTransition(EVENT_FINDTARGET);
			}
		}

	}
}


void AttackState::Process(Player* player, Npc* npc)
{
	std::cout << "���Ϳ��� ������ ���߽��ϴ�.\n";
	int randInt = rand() % 100;
	if (randInt < 10)
	{
		std::cout << "���� ���� ����\n";
		// 20~30 ��
		float playerPos = rand() % (30 - 20 + 1) + 20;
		player->m_Pos[0] = playerPos;
		// 1~10 ��
		float npcPos = rand() % (10 - 0 + 1) + 0;
		npc->m_NpcPos[0] = npcPos;
		m_pOwner->SetTransition(EVENT_LOSTTARGET);
	}
}


void Npc::Process(Player* player)
{
	m_pCurentState->Process(player, this);
}

void Npc::SetTransition(DWORD dwEvent)
{
	DWORD dwOutput = m_pFsm->GetTransition(m_dwState, dwEvent);
	m_pCurentState = m_pActionList[dwOutput];
}



Npc::Npc(FSM* fsm)
{
	m_pFsm = fsm;
	m_pActionList.push_back(new StandState(this));
	m_pActionList.push_back(new MoveState(this));
	m_pActionList.push_back(new AttackState(this));
	m_pActionList.push_back(new PointMovekState(this));
	// ���� ���´� StandState �ִ� ���·� ����
	m_pCurentState = m_pActionList[0];
	m_dwState = STATE_STAND;

	m_NpcPos[0] = 0;
	m_NpcPos[1] = 0;
}

Npc::~Npc()
{
	for (auto list : m_pActionList)
	{
		delete list;
	}
	m_pActionList.clear();
}

