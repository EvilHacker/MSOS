#ifndef _IntertaskBroadcastEvent_h_
#define _IntertaskBroadcastEvent_h_

#include "../cPrimitiveTypes.h"
#include "TaskSynchronizer.h"

//------------------------------------------------------------------------------------------------
// * class IntertaskBroadcastEvent
//
// This class provides a mechanism for synchronizing tasks.
//------------------------------------------------------------------------------------------------

class IntertaskBroadcastEvent : public TaskSynchronizer
{
public:
	// constructor
	inline IntertaskBroadcastEvent();

	// synchronizing
	inline void signal();
	inline void clear();
};

//------------------------------------------------------------------------------------------------
// * IntertaskBroadcastEvent::IntertaskEvent
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline IntertaskBroadcastEvent::IntertaskBroadcastEvent()
{
	handle = CreateEvent(null, true, false, null);
}

//------------------------------------------------------------------------------------------------
// * IntertaskBroadcastEvent::signal
//
// Signals this event and unblocks an awaiting task (if any).
//------------------------------------------------------------------------------------------------

inline void IntertaskBroadcastEvent::signal()
{
	SetEvent(handle);
}

//------------------------------------------------------------------------------------------------
// * IntertaskBroadcastEvent::clear
//
// Force the event to not be signalled.
//------------------------------------------------------------------------------------------------

inline void IntertaskBroadcastEvent::clear()
{
	ResetEvent(handle);
}

#endif // _IntertaskEvent_h_
