#include "TaskBlocker.h"
#include "UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * TaskBlocker::unblockTask
//
// Unblocks the highest priority task so that it may continue to run.
//------------------------------------------------------------------------------------------------

Task *TaskBlocker::unblockTask()
{
	UninterruptableSection criticalSection;
	Task *pUnblockedTask = getFirstTask();
	pUnblockedTask->unblock();
	return pUnblockedTask;
}

//------------------------------------------------------------------------------------------------
// * TaskBlocker::unblockAllTasks
//
// Unblocks all tasks that are blocked on this TaskBlocker.
//------------------------------------------------------------------------------------------------

void TaskBlocker::unblockAllTasks()
{
	UninterruptableSection criticalSection;
	while(!isEmpty())
	{
		getFirstTask()->unblock();
	}
}

//------------------------------------------------------------------------------------------------
// * TaskBlocker::resumeTask
//
// Resumes the execution (or blocking) of <pTask>.
//------------------------------------------------------------------------------------------------

void TaskBlocker::resumeTask(Task *pTask)
{
	UninterruptableSection criticalSection;
	pTask->unblock();
	waitTask(pTask);
}
