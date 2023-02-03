#include "Npc.h"
#include "Player.h"

// TODO : Random Int가 대신 현재 Npc 좌표에 +-를 통해서 값을 증가 시키고 
// 숫자와 비교하는 것 대신 Player 좌표에 따라서 처리 될 수 있도록 수정해야한다.

void StandState::Process(Player* player)
{
	std::cout << "Stand State\n";
	// 0,1 중 0 나오면 MoveState::Process 실행
	int randInt = rand() % 2;
	std::printf("Stand randInt : %d || 현재 플레이어 좌표 X : %d, Y :%d\n", randInt, (int)player->m_Pos[0], (int)player->m_Pos[1]);
	if (randInt == 0)
	{
		// MoveState npc 상태 변경
		m_pOwner->SetTransition(EVENT_TIMEMOVE);
	}
	// 0,1 중 1 나오면 AttackState::Process 실행
	if (randInt == 1)
	{
		// AttackState npc 상태 변경
		m_pOwner->SetTransition(EVENT_FINDTARGET);
	}
}

void MoveState::Process(Player* player)
{
	std::cout << "Move State\n";
	// 0 ~ 100 중 5보다 작으면 상태변환
	int randInt = rand() % 100;
	std::printf("Move randInt : %d\n", randInt);
	
	// TODO : 이 부분을 NPC 좌표 변경으로 바꿔야한다.
	player->m_Pos[0] -= 1;
	player->m_Pos[1] -= 2;
	if (randInt < 10)
	{
		// npc 상태 변경
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
	// 최초 상태는 StandState 있는 상태로 시작
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
