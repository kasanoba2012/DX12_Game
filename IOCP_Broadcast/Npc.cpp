#include "Npc.h"
#include "Player.h"
#include <stdio.h>

// TODO : Random Int가 대신 현재 Npc 좌표에 +-를 통해서 값을 증가 시키고 
// 숫자와 비교하는 것 대신 Player 좌표에 따라서 처리 될 수 있도록 수정해야한다.

void StandState::Process(Player* player, Npc* npc)
{
	std::cout << "Stand State\n";
	// 0,1 중 0 나오면 MoveState::Process 실행

	std::printf("현재 플레이어 좌표 X : %d, Y :%d\n", (int)player->m_Pos[0], (int)player->m_Pos[1]);
	std::printf("Stand Npc 좌표 X : %d Y : %d\n", (int)npc->m_NpcPos[0], (int)npc->m_NpcPos[1]);
	m_pOwner->SetTransition(EVENT_TIMEMOVE);

	// 타켓과 거리 측정
	if ((int)npc->m_NpcPos[0] != (int)player->m_Pos[0])
	{
		if ((int)max(npc->m_NpcPos[0], player->m_Pos[0]) - (int)min(npc->m_NpcPos[0], player->m_Pos[0]) <= 10)
		{
			// 타켓 찾었다
			printf("거리 20이내\n");
			m_pOwner->SetTransition(EVENT_POINTMOVE);
		}
		else {
			// 타켓어디?
			printf("거리 멀어요\n");
		}
	}
}

void MoveState::Process(Player* player, Npc* npc)
{
	int npcMove = (rand() % 5) + 1;
	std::printf("몬스터가 %d만큼 이동합니다.\n", npcMove);
	// Npc 이동
	npc->m_NpcPos[0] += npcMove;
	npc->m_NpcPos[1] += 0;

	// npc 상태 변경
	m_pOwner->SetTransition(EVENT_STOPMOVE);

}

void PointMovekState::Process(Player* player, Npc* npc)
{
	std::printf("목표물을 포착했다\n");
	std::printf("현재 플레이어 좌표 X : %d, Y :%d\n", (int)player->m_Pos[0], (int)player->m_Pos[1]);
	std::printf("Stand Npc 좌표 X : %d Y : %d\n", (int)npc->m_NpcPos[0], (int)npc->m_NpcPos[1]);
	if ((int)npc->m_NpcPos[0] != (int)player->m_Pos[0])
	{
		if ((int)npc->m_NpcPos[0] > (int)player->m_Pos[0])
		{
			// npc 값을 내려서 타켓 쫓기
			npc->m_NpcPos[0] -= 1;
			if ((int)npc->m_NpcPos[0] == (int)player->m_Pos[0])
			{
				// 타켓 발견 공격시작
				m_pOwner->SetTransition(EVENT_FINDTARGET);
			}
		}
		else
		{
			// npc 값을 올려서 타켓 쫓기
			npc->m_NpcPos[0] += 1;
			if ((int)npc->m_NpcPos[0] == (int)player->m_Pos[0])
			{
				// 타켓 발견 공격시작
				m_pOwner->SetTransition(EVENT_FINDTARGET);
			}
		}

	}
}


void AttackState::Process(Player* player, Npc* npc)
{
	std::cout << "몬스터에게 공격을 당했습니다.\n";
	int randInt = rand() % 100;
	if (randInt < 10)
	{
		std::cout << "전투 종료 리셋\n";
		// 20~30 중
		float playerPos = rand() % (30 - 20 + 1) + 20;
		player->m_Pos[0] = playerPos;
		// 1~10 중
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

