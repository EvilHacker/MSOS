#ifndef _Mx1MemoryCache_h_
#define _Mx1MemoryCache_h_

#include "../cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * class Mx1MemoryCache
//
// Interface to the MX1 Caches.
//------------------------------------------------------------------------------------------------

class Mx1MemoryCache
{
public:
	// accessing
	inline static Mx1MemoryCache *getCurrentMemoryCache();

	// flushing
	inline void flushInstructionCache();
	void flushDataCache();
	void cleanDataCache();
	inline void flushDataCacheEntry(const void *address);
	inline void cleanDataCacheEntry(const void *address);
	void flushDataCacheEntries(const void *address, UInt length);
	void cleanDataCacheEntries(const void *address, UInt length);
	inline void drainWriteBuffer();

private:
	// constructor
	inline Mx1MemoryCache();

	// cache organization
	static const UInt instructionCacheSize = 16384;
	static const UInt dataCacheSize = 16384;
	static const UInt cacheLineSize = 32;

	// singleton
	static Mx1MemoryCache currentMemoryCache;
};

//------------------------------------------------------------------------------------------------
// * Mx1MemoryCache::Mx1MemoryCache
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline Mx1MemoryCache::Mx1MemoryCache()
{
}

//------------------------------------------------------------------------------------------------
// * Mx1MemoryCache::getCurrentMemoryCache
//
// Returns the singleton instance.
//------------------------------------------------------------------------------------------------

inline Mx1MemoryCache *Mx1MemoryCache::getCurrentMemoryCache()
{
	return &currentMemoryCache;
}

//------------------------------------------------------------------------------------------------
// * Mx1MemoryCache::flushInstructionCache
//
// Flush the entire instruction cache.
//------------------------------------------------------------------------------------------------

inline void Mx1MemoryCache::flushInstructionCache()
{
	asm
	{
		mcr		p15, 0, 0, c7, c5, 0
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1MemoryCache::flushDataCacheEntry
//
// Flush one cache line from the data cache.
//------------------------------------------------------------------------------------------------

inline void Mx1MemoryCache::flushDataCacheEntry(const void *address)
{
	asm
	{
		// clean and invalidate one cache line
		mcr		p15, 0, address, c7, c14, 1
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1MemoryCache::cleanDataCacheEntry
//
// Clean one cache line from the data cache.
//------------------------------------------------------------------------------------------------

inline void Mx1MemoryCache::cleanDataCacheEntry(const void *address)
{
	asm
	{
		// clean one cache line
		mcr		p15, 0, address, c7, c10, 1
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1MemoryCache::drainWriteBuffer
//
// Wait for data in the write buffer to be written to main memory.
//------------------------------------------------------------------------------------------------

inline void Mx1MemoryCache::drainWriteBuffer()
{
	asm
	{
		mcr		p15, 0, 0, c7, c10, 4
	}
}

#endif // _Mx1MemoryCache_h_
