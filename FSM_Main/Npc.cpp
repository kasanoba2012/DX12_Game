#include "Npc.h"
#include "Player.h"

// TODO : Random Int�� ��� ���� Npc ��ǥ�� +-�� ���ؼ� ���� ���� ��Ű�� 
// ���ڿ� ���ϴ� �� ��� Player ��ǥ�� ���� ó�� �� �� �ֵ��� �����ؾ��Ѵ�.

void StandState::Process(Player* player)
{
	std::cout << "Stand State\n";
	// 0,1 �� 0 ������ MoveState::Process ����
	int randInt = rand() % 2;
	std::printf("Stand randInt : %d || ���� �÷��̾� ��ǥ X : %d, Y :%d\n", randInt, (int)player->m_Pos[0], (int)player->m_Pos[1]);
	if (randInt == 0)
	{
		// MoveState npc ���� ����
		m_pOwner->SetTransition(EVENT_TIMEMOVE);
	}
	// 0,1 �� 1 ������ AttackState::Process ����
	if (randInt == 1)
	{
		// AttackState npc ���� ����
		m_pOwner->SetTransition(EVENT_FINDTARGET);
	}
}

void MoveState::Process(Player* player)
{
	std::cout << "Move State\n";
	// 0 ~ 100 �� 5���� ������ ���º�ȯ
	int randInt = rand() % 100;
	std::printf("Move randInt : %d\n", randInt);
	
	// TODO : �� �κ��� NPC ��ǥ �������� �ٲ���Ѵ�.
	player->m_Pos[0] -= 1;
	player->m_Pos[1] -= 2;
	if (randInt < 10)
	{
		// npc ���� ����
		m_pOwner->SetTransition(EVENT_STOPMOVE);
	}
}

void AttackState::Process(Player* player)
{
	std::cout << "Attack State\n";
	int randInt = rand() % 100;
	std::printf("Attack randInt : %d\n", randInt);
	if (randInt < 10)
	{
		m_pOwner->SetTransition(EVENT_LOSTTARGET);
	}
}


void Npc::Process(Player* player)
{
	m_pCurentState->Process(player);
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
