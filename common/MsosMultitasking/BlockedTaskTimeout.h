#ifndef _BlockedTaskTimeout_h_
#define _BlockedTaskTimeout_h_

#include "../cPrimitiveTypes.h"
#include "TimeInterval.h"
#include "TaskScheduler.h"
class Task;

//------------------------------------------------------------------------------------------------
// * class BlockedTaskTimeout
//
// This class is used to unblock a blocked task after a certain time period.
//------------------------------------------------------------------------------------------------

class BlockedTaskTimeout : public TimeInterval
{
public:
	// constructor
	inline BlockedTaskTimeout(
		Timer &timer,
		Task *pTask = TaskScheduler::getCurrentTaskScheduler()->getCurrentTask());

	// testing
	inline Bool isTimedOut() const;

private:
	// behaviour
	void handleExpiry();

	// representation
	Task *pTask;
	Bool timedOut;
};

//------------------------------------------------------------------------------------------------
// * BlockedTaskTimeout::BlockedTaskTimeout
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline BlockedTaskTimeout::BlockedTaskTimeout(Timer &timer, Task *pTask) :
	TimeInterval(timer)
{
	this->pTask = pTask;
	timedOut = false;
}

//------------------------------------------------------------------------------------------------
// * BlockedTaskTimeout::isTimedOut
//
// Test whether a task has been unblocked because it was waiting too long.
//------------------------------------------------------------------------------------------------

inline Bool BlockedTaskTimeout::isTimedOut() const
{
	return timedOut;
}

#endif // _BlockedTaskTimeout_h_
