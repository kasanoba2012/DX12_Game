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
	virtual void Process(Player* player, Npc* npc) = 0;
	NpcState() = delete;
	NpcState(Npc* npc)
	{
		m_pOwner = npc;
	}
};

class StandState : public NpcState
{
public:
	virtual void Process(Player* player, Npc* npc);
	StandState() = delete;
	StandState(Npc* npc) : NpcState(npc) {}
};

class MoveState : public NpcState
{
public:
	virtual void Process(Player* player, Npc* npc);
	MoveState() = delete;
	MoveState(Npc* npc) :NpcState(npc) {}
};

class AttackState : public NpcState
{
public:
	virtual void Process(Player* player, Npc* npc);
	AttackState() = delete;
	AttackState(Npc* npc) : NpcState(npc) {}
};

class PointMovekState : public NpcState
{
public:
	virtual void Process(Player* player, Npc* npc);
	PointMovekState() = delete;
	PointMovekState(Npc* npc) : NpcState(npc) {}
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
	void FsmAdd(FSM* fsm);
public:
	float m_NpcPos[2];
	Npc(FSM* fsm);
	Npc();
	virtual ~Npc();
};

