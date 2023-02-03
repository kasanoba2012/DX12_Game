#pragma once
#include <windows.h>
#include <vector>
#include <iostream>
#include "FSM.h"

class Npc;
class Player;

class NpcState
{
public:
	Npc* m_pOwner = nullptr;
public:
	virtual void Process(Player* player) = 0;
	NpcState() = delete;
	NpcState(Npc* npc)
	{
		m_pOwner = npc;
	}
};

class StandState : public NpcState
{
public:
	virtual void Process(Player* player);
	StandState() = delete;
	StandState(Npc* npc) : NpcState(npc) {}
};

class MoveState : public NpcState
{
public:
	virtual void Process(Player* player);
	MoveState() = delete;
	MoveState(Npc* npc) :NpcState(npc) {}
};

class AttackState : public NpcState
{
public:
	virtual void Process(Player* player);
	AttackState() = delete;
	AttackState(Npc* npc) : NpcState(npc) {}
};

class Npc
{
	DWORD m_dwState;
	FSM* m_pFsm = nullptr;
public:
	NpcState* m_pCurentState = nullptr;
	std::vector<NpcState*> m_pActionList;
	void Process(Player* player);
	void SetTransition(DWORD dwEvent);
public:
	float m_NpcPos[2];
	Npc(FSM* fsm);
	virtual ~Npc();
};

