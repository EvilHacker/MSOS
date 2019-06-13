#include "PriorityMutex.h"
#include "TaskScheduler.h"
#include "Task.h"
#include "UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * PriorityMutex::waitTask (lockTask)
//
// Causes <pTask> to block until this mutex has been unlocked.
// A single task may recursively lock this mutex without blocking.
// If <pTask> is to block and it has a higher priority than the locking task then
// the locking task will have its priority temporarily raised.
//------------------------------------------------------------------------------------------------

void PriorityMutex::waitTask(Task *pTask)
{
	UninterruptableSection criticalSection;
	if(pLockingTask == null)
	{
		// this mutex is not locked, the current task can lock it
		pLockingTask = pTask;
		lockingTaskOriginalPriority = pTask->getPriority();
		++lockCount;
	}
	else if(pLockingTask == pTask)
	{
		// this mutex has already been locked by this task, count the number of recursive locks
		++lockCount;
	}
	else
	{		
		// this mutex has already been locked by another task, block the current task
		blockTask(pTask);

		// check if current task has a higher priority than the locking task
		if (pTask->getPriority() > lockingTaskOriginalPriority)
		{
			// bump up the priority of the locking task
			pLockingTask->setPriority(pTask->getPriority());
		}
	}
}

//------------------------------------------------------------------------------------------------
// * PriorityMutex::signal (unlock)
//
// Unlocks this mutex and unblocks an awaiting task (if any).
// If this mutex had been locked recursively by a single task, the mutex will not become unlocked
// until the task calls unlock() as many times as it had called lock().
// The original priority of the locking task is restored (if it was bumped up).
//------------------------------------------------------------------------------------------------

void PriorityMutex::signal()
{
	UninterruptableSection criticalSection;
	if(--lockCount == 0)
	{
		// restore the locking task's original priority
		if (pLockingTask->getPriority() != lockingTaskOriginalPriority)
		{
			pLockingTask->setPriority(lockingTaskOriginalPriority);
		}
		
		// the last recursive lock has been undone, check if there are any other tasks blocked
		if(isEmpty())
		{
			// the mutex remains unlocked
			pLockingTask = null;
		}
		else
		{
			// lock the mutex by the next waiting task
			pLockingTask = unblockTask();
			++lockCount;
		}
	}
}
