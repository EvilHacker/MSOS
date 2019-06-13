#ifndef _IntertaskCondition_h_
#define _IntertaskCondition_h_

#include "TaskSynchronizer.h"
#include "Mutex.h"
#include "IntertaskBroadcastEvent.h"

//------------------------------------------------------------------------------------------------
// * class IntertaskCondition
//
// This class provides a mechanism for synchronizing tasks.
//------------------------------------------------------------------------------------------------

class IntertaskCondition : public TaskSynchronizer
{
public:
	// constructor
	inline IntertaskCondition();

	// testing
	inline Bool isSignalled() const;

	// accessing
	inline Mutex &getMutex();

	// synchronizing
	inline void signal();

protected:
	// synchronizing
	void waitTask(Task *pTask);

	// representation
	Mutex mutex;
	IntertaskBroadcastEvent event;
};

//------------------------------------------------------------------------------------------------
// * IntertaskCondition::IntertaskCondition
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline IntertaskCondition::IntertaskCondition()
{
}

//------------------------------------------------------------------------------------------------
// * IntertaskCondition::isSignalled
//
// Tests whether the condition has changed.
//------------------------------------------------------------------------------------------------

inline Bool IntertaskCondition::isSignalled() const
{
	return event.isSignalled();
}

//------------------------------------------------------------------------------------------------
// * IntertaskCondition::getMutex
//
// Returns the mutex that protects access to the condition.
//------------------------------------------------------------------------------------------------

inline Mutex &IntertaskCondition::getMutex()
{
	return mutex;
}

//------------------------------------------------------------------------------------------------
// * IntertaskCondition::signal
//
// Notifies all awaiting tasks that the condition has changed.
//------------------------------------------------------------------------------------------------

inline void IntertaskCondition::signal()
{
	event.signal();
}

#endif // _IntertaskCondition_h_
