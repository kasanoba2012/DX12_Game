#pragma once
#include <Windows.h>
#include <map>

// ���� ���� ������
enum FSMState
{
	STATE_STAND = 0, // ���ֱ�(����)
	STATE_MOVE, // ����
	STATE_ATTACK, // ����
	STATE_POINT_MOVE, // Ÿ�� ����
	STATE_ATTACK_HIT, // �ǰ�
	STATE_COUNT,
};

// �̺�Ʈ ������
enum FSMEvent
{
	EVENT_STOPMOVE = 0, // ����
	EVENT_STARTMOVE, // �̵�
	EVENT_TIMEMOVE, // ���� �ð� �� �����̱�
	EVENT_TRUNMOVE, // ���� ��ȯ
	EVENT_POINTMOVE, //  Ÿ�� ����
	EVENT_FINDTARGET, // Ÿ�� �߰�
	EVENT_LOSTTARGET, // Ÿ�� ��ħ
	EVENT_ATTACK_HIT, // �ǰ�
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

