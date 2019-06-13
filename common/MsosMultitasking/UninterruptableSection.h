#ifndef _UninterruptableSection_h_
#define _UninterruptableSection_h_

//------------------------------------------------------------------------------------------------
// * class UninterruptableSection
//
// This class enters a critical section an ensures that it will be exited.
// An instance of this class should be declared as a stack variable.
//------------------------------------------------------------------------------------------------

class UninterruptableSection
{
public:
	// constructor and destructor
	inline UninterruptableSection();
	inline ~UninterruptableSection();
};

#include "TaskScheduler.h"

//------------------------------------------------------------------------------------------------
// * UninterruptableSection::UninterruptableSection
//
// This contructor has the side effect of entering a critical section.
//------------------------------------------------------------------------------------------------

inline UninterruptableSection::UninterruptableSection()
{
	TaskScheduler::getCurrentTaskScheduler()->enterUninterruptableSection();
}

//------------------------------------------------------------------------------------------------
// * UninterruptableSection::~UninterruptableSection
//
// This destructor has the side effect of exiting a critical section.
//------------------------------------------------------------------------------------------------

inline UninterruptableSection::~UninterruptableSection()
{
	TaskScheduler::getCurrentTaskScheduler()->exitUninterruptableSection();
}

#endif // _UninterruptableSection_h_
