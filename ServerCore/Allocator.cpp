#include "pch.h"
#include "Allocator.h"

/*-------------------
	BaseAllocator
-------------------*/

void* BaseAllocator::Alloc(int32 size)
{
	// 皋葛府 积己
	return ::malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
	// 皋葛府 昏力
	::free(ptr);
}
