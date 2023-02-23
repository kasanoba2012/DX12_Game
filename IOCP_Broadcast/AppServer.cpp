#include "AppServer.h"
//FSM
//#include "FSM.h"
//#include "BlueNpc.h"
//#include "RedNpc.h"
//#include "Player.h"

AppServer::AppServer()
{
	//FsmInit();
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
		std::cout << "Ÿ Ŭ�� �Լ� ȣ��\n";
		Sleep(500);
	}
}

void AppServer::FSM_RUN()
{
	// ������ ���ִٰ� �ð� ������ �����̱�
	fsm.AddTransition(STATE_STAND, EVENT_TIMEMOVE, STATE_MOVE);
	// �̵� ����
	fsm.AddTransition(STATE_STAND, EVENT_STARTMOVE, STATE_MOVE);
	// ���� ��ȯ
	fsm.AddTransition(STATE_MOVE, EVENT_TRUNMOVE, STATE_MOVE);
	// Ÿ�� �߰��ϸ� Ÿ�Ͽ��� �ٰ�����
	fsm.AddTransition(STATE_STAND, EVENT_POINTMOVE, STATE_POINT_MOVE);
	// ������ ���ִٰ� Ÿ�� �߰��ϸ� ����
	fsm.AddTransition(STATE_STAND, EVENT_FINDTARGET, STATE_ATTACK);
	// �����̴ٰ� ���߱�
	fsm.AddTransition(STATE_MOVE, EVENT_STOPMOVE, STATE_STAND);
	// �����ϴٰ� Ÿ�� �������� ���߱�
	fsm.AddTransition(STATE_ATTACK, EVENT_LOSTTARGET, STATE_STAND);
	
	blue_npc_.SetFsm(&fsm);

	while (1)
	{
		blue_npc_.Process(&player, &red_npc_);

		Sleep(1000);
	}
}
