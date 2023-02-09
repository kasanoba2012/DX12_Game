#include "FSMServer.h"

FSMServer::FSMServer()
{
	npc.FsmAdd(&fsm);
}

void FSMServer::NpcMovement(Npc* npc)
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

		printf("npc 위치 X : %d Y : %d\n", (int)npc->m_NpcPos[0], (int)npc->m_NpcPos[1]);
	}
}

void FSMServer::ThreadTestfuntion()
{
	while (1)
	{
		std::cout << "타 클라스 함수 호출\n";
		Sleep(500);
	}
}
