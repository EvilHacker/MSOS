#ifndef _IntertaskBroadcastEvent_h_
#define _IntertaskBroadcastEvent_h_

#include "IntertaskEvent.h"

//------------------------------------------------------------------------------------------------
// * class IntertaskBroadcastEvent
//
// This class provides a mechanism for synchronizing tasks.
//------------------------------------------------------------------------------------------------

class IntertaskBroadcastEvent : public IntertaskEvent
{
public:
	// synchronizing
	void signal();

protected:
	// synchronizing
	void waitTask(Task *pTask);
};

#endif // _IntertaskEvent_h_
