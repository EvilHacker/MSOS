#include "BlockedTaskTimeout.h"
#include "Task.h"

//------------------------------------------------------------------------------------------------
// * BlockedTaskTimeout::handleExpiry
//
// Handles the expiry of this time interval.
//------------------------------------------------------------------------------------------------

void BlockedTaskTimeout::handleExpiry()
{
	UninterruptableSection criticalSection;

	// check if the task has not been unblocked yet
	if(pTask->isBlocked())
	{
		// the task has been blocked too long
		// time-out and unblock the task
		timedOut = true;
		pTask->unblock();
	}
}
