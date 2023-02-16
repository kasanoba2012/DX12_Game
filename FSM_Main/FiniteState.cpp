#include "FiniteState.h"

void FiniteState::AddTransition(DWORD dwEvent, DWORD outState)
{
	m_State[dwEvent] = outState;
}
