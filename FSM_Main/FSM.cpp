#include <iostream>
#include "FSM.h"

void FSM::AddTransition(DWORD inState, DWORD dwEvent, DWORD outState)
{
    FiniteState* pState = nullptr;
    // 들어온 State 찾기
    auto ret = m_State.find(inState);
    // 들어온 State 있는 경우
    if (ret != m_State.end())
    {
        // pState에 들어온 상태값 넣기
        pState = ret->second;
    }
    else {
        pState = new FiniteState();
        m_State[inState] = pState;
    }
    pState->AddTransition(dwEvent, outState);
}

DWORD FSM::GetTransition(DWORD dwState, DWORD dwEvent)
{
    return m_State[dwState]->GetTransition(dwEvent);
}

FSM::~FSM()
{
    for (auto list : m_State)
    {
        delete list.second;
    }
    m_State.clear();
}
