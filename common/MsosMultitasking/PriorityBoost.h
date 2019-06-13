#ifndef _PriorityBoost_h_
#define _PriorityBoost_h_

#include "../cPrimitiveTypes.h"
#include "TaskSynchronizer.h"

//------------------------------------------------------------------------------------------------
// * class PriorityBoost
//
// This class provides a mechanism for synchronizing tasks.
// Tasks using this lock are assumed to have a priority less than <lockingPriority>.
// When locked, the task's priority will be temporarily increased to <lockingPriority>.
// Other lower priority tasks will not be scheduled to run during this time.
// When unlocked, the task's priority will be restored.
// A task must not block while holding a priority lock!
//------------------------------------------------------------------------------------------------

class PriorityBoost : public TaskSynchronizer
{
public:
	// constructor
	inline PriorityBoost(UInt lockingPriority);

	// testing
	inline Bool isSignalled() const;

	// accesssing
	inline UInt getLockingPriority() const;

	// synchronizing
	void signal();

private:
	// synchronizing
	void waitTask(Task *pTask);

	// represenation
	UInt lockingPriority;
	UInt savedPriority;
};

//------------------------------------------------------------------------------------------------
// * PriorityBoost::PriorityBoost
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline PriorityBoost::PriorityBoost(UInt lockingPriority)
{
	this->lockingPriority = lockingPriority;
}

//------------------------------------------------------------------------------------------------
// * PriorityBoost::isSignalled
//
// Tests whether the mutex is unlocked.
//------------------------------------------------------------------------------------------------

inline Bool PriorityBoost::isSignalled() const
{
	return true;
}

//------------------------------------------------------------------------------------------------
// * PriorityBoost::getLockingPriority
//
// Returns the priority at which a task will run while holding this priority lock.
//------------------------------------------------------------------------------------------------

inline UInt PriorityBoost::getLockingPriority() const
{
	return lockingPriority;
}

#endif // _PriorityBoost_h_
