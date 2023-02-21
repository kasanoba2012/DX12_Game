#pragma once
#include <iostream>
#include "FSM.h"
#include "BlueNpc.h"
#include "RedNpc.h"
#include "Player.h"

using namespace std;

class Vector3
{
public:
	float x;
	float y;
	float z;

	Vector3()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	Vector3(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

#ifndef 연산자 오버로딩

	friend Vector3 operator+(const Vector3& a, const Vector3& b) // 벡터 덧셈
	{
		float Vx = a.x + b.x;
		float Vy = a.y + b.y;
		float Vz = a.z + b.z;

		return Vector3(Vx, Vy, Vz);
	}
	friend Vector3 operator-(const Vector3& a, const Vector3& b) // 벡터 뺼셈
	{
		float Vx = a.x - b.x;
		float Vy = a.y - b.y;
		float Vz = a.z - b.z;

		return Vector3(Vx, Vy, Vz);
	}
	friend float operator*(const Vector3& a, const Vector3& b) //내적 구하기(스칼라 곱)
	{
		float Vx = a.x * b.x;
		float Vy = a.y * b.y;
		float Vz = a.z * b.z;

		return Vx + Vy + Vz;
	}
	bool operator==(Vector3 a) // 두 벡터가 같을떄
	{
		if (x == a.x && y == a.y && z == a.z)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

#endif // 연산자 오버로딩

	static float InnerCos(Vector3 v1, Vector3 v2) //각도 구하기
	{
		float Cos = (v1 * v2) / ((v1.Magnitude() * v2.Magnitude()));

		return Cos;
	}
	Vector3 Normalrize() // 정규화(단위벡터로 만들기)
	{
		Vector3 normal;
		float magnitude = this->Magnitude();
		normal.x = x / magnitude;
		normal.y = y / magnitude;
		normal.z = z / magnitude;

		return normal;
	}
	float Magnitude() // 벡터 크기
	{
		auto magnitude = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

		return magnitude;
	}
	static double Seta(float cos) //세타값 구하기 각도(디그리)
	{
		return acos(cos) * (180 / 3.14);
	}

	static Vector3 Proj(Vector3 v1, Vector3 v2) // 방향(normal) * 크기(magnitude = Proj(V1)
	{
		Vector3 result;

		result.x = ((v1 * v2) / (v2 * v2)) * v2.x;
		result.y = ((v1 * v2) / (v2 * v2)) * v2.y;
		result.z = ((v1 * v2) / (v2 * v2)) * v2.z;

		return result;
	}
	static  Vector3 Orthogonal_Projection(Vector3 v1, Vector3 proj) // 직교화
	{
		Vector3 ortho;
		ortho = v1 - proj;
		return ortho;
	}
	static Vector3 CrossProduct(Vector3 v1, Vector3 v2) //외적
	{
		Vector3 cross;
		cross.x = v1.y * v2.z - v1.z * v2.y;
		cross.y = v1.z * v2.x - v1.x - v2.z;
		cross.z = v1.x * v2.y - v1.y * v2.x;

		return cross;
	}
};

// 현재상태         이벤트				전이상태
/*--------------------------
Stand 멈춘 상태
// STAND         EVENT_TIMEMOVE           MOVE
// STAND         EVENT_POINTMOVE          MOVE
// STAND         EVENT_FINDTARGET         ATTACK
--------------------------*/

/*--------------------------
Move 움직이는 중
// MOVE          EVENT_STOPMOVE           STAND7
// MOVE          EVENT_DIRECTION          STAND
--------------------------*/

/*--------------------------
Attack 공격
// ATTACK        EVENT_LOSTTARGET         
--------------------------*/

int main()
{
	//DirectX::SimpleMath
	FSM fsm;
	// 가만히 서있다가 시간 지나면 움직이기
	fsm.AddTransition(STATE_STAND, EVENT_TIMEMOVE, STATE_MOVE);
	// 이동 시작
	fsm.AddTransition(STATE_STAND, EVENT_STARTMOVE, STATE_MOVE);
	// 방향 전환
	fsm.AddTransition(STATE_MOVE, EVENT_TRUNMOVE, STATE_MOVE);
	// 타켓 발견하면 타켓에게 다가가기
	fsm.AddTransition(STATE_STAND, EVENT_POINTMOVE, STATE_POINT_MOVE);
	// 가만히 서있다가 타켓 발견하면 공격
	fsm.AddTransition(STATE_STAND, EVENT_FINDTARGET, STATE_ATTACK);
	// 움직이다가 멈추기
	fsm.AddTransition(STATE_MOVE, EVENT_STOPMOVE, STATE_STAND);
	// 공격하다가 타켓 없어지면 멈추기
	fsm.AddTransition(STATE_ATTACK, EVENT_LOSTTARGET, STATE_STAND);

	Player player;
	BlueNpc blue_npc_(&fsm);
	RedNpc red_npc_(&fsm);

	while (1)
	{
		blue_npc_.Process(&player, &red_npc_);
		//red_npc_.Process(&player);
		// 5초에 한번씩 실행하기

		Sleep(1000);
	}
}