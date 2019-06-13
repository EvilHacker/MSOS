#ifndef _Task_h_
#define _Task_h_

#include "../cPrimitiveTypes.h"
#include "TimeValue.h"
#include "IntertaskEvent.h"
class TaskScheduler;
#include <windows.h>

//------------------------------------------------------------------------------------------------
// * class Task
//
// An instance of this class represents an asynchronous task executed by the processor.
//------------------------------------------------------------------------------------------------

class Task
{
public:
	// predefined task priority levels
	enum
	{
		lowPriority = (THREAD_PRIORITY_LOWEST - THREAD_BASE_PRIORITY_IDLE) * 1000000,
		mediumPriority = (THREAD_PRIORITY_NORMAL - THREAD_BASE_PRIORITY_IDLE) * 1000000,
		highPriority = (THREAD_PRIORITY_HIGHEST - THREAD_BASE_PRIORITY_IDLE) * 1000000,
		realtimePriority = (THREAD_PRIORITY_TIME_CRITICAL - THREAD_BASE_PRIORITY_IDLE - 1) * 1000000,
		maximumPriority = (THREAD_PRIORITY_TIME_CRITICAL - THREAD_BASE_PRIORITY_IDLE) * 1000000,

		defaultPriority = mediumPriority,
		normalPriority = mediumPriority,
		debuggerPriority = realtimePriority - 1
	};

	// constructors and destructors
	Task(UInt priority, UInt stackSize);
	virtual ~Task();

	// testing
	inline Bool isTerminated() const;

	// accessing
	inline TaskScheduler *getScheduler() const;
	inline UInt getPriority() const;
	inline void setPriority(UInt priority);

	// behaviour
	inline void suspend();
	inline void resume();
	inline void yield();
	inline void sleep(TimeValue timerTickCount);
	//virtual void sleep(TimeValue timerTickCount, Timer *pTimer);
	virtual void terminate();
	virtual void waitForTermination();

protected:
	// overloadable main function for this task
	virtual void main() = 0;

	// representation
	IntertaskEvent terminationEvent;

private:
	// hidden entry point
	static DWORD WINAPI entry(LPVOID lpParameter);

	// representation
	Bool terminated;
	HANDLE threadHandle;
	static UInt threadLocalStorageIndex;

	// friends
	friend class TaskScheduler;
	friend class TaskSynchronizer;
};

#include "TaskScheduler.h"

//------------------------------------------------------------------------------------------------
// * Task::isTerminated
//
// Return task statue
//------------------------------------------------------------------------------------------------

inline Bool Task::isTerminated() const
{
	return terminated;
}

//------------------------------------------------------------------------------------------------
// * Task::getPriority
//
// Returns the priority level of this task.
//------------------------------------------------------------------------------------------------

inline UInt Task::getPriority() const
{
	return (GetThreadPriority(threadHandle) - THREAD_BASE_PRIORITY_IDLE) * 1000000;
}

//------------------------------------------------------------------------------------------------
// * Task::setPriority
//
// Changes the <priority> level of this task.
//------------------------------------------------------------------------------------------------

inline void Task::setPriority(UInt priority)
{
	SetThreadPriority(threadHandle, (SInt)priority / 1000000 + THREAD_BASE_PRIORITY_IDLE);
}

//------------------------------------------------------------------------------------------------
// * Task::getScheduler
//
// Returns the TaskScheduler that controls this task.
//------------------------------------------------------------------------------------------------

inline TaskScheduler *Task::getScheduler() const
{
	return TaskScheduler::getCurrentTaskScheduler();
}

//------------------------------------------------------------------------------------------------
// * Task::suspend
//
// Removes this task from the scheduler or blocker it is blocked on
// so that it will no longer run.
//------------------------------------------------------------------------------------------------

inline void Task::suspend()
{
	SuspendThread(threadHandle);
}

//------------------------------------------------------------------------------------------------
// * Task::resume
//
// Adds this task to the scheduler or blocker it was blocked on so that it can run.
//------------------------------------------------------------------------------------------------

inline void Task::resume()
{
	ResumeThread(threadHandle);
}

//------------------------------------------------------------------------------------------------
// * Task::yield
//
// Allows another task of greater or equal priority to run.
//------------------------------------------------------------------------------------------------

inline void Task::yield()
{
	Sleep(0);
}

//------------------------------------------------------------------------------------------------
// * Task::sleep
//
// Suspends a task for a certain time period after which it will be resumed automatically.
//------------------------------------------------------------------------------------------------

inline void Task::sleep(TimeValue timerTickCount)
{
	Sleep(timerTickCount);
}

#endif // _Task_h_
