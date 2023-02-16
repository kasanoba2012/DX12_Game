#include <iostream>
#include "FSM.h"

void FSM::AddTransition(DWORD inState, DWORD dwEvent, DWORD outState)
{
    FiniteState* pState = nullptr;
    // ���� State ã��
    auto ret = m_State.find(inState);
    // ���� State �ִ� ���
    if (ret != m_State.end())
    {
        // pState�� ���� ���°� �ֱ�
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
