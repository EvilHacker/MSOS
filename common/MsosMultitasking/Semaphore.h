#ifndef _Semaphore_h_
#define _Semaphore_h_

#include "../cPrimitiveTypes.h"
#include "TaskBlocker.h"

//------------------------------------------------------------------------------------------------
// * class Semaphore
//
// This class provides a mechanism for synchronizing tasks.
//------------------------------------------------------------------------------------------------

class Semaphore : public TaskBlocker
{
public:
	// constructor
	inline Semaphore(UInt excessSignalCount = 0);

	// testing
	inline Bool isSignalled() const;

	// accesssing
	inline UInt getExcessSignalCount() const;

	// synchronizing
	void signal();

private:
	// synchronizing
	void waitTask(Task *pTask);

	// represenation
	UInt excessSignalCount;
};

//------------------------------------------------------------------------------------------------
// * Semaphore::Semaphore
//
// Constructs a new semaphore with an initial <excessSignalCount>.
//------------------------------------------------------------------------------------------------

inline Semaphore::Semaphore(UInt excessSignalCount)
{
	this->excessSignalCount = excessSignalCount;
}

//------------------------------------------------------------------------------------------------
// * Semaphore::isSignalled
//
// Tests whether the mutex is unlocked.
//------------------------------------------------------------------------------------------------

inline Bool Semaphore::isSignalled() const
{
	return excessSignalCount > 0;
}

//------------------------------------------------------------------------------------------------
// * Semaphore::getExcessSignalCount
//
// Return the number of times the semaphore has been signalled without a corresponding wait.
//------------------------------------------------------------------------------------------------

inline UInt Semaphore::getExcessSignalCount() const
{
	return excessSignalCount;
}

#endif // _Semaphore_h_
