#include "RedNpc.h"
#include "Player.h"
#include <stdio.h>

// TODO : Random Int�� ��� ���� Npc ��ǥ�� +-�� ���ؼ� ���� ���� ��Ű�� 
// ���ڿ� ���ϴ� �� ��� Player ��ǥ�� ���� ó�� �� �� �ֵ��� �����ؾ��Ѵ�.

// ���� ����
void RedStandState::Process(Player* player, RedNpc* red_npc)
{
	std::cout << "���� : ����\n";

	std::printf("Stand Npc ��ǥ X : %d Y : %d\n", (int)red_npc->npc_info_.npc_pos_[0], (int)red_npc->npc_info_.npc_pos_[1]);
	switch (stand_sw_)
	{
	case 0: // ����
		m_pOwner->SetTransition(EVENT_STARTMOVE);
		break;
	case 1: // Ÿ�� ����
		//m_pOwner->SetTransition(EVENT_TIMEMOVE);
		break;
	case 2: // ���� ���� üũ
		break;
	case 3: // �ǰ�
		break;
	case 4:
		break;
	default:
		break;
	}
}

// �̵� ����
void RedMoveState::Process(Player* player, RedNpc* red_npc)
{
	std::cout << "���� : �̵�\n";
	// rand() % 8 : 0 ~ 7���� ����
	//npc->npc_info_.npc_pos_dir_ = (rand() % 8);
	std::printf("���Ͱ� %d��ŭ �̵��մϴ�.\n", red_npc->npc_info_.npc_pos_dir_);
	switch (red_npc->npc_info_.npc_pos_dir_)
	{
		// Move Direction
	case 0: // 12
		red_npc->npc_info_.npc_pos_[1] -= red_npc->npc_info_.npc_speed;
		break;
	case 1: // 1
		red_npc->npc_info_.npc_pos_[0] += red_npc->npc_info_.npc_speed;
		red_npc->npc_info_.npc_pos_[1] -= red_npc->npc_info_.npc_speed;
		break;
	case 2: // 3
		red_npc->npc_info_.npc_pos_[0] += red_npc->npc_info_.npc_speed;
		break;
	case 3: // 5
		red_npc->npc_info_.npc_pos_[0] += red_npc->npc_info_.npc_speed;
		red_npc->npc_info_.npc_pos_[1] += red_npc->npc_info_.npc_speed;
		break;
	case 4: // 6
		red_npc->npc_info_.npc_pos_[1] += red_npc->npc_info_.npc_speed;
		break;
	case 5: // 7
		red_npc->npc_info_.npc_pos_[0] -= red_npc->npc_info_.npc_speed;
		red_npc->npc_info_.npc_pos_[1] += red_npc->npc_info_.npc_speed;
		break;
	case 6: // 9
		red_npc->npc_info_.npc_pos_[0] -= red_npc->npc_info_.npc_speed;
		break;
	case 7: // 10
		red_npc->npc_info_.npc_pos_[0] -= red_npc->npc_info_.npc_speed;
		red_npc->npc_info_.npc_pos_[1] -= red_npc->npc_info_.npc_speed;
		break;
	default:
		break;
	}

	// npc ���� ����
	m_pOwner->SetTransition(EVENT_STOPMOVE);

}

void RedPointMovekState::Process(Player* player, RedNpc* red_npc)
{
	std::printf("��ǥ���� �����ߴ�\n");
	std::printf("���� �÷��̾� ��ǥ X : %d, Y :%d\n", (int)player->m_Pos[0], (int)player->m_Pos[1]);
	std::printf("Stand Npc ��ǥ X : %d Y : %d\n", (int)red_npc->npc_info_.npc_pos_[0], (int)red_npc->npc_info_.npc_pos_[1]);
	if ((int)red_npc->npc_info_.npc_pos_[0] != (int)player->m_Pos[0])
	{
		if ((int)red_npc->npc_info_.npc_pos_[0] > (int)player->m_Pos[0])
		{
			// npc ���� ������ Ÿ�� �ѱ�
			red_npc->npc_info_.npc_pos_[0] -= 1;
			if ((int)red_npc->npc_info_.npc_pos_[0] == (int)player->m_Pos[0])
			{
				// Ÿ�� �߰� ���ݽ���
				m_pOwner->SetTransition(EVENT_FINDTARGET);
			}
		}
		else
		{
			// npc ���� �÷��� Ÿ�� �ѱ�
			red_npc->npc_info_.npc_pos_[0] += 1;
			if ((int)red_npc->npc_info_.npc_pos_[0] == (int)player->m_Pos[0])
			{
				// Ÿ�� �߰� ���ݽ���
				m_pOwner->SetTransition(EVENT_FINDTARGET);
			}
		}

	}
}


void RedAttackState::Process(Player* player, RedNpc* red_npc)
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
		red_npc->npc_info_.npc_pos_[0] = npcPos;
		m_pOwner->SetTransition(EVENT_LOSTTARGET);
	}
}


void RedNpc::Process(Player* player)
{
	m_pCurentState->Process(player, this);
}

void RedNpc::SetTransition(DWORD dwEvent)
{
	DWORD dwOutput = m_pFsm->GetTransition(m_dwState, dwEvent);
	m_pCurentState = m_pActionList[dwOutput];
}

RedNpc::RedNpc(FSM* fsm)
{
	m_pFsm = fsm;
	m_pActionList.push_back(new RedStandState(this));
	m_pActionList.push_back(new RedMoveState(this));
	m_pActionList.push_back(new RedAttackState(this));
	m_pActionList.push_back(new RedPointMovekState(this));
	// ���� ���´� StandState �ִ� ���·� ����
	m_pCurentState = m_pActionList[0];
	m_dwState = STATE_STAND;

	// �ʱ� npc ��ġ
	if (npc_info_.team_color == 0)
	{
		// red team ���� ��ġ
		npc_info_.npc_pos_[0] = 50;
		npc_info_.npc_pos_[1] = 0;
	}
	else
	{
		// blue team ���� ��ġ
		npc_info_.npc_pos_[0] = 0;
		npc_info_.npc_pos_[1] = 50;

	}
}

RedNpc::~RedNpc()
{
	for (auto list : m_pActionList)
	{
		delete list;
	}
	m_pActionList.clear();
}

