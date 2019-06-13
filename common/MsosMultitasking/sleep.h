#ifndef _sleep_h_
#define _sleep_h_

#include "TaskScheduler.h"
#include "IntertaskEvent.h"
#if defined(__TARGET_CPU_ARM920T) || defined(__TARGET_CPU_SA_1100)
	#define TIMER_SUPPORTED
#endif

//------------------------------------------------------------------------------------------------
// yield
//
// Switch to another task of equal or greater priority (if any).
//------------------------------------------------------------------------------------------------

inline void yield()
{
	TaskScheduler::getCurrentTaskScheduler()->getCurrentTask()->yield();
}

//------------------------------------------------------------------------------------------------
// sleepUntil
//
// Suspends the current task for a certain time period after which it will be resumed automatically.
//------------------------------------------------------------------------------------------------

inline void sleepUntil(TimeValue endTime, Timer *pTimer)
{
	#if defined(TIMER_SUPPORTED)
		// wait on an event that will never be signalled
		IntertaskEvent event;
		event.waitUntil(endTime, pTimer);
	#else
		// timer not supported, yield instead
		yield();
	#endif
}

//------------------------------------------------------------------------------------------------
// sleepForTicks
//
// Suspends the current task for a certain time period after which it will be resumed automatically.
//------------------------------------------------------------------------------------------------

inline void sleepForTicks(TimeValue numberOfClockTicks, Timer *pTimer)
{
	#if defined(TIMER_SUPPORTED)
		// wait on an event that will never be signalled
		IntertaskEvent event;
		event.wait(numberOfClockTicks, pTimer);
	#else
		// timer not supported, yield instead
		yield();
	#endif
}

//------------------------------------------------------------------------------------------------
// sleepForSeconds
//
// Suspends the current task for a certain time period after which it will be resumed automatically.
//------------------------------------------------------------------------------------------------

inline void sleepForSeconds(UInt timeInSeconds)
{
	#if defined(TIMER_SUPPORTED)
		// wait on an event that will never be signalled
		IntertaskEvent event;
		event.waitForSeconds(timeInSeconds);
	#else
		// timer not supported, yield instead
		yield();
	#endif
}

//------------------------------------------------------------------------------------------------
// sleepForMilliseconds
//
// Suspends the current task for a certain time period after which it will be resumed automatically.
//------------------------------------------------------------------------------------------------

inline void sleepForMilliseconds(UInt timeInMilliseconds)
{
	#if defined(TIMER_SUPPORTED)
		// wait on an event that will never be signalled
		IntertaskEvent event;
		event.waitForMilliseconds(timeInMilliseconds);
	#else
		// timer not supported, yield instead
		yield();
	#endif
}

//------------------------------------------------------------------------------------------------
// sleepForMicroseconds
//
// Suspends the current task for a certain time period after which it will be resumed automatically.
//------------------------------------------------------------------------------------------------

inline void sleepForMicroseconds(UInt timeInMicroseconds)
{
	#if defined(TIMER_SUPPORTED)
		// wait on an event that will never be signalled
		IntertaskEvent event;
		event.waitForMicroseconds(timeInMicroseconds);
	#else
		// timer not supported, yield instead
		yield();
	#endif
}

#endif // _sleep_h_
