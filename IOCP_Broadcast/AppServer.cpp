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
	// FSM ����
	//BlueNpc blue_npc_(&fsm);
}

void AppServer::ThreadTestfuntion()
{
	while (1)
	{
		//std::cout << "Ÿ Ŭ�� �Լ� ȣ��\n";
		Sleep(500);
	}
}
