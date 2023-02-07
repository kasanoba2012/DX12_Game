#pragma once
#include "IOCompletionPort.h"
#include "Npc.h"
#include "Player.h"

class FSMServer : public IOCompletionPort
{

public:
	// FSM 세팅
	FSM fsm;
	Npc npc;

	bool MovementSw = true;
public:
	FSMServer();
	void NpcMovement(Npc* npc);
	void ThreadTestfuntion();

	virtual void OnConnect(const UINT32 clientIndex_) override
	{
		// 접속 되면 플레이어 생성
		Player player;

		// TODO 접속하면 npc 따라가기 시작 근데 미리 좌표는 움직이고 있어야한다.
		while (1)
		{
			printf("Connect npc 좌표 : %d\n", (int)npc.m_NpcPos[0]);

			MovementSw = false;

			char npcPosMsg[256] = { 0, };
			*npcPosMsg = npc.m_NpcPos[0];
			npcPosMsg[1] = '\0';
			mainSendMsg(npcPosMsg);

			npc.Process(&player);
			Sleep(3000);
		}

		printf("[OnConnect] 클라이언트: Index(%d)\n", clientIndex_);
	}

	virtual void OnClose(const UINT32 clientIndex_) override
	{
		printf("[OnClose] 클라이언트: Index(%d)\n", clientIndex_);
	}

	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) override
	{
		printf("[OnReceive] 클라이언트: Index(%d), dataSize(%d)\n", clientIndex_, size_);
	}
};

