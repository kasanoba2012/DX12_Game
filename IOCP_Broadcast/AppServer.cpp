#include "AppServer.h"

AppServer::AppServer()
{
	npc.FsmAdd(&fsm);
}

void AppServer::NpcMovement(Blue_Npc* npc)
{
	while (1)
	{
		if (MovementSw) {
			npc->m_NpcPos[0] += 1;
			Sleep(300);
			if (npc->m_NpcPos[0] >= 30)
			{
				MovementSw = false;
			}
		}

		else {
			npc->m_NpcPos[0] -= 1;
			Sleep(300);
			if (npc->m_NpcPos[0] <= 0)
			{
				MovementSw = true;
			}
		}

		printf("npc ��ġ X : %d Y : %d\n", (int)npc->m_NpcPos[0], (int)npc->m_NpcPos[1]);
	}
}

void AppServer::ThreadTestfuntion()
{
	while (1)
	{
		std::cout << "Ÿ Ŭ�� �Լ� ȣ��\n";
		Sleep(500);
	}
}
