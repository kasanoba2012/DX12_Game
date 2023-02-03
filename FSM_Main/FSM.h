#pragma once
#include "FiniteState.h"

class FSM
{
private:
	// 상태(동작), 해당 클래스
	std::map<DWORD, FiniteState*> m_State;
public:
	void AddTransition(DWORD inState, DWORD event, DWORD outState);
	DWORD GetTransition(DWORD dwState, DWORD dwEvent);

	~FSM();
};

