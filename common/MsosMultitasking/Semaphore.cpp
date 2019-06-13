#include "Semaphore.h"
#include "UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * Semaphore::waitTask
//
// Causes <pTask> to block until this semaphore has been signalled.
//------------------------------------------------------------------------------------------------

void Semaphore::waitTask(Task *pTask)
{
	UninterruptableSection criticalSection;
	if(excessSignalCount == 0)
	{
		blockTask(pTask);
	}
	else
	{
		--excessSignalCount;
	}
}

//------------------------------------------------------------------------------------------------
// * Semaphore::signal
//
// Signals this semaphore and unblocks an awaiting task (if any).
//------------------------------------------------------------------------------------------------

void Semaphore::signal()
{
	UninterruptableSection criticalSection;
	if(isEmpty())
	{
		++excessSignalCount;
	}
	else
	{
		unblockTask();
	}
}
