#pragma once
#include <Windows.h>
#include <map>

// 현재 상태 열거형
enum FSMState
{
	STATE_STAND = 0,
	STATE_MOVE,
	STATE_ATTACK,
	STATE_COUNT,
};

// 이벤트 열거형
enum FSMEvent
{
	EVENT_STOPMOVE = 0,
	EVENT_TIMEMOVE,
	EVENT_POINTMOVE,
	EVENT_FINDTARGET,
	EVENT_LOSTTARGET,
	EVENT_COUNT,
};

class FiniteState
{
public:
	// 이벤트, 결과 상태
	std::map<DWORD, DWORD> m_State;

public:
	void  AddTransition(DWORD dwEvent, DWORD outState);
	DWORD GetTransition(DWORD dwEvent)
	{
		return m_State[dwEvent];
	}
};

