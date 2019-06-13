#ifndef _Mutex_h_
#define _Mutex_h_

#include "../cPrimitiveTypes.h"
#include "TaskSynchronizer.h"

//------------------------------------------------------------------------------------------------
// * class Mutex
//
// This class provides a mechanism for synchronizing tasks.
// A mutex allows only a single task to recursively lock it.
//------------------------------------------------------------------------------------------------

class Mutex : public TaskSynchronizer
{
public:
	// constructor
	inline Mutex();

	// synchronizing
	inline void signal();
};

//------------------------------------------------------------------------------------------------
// * Mutex::Mutex
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline Mutex::Mutex()
{
	handle = CreateMutex(null, false, null);
}

//------------------------------------------------------------------------------------------------
// * Mutex::signal
//
// Signals this mutex and unblocks an awaiting task (if any).
//------------------------------------------------------------------------------------------------

inline void Mutex::signal()
{
	ReleaseMutex(handle);
}

#endif // _Mutex_h_
