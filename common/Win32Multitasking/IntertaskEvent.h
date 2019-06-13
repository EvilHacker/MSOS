#ifndef _IntertaskEvent_h_
#define _IntertaskEvent_h_

#include "../cPrimitiveTypes.h"
#include "TaskSynchronizer.h"

//------------------------------------------------------------------------------------------------
// * class IntertaskEvent
//
// This class provides a mechanism for synchronizing tasks.
//------------------------------------------------------------------------------------------------

class IntertaskEvent : public TaskSynchronizer
{
public:
	// constructor
	inline IntertaskEvent();

	// synchronizing
	inline void signal();
	inline void clear();
};

//------------------------------------------------------------------------------------------------
// * IntertaskEvent::IntertaskEvent
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline IntertaskEvent::IntertaskEvent()
{
	handle = CreateEvent(null, false, false, null);
}

//------------------------------------------------------------------------------------------------
// * IntertaskEvent::signal
//
// Signals this event and unblocks an awaiting task (if any).
//------------------------------------------------------------------------------------------------

inline void IntertaskEvent::signal()
{
	SetEvent(handle);
}

//------------------------------------------------------------------------------------------------
// * IntertaskEvent::clear
//
// Force the event to not be signalled.
//------------------------------------------------------------------------------------------------

inline void IntertaskEvent::clear()
{
	ResetEvent(handle);
}

#endif // _IntertaskEvent_h_
