#include "IntertaskEvent.h"
#include "UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * IntertaskEvent::waitTask
//
// Causes <pTask> to block until this event has been signalled.
//------------------------------------------------------------------------------------------------

void IntertaskEvent::waitTask(Task *pTask)
{
	UninterruptableSection criticalSection;
	if(signalled)
	{
		signalled = false;
	}
	else
	{
		blockTask(pTask);
	}
}

//------------------------------------------------------------------------------------------------
// * IntertaskEvent::signal
//
// Signals this event and unblocks an awaiting task (if any).
//------------------------------------------------------------------------------------------------

void IntertaskEvent::signal()
{
	UninterruptableSection criticalSection;
	if(isEmpty())
	{
		signalled = true;
	}
	else
	{
		unblockTask();
	}
}
