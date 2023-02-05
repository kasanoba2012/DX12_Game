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

FSM::FSM()
{
    AddTransition(STATE_STAND, EVENT_TIMEMOVE, STATE_MOVE);
    // 타켓 발견하면 타켓에게 다가가기
    AddTransition(STATE_STAND, EVENT_POINTMOVE, STATE_POINT_MOVE);
    // 가만히 서있다가 타켓 발견하면 공격
    AddTransition(STATE_STAND, EVENT_FINDTARGET, STATE_ATTACK);
    // 움직이다가 멈추기
    AddTransition(STATE_MOVE, EVENT_STOPMOVE, STATE_STAND);
    // 공격하다가 타켓 없어지면 멈추기
    AddTransition(STATE_ATTACK, EVENT_LOSTTARGET, STATE_STAND);
}

FSM::~FSM()
{
    for (auto list : m_State)
    {
        delete list.second;
    }
    m_State.clear();
}
