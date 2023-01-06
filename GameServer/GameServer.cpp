#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include <future>

using namespace std;

int64 Calculate()
{
	int64 sum = 0;

	for (int32 i = 0; i < 100'000; i++)
		sum += i;

	return sum;
}

void PromiseWorker(promise<string>&& promise)
{
	promise.set_value("Secret Message");
}

void TaskWorker(packaged_task<int64(void)>&& task)
{
	task();
}

int main()
{

	// std::future
	{
		// 1) deferred -> lazy evaluation 지연해서 실행하세요
		// 2) async -> 별도의 쓰레드를 만들어서 실행하세요
		// 3) deferred | async -> 둘 중 알아서 골라주세요

		// 언젠가 미래에 결과물을 뱉어줄거야!
		future<int64> future = async(launch::async, Calculate);

		// TODO
		// 결과물 필요시 get 호출

		int sum = future.get(); // 결과물이 이제서야 필요하다 get은 한번만 호출해야 한다.
	}

	// std::promise
	{
		promise<string> promise;
		future<string> future = promise.get_future();

		thread t(PromiseWorker, move(promise));

		string message = future.get();
		cout << message << endl;

		t.join();
	}

	// std::packaged_task
	{
		packaged_task <int64(void)> task(Calculate);
		future<int64> future = task.get_future();

		thread t(TaskWorker, move(task));

		int64 sum = future.get();
		cout << sum << endl;

		t.join();
	}

	// 결론)
	// mutex, condition_variable 까지 가지 않고 단순한 애들을 처리할 수 있음
	// 특히나, 한번 발생하는 단발성 이벤트에 적합하다.
	// 닭잡는데 소잡는 칼을 쓸 필요 없다.
	// 1) async
	// 원하는 함수를 비동기적으로 실행
	// 2) promise
	// 결과물을 promise를 통해 future로 받아줌
	// 3) packaged_task
	// 원하는 함수의 싫행 결과를 packaged_task를 통해 future로 받아줌
}