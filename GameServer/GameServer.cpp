#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>

mutex m; 
queue<int32> q;
HANDLE handle;

// 참고) CV는 User-Level Object(커널 오브젝트X)
condition_variable cv;

void Producer()
{
	while (true)
	{
		// Event 보다는 condition_variable 사용을 권장
		// condition_variable
		// 1) Lock을 잡고
		// 2) 공유 변수 값을 수정
		// 3) Lock을 풀고
		// 4) 조건변수를 통해 다른 쓰레드에게 통지
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		cv.notify_one(); // wait중인 쓰레드가 있으면 딱 1개를 깨운다
	}	
}

void Consumer()
{
	while (true)
	{
		unique_lock<mutex> lock(m);
		cv.wait(lock, []() {return q.empty() == false; });

		// 1) Lock을 잡고
		// 2) 조건 확인
		// - 만족 O => 빠져 나와서 이어서 코드를 짆애
		// - 만족 X => Lock을 풀어주고 대기
		{
			int32 data = q.front();
			q.pop();
			cout << data << endl;
		}
	}
}

int main()
{
	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();
}