#include "exceptionHandlers.h"
#include "../TaskScheduler.h"
#if defined(__TARGET_CPU_ARM920T)
	#include "../Mx1Devices/Mx1MemoryCache.h"
	typedef Mx1MemoryCache MemoryCache;
#endif
#if defined(__TARGET_CPU_SA_1100)
	#include "../Sa1110Devices/Sa1110MemoryCache.h"
	typedef Sa1110MemoryCache MemoryCache;
#endif

//------------------------------------------------------------------------------------------------
// * setExceptionHandler
//
// Sets the exception handler in the vector table at <vectorIndex> to <handler>.
//------------------------------------------------------------------------------------------------

void setExceptionHandler(ExceptionVectorIndex vectorIndex, ExceptionHandler handler)
{
	// add handler to the virtual vector table
	((ExceptionHandler *)0x20)[vectorIndex] = handler;

	// the real vector table makes a jump through the virtual vector table with the instruction
	// ldr pc, [pc, #0x18]
	((UInt *)0)[vectorIndex] = 0xe59ff018;

	// ensure coherency between data and instruction caches
	MemoryCache::getCurrentMemoryCache()->flushDataCacheEntry(&((UInt *)0)[vectorIndex]);
	MemoryCache::getCurrentMemoryCache()->drainWriteBuffer();
	MemoryCache::getCurrentMemoryCache()->flushInstructionCache();
}
