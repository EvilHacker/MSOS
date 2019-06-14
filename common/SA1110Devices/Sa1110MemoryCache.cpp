#include "Sa1110MemoryCache.h"
#include "flushSa1110DataCache.h"
#include "../multitasking/UninterruptableSection.h"
#include "../pointerArithmetic.h"

//------------------------------------------------------------------------------------------------
// * Sa1110MemoryCache::flushDataCache
//
// Flush the entire data cache.
//------------------------------------------------------------------------------------------------

void Sa1110MemoryCache::flushDataCache()
{
	UninterruptableSection criticalSection;
	::flushSa1110DataCache();
}

//------------------------------------------------------------------------------------------------
// * Sa1110MemoryCache::flushDataCacheEntries
//
// Flush all data cache entries that overlap the specified buffer.
//------------------------------------------------------------------------------------------------

void Sa1110MemoryCache::flushDataCacheEntries(const void *address, UInt length)
{
	SInt paddedLength = length + ((UInt)address & (cacheLineSize - 1));

	UninterruptableSection criticalSection;
	do
	{
		asm
		{
			// clean and flush one cache line
			//mcr	p15, 0, address, c7, c14, 1
			mcr		p15, 0, address, c7, c10, 1
			mcr		p15, 0, address, c7, c6, 1
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
// * Sa1110MemoryCache static variables
//------------------------------------------------------------------------------------------------

Sa1110MemoryCache Sa1110MemoryCache::currentMemoryCache;
