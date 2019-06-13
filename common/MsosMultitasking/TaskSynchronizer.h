#ifndef _TaskSynchronizer_h_
#define _TaskSynchronizer_h_

#include "TaskScheduler.h"
#include "Timer.h"

//------------------------------------------------------------------------------------------------
// * class TaskSynchronizer
//
// This is an abstract class which provides an interface for sychronizing tasks.
//------------------------------------------------------------------------------------------------

class TaskSynchronizer
{
public:
	// testing
	virtual Bool isSignalled() const = 0;

	// testing synonyms
	inline Bool isLocked() const;

	// synchronizing
	inline void wait();
	Bool wait(
		TimeValue timerTickCount,
		Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer());
	Bool waitUntil(
		TimeValue endTime,
		Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer());
	inline Bool waitForSeconds(UInt timeoutInSeconds);
	inline Bool waitForMilliseconds(UInt timeoutInMilliseconds);
	inline Bool waitForMicroseconds(UInt timeoutInMicroseconds);
	virtual void signal() = 0;
	Bool tryWait();

	// synchronizing synonyms
	inline void lock();
	inline Bool lock(
		TimeValue timerTickCount,
		Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer());
	inline Bool lockUntil(
		TimeValue endTime,
		Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer());
	inline Bool lockForSeconds(UInt timeoutInSeconds);
	inline Bool lockForMilliseconds(UInt timeoutInMilliseconds);
	inline Bool lockForMicroseconds(UInt timeoutInMicroseconds);
	inline void unlock();
	inline Bool tryLock();

protected:
	// synchronizing
	virtual void waitTask(Task *pTask) = 0;
};

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::isLocked
//
// Tests whether this synchronizer is locked.
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::isLocked() const
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
	waitTask(TaskScheduler::getCurrentTaskScheduler()->getCurrentTask());
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::waitForSeconds
//
// Wait the current task with timeout.
// Returns true if a timeout occurred, false otherwise.
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::waitForSeconds(UInt timeoutInSeconds)
{
	Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer();
	return waitUntil(pTimer->getTime() + pTimer->convertSeconds(timeoutInSeconds), pTimer);
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::waitForMilliseconds
//
// Wait the current task with timeout.
// Returns true if a timeout occurred, false otherwise.
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::waitForMilliseconds(UInt timeoutInMilliseconds)
{
	Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer();
	return waitUntil(pTimer->getTime() + pTimer->convertMilliseconds(timeoutInMilliseconds), pTimer);
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::waitForMicroseconds
//
// Wait the current task with timeout.
// Returns true if a timeout occurred, false otherwise.
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::waitForMicroseconds(UInt timeoutInMicroseconds)
{
	Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer();
	return waitUntil(pTimer->getTime() + pTimer->convertMicroseconds(timeoutInMicroseconds), pTimer);
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

inline Bool TaskSynchronizer::lock(TimeValue timerTickCount, Timer *pTimer)
{
	return wait(timerTickCount, pTimer);
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::lockUntil
//
// A synonym for waitUntil().
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::lockUntil(TimeValue endTime, Timer *pTimer)
{
	return waitUntil(endTime, pTimer);
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::lockForSeconds
//
// A synonym for waitForSeconds().
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::lockForSeconds(UInt timeoutInSeconds)
{
	return waitForSeconds(timeoutInSeconds);
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::lockForMilliseconds
//
// A synonym for waitForMilliseconds().
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::lockForMilliseconds(UInt timeoutInMilliseconds)
{
	return waitForMilliseconds(timeoutInMilliseconds);
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::lockForMicroseconds
//
// A synonym for waitForMicroseconds().
//------------------------------------------------------------------------------------------------

inline Bool TaskSynchronizer::lockForMicroseconds(UInt timeoutInMicroseconds)
{
	return waitForMicroseconds(timeoutInMicroseconds);
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
