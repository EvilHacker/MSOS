#include "Mx1MemoryCache.h"
#include "../pointerArithmetic.h"

//------------------------------------------------------------------------------------------------
// * Mx1MemoryCache::flushDataCache
//
// Flush the entire data cache.
//------------------------------------------------------------------------------------------------

void Mx1MemoryCache::flushDataCache()
{
	asm
	{
		mov		a1, #0
		mov		a2, #8
	segmentLoop:
	indexLoop:
		mcr		p15, 0, a1, c7, c14, 2
		adds	a1, a1, #0x04000000
		bcc		indexLoop
		subs	a2, a2, #1
		add		a1, a1, #0x00000020
		bne		segmentLoop
		mcr		p15, 0, a2, c7, c10, 4
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1MemoryCache::cleanDataCache
//
// Clean the entire data cache.
//------------------------------------------------------------------------------------------------

void Mx1MemoryCache::cleanDataCache()
{
	asm
	{
		mov		a1, #0
		mov		a2, #8
	segmentLoop:
	indexLoop:
		mcr		p15, 0, a1, c7, c10, 2
		adds	a1, a1, #0x04000000
		bcc		indexLoop
		subs	a2, a2, #1
		add		a1, a1, #0x00000020
		bne		segmentLoop
		mcr		p15, 0, a2, c7, c10, 4
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1MemoryCache::flushDataCacheEntries
//
// Flush all data cache entries that overlap the specified buffer.
//------------------------------------------------------------------------------------------------

void Mx1MemoryCache::flushDataCacheEntries(const void *address, UInt length)
{
	SInt paddedLength = length + ((UInt)address & (cacheLineSize - 1));

	do
	{
		asm
		{
			// clean and invalidate one cache line
			mcr		p15, 0, address, c7, c14, 1
		}

		// advance to the next cache line
		address = addToPointer(address, cacheLineSize);
		paddedLength -= cacheLineSize;
	}
	while(paddedLength > 0);

	asm
	{
		// drain the write buffer
		mcr		p15, 0, 0, c7, c10, 4
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1MemoryCache::cleanDataCacheEntries
//
// Clean all data cache entries that overlap the specified buffer.
//------------------------------------------------------------------------------------------------

void Mx1MemoryCache::cleanDataCacheEntries(const void *address, UInt length)
{
	SInt paddedLength = length + ((UInt)address & (cacheLineSize - 1));

	do
	{
		asm
		{
			// clean one cache line
			mcr		p15, 0, address, c7, c10, 1
		}

		// advance to the next cache line
		address = addToPointer(address, cacheLineSize);
		paddedLength -= cacheLineSize;
	}
	while(paddedLength > 0);

	asm
	{
		// drain the write buffer
		mcr		p15, 0, 0, c7, c10, 4
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1MemoryCache static variables
//------------------------------------------------------------------------------------------------

Mx1MemoryCache Mx1MemoryCache::currentMemoryCache;
