#ifndef _TaskScheduler_h_
#define _TaskScheduler_h_

class Task;

//------------------------------------------------------------------------------------------------
// * class TaskScheduler
//
// Keeps track of running tasks and schedules them to run based on their priorities.
//------------------------------------------------------------------------------------------------

class TaskScheduler
{
public:
	// destructor
	virtual ~TaskScheduler();

	// querying
	inline static TaskScheduler *getCurrentTaskScheduler();
	inline Task *getCurrentTask() const;

	// behaviour
	inline void start();

private:
	// constructor (private because the class has a singleton instance)
	TaskScheduler();

	// representation
	static TaskScheduler currentTaskScheduler;
};

#include "Task.h"

//------------------------------------------------------------------------------------------------
// * TaskScheduler::getCurrentTaskScheduler
//
// Returns the task scheduler that is controlling the processor.
//------------------------------------------------------------------------------------------------

inline TaskScheduler *TaskScheduler::getCurrentTaskScheduler()
{
	return &currentTaskScheduler;
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::getCurrentTask
//
// Returns the task that is currently running.
//------------------------------------------------------------------------------------------------

inline Task *TaskScheduler::getCurrentTask() const
{
	// each thread has its own unique task pointer in "thread local storage"
	return (Task *)TlsGetValue(Task::threadLocalStorageIndex);
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::start
//
// Starts the scheduler and multitasking.
// This function will not return to the caller, only Task objects will run.
// This function must be called only once.
//------------------------------------------------------------------------------------------------

inline void TaskScheduler::start()
{
}

#endif // _TaskScheduler_h_
