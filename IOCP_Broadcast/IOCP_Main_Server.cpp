﻿#include "IOCP_Net_Server.h"
#include <string>
#include <iostream>
#include "FSM.h"
#include "FSMServer.h"

const UINT16 SERVER_PORT = 10000;
const UINT16 MAX_CLIENT = 3;		//총 접속할수 있는 클라이언트 수
const int SLEEP_TIME = 3000;
const UINT32 MAX_IO_WORKER_THREAD = 4;

FSMServer iocp_net_server_;
bool  MovementSw = true;


void MonsterMovemnet()
{
	while (iocp_net_server_.MovementSw)
	{
		if (MovementSw == true) {
			iocp_net_server_.npc.m_NpcPos[0] += 1;

			char npcPosMsg[256] = { 0, };
			*npcPosMsg = iocp_net_server_.npc.m_NpcPos[0];
			npcPosMsg[1] = '\0';
			iocp_net_server_.mainSendMsg(npcPosMsg);

			Sleep(SLEEP_TIME);
			if (iocp_net_server_.npc.m_NpcPos[0] >= 30)
			{
				MovementSw = false;
				//break;
			}
		}
		else {
			iocp_net_server_.npc.m_NpcPos[0] -= 1;

			char npcPosMsg[256] = { 0, };
			*npcPosMsg = iocp_net_server_.npc.m_NpcPos[0];
			npcPosMsg[1] = '\0';
			iocp_net_server_.mainSendMsg(npcPosMsg);

			Sleep(SLEEP_TIME);
			if (iocp_net_server_.npc.m_NpcPos[0] <= 0)
			{
				MovementSw = true;
			}
		}

		std::printf("MonsterMovemnet Thread 작동 몬스터 X : %d\n", (int)iocp_net_server_.npc.m_NpcPos[0]);
	}
}

int main()
{
	void (*P_MonsterMovemnet)(void) = *MonsterMovemnet;

	//소켓을 초기화
	iocp_net_server_.Init(MAX_IO_WORKER_THREAD);

	//소켓과 서버 주소를 연결하고 등록 시킨다.
	iocp_net_server_.BindandListen(SERVER_PORT);

	iocp_net_server_.Run(MAX_CLIENT);

	//std::thread t1(P_MonsterMovemnet);

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

