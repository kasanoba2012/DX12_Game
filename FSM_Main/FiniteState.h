#pragma once
#include <Windows.h>
#include <map>

// ���� ���� ������
enum FSMState
{
	STATE_STAND = 0,
	STATE_MOVE,
	STATE_ATTACK,
	STATE_COUNT,
};

// �̺�Ʈ ������
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
	// �̺�Ʈ, ��� ����
	std::map<DWORD, DWORD> m_State;

public:
	void  AddTransition(DWORD dwEvent, DWORD outState);
	DWORD GetTransition(DWORD dwEvent)
	{
		return m_State[dwEvent];
	}
};

