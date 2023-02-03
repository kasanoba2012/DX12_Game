#pragma once
#include "FiniteState.h"

class FSM
{
private:
	// ����(����), �ش� Ŭ����
	std::map<DWORD, FiniteState*> m_State;
public:
	void AddTransition(DWORD inState, DWORD event, DWORD outState);
	DWORD GetTransition(DWORD dwState, DWORD dwEvent);

	~FSM();
};

