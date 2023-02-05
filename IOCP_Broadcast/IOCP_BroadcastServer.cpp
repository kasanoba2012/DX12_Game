#include "IOCompletionPort.h"
#include <string>
#include <iostream>
#include "FSM.h"
#include "FSMServer.h"

const UINT16 SERVER_PORT = 10000;
const UINT16 MAX_CLIENT = 5;		//총 접속할수 있는 클라이언트 수

FSMServer ioCompletionPort;
bool  MovementSw = true;

void fun1()
{
	while (ioCompletionPort.MovementSw)
	{
		if (MovementSw) {
			ioCompletionPort.npc.m_NpcPos[0] += 1;
			Sleep(300);
			if (ioCompletionPort.npc.m_NpcPos[0] >= 30)
			{
				MovementSw = false;
				break;
			}
		}

		else {
			ioCompletionPort.npc.m_NpcPos[0] -= 1;
			Sleep(300);
			if (ioCompletionPort.npc.m_NpcPos[0] <= 0)
			{
				MovementSw = true;
			}
		}

		std::printf("쓰레드1 작동 : %d\n", (int)ioCompletionPort.npc.m_NpcPos[0]);
	}
}

int main()
{
	void (*testPtr)(void) = *fun1;

	//소켓을 초기화
	ioCompletionPort.InitSocket();

	//소켓과 서버 주소를 연결하고 등록 시킨다.
	ioCompletionPort.BindandListen(SERVER_PORT);

	ioCompletionPort.StartServer(MAX_CLIENT);

	//std::thread t1(testPtr);

	printf("아무 키나 누를 때까지 대기합니다\n");
	while (true)
	{
		char szSendMsg[256] = { 0, };
		fgets(szSendMsg, 256, stdin);
		ioCompletionPort.mainSendMsg(szSendMsg);
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

	ioCompletionPort.DestroyThread();
	return 0;
}

