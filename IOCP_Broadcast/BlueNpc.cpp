#include "BlueNpc.h"
#include "RedNpc.h"
#include "Player.h"
#include <stdio.h>

// TODO : Random Int가 대신 현재 Npc 좌표에 +-를 통해서 값을 증가 시키고 
// 숫자와 비교하는 것 대신 Player 좌표에 따라서 처리 될 수 있도록 수정해야한다.

// 정지 상태
void BlueStandState::Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc)
{
	std::cout << "상태 : 정지\n";

	std::printf("Blue Npc 좌표 X : %d Y : %d 이벤트 카운트 : %d 디렉션 : %d\n", (int)blue_npc->npc_info_.npc_pos_[0], (int)blue_npc->npc_info_.npc_pos_[1], blue_npc->event_cnt_, blue_npc->npc_info_.npc_pos_dir_);
	switch (stand_sw_)
	{
	case 0: // 무브
		m_pOwner->SetTransition(EVENT_STARTMOVE);
		break;
	case 1: // 타켓 추적
		//m_pOwner->SetTransition(EVENT_TIMEMOVE);
		break;
	case 2: // 공격 범위 체크
		break;
	case 3: // 피격
		break;
	case 4:
		break;
	default:
		break;
	}
	//m_pOwner->SetTransition(EVENT_TIMEMOVE);
	// 0,1 중 0 나오면 MoveState::Process 실행

	//std::printf("현재 플레이어 좌표 X : %d, Y :%d\n",(int)player->m_Pos[0], (int)player->m_Pos[1]);
	//std::printf("Stand Npc 좌표 X : %d Y : %d\n", (int)npc->m_NpcPos[0], (int)npc->m_NpcPos[1]);
	//m_pOwner->SetTransition(EVENT_TIMEMOVE);

	// 타켓과 거리 측정
	//if ((int)npc->m_NpcPos[0] != (int)player->m_Pos[0])
	//{
	//	if ((int)max(npc->m_NpcPos[0], player->m_Pos[0]) - (int)min(npc->m_NpcPos[0], player->m_Pos[0]) <= 10)
	//	{
	//		 타켓 찾었다
	//		printf("거리 20이내\n");
	//		m_pOwner->SetTransition(EVENT_POINTMOVE);
	//	}
	//	else {
	//		 타켓어디?
	//		printf("거리 멀어요\n");
	//	}
	//}
}

// 이동 상태
void BlueMoveState::Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc)
{
	std::cout << "상태 : 이동\n";
	//// rand() % 8 : 0 ~ 7까지 난수
	//if (blue_npc->event_cnt_ == 0)
	//{
	//	blue_npc->npc_info_.npc_pos_dir_ = (rand() % 8);
	//}
	//else if (blue_npc->event_cnt_ == 2)
	//{
	//	blue_npc->event_cnt_ = 0;
	//	m_pOwner->MoveBroadCast(player, blue_npc, red_npc);
	//	//m_pOwner->SetTransition(EVENT_STOPMOVE);
	//	// TODO 이때 방향전환 이벤트 호출 해야함
	//	
	//	return;
	//}

	////std::printf("몬스터가 %d만큼 이동합니다.\n", blue_npc->npc_info_.npc_pos_dir_);
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

	//// npc 상태 변경
	//m_pOwner->SetTransition(EVENT_STOPMOVE);
	//m_pOwner->MoveBroadCast(player, blue_npc, red_npc);
}

void BluePointMovekState::Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc)
{
	std::printf("목표물을 포착했다\n");
	std::printf("현재 플레이어 좌표 X : %d, Y :%d\n", (int)player->m_Pos[0], (int)player->m_Pos[1]);
	std::printf("Stand Npc 좌표 X : %d Y : %d\n", (int)blue_npc->npc_info_.npc_pos_[0], (int)blue_npc->npc_info_.npc_pos_[1]);
	if ((int)blue_npc->npc_info_.npc_pos_[0] != (int)player->m_Pos[0])
	{
		if ((int)blue_npc->npc_info_.npc_pos_[0] > (int)player->m_Pos[0])
		{
			// npc 값을 내려서 타켓 쫓기
			blue_npc->npc_info_.npc_pos_[0] -= 1;
			if ((int)blue_npc->npc_info_.npc_pos_[0] == (int)player->m_Pos[0])
			{
				// 타켓 발견 공격시작
				m_pOwner->SetTransition(EVENT_FINDTARGET);
			}
		}
		else
		{
			// npc 값을 올려서 타켓 쫓기
			blue_npc->npc_info_.npc_pos_[0] += 1;
			if ((int)blue_npc->npc_info_.npc_pos_[0] == (int)player->m_Pos[0])
			{
				// 타켓 발견 공격시작
				m_pOwner->SetTransition(EVENT_FINDTARGET);
			}
		}

	}
}


void BlueAttackState::Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc)
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
	// 최초 상태는 StandState 있는 상태로 시작
	m_pCurentState = m_pActionList[0];
	m_dwState = STATE_STAND;

	// 초기 npc 위치
	if (npc_info_.team_color == 0)
	{
		// red team 시작 위치
		npc_info_.npc_pos_[0] = 50;
		npc_info_.npc_pos_[1] = 0;
	}
	else
	{
		// blue team 시작 위치
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
	// 최초 상태는 StandState 있는 상태로 시작
	m_pCurentState = m_pActionList[0];
	m_dwState = STATE_STAND;

	// 초기 npc 위치
	if (npc_info_.team_color == 0)
	{
		// red team 시작 위치
		npc_info_.npc_pos_[0] = 50;
		npc_info_.npc_pos_[1] = 0;
	}
	else
	{
		// blue team 시작 위치
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

