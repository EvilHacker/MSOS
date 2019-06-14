#ifndef _WatchdogSection_h_
#define _WatchdogSection_h_

#if (defined(__TARGET_CPU_ARM920T) || defined(__TARGET_CPU_SA_1100)) && !defined(INCLUDE_DEBUGGER)

#include "../cPrimitiveTypes.h"
#include "../Collections/Link.h"
#include "TimeValue.h"

//------------------------------------------------------------------------------------------------
// * class WatchdogSection
//
// Resets the device if the destructor in not called before a certain amount of time.
//------------------------------------------------------------------------------------------------

class WatchdogSection : public Link
{
public:
	// constructor and destructor
	WatchdogSection(UInt milliseconds);
	~WatchdogSection();

	// querying
	inline TimeValue getEndTime() const;

private:
	// representation
	TimeValue endTime;
};

//------------------------------------------------------------------------------------------------
// * WatchdogSection::getEndTime
//
// Returns the time at which the device should be reset.
//------------------------------------------------------------------------------------------------

TimeValue WatchdogSection::getEndTime() const
{
	return endTime;
}

#else

#include "../cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * class WatchdogSection
//
// Placeholder for platforms that do not have a watchdog timer.
//------------------------------------------------------------------------------------------------

typedef const UInt WatchdogSection;

#endif

#endif // _WatchdogSection_h_
