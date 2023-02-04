#include "IOCompletionPort.h"
#include <string>
#include <iostream>
#include "FSM.h"
#include "Npc.h"
#include "Player.h"

const UINT16 SERVER_PORT = 10000;
const UINT16 MAX_CLIENT = 100;		//총 접속할수 있는 클라이언트 수

int main()
{
	IOCompletionPort ioCompletionPort;

	//소켓을 초기화
	ioCompletionPort.InitSocket();

	//소켓과 서버 주소를 연결하고 등록 시킨다.
	ioCompletionPort.BindandListen(SERVER_PORT);

	ioCompletionPort.StartServer(MAX_CLIENT);

	// FSM 세팅
	FSM fsm;
	// 가만히 서있다가 시간 지나면 움직이기
	fsm.AddTransition(STATE_STAND, EVENT_TIMEMOVE, STATE_MOVE);
	// 타켓 발견하면 타켓에게 다가가기
	fsm.AddTransition(STATE_STAND, EVENT_POINTMOVE, STATE_POINT_MOVE);
	// 가만히 서있다가 타켓 발견하면 공격
	fsm.AddTransition(STATE_STAND, EVENT_FINDTARGET, STATE_ATTACK);
	// 움직이다가 멈추기
	fsm.AddTransition(STATE_MOVE, EVENT_STOPMOVE, STATE_STAND);
	// 공격하다가 타켓 없어지면 멈추기
	fsm.AddTransition(STATE_ATTACK, EVENT_LOSTTARGET, STATE_STAND);

	Player player;
	Npc npc(&fsm);

	printf("아무 키나 누를 때까지 대기합니다\n");
	while (true)
	{
		//char szSendMsg[256] = { 0, };
		//std::string inputCmd;
		//std::getline(std::cin, inputCmd);
		//std::fgets(szSendMsg, 256, stdin);

		//ioCompletionPort.PublicSendMsg();
		//if (inputCmd == "quit")
		//{
		//	break;
		//}

		npc.Process(&player);
		// 5초에 한번씩 실행하기

		Sleep(1000);
	}

	ioCompletionPort.DestroyThread();
	return 0;
}

