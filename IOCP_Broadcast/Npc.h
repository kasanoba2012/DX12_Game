#pragma once
#include <windows.h>
#include <vector>
#include <iostream>
#include "FSM.h"

class Npc;
class Player;

class BlueNpcState
{
public:
	Npc* m_pOwner = nullptr;
public:
	virtual void Process(Player* player, Npc* npc) = 0;
	BlueNpcState() = delete;
	BlueNpcState(Npc* npc)
	{
		m_pOwner = npc;
	}
};

class StandState : public BlueNpcState
{
public:
	virtual void Process(Player* player, Npc* npc);
	StandState() = delete;
	StandState(Npc* npc) : BlueNpcState(npc) {}
};

class MoveState : public BlueNpcState
{
public:
	virtual void Process(Player* player, Npc* npc);
	MoveState() = delete;
	MoveState(Npc* npc) :BlueNpcState(npc) {}
};

class AttackState : public BlueNpcState
{
public:
	virtual void Process(Player* player, Npc* npc);
	AttackState() = delete;
	AttackState(Npc* npc) : BlueNpcState(npc) {}
};

class PointMovekState : public BlueNpcState
{
public:
	virtual void Process(Player* player, Npc* npc);
	PointMovekState() = delete;
	PointMovekState(Npc* npc) : BlueNpcState(npc) {}
};

class Npc
{
	DWORD m_dwState;
	FSM* m_pFsm = nullptr;
public:
	BlueNpcState* m_pCurentState = nullptr;
	std::vector<BlueNpcState*> m_pActionList;
	void Process(Player* player);
	void SetTransition(DWORD dwEvent);
	void FsmAdd(FSM* fsm);
public:
	float m_NpcPos[2];
	Npc(FSM* fsm);
	Npc();
	virtual ~Npc();
};

