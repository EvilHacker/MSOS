#ifndef _Task_h_
#define _Task_h_

#include "../cPrimitiveTypes.h"
#include "../Collections/Link.h"
class TaskGroup;
class TaskScheduler;
class TaskBlocker;

//------------------------------------------------------------------------------------------------
// * class Task
//
// An instance of this class represents an asynchronous task executed by the processor.
//------------------------------------------------------------------------------------------------

class Task : public Link
{
public:
	// predefined task priority levels
	enum
	{
		lowPriority = 20 * 1000000,
		mediumPriority = 40 * 1000000,
		highPriority = 60 * 1000000,
		realtimePriority = 80 * 1000000,
		maximumPriority = maxUInt,

		defaultPriority = mediumPriority,
		normalPriority = mediumPriority,
		debuggerPriority = realtimePriority - 1
	};

	// constructors and destructors
	Task(UInt priority, UInt stackSize);
	virtual ~Task();

	// testing
	inline Bool isSuspended() const;
	inline Bool isBlocked() const;

	// accessing
	inline TaskScheduler *getScheduler() const;
	inline UInt getPriority() const;
	void setPriority(UInt priority);

	// behaviour
	void suspend();
	void resume();
	void yield();
	void blockOn(TaskBlocker *pTaskBlocker);
	void unblock();

	// list node accessing
	inline Task *getPreviousTask() const;
	inline Task *getNextTask() const;
	inline void setPreviousTask(Task *pPreviousTask);
	inline void setNextTask(Task *pNextTask);

protected:
	// overloadable main function for this task
	virtual void main() = 0;

private:
	// hidden entry point
	void entry();

	// representation
	void *pStack;
	void *pStackTop;
	UInt priority;
	UInt suspendCount;
	TaskGroup *pGroup;

	// friends
	friend class TaskScheduler;
	friend class RemoteDebuggerAgent;
};

#include "TaskScheduler.h"

//------------------------------------------------------------------------------------------------
// * Task::isSuspended
//
// Test whether this task is suspended from running.
//------------------------------------------------------------------------------------------------

inline Bool Task::isSuspended() const
{
	return suspendCount != 0;
}

//------------------------------------------------------------------------------------------------
// * Task::isBlocked
//
// Test whether this task is blocked.
//------------------------------------------------------------------------------------------------

inline Bool Task::isBlocked() const
{
	return pGroup != getScheduler();
}

//------------------------------------------------------------------------------------------------
// * Task::getPriority
//
// Returns the priority level of this task.
//------------------------------------------------------------------------------------------------

inline UInt Task::getPriority() const
{
	return priority;
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
// * Task::getPreviousTask
//
// Returns the previous task in the list.
//------------------------------------------------------------------------------------------------

inline Task *Task::getPreviousTask() const
{
	return (Task *)Link::getPrevious();
}

//------------------------------------------------------------------------------------------------
// * Task::getNextTask
//
// Returns the next task in the list.
//------------------------------------------------------------------------------------------------

inline Task *Task::getNextTask() const
{
	return (Task *)Link::getNext();
}

//------------------------------------------------------------------------------------------------
// * Task::setPreviousTask
//
// Changes the task that comes before this one in the list.
//------------------------------------------------------------------------------------------------

inline void Task::setPreviousTask(Task *pPreviousTask)
{
	Link::setPrevious(pPreviousTask);
}

//------------------------------------------------------------------------------------------------
// * Task::setNextTask
//
// Changes the task that comes after this one in the list.
//------------------------------------------------------------------------------------------------

inline void Task::setNextTask(Task *pNextTask)
{
	Link::setNext(pNextTask);
}

#endif // _Task_h_
