#include "Mutex.h"
#include "TaskScheduler.h"
#include "Task.h"
#include "UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * Mutex::waitTask (lockTask)
//
// Causes <pTask> to block until this mutex has been unlocked.
// A single task may recursively lock this mutex without blocking.
//------------------------------------------------------------------------------------------------

void Mutex::waitTask(Task *pTask)
{
	UninterruptableSection criticalSection;
	if(pLockingTask == null)
	{
		// this mutex is not locked, the current task can lock it
		pLockingTask = pTask;
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
	}
}

//------------------------------------------------------------------------------------------------
// * Mutex::signal (unlock)
//
// Unlocks this mutex and unblocks an awaiting task (if any).
// If this mutex had been locked recursively by a single task, the mutex will not become unlocked
// until the task calls unlock() as many times as it had called lock().
//------------------------------------------------------------------------------------------------

void Mutex::signal()
{
	UninterruptableSection criticalSection;
	if(--lockCount == 0)
	{
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
