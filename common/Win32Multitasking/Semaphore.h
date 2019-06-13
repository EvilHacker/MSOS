#ifndef _Semaphore_h_
#define _Semaphore_h_

#include "../cPrimitiveTypes.h"
#include "TaskSynchronizer.h"

//------------------------------------------------------------------------------------------------
// * class Semaphore
//
// This class provides a mechanism for synchronizing tasks.
//------------------------------------------------------------------------------------------------

class Semaphore : public TaskSynchronizer
{
public:
	// constructor
	inline Semaphore(UInt excessSignalCount = 0);

	// accesssing
	inline UInt getExcessSignalCount() const;

	// synchronizing
	inline void signal();
};

//------------------------------------------------------------------------------------------------
// * Semaphore::Semaphore
//
// Constructs a new semaphore with an initial <excessSignalCount>.
//------------------------------------------------------------------------------------------------

inline Semaphore::Semaphore(UInt excessSignalCount)
{
	handle = CreateSemaphore(null, excessSignalCount, maxSInt32, null);
}

//------------------------------------------------------------------------------------------------
// * Semaphore::getExcessSignalCount
//
// Return the number of times the semaphore has been signalled without a corresponding wait.
//------------------------------------------------------------------------------------------------

inline UInt Semaphore::getExcessSignalCount() const
{
	// it is impossible to get semaphore counter without using wait on it
	//SInt32 excessSignalCount;
	//ReleaseSemaphore(handle, 0, &excessSignalCount);
	//return excessSignalCount;

	if(WaitForSingleObject(handle, 0) == WAIT_OBJECT_0)
	{
		SInt32 excessSignalCount = 0;
		// semaphore was decreased once, so signal semaphore once and get previouse counter
		ReleaseSemaphore(handle, 1, &excessSignalCount);
		return ++excessSignalCount;
	}

	return 0;
}

//------------------------------------------------------------------------------------------------
// * Semaphore::signal
//
// Signals this semaphore and unblocks an awaiting task (if any).
//------------------------------------------------------------------------------------------------

inline void Semaphore::signal()
{
	ReleaseSemaphore(handle, 1, null);
}

#endif // _Semaphore_h_
