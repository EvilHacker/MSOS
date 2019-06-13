#include "Task.h"

//------------------------------------------------------------------------------------------------
// * Task::Task
//
// Constructs a new task given its <priority> level and <stackSize> in bytes.
//------------------------------------------------------------------------------------------------

Task::Task(UInt priority, UInt stackSize) :
	terminated(false)
{
	DWORD threadId;
	threadHandle = CreateThread(null, stackSize, entry, this, CREATE_SUSPENDED, &threadId);

	if(threadHandle == null)
	{
		// check if this compiler does or does not support exceptions
		#if !defined(UNDER_CE)
			throw;
		#else
			// exceptions are not supported in EVC++
			exit(-1);
		#endif
	}

	setPriority(priority);
}

//------------------------------------------------------------------------------------------------
// * Task::~Task
//
// Destructor.
//------------------------------------------------------------------------------------------------

Task::~Task()
{
	if(threadHandle != null)
	{
		CloseHandle(threadHandle);
	}
}

//------------------------------------------------------------------------------------------------
// * Task::waitThreadExit
//
// Wait for thread free allocated memory and exits
//------------------------------------------------------------------------------------------------

void Task::waitForTermination()
{
	WaitForSingleObject(threadHandle, INFINITE);
}

//------------------------------------------------------------------------------------------------
// * Task::terminate
//
// Signal to terminate task operation
// Only the current task may call this function.
//------------------------------------------------------------------------------------------------

void Task::terminate()
{
	terminated = true;
	terminationEvent.signal();
}

/*
//------------------------------------------------------------------------------------------------
// * Task::sleep
//
// Suspends a task for a certain time period after which it will be resumed automatically.
// Only the current task may call this function, you can not make another task sleep.
//------------------------------------------------------------------------------------------------

void Task::sleep(TimeValue timerTickCount, Timer *pTimer)
{
}
*/

//------------------------------------------------------------------------------------------------
// * Task::entry
//
// This is the hidden entry point of a task.
// This function calls the user definable main() function and performs any cleanup required
// when the main() function completes.
//------------------------------------------------------------------------------------------------

DWORD WINAPI Task::entry(LPVOID lpParameter)
{
	// <lpParameter> is used as the pointer to the task
	Task *pCurrentTask = (Task *)lpParameter;

	// save task pointer into "thread local storage"
	// each thread will have its own unique task pointer
	TlsSetValue(threadLocalStorageIndex, pCurrentTask);

	// execute overridable main function
	pCurrentTask->main();
	return 0;
}

//------------------------------------------------------------------------------------------------
// * Task static variables
//------------------------------------------------------------------------------------------------

UInt Task::threadLocalStorageIndex = TlsAlloc();
