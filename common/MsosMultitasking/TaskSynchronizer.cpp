#include "TaskSynchronizer.h"
#include "TaskScheduler.h"
#include "BlockedTaskTimeout.h"
#include "UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::wait
//
// Wait with timeout.
// Returns true if a timeout occurred, false otherwise.
//------------------------------------------------------------------------------------------------

Bool TaskSynchronizer::wait(TimeValue timerTickCount, Timer *pTimer)
{
	if(timerTickCount == infiniteTime)
	{
		// wait with no timeout
		wait();
		return false;
	}
	else
	{
		// wait with timeout
		return waitUntil(pTimer->getTime() + timerTickCount, pTimer);
	}
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::waitUntil
//
// Wait with timeout.
// Returns true if a timeout occurred, false otherwise.
//------------------------------------------------------------------------------------------------

Bool TaskSynchronizer::waitUntil(TimeValue endTime, Timer *pTimer)
{
	// time interval that will unblock this task if it is waiting too long
	BlockedTaskTimeout timeout(*pTimer);

	// start timing and wait
	{
		UninterruptableSection criticalSection;
		timeout.beginTimingUntil(endTime);
		wait();
	}

	// check if the task was timed-out
	return timeout.isTimedOut();
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::tryWait
//
// Wait on the receiver only if it has been signalled.
// Returns true if the wait was successful, false if the receiver was not signalled.
//------------------------------------------------------------------------------------------------

Bool TaskSynchronizer::tryWait()
{
	// check if the receiver is signalled
	UninterruptableSection criticalSection;
	if(isSignalled())
	{
		wait();
		return true;
	}
	else
	{
		return false;
	}
}
