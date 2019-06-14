#if !defined(INCLUDE_DEBUGGER)

#include "WatchdogSection.h"
#include "../multitasking/TaskScheduler.h"
#if defined(__TARGET_CPU_ARM920T)
	#include "../Mx1Devices/Mx1WatchdogTimer.h"
	typedef Mx1WatchdogTimer WatchdogTimer;
#endif
#if defined(__TARGET_CPU_SA_1100)
	#include "../Sa1110Devices/Sa1110WatchdogTimer.h"
	typedef Sa1110WatchdogTimer WatchdogTimer;
#endif

//------------------------------------------------------------------------------------------------
// * WatchdogSection::WatchdogSection
//
// Constructor.
//------------------------------------------------------------------------------------------------

WatchdogSection::WatchdogSection(UInt milliseconds)
{
	Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer();
	endTime = pTimer->getTime() + pTimer->convertMilliseconds(milliseconds);
	WatchdogTimer::getCurrentWatchdogTimer()->addSection(this);
}

//------------------------------------------------------------------------------------------------
// * WatchdogSection::~WatchdogSection
//
// Destructor.
//------------------------------------------------------------------------------------------------

WatchdogSection::~WatchdogSection()
{
	WatchdogTimer::getCurrentWatchdogTimer()->removeSection(this);
}

#endif
