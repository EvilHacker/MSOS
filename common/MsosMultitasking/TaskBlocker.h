#ifndef _TaskBlocker_h_
#define _TaskBlocker_h_

#include "TaskGroup.h"
#include "TaskSynchronizer.h"

//------------------------------------------------------------------------------------------------
// * class TaskBlocker
//
// This class can block and unblock running tasks.
// This class also keeps track of all tasks that are blocked by it.
//------------------------------------------------------------------------------------------------

class TaskBlocker :
	public TaskGroup,
	public TaskSynchronizer
{
protected:
	// blocking and unblocking tasks
	inline void blockTask(Task *pTask);
	Task *unblockTask();
	void unblockAllTasks();

	// controlling tasks
	void resumeTask(Task *pTask);
};

//------------------------------------------------------------------------------------------------
// * TaskBlocker::blockTask
//
// Block the specified <pTask> from running.
//------------------------------------------------------------------------------------------------

inline void TaskBlocker::blockTask(Task *pTask)
{
	pTask->blockOn(this);
}

#endif // _TaskBlocker_h_
