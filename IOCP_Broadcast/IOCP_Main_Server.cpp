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

	iocp_net_server_.NpcRun();

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

