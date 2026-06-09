#include "MemoryAllocator.h"

// Should delete this

#include <cstring>
#include <string>

#ifdef _WIN32
#define memalign(a, sz) malloc(sz)
#endif
#ifdef __wii__
#define HW_RVL
unsigned int MALLOC_MEM2 __attribute__((weak)) = 1;
#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#endif
size_t heapSize = 0;

static size_t alingTo(size_t cbSize, size_t cbAlign)
{
	size_t mask = cbAlign - 1;
	return (cbSize + mask) & ~mask;
}

#ifdef __wii__

void* sysAlloc(size_t size)
{
	size_t sz = size; // alingTo(size, 32) + 8;
	void* ptr = malloc(sz);
	memset(ptr, 0, sz);
	//reinterpret_cast<size_t*>(ptr)[0] = sz;
	//heapSize += sz;
	return reinterpret_cast<char*>(ptr);
}

void* sysAlignedAlloc(size_t size, size_t align)
{
	size_t sz = size;
	void* ptr = memalign(align, sz);
	memset(ptr, 0, sz);
	return ptr;
}

void sysFree(void* ptr)
{
	if (!ptr)
		return;
	ptr = reinterpret_cast<char*>(ptr);
	//heapSize -= reinterpret_cast<size_t*>(ptr)[0];
	free(ptr);
}

#else

void* sysAlloc(size_t size)
{
	auto ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
	//size_t sz = alingTo(size, 32) + 8;
	//void* ptr = malloc(sz);
	//memset(ptr, 0, sz);
	//reinterpret_cast<size_t*>(ptr)[0] = sz;
	//heapSize += sz;
	//return reinterpret_cast<char*>(ptr) + sizeof(size_t);
}

void* sysAlignedAlloc(size_t size, size_t align)
{
	auto ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
	//size_t sz = size;
	//void* ptr = memalign(align, sz);
	//memset(ptr, 0, sz);
	//return ptr;
}

void sysFree(void* ptr)
{
	if (!ptr)
		return;
	free(ptr);
	return;
	ptr = reinterpret_cast<char*>(ptr) - sizeof(size_t);
	heapSize -= reinterpret_cast<size_t*>(ptr)[0];
	free(ptr);
}

#endif

size_t sysMemoryUsage()
{
#ifdef __wii__
	return 0x05800000 - (SYS_GetArena1Size() + SYS_GetArena2Size());
#endif
	return heapSize;
}

void* operator new(std::size_t size)
{
	return sysAlloc(size);
}

void operator delete(void* ptr) noexcept
{
	sysFree(ptr);
}