#include "IOCompletionPort.h"
#include <string>
#include <iostream>
#include "FSM.h"
#include "FSMServer.h"

const UINT16 SERVER_PORT = 10000;
const UINT16 MAX_CLIENT = 3;		//총 접속할수 있는 클라이언트 수
const int SLEEP_TIME = 3000;

FSMServer ioCompletionPort;
bool  MovementSw = true;


void MonsterMovemnet()
{
	while (ioCompletionPort.MovementSw)
	{
		if (MovementSw == true) {
			ioCompletionPort.npc.m_NpcPos[0] += 1;

			char npcPosMsg[256] = { 0, };
			*npcPosMsg = ioCompletionPort.npc.m_NpcPos[0];
			npcPosMsg[1] = '\0';
			ioCompletionPort.mainSendMsg(npcPosMsg);

			Sleep(SLEEP_TIME);
			if (ioCompletionPort.npc.m_NpcPos[0] >= 30)
			{
				MovementSw = false;
				//break;
			}
		}
		else {
			ioCompletionPort.npc.m_NpcPos[0] -= 1;

			char npcPosMsg[256] = { 0, };
			*npcPosMsg = ioCompletionPort.npc.m_NpcPos[0];
			npcPosMsg[1] = '\0';
			ioCompletionPort.mainSendMsg(npcPosMsg);

			Sleep(SLEEP_TIME);
			if (ioCompletionPort.npc.m_NpcPos[0] <= 0)
			{
				MovementSw = true;
			}
		}

		std::printf("MonsterMovemnet Thread 작동 몬스터 X : %d\n", (int)ioCompletionPort.npc.m_NpcPos[0]);
	}
}

int main()
{
	void (*P_MonsterMovemnet)(void) = *MonsterMovemnet;

	//소켓을 초기화
	ioCompletionPort.InitSocket();

	//소켓과 서버 주소를 연결하고 등록 시킨다.
	ioCompletionPort.BindandListen(SERVER_PORT);

	ioCompletionPort.StartServer(MAX_CLIENT);

	std::thread t1(P_MonsterMovemnet);

	printf("아무 키나 누를 때까지 대기합니다\n");
	while (true)
	{
		char szSendMsg[256] = { 0, };
		
		fgets(szSendMsg, 256, stdin);
		//ioCompletionPort.mainSendMsg(szSendMsg);

		char npcPosMsg[256] = { 0, };
		*npcPosMsg = ioCompletionPort.npc.m_NpcPos[0];
		npcPosMsg[1] = '\0';
		ioCompletionPort.mainSendMsg(npcPosMsg);
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

	ioCompletionPort.DestroyThread();
	return 0;
}

