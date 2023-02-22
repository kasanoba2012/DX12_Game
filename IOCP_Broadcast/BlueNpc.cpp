#include "BlueNpc.h"
#include "RedNpc.h"
#include "Player.h"
#include <stdio.h>

// TODO : Random Int�� ��� ���� Npc ��ǥ�� +-�� ���ؼ� ���� ���� ��Ű�� 
// ���ڿ� ���ϴ� �� ��� Player ��ǥ�� ���� ó�� �� �� �ֵ��� �����ؾ��Ѵ�.

// ���� ����
void BlueStandState::Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc)
{
	std::cout << "���� : ����\n";

	std::printf("Blue Npc ��ǥ X : %d Y : %d �̺�Ʈ ī��Ʈ : %d �𷺼� : %d\n", (int)blue_npc->npc_info_.npc_pos_[0], (int)blue_npc->npc_info_.npc_pos_[1], blue_npc->event_cnt_, blue_npc->npc_info_.npc_pos_dir_);
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
	//m_pOwner->SetTransition(EVENT_TIMEMOVE);
	// 0,1 �� 0 ������ MoveState::Process ����

	//std::printf("���� �÷��̾� ��ǥ X : %d, Y :%d\n",(int)player->m_Pos[0], (int)player->m_Pos[1]);
	//std::printf("Stand Npc ��ǥ X : %d Y : %d\n", (int)npc->m_NpcPos[0], (int)npc->m_NpcPos[1]);
	//m_pOwner->SetTransition(EVENT_TIMEMOVE);

	// Ÿ�ϰ� �Ÿ� ����
	//if ((int)npc->m_NpcPos[0] != (int)player->m_Pos[0])
	//{
	//	if ((int)max(npc->m_NpcPos[0], player->m_Pos[0]) - (int)min(npc->m_NpcPos[0], player->m_Pos[0]) <= 10)
	//	{
	//		 Ÿ�� ã����
	//		printf("�Ÿ� 20�̳�\n");
	//		m_pOwner->SetTransition(EVENT_POINTMOVE);
	//	}
	//	else {
	//		 Ÿ�Ͼ��?
	//		printf("�Ÿ� �־��\n");
	//	}
	//}
}

// �̵� ����
void BlueMoveState::Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc)
{
	std::cout << "���� : �̵�\n";
	//// rand() % 8 : 0 ~ 7���� ����
	//if (blue_npc->event_cnt_ == 0)
	//{
	//	blue_npc->npc_info_.npc_pos_dir_ = (rand() % 8);
	//}
	//else if (blue_npc->event_cnt_ == 2)
	//{
	//	blue_npc->event_cnt_ = 0;
	//	m_pOwner->MoveBroadCast(player, blue_npc, red_npc);
	//	//m_pOwner->SetTransition(EVENT_STOPMOVE);
	//	// TODO �̶� ������ȯ �̺�Ʈ ȣ�� �ؾ���
	//	
	//	return;
	//}

	////std::printf("���Ͱ� %d��ŭ �̵��մϴ�.\n", blue_npc->npc_info_.npc_pos_dir_);
	//switch (blue_npc->npc_info_.npc_pos_dir_)
	//{
	//	// Move Direction
	//case 0: // 12
	//	blue_npc->npc_info_.npc_pos_[1] -= blue_npc->npc_info_.npc_speed;
	//	blue_npc->event_cnt_++;
	//	break;
	//case 1: // 1
	//	blue_npc->npc_info_.npc_pos_[0] += blue_npc->npc_info_.npc_speed;
	//	blue_npc->npc_info_.npc_pos_[1] -= blue_npc->npc_info_.npc_speed;
	//	blue_npc->event_cnt_++;
	//	break;
	//case 2: // 3
	//	blue_npc->npc_info_.npc_pos_[0] += blue_npc->npc_info_.npc_speed;
	//	blue_npc->event_cnt_++;
	//	break;
	//case 3: // 5
	//	blue_npc->npc_info_.npc_pos_[0] += blue_npc->npc_info_.npc_speed;
	//	blue_npc->npc_info_.npc_pos_[1] += blue_npc->npc_info_.npc_speed;
	//	blue_npc->event_cnt_++;
	//	break;
	//case 4: // 6
	//	blue_npc->npc_info_.npc_pos_[1] += blue_npc->npc_info_.npc_speed;
	//	blue_npc->event_cnt_++;
	//	break;
	//case 5: // 7
	//	blue_npc->npc_info_.npc_pos_[0] -= blue_npc->npc_info_.npc_speed;
	//	blue_npc->npc_info_.npc_pos_[1] += blue_npc->npc_info_.npc_speed;
	//	blue_npc->event_cnt_++;
	//	break;
	//case 6: // 9
	//	blue_npc->npc_info_.npc_pos_[0] -= blue_npc->npc_info_.npc_speed;
	//	blue_npc->event_cnt_++;
	//	break;
	//case 7: // 10
	//	blue_npc->npc_info_.npc_pos_[0] -= blue_npc->npc_info_.npc_speed;
	//	blue_npc->npc_info_.npc_pos_[1] -= blue_npc->npc_info_.npc_speed;
	//	blue_npc->event_cnt_++;
	//	break;
	//default:
	//	break;
	//}

	//// npc ���� ����
	//m_pOwner->SetTransition(EVENT_STOPMOVE);
	//m_pOwner->MoveBroadCast(player, blue_npc, red_npc);
}

