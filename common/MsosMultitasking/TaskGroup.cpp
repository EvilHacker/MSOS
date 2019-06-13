#include "TaskGroup.h"
#include "UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * TaskGroup::~TaskGroup
//
// Destructor.
//------------------------------------------------------------------------------------------------

TaskGroup::~TaskGroup()
{
}

//------------------------------------------------------------------------------------------------
// * TaskGroup::compareTasks
//
// Compares the priorities of two tasks.
//------------------------------------------------------------------------------------------------

SInt TaskGroup::compareTasks(const Link *pTask1, const Link *pTask2)
{
	return (SInt)((Task *)pTask2)->getPriority() + 1 - (SInt)((Task *)pTask1)->getPriority();
}

//------------------------------------------------------------------------------------------------
// * TaskGroup::suspendTask
//
// Suspends the execution of <pTask>.
//------------------------------------------------------------------------------------------------

void TaskGroup::suspendTask(Task *pTask)
{
}

//------------------------------------------------------------------------------------------------
// * TaskGroup::resumeTask
//
// Resumes the execution of <pTask>.
//------------------------------------------------------------------------------------------------

void TaskGroup::resumeTask(Task *pTask)
{
}

//------------------------------------------------------------------------------------------------
// * TaskGroup::yieldTask
//
// Allows another task of greater or equal priority to be placed ahead of <pTask>.
//------------------------------------------------------------------------------------------------

void TaskGroup::yieldTask(Task *pTask)
{
	UninterruptableSection criticalSection;
	removeTask(pTask);
	addTask(pTask);
}
