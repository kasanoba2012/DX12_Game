#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
vector<int32> v;

// Mutual Exclusive(상호배타적)
mutex m;

void Push()
{
	for (int32 i = 0; i < 10000; i++)
	{
		// 자물쇠 잠그기
		m.lock();
		v.push_back(i);
		// 자물쇠 풀기
		m.unlock();

		// 자동으로 잠궜다가 소멸자 호출 시 자동 잠그기 해제
		std::lock_guard<std::mutex> lockGuard(m);
		v.push_back(i);
	}
}
int main()
{
	std::thread t1(Push);
	std::thread t2(Push);
	t1.join();
	t2.join();

	cout << v.size() << endl;
}