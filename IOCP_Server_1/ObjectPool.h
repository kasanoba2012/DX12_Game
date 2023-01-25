#pragma once
#include <queue>
#include <stdexcept>
#include <memory>
using std::shared_ptr;

template<typename T>
class ClassPool
{
public:
	ClassPool(size_t chunkSize = DefalultChunkSize);
	shared_ptr<T> NewChunk();
	void DeleteChunk(shared_ptr<T> obj);
protected:
	size_t m_ChunkSize;
	static const size_t DefalultChunkSize = 10;
	std::queue<shared_ptr<T>> m_List;
	void AllocateChunk();
};

template<typename T>
ClassPool<T>::ClassPool(size_t chunkSize)
{
	m_ChunkSize = chunkSize;
	AllocateChunk();
}

template<typename T>
void ClassPool<T>::AllocateChunk()
{
	for (size_t i = 0; i < m_ChunkSize; i++)
	{
		m_List.push(std::make_shared<T>());
	}
}

template<typename T>
shared_ptr<T> ClassPool<T>::NewChunk()
{
	if (m_List.empty())
	{
		m_List.push(std::make_shared<T>());
		m_ChunkSize++;
	}
	auto chunk = m_List.front();
	m_List.pop();
	return chunk;
}

template<typename T>
void ClassPool<T>::DeleteChunk(shared_ptr<T> obj)
{
	m_List.push(obj);
}

template<typename T>
class ObjectPool
{
public:
	enum
	{
		POOL_MAX_SIZE = 4096,// 2n승
		POOL_SIZE_MASK = POOL_MAX_SIZE - 1,
	};
private:
	static void* m_mPool[POOL_MAX_SIZE];
	static long long m_HeadPos;
	static long long m_TailPos;
public:
	static void Allocation()
	{
		for (size_t i = 0; i < POOL_MAX_SIZE; i++)
		{
			// m_mPool[i] % MEMORY_ALLOCATION_ALIGNMENT = 0
			// _aligned_malloc : 메모리 시작 주소를 16바이트 정렬하겠다.
			// https://univ-developer.tistory.com/entry/c6
			m_mPool[i] = _aligned_malloc(sizeof(T), MEMORY_ALLOCATION_ALIGNMENT);
		}
		InterlockedAdd64(&m_TailPos, POOL_MAX_SIZE);
	}
	static void Release()
	{
		for (size_t i = 0; i < POOL_MAX_SIZE; i++)
		{
			// _aligned_free : _aligned_malloc 할당된 메모리 해제
			_aligned_free(m_mPool[i]);
		}
	}
	//POOL_SIZE_MASK = 0111
	//m_HeadPos = 0,  0000 & 0111 = 0
	//m_HeadPos = 1,  0001 & 0111 = 1
	//m_HeadPos = 2,  0010 & 0111 = 2
	//m_HeadPos = 3,  0011 & 0111 = 3
	//m_HeadPos = 4,  0100 & 0111 = 4
	//m_HeadPos = 5,  0101 & 0111 = 5
	//m_HeadPos = 6,  0110 & 0111 = 6
	//m_HeadPos = 7,  0111 & 0111 = 7
	static void* operator new (size_t size)
	{
		// InterlockedIncrement64 : 하나의 쓰레드에서 작동 할수 있도록 Lock
		size_t pos = InterlockedIncrement64(&m_HeadPos) - 1;
		size_t realpos = pos & POOL_SIZE_MASK;
		// ret=m_mPool[0] -> m_mPool[0]=null
		// InterlockedExchangePointer : 두번째 매개변수 주소를 첫번째 매개 변수에 복사하고 첫째 주소 리턴
		// 즉 m_mPool을 nullptr로 만듬
		void* ret = InterlockedExchangePointer(&m_mPool[realpos], nullptr);
		if (ret != nullptr)
		{
			return ret;
		}
		return _aligned_malloc(sizeof(T), MEMORY_ALLOCATION_ALIGNMENT);
	}
	static void operator delete (void* obj)
	{
		// InterlockedIncrement64 : 하나의 쓰레드에서 작동 할수 있도록 Lock
		size_t pos = InterlockedIncrement64(&m_TailPos) - 1;
		size_t realpos = pos & POOL_SIZE_MASK;
		// ret=m_mPool[0] -> m_mPool[0]=null
		// 교체전 값 반환.
		// InterlockedExchangePointer : 두번째 매개변수 주소를 첫번째 매개 변수에 복사하고 첫째 주소 리턴
		// 즉 m_mPool을 nullptr로 만듬
		void* ret = InterlockedExchangePointer(&m_mPool[realpos], obj);
		if (ret != nullptr)
		{
			// 메모리 해제
			_aligned_free(ret);
		}
	}
};
template<typename T>
void* ObjectPool<T>::m_mPool[POOL_MAX_SIZE] = {};
template<typename T>
long long ObjectPool<T>::m_HeadPos(0);
template<typename T>
long long ObjectPool<T>::m_TailPos(0);