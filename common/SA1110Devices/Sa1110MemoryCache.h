#ifndef _Sa1110MemoryCache_h_
#define _Sa1110MemoryCache_h_

#include "../cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * class Sa1110MemoryCache
//
// Interface to the SA-1110 Caches.
//------------------------------------------------------------------------------------------------

class Sa1110MemoryCache
{
public:
	// accessing
	inline static Sa1110MemoryCache *getCurrentMemoryCache();

	// flushing
	inline void flushInstructionCache();
	void flushDataCache();
	inline void flushDataCacheEntry(const void *address);
	void flushDataCacheEntries(const void *address, UInt length);
	inline void drainWriteBuffer();

private:
	// constructor
	inline Sa1110MemoryCache();

	// cache organization
	static const UInt instructionCacheSize = 16384;
	static const UInt dataCacheSize = 8192;
	static const UInt cacheLineSize = 32;

	// singleton
	static Sa1110MemoryCache currentMemoryCache;
};

#include "../../multitasking/UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * Sa1110MemoryCache::Sa1110MemoryCache
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline Sa1110MemoryCache::Sa1110MemoryCache()
{
}

//------------------------------------------------------------------------------------------------
// * Sa1110MemoryCache::getCurrentMemoryCache
//
// Returns the singleton instance.
//------------------------------------------------------------------------------------------------

inline Sa1110MemoryCache *Sa1110MemoryCache::getCurrentMemoryCache()
{
	return &currentMemoryCache;
}

//------------------------------------------------------------------------------------------------
// * Sa1110MemoryCache::flushInstructionCache
//
// Flush the entire instruction cache.
//------------------------------------------------------------------------------------------------

inline void Sa1110MemoryCache::flushInstructionCache()
{
	asm
	{
		mcr		p15, 0, 0, c7, c5, 0
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110MemoryCache::flushDataCacheEntry
//
// Flush one cache line from the data cache.
//------------------------------------------------------------------------------------------------

inline void Sa1110MemoryCache::flushDataCacheEntry(const void *address)
{
	UninterruptableSection criticalSection;
	asm
	{
		// clean and flush one cache line
		//mcr	p15, 0, address, c7, c14, 1
		mcr		p15, 0, address, c7, c10, 1
		mcr		p15, 0, address, c7, c6, 1

		// drain the write buffer
		mcr		p15, 0, 0, c7, c10, 4
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110MemoryCache::drainWriteBuffer
//
// Wait for data in the write buffer to be written to main memory.
//------------------------------------------------------------------------------------------------

inline void Sa1110MemoryCache::drainWriteBuffer()
{
	asm
	{
		mcr		p15, 0, 0, c7, c10, 4
	}
}

#endif // _Sa1110MemoryCache_h_
