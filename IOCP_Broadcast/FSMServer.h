#pragma once
#include "IOCompletionPort.h"
#include "Npc.h"
#include "Player.h"

class FSMServer : public IOCompletionPort
{

public:
	// FSM ����
	FSM fsm;
	Npc npc;

	bool MovementSw = true;
public:
	FSMServer();
	void NpcMovement(Npc* npc);
	void ThreadTestfuntion();

	virtual void OnConnect(const UINT32 clientIndex_) override
	{
		// ���� �Ǹ� �÷��̾� ����
		Player player;

		// TODO �����ϸ� npc ���󰡱� ���� �ٵ� �̸� ��ǥ�� �����̰� �־���Ѵ�.
		while (1)
		{
			printf("Connect npc ��ǥ : %d\n", (int)npc.m_NpcPos[0]);

			MovementSw = false;

			char npcPosMsg[256] = { 0, };
			*npcPosMsg = npc.m_NpcPos[0];
			npcPosMsg[1] = '\0';
			mainSendMsg(npcPosMsg);

			npc.Process(&player);
			Sleep(3000);
		}

		printf("[OnConnect] Ŭ���̾�Ʈ: Index(%d)\n", clientIndex_);
	}

	virtual void OnClose(const UINT32 clientIndex_) override
	{
		printf("[OnClose] Ŭ���̾�Ʈ: Index(%d)\n", clientIndex_);
	}

	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) override
	{
		printf("[OnReceive] Ŭ���̾�Ʈ: Index(%d), dataSize(%d)\n", clientIndex_, size_);
	}
};

