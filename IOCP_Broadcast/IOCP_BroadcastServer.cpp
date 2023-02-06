#include "IOCompletionPort.h"
#include <string>
#include <iostream>
#include <mutex>
#include "FSM.h"
#include "FSMServer.h"

const UINT16 SERVER_PORT = 10000;
const UINT16 MAX_CLIENT = 5;		//총 접속할수 있는 클라이언트 수
const int SLEEP_TIME = 1000;

FSMServer ioCompletionPort;
bool  Main_MovementSw = true;

// TODO 이새끼를 돌리면 accept가 병신이 되는데 .......
void MonsterMovemnet()
{
	while (ioCompletionPort.MovementSw)
	{
		if (Main_MovementSw == true) {
			ioCompletionPort.npc.m_NpcPos[0] += 1;

			//char npcPosMsg[256] = { 0, };
			//*npcPosMsg = ioCompletionPort.npc.m_NpcPos[0];
			//npcPosMsg[1] = '\0';
			//ioCompletionPort.broadcastSendMsg(npcPosMsg);

			Sleep(SLEEP_TIME);
			if (ioCompletionPort.npc.m_NpcPos[0] >= 30)
			{
				Main_MovementSw = false;
				//break;
			}
		}
		else {
			ioCompletionPort.npc.m_NpcPos[0] -= 1;

			//char npcPosMsg[256] = { 0, };
			//*npcPosMsg = ioCompletionPort.npc.m_NpcPos[0];
			//npcPosMsg[1] = '\0';
			//ioCompletionPort.broadcastSendMsg(npcPosMsg);

			Sleep(SLEEP_TIME);
			if (ioCompletionPort.npc.m_NpcPos[0] <= 0)
			{
				Main_MovementSw = true;
			}
		}

		//std::printf("MonsterMovemnet Thread 작동 몬스터 X : %d\n", (int)ioCompletionPort.npc.m_NpcPos[0]);
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
		//std::string inputCmd;
		//std::getline(std::cin, inputCmd);

		//if (inputCmd == "quit")
		//{
		//	break;
		//}

		// fgets 블럭킹하면 정상적으로 접속을 받는데 while로 돌리면 씨바 ......
		char szSendMsg[256] = { 0, };
		fgets(szSendMsg, 256, stdin);
		*szSendMsg = ioCompletionPort.npc.m_NpcPos[0];
		ioCompletionPort.broadcastSendMsg(szSendMsg);

		//char npcPosMsg[256] = { 0, };
		//*npcPosMsg = ioCompletionPort.npc.m_NpcPos[0];
		//npcPosMsg[1] = '\0';
		//ioCompletionPort.broadcastSendMsg(npcPosMsg);
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

