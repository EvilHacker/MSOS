#include "LockedSection.h"

//------------------------------------------------------------------------------------------------
// * LockedSection::~LockedSection
//
// This destructor has the side effect of unlocking the synchronizer
// which the constructor had locked.
//------------------------------------------------------------------------------------------------

LockedSection::~LockedSection()
{
	taskSynchronizer.unlock();
}
