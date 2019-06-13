#ifndef _TaskSynchronizer_h_
#define _TaskSynchronizer_h_

#include "../cPrimitiveTypes.h"
#include "TimeValue.h"
#include <windows.h>

//------------------------------------------------------------------------------------------------
// * class TaskSynchronizer
//
// This is an abstract class which provides an interface for sychronizing tasks.
//------------------------------------------------------------------------------------------------

class TaskSynchronizer
{
public:
	// destructor
	virtual ~TaskSynchronizer();
	// testing
	Bool isSignalled();

	// testing synonyms
	inline Bool isLocked();

	// synchronizing
	virtual void wait();
	virtual Bool wait(TimeValue timerTickCount);
	virtual void signal() = 0;
	inline Bool tryWait();

	// synchronizing synonyms
	inline void lock();
	inline Bool lock(TimeValue timerTickCount);
	inline void unlock();
	inline Bool tryLock();

protected:
	// representation
	HANDLE handle;
};

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::isLocked
//
// Tests whether this synchronizer is locked.
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::isLocked()
{
	return !isSignalled();
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::wait
//
// Wait the current task.
//------------------------------------------------------------------------------------------------

inline void TaskSynchronizer::wait()
{
	WaitForSingleObject(handle, INFINITE);
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::wait
//
// Wait with timeout.
// Returns true if a timeout occurred, false otherwise.
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::wait(TimeValue timerTickCount)
{
	// infiniteTime has been made equal to INFINITE, otherwise this would be neccessary
	//if(timerTickCount == infiniteTime)
	//{
	//	timerTickCount = INFINITE;
	//}

	return WaitForSingleObject(handle, timerTickCount) == WAIT_TIMEOUT;
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::tryWait
//
// Wait on the receiver only if it has been signalled.
// Returns true if the wait was successful, false if the receiver was not signalled.
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::tryWait()
{
	return WaitForSingleObject(handle, 0) != WAIT_TIMEOUT;
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::lock
//
// A synonym for wait().
//------------------------------------------------------------------------------------------------

inline void TaskSynchronizer::lock()
{
	wait();
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::lock
//
// A synonym for wait().
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::lock(TimeValue timerTickCount)
{
	return wait(timerTickCount);
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::unlock
//
// A synonym for signal().
//------------------------------------------------------------------------------------------------

inline void TaskSynchronizer::unlock()
{
	signal();
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::tryLock
//
// A synonym for tryWait().
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::tryLock()
{
	return tryWait();
}

#endif // _TaskSynchronizer_h_
