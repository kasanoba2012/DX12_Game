#pragma once

#include <mutex>

template<typename T>

class LockStack
{
public:
	LockStack() {}

	// 복사 기능 삭제
	LockStack(const LockStack&) = delete;
	// 대입 기능 삭제
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_stack.push(std::move(value));
		// notify_one : _condVar 통지
		_condVar.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_stack.empty())
			return false;

		// empty -> top -> pop
		value = std::move(_stack.top());
		_stack.pop();

		return true;
	}

	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(_mutex);
		// _condVar.wait : _condVar.notify_one 통지가 올 때 까지 대기
		_condVar.wait(lock, [this] {return _stack.empty() == false; });
		// top() 값은 꺼내지 않고 제일 위에 값을 리턴
		value = std::move(_stack.top());
		_stack.pop();
	}

private:
	stack<T> _stack;
	mutex _mutex;
	condition_variable _condVar;
};

