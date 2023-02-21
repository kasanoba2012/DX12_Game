#pragma once
#include <Windows.h>
#include <map>

// 현재 상태 열거형
enum FSMState
{
	STATE_STAND = 0, // 서있기(정지)
	STATE_MOVE, // 무브
	STATE_ATTACK, // 공격
	STATE_POINT_MOVE, // 타켓 추적
	STATE_ATTACK_HIT, // 피격
	STATE_COUNT,
};

// 이벤트 열거형
enum FSMEvent
{
	EVENT_STOPMOVE = 0, // 정지
	EVENT_STARTMOVE, // 이동
	EVENT_TIMEMOVE, // 일정 시간 뒤 움직이기
	EVENT_TRUNMOVE, // 방향 전환
	EVENT_POINTMOVE, //  타켓 추적
	EVENT_FINDTARGET, // 타켓 발견
	EVENT_LOSTTARGET, // 타켓 놓침
	EVENT_ATTACK_HIT, // 피격
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

