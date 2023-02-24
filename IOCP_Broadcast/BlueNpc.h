#pragma once
#include <windows.h>
#include <vector>
#include <iostream>
#include "FSM.h"

class BlueNpc;
class RedNpc;
class Player;

class BlueNpcState
{
public:
	BlueNpc* m_pOwner = nullptr;
public:
	virtual void Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc) = 0;
	BlueNpcState() = delete;
	BlueNpcState(BlueNpc* blue_npc)
	{
		m_pOwner = blue_npc;
	}
};

class BlueStandState : public BlueNpcState
{
public:
	virtual void Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc);
	BlueStandState() = delete;
	BlueStandState(BlueNpc* blue_npc) : BlueNpcState(blue_npc) {}
	int stand_sw_ = 0;
};

class BlueMoveState : public BlueNpcState
{
public:
	virtual void Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc);
	BlueMoveState() = delete;
	BlueMoveState(BlueNpc* blue_npc) :BlueNpcState(blue_npc) {}
};

class BlueAttackState : public BlueNpcState
{
public:
	virtual void Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc);
	BlueAttackState() = delete;
	BlueAttackState(BlueNpc* blue_npc) : BlueNpcState(blue_npc) {}
};


//void BluePointMovekState::Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc)
class BluePointMovekState : public BlueNpcState
{
public:
	virtual void Process(Player* player, BlueNpc* blue_npc, RedNpc* red_npc);
	BluePointMovekState() = delete;
	BluePointMovekState(BlueNpc* blue_npc) : BlueNpcState(blue_npc) {}
};

class BlueNpc
{

	struct minion
	{
		int my_index = 0;
		int npc_pos_[3];
		int npc_pos_dir_ = 5;
		int team_color = 0;
		int npc_speed = 1;
	};
	DWORD m_dwState;
	FSM* m_pFsm = nullptr;
public:
	BlueNpcState* m_pCurentState = nullptr;
	std::vector<BlueNpcState*> m_pActionList;
	void Process(Player* player, RedNpc* red_npc);
	void SetTransition(DWORD dwEvent);
	bool NpcChangeDirection();
private:
	bool TargetRange(RedNpc* red_npc);
public:
	//float m_NpcPos[2];
	BlueNpc();
	BlueNpc(FSM* fsm);
	void SetFsm(FSM* fsm);
	virtual ~BlueNpc();
	minion npc_info_;
	int npc_event = 0;
	int event_cnt_ = 0;

	bool move_sw_ = false;
};

