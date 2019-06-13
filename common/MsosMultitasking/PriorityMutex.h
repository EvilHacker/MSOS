#ifndef _PriorityMutex_h_
#define _PriorityMutex_h_

#include "../cPrimitiveTypes.h"
#include "TaskBlocker.h"
class Task;

//------------------------------------------------------------------------------------------------
// * class PriorityMutex
//
// This class provides a mechanism for synchronizing tasks.
// A mutex allows only a single task to recursively lock it.
// This type of mutex handles priority inversion by temporarily bumping up the locking task's
// priority to that of the highest waiting task's priority.
//------------------------------------------------------------------------------------------------

class PriorityMutex : public TaskBlocker
{
public:
	// constructor
	inline PriorityMutex();

	// testing
	inline Bool isSignalled() const;

	// synchronizing
	void signal();

private:
	// synchronizing
	void waitTask(Task *pTask);

	// represenation
	UInt lockCount;
	Task *pLockingTask;
	UInt lockingTaskOriginalPriority;
};

//------------------------------------------------------------------------------------------------
// * PriorityMutex::PriorityMutex
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline PriorityMutex::PriorityMutex()
{
	lockCount = 0;
	pLockingTask = null;
}

//------------------------------------------------------------------------------------------------
// * PriorityMutex::isSignalled
//
// Tests whether the mutex is unlocked.
//------------------------------------------------------------------------------------------------

inline Bool PriorityMutex::isSignalled() const
{
	return lockCount == 0;
}

#endif // _PriorityMutex_h_
