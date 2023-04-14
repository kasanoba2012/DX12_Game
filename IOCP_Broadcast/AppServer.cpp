#include "AppServer.h"
//FSM
#include "FSM.h"
#include "BlueNpc.h"
#include "RedNpc.h"
#include "Player.h"

AppServer::AppServer()
{
	FsmInit();
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
		//std::cout << "타 클라스 함수 호출\n";
		Sleep(500);
	}
}
