#pragma once
#include <iostream>
#include "FSM.h"
#include "Npc.h"
#include "Player.h"

// 현재상태         이벤트				전이상태
// STAND         EVENT_TIMEMOVE           MOVE
// STAND         EVENT_POINTMOVE          MOVE
// STAND         EVENT_FINDTARGET         ATTACK
// MOVE          EVENT_STOPMOVE           STAND
// ATTACK        EVENT_LOSTTARGET         STAND

int main()
{
	FSM fsm;
	// 가만히 서있다가 시간 지나면 움직이기
	fsm.AddTransition(STATE_STAND, EVENT_TIMEMOVE, STATE_MOVE);
	// 타켓 발견하면 타켓에게 다가가기
	fsm.AddTransition(STATE_STAND, EVENT_POINTMOVE, STATE_POINT_MOVE);
	// 가만히 서있다가 타켓 발견하면 공격
	fsm.AddTransition(STATE_STAND, EVENT_FINDTARGET, STATE_ATTACK);
	// 움직이다가 멈추기
	fsm.AddTransition(STATE_MOVE, EVENT_STOPMOVE, STATE_STAND);
	// 공격하다가 타켓 없어지면 멈추기
	fsm.AddTransition(STATE_ATTACK, EVENT_LOSTTARGET, STATE_STAND);

	Player player;
	Npc npc(&fsm);

	while (1)
	{
		npc.Process(&player);
		// 5초에 한번씩 실행하기

		Sleep(1000);
	}
}