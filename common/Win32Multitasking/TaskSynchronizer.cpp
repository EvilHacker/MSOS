#include "TaskSynchronizer.h"

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::~TaskSynchronizer
//
// Destructor.
//------------------------------------------------------------------------------------------------

TaskSynchronizer::~TaskSynchronizer()
{
	CloseHandle(handle);
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::isSignalled
//
// Tests whether this synchronizer is signalled.
//------------------------------------------------------------------------------------------------

Bool TaskSynchronizer::isSignalled()
{
	if(tryWait())
	{
		signal();
		return true;
	}
	else
	{
		return false;
	}
}

//------------------------------------------------------------------------------------------------
// * TaskSynchronizer::waitWithTermination
//
// Wait until this synchronizer is signalled or the current task is terminated.
// Returns true if termination occurred, false otherwise.
//------------------------------------------------------------------------------------------------

/*
Bool TaskSynchronizer::waitWithTermination()
{
	HANDLE waitArray[] =
	{
		TaskScheduler::getCurrentTaskScheduler()->getCurrentTask()->terminationEvent.handle,
		handle
	};
	return WaitForMultipleObjects(2, waitArray, FALSE, INFINITE) == WAIT_OBJECT_0;
}
*/