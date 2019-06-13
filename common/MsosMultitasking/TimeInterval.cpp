#include "TimeInterval.h"
#include "Timer.h"
#include "TaskScheduler.h"
#include "UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * TimeInterval::TimeInterval
//
// Constructor.
//------------------------------------------------------------------------------------------------

TimeInterval::TimeInterval(Timer &timer) :
	timer(timer)
{
	expired = true;
}

//------------------------------------------------------------------------------------------------
// * TimeInterval::~TimeInterval
//
// Destructor.
//------------------------------------------------------------------------------------------------

TimeInterval::~TimeInterval()
{
	UninterruptableSection criticalSection;

	// check if the timer has not yet expired
	if(!expired)
	{
		// the timer no longer needs this interval
		timer.removeInterval(this);
		timer.updateIntervals();
		timer.tick();
	}
}

//------------------------------------------------------------------------------------------------
// * TimeInterval::beginTimingUntil
//
// Starts timing now.
//------------------------------------------------------------------------------------------------

void TimeInterval::beginTimingUntil(TimeValue expiryTime)
{
	UninterruptableSection criticalSection;

	// check if the time interval has not yet expired
	if(!expired)
	{
		// the timer no longer needs this interval
		timer.removeInterval(this);
	}

	// this time interval has not yet expired
	expired = false;

	// set the time of expiry
	this->expiryTime = expiryTime;

	// the timer needs to know of this interval
	timer.addInterval(this);
	timer.updateIntervals();
	timer.tick();
}

//------------------------------------------------------------------------------------------------
// * TimeInterval::expire
//
// Makes this time interval expire.
//------------------------------------------------------------------------------------------------

void TimeInterval::expire()
{
	//UninterruptableSection criticalSection;

	// check if the timer has not yet expired
	if(!expired)
	{
		// do something upon expiry
		handleExpiry();

		// set flag to indicate expiry
		expired = true;

		// the timer no longer needs this interval
		timer.removeInterval(this);
	}
}
