#include "PriorityBoost.h"
#include "TaskScheduler.h"
#include "Task.h"

//------------------------------------------------------------------------------------------------
// * PriorityBoost::waitTask (lockTask)
//
// Increases <pTask>'s priority to the <lockingPriority>.
// Other lower priority task's will not be scheduled to run.
//------------------------------------------------------------------------------------------------

void PriorityBoost::waitTask(Task *pTask)
{
	// increase the current task's priority
	const UInt currentPriority = pTask->getPriority();
	pTask->setPriority(lockingPriority);
	savedPriority = currentPriority;
}

//------------------------------------------------------------------------------------------------
// * PriorityBoost::signal (unlock)
//
// Restores the current task's priority.
//------------------------------------------------------------------------------------------------

void PriorityBoost::signal()
{
	// restore the current task's priority
	Task *pCurrentTask = TaskScheduler::getCurrentTaskScheduler()->getCurrentTask();
	pCurrentTask->setPriority(savedPriority);
}
