#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>
#include "ThreadManager.h"

#include <vector>
#include <thread>

// 소수 구하기
bool IsPrime(int number)
{
	// 1이거나 1보다 작으면 끝
	if (number <= 1)
		return false;
	// 2거나 3면 소수
	if (number == 2 || number == 3)
		return true;
	// 1 2 3 이 아닌 그외 숫자들
	for (int i = 2; i < number; i++)
	{
		if ((number % i) == 0)
			return false;
	}
	// 나눠지는 수가 없으면 소수
	return true;
}

// [start ~ end]
int CountPrime(int start, int end)
{
	int count = 0;
	for (int number = start; number <= end; number++)
	{
		if (IsPrime(number))
			count++;
	}
	return count;
}

using namespace std;

// 1과 자기 자신으로만 나뉘면 그것을 소수라고 함

int main()
{
	const int MAX_NUMBER = 100'0000;
	// 1~MAX_NUMBER까지의 소수 개수
	vector<thread> threads;

	// 1000 = 168
	// 1'0000 = 1229
	// 100'0000 = 78498
	
	// hardware_concurrency() : CPU 하나에, 코어가 4개 달린 CPU를 사용중이라면 결과는 4가 나온다.
	int coreCount = thread::hardware_concurrency();
	// [(하드웨어 스레드 컨텍스트 수 * 2) + 1]만큼을 사용하면 효율이 좋다고 한다.
	int jobCount = (MAX_NUMBER / coreCount) + 1;

	atomic<int> primeCount = 0;

	for (int i = 0; i < coreCount; i++)
	{
		int start = (i * jobCount) + 1;
		int end = min(MAX_NUMBER, ((i + 1) * jobCount));

		threads.push_back(thread([start, end, &primeCount]()
			{
				primeCount += CountPrime(start, end);
			}));
	}

	for (thread& t : threads)
		t.join();

	cout << primeCount << endl;
}