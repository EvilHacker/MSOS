#ifndef _Mutex_h_
#define _Mutex_h_

#include "../cPrimitiveTypes.h"
#include "TaskBlocker.h"
class Task;

//------------------------------------------------------------------------------------------------
// * class Mutex
//
// This class provides a mechanism for synchronizing tasks.
// A mutex allows only a single task to recursively lock it.
//------------------------------------------------------------------------------------------------

class Mutex : public TaskBlocker
{
public:
	// constructor
	inline Mutex();

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
};

//------------------------------------------------------------------------------------------------
// * Mutex::Mutex
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline Mutex::Mutex()
{
	lockCount = 0;
	pLockingTask = null;
}

//------------------------------------------------------------------------------------------------
// * Mutex::isSignalled
//
// Tests whether the mutex is unlocked.
//------------------------------------------------------------------------------------------------

inline Bool Mutex::isSignalled() const
{
	return lockCount == 0;
}

#endif // _Mutex_h_
