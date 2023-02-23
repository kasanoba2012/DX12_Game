#include "AppServer.h"
//FSM
//#include "FSM.h"
//#include "BlueNpc.h"
//#include "RedNpc.h"
//#include "Player.h"

AppServer::AppServer()
{
	//FsmInit();
}


void AppServer::FsmInit()
{
	// FSM 세팅
	//BlueNpc blue_npc_(&fsm);
}

void AppServer::ThreadTestfuntion()
{
	while (1)
	{
		std::cout << "타 클라스 함수 호출\n";
		Sleep(500);
	}
}

void AppServer::FSM_RUN()
{
	// 가만히 서있다가 시간 지나면 움직이기
	fsm.AddTransition(STATE_STAND, EVENT_TIMEMOVE, STATE_MOVE);
	// 이동 시작
	fsm.AddTransition(STATE_STAND, EVENT_STARTMOVE, STATE_MOVE);
	// 방향 전환
	fsm.AddTransition(STATE_MOVE, EVENT_TRUNMOVE, STATE_MOVE);
	// 타켓 발견하면 타켓에게 다가가기
	fsm.AddTransition(STATE_STAND, EVENT_POINTMOVE, STATE_POINT_MOVE);
	// 가만히 서있다가 타켓 발견하면 공격
	fsm.AddTransition(STATE_STAND, EVENT_FINDTARGET, STATE_ATTACK);
	// 움직이다가 멈추기
	fsm.AddTransition(STATE_MOVE, EVENT_STOPMOVE, STATE_STAND);
	// 공격하다가 타켓 없어지면 멈추기
	fsm.AddTransition(STATE_ATTACK, EVENT_LOSTTARGET, STATE_STAND);
	
	blue_npc_.SetFsm(&fsm);

	while (1)
	{
		blue_npc_.Process(&player, &red_npc_);

		Sleep(1000);
	}
}
