#include "Timer.h"
#include "TaskScheduler.h"
#include "UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * Timer::~Timer
//
// Destructor.
//------------------------------------------------------------------------------------------------

Timer::~Timer()
{
}

//------------------------------------------------------------------------------------------------
// * Timer::tick
//
// Handles a change in time.
// Checks for expired timer intervals.
//------------------------------------------------------------------------------------------------

void Timer::tick()
{
	UninterruptableSection criticalSection;

	// because the time can change during this procedure we must
	// keep checking for expired time intervals until we are sure there are no more
	while(true)
	{
		// assume that no expired intervals
		Bool intervalExpired = false;

		// check for expired time intervals
		const TimeValue currentTime = getTime();
		TimeInterval *pInterval;
		while((pInterval = getFirstInterval()) != null
			&& compareTimes(currentTime, pInterval->getExpiryTime()) >= 0)
		{
			// expire the interval
			pInterval->expire();

			// flag that an interval has expired
			intervalExpired = true;
		}

		// check if any intervals have expired
		if(intervalExpired)
		{
			// intervals have changed
			updateIntervals();
		}
		else
		{
			// no changes, we are finished
			break;
		}
	}
}

//------------------------------------------------------------------------------------------------
// * Timer::compareIntervals
//
// Compares the expiry times of two time intervals.
//------------------------------------------------------------------------------------------------

SInt Timer::compareIntervals(
	const Link *pInterval1,
	const Link *pInterval2)
{
	// make sure that the comparisson works even when the time values wrap around
	return compareTimes(
		((TimeInterval *)pInterval1)->getExpiryTime(),
		((TimeInterval *)pInterval2)->getExpiryTime());
}
