#ifndef _LockedSection_h_
#define _LockedSection_h_

#include "TaskSynchronizer.h"

//------------------------------------------------------------------------------------------------
// * class LockedSection
//
// This class locks a TaskSynchronizer and ensures that it will be unlocked.
// An instance of this class should be declared as a stack variable.
// The TaskSynchronizer is locked where the the LockedSection has been declare and
// is unlocked when the scope of the LockedSection exits.
// All return, break, continue, or throw statements that exit the scope of the
// LockedSection will cause the TaskSynchronizer to be unlocked.
//------------------------------------------------------------------------------------------------

class LockedSection
{
public:
	struct DoNotLock {};

	// constructors and destructors
	inline LockedSection(TaskSynchronizer &taskSynchronizer);
	inline LockedSection(TaskSynchronizer &taskSynchronizer, DoNotLock);
	virtual ~LockedSection();

private:
	// representation
	TaskSynchronizer &taskSynchronizer;
};

//------------------------------------------------------------------------------------------------
// * LockedSection::LockedSection
//
// This contructor has the side effect of locking the specified <pTaskSynchronizer>.
//------------------------------------------------------------------------------------------------

inline LockedSection::LockedSection(TaskSynchronizer &taskSynchronizer) :
	taskSynchronizer(taskSynchronizer)
{
	taskSynchronizer.lock();
}

//------------------------------------------------------------------------------------------------
// * LockedSection::LockedSection
//
// This contructor does not lock <pTaskSynchronizer>.
//------------------------------------------------------------------------------------------------

inline LockedSection::LockedSection(TaskSynchronizer &taskSynchronizer, DoNotLock) :
	taskSynchronizer(taskSynchronizer)
{
}

#endif // _LockedSection_h_
