#include "IocpNetServer.h"
#include <string>
#include <iostream>
#include "AppServer.h"

const UINT16 SERVER_PORT = 10000;
const UINT16 MAX_CLIENT = 3;		//총 접속할수 있는 클라이언트 수
const int SLEEP_TIME = 3000;
const UINT32 MAX_IO_WORKER_THREAD = 4;

AppServer iocp_net_server_;

int main()
{
	//소켓을 초기화
	iocp_net_server_.Init(MAX_IO_WORKER_THREAD);

	//소켓과 서버 주소를 연결하고 등록 시킨다.
	iocp_net_server_.BindAndListen(SERVER_PORT);

	iocp_net_server_.Run(MAX_CLIENT);

	FSM fsm;

	// AppServer fsm 객체 떼고 연동시 생성자에 넣기
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
	}


	Player player;
	BlueNpc blue_npc_(&fsm);
	RedNpc red_npc_(&fsm);

	while (1)
	{
		blue_npc_.Process(&player, &red_npc_);
		//red_npc_.Process(&player);
		// 5초에 한번씩 실행하기

		Sleep(1000);
	}

	printf("아무 키나 누를 때까지 대기합니다\n");
	while (true)
	{
		std::string inputCmd;
		std::getline(std::cin, inputCmd);

		if (inputCmd == "quit")
		{
			break;
		}
		//char szSendMsg[256] = { 0, };
		//
		//fgets(szSendMsg, 256, stdin);
		//ioCompletionPort.mainSendMsg(szSendMsg);

		//char npcPosMsg[256] = { 0, };
		//*npcPosMsg = ioCompletionPort.npc.m_NpcPos[0];
		//npcPosMsg[1] = '\0';
		//ioCompletionPort.mainSendMsg(npcPosMsg);
		//Sleep(1000);


		//std::string inputCmd;
		//std::getline(std::cin, inputCmd);

		//ioCompletionPort.SendMsg(ioCompletionPort.)
		//ioCompletionPort

		//ioCompletionPort.PublicSendMsg();
		//if (inputCmd == "quit")
		//{
		//	break;
		//}

		//npc.Process(&player);
		//// 5초에 한번씩 실행하기
		//Sleep(1000);
	}

	//t1.join();

	iocp_net_server_.End();
	return 0;
}

