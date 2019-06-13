#include "TaskScheduler.h"

//------------------------------------------------------------------------------------------------
// * TaskScheduler::TaskScheduler
//
// Constructor.
// The scheduler will start with preemption disabled.
// The user must call the start() function once to begin scheduling.
//------------------------------------------------------------------------------------------------

TaskScheduler::TaskScheduler()
{
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::~TaskScheduler
//
// Destructor.
//------------------------------------------------------------------------------------------------

TaskScheduler::~TaskScheduler()
{
	TlsFree(Task::threadLocalStorageIndex);
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler static variables
//------------------------------------------------------------------------------------------------

TaskScheduler TaskScheduler::currentTaskScheduler;