void BluePointMovekState::Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc)
{
	std::printf("��ǥ���� �����ߴ�\n");
	std::printf("���� �÷��̾� ��ǥ X : %d, Y :%d\n", (int)player->m_Pos[0], (int)player->m_Pos[1]);
	std::printf("Stand Npc ��ǥ X : %d Y : %d\n", (int)blue_npc->npc_info_.npc_pos_[0], (int)blue_npc->npc_info_.npc_pos_[1]);
	if ((int)blue_npc->npc_info_.npc_pos_[0] != (int)player->m_Pos[0])
	{
		if ((int)blue_npc->npc_info_.npc_pos_[0] > (int)player->m_Pos[0])
		{
			// npc ���� ������ Ÿ�� �ѱ�
			blue_npc->npc_info_.npc_pos_[0] -= 1;
			if ((int)blue_npc->npc_info_.npc_pos_[0] == (int)player->m_Pos[0])
			{
				// Ÿ�� �߰� ���ݽ���
				m_pOwner->SetTransition(EVENT_FINDTARGET);
			}
		}
		else
		{
			// npc ���� �÷��� Ÿ�� �ѱ�
			blue_npc->npc_info_.npc_pos_[0] += 1;
			if ((int)blue_npc->npc_info_.npc_pos_[0] == (int)player->m_Pos[0])
			{
				// Ÿ�� �߰� ���ݽ���
				m_pOwner->SetTransition(EVENT_FINDTARGET);
			}
		}

	}
}


void BlueAttackState::Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc)
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
		blue_npc->npc_info_.npc_pos_[0] = npcPos;
		m_pOwner->SetTransition(EVENT_LOSTTARGET);
	}
}


void BlueNpc::Process(Player* player, RedNpc* red_npc)
{
	TestOverrid();
	m_pCurentState->Process(player, this, red_npc);
}

void BlueNpc::SetTransition(DWORD dwEvent)
{
	DWORD dwOutput = m_pFsm->GetTransition(m_dwState, dwEvent);
	m_pCurentState = m_pActionList[dwOutput];
}

bool BlueNpc::TargetRange(RedNpc* red_npc)
{
	if (this->npc_info_.npc_pos_[0])
		return false;
}

void BlueNpc::SetFsm(FSM* fsm)
{
	m_pFsm = fsm;
}

BlueNpc::BlueNpc()
{
	m_pActionList.push_back(new BlueStandState(this));
	m_pActionList.push_back(new BlueMoveState(this));
	m_pActionList.push_back(new BlueAttackState(this));
	m_pActionList.push_back(new BluePointMovekState(this));
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

BlueNpc::BlueNpc(FSM* fsm)
{
	m_pFsm = fsm;
	m_pActionList.push_back(new BlueStandState(this));
	m_pActionList.push_back(new BlueMoveState(this));
	m_pActionList.push_back(new BlueAttackState(this));
	m_pActionList.push_back(new BluePointMovekState(this));
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

BlueNpc::~BlueNpc()
{
	for (auto list : m_pActionList)
	{
		delete list;
	}
	m_pActionList.clear();
}

