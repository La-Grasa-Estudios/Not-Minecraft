#pragma once

#include <cstddef>
#include <utility>

void* sysAlloc(size_t size);
void* sysAlignedAlloc(size_t size, size_t align);
// Allocates a memory block in shared memory, always aligned to 32 bytes
void sysFree(void* ptr);
size_t sysMemoryUsage();

template<typename T, typename... Args> T* sysNew(Args&&... args)
{
	T* ptr = reinterpret_cast<T*>(sysAlloc(sizeof(T)));
	new (ptr) T(std::forward<Args>(args)...);
	return ptr;
}

template<typename T>
void sysDelete(T* ptr)
{
	ptr->~T();
	sysFree(ptr);
}
