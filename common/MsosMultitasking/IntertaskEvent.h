#ifndef _IntertaskEvent_h_
#define _IntertaskEvent_h_

#include "TaskBlocker.h"

//------------------------------------------------------------------------------------------------
// * class IntertaskEvent
//
// This class provides a mechanism for synchronizing tasks.
//------------------------------------------------------------------------------------------------

class IntertaskEvent : public TaskBlocker
{
public:
	// constructor
	inline IntertaskEvent();

	// testing
	inline Bool isSignalled() const;

	// synchronizing
	virtual void signal();
	inline void clear();

protected:
	// synchronizing
	void waitTask(Task *pTask);

	// representation
	Bool signalled;
};

//------------------------------------------------------------------------------------------------
// * IntertaskEvent::IntertaskEvent
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline IntertaskEvent::IntertaskEvent()
{
	signalled = false;
}

//------------------------------------------------------------------------------------------------
// * IntertaskEvent::isSignalled
//
// Tests whether the event has been signalled.
//------------------------------------------------------------------------------------------------

inline Bool IntertaskEvent::isSignalled() const
{
	return signalled;
}

//------------------------------------------------------------------------------------------------
// * IntertaskEvent::clear
//
// Force the event to not be signalled.
//------------------------------------------------------------------------------------------------

inline void IntertaskEvent::clear()
{
	signalled = false;
}

#endif // _IntertaskEvent_h_
