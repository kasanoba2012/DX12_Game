#pragma once
#include <windows.h>
#include <vector>
#include <iostream>
#include "FSM.h"

class RedNpc;
class Player;

class RedNpcState
{
public:
	RedNpc* m_pOwner = nullptr;
public:
	virtual void Process(Player* player, RedNpc* red_npc) = 0;
	RedNpcState() = delete;
	RedNpcState(RedNpc* red_npc)
	{
		m_pOwner = red_npc;
	}
};

class RedStandState : public RedNpcState
{
public:
	virtual void Process(Player* player, RedNpc* red_npc);
	RedStandState() = delete;
	RedStandState(RedNpc* red_npc) : RedNpcState(red_npc) {}
	int stand_sw_ = 0;
};

class RedMoveState : public RedNpcState
{
public:
	virtual void Process(Player* player, RedNpc* red_npc);
	RedMoveState() = delete;
	RedMoveState(RedNpc* red_npc) :RedNpcState(red_npc) {}
};

class RedAttackState : public RedNpcState
{
public:
	virtual void Process(Player* player, RedNpc* red_npc);
	RedAttackState() = delete;
	RedAttackState(RedNpc* red_npc) : RedNpcState(red_npc) {}
};

class RedPointMovekState : public RedNpcState
{
public:
	virtual void Process(Player* player, RedNpc* red_npc);
	RedPointMovekState() = delete;
	RedPointMovekState(RedNpc* red_npc) : RedNpcState(red_npc) {}
};

class RedNpc
{

	struct minion
	{
		float npc_pos_[3];
		int npc_pos_dir_ = 1;
		int team_color = 1;
		int npc_speed = 1;
	};
	DWORD m_dwState;
	FSM* m_pFsm = nullptr;
public:
	RedNpcState* m_pCurentState = nullptr;
	std::vector<RedNpcState*> m_pActionList;
	void Process(Player* player);
	void SetTransition(DWORD dwEvent);
public:
	//float m_NpcPos[2];
	RedNpc(FSM* fsm);
	virtual ~RedNpc();
	minion npc_info_;
	int npc_event = 0;
};

