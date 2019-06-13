#include "IntertaskBroadcastEvent.h"
#include "UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * IntertaskBroadcastEvent::signal
//
// Signals this event and unblocks all waiting tasks.
//------------------------------------------------------------------------------------------------

void IntertaskBroadcastEvent::signal()
{
	UninterruptableSection criticalSection;
	signalled = true;
	unblockAllTasks();
}

//------------------------------------------------------------------------------------------------
// * IntertaskBroadcastEvent::waitTask
//
// Causes <pTask> to block until this event has been signalled.
//------------------------------------------------------------------------------------------------

void IntertaskBroadcastEvent::waitTask(Task *pTask)
{
	UninterruptableSection criticalSection;
	if(!signalled)
	{
		blockTask(pTask);
	}
}

