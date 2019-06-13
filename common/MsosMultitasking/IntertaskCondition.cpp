#include "IntertaskCondition.h"
#include "TaskScheduler.h"

//------------------------------------------------------------------------------------------------
// * IntertaskCondition::waitTask
//
// Causes <pTask> to block until the condition is changed.
// This function must be called with the mutex locked once by the same task.
//------------------------------------------------------------------------------------------------

void IntertaskCondition::waitTask(Task *pTask)
{
	// increase to maximum priority so that the remaining code will be atomic
	// with respect to any signaling task, and no change in condition will be missed
	const UInt savedPriority = pTask->getPriority();
	pTask->setPriority(Task::maximumPriority);

	// keep the mutex unlocked while waiting for the condition to change
	mutex.unlock();
	event.wait();
	mutex.lock();

	// restore the original priority of this task
	pTask->setPriority(savedPriority);
}
