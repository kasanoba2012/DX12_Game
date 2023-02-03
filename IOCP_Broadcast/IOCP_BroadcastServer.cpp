﻿#include "IOCompletionPort.h"
#include <string>
#include <iostream>

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
	}

	ioCompletionPort.DestroyThread();
	return 0;
}

