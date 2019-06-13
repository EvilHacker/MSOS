#ifndef _UnpreemptableSection_h_
#define _UnpreemptableSection_h_

//------------------------------------------------------------------------------------------------
// * class UnpreemptableSection
//
// This class enters a critical section an ensures that it will be exited.
// An instance of this class should be declared as a stack variable.
//------------------------------------------------------------------------------------------------

class UnpreemptableSection
{
public:
	// constructor and destructor
	inline UnpreemptableSection();
	inline ~UnpreemptableSection();
};

#include "TaskScheduler.h"

//------------------------------------------------------------------------------------------------
// * UnpreemptableSection::UnpreemptableSection
//
// This contructor has the side effect of entering a critical section.
//------------------------------------------------------------------------------------------------

inline UnpreemptableSection::UnpreemptableSection()
{
	TaskScheduler::getCurrentTaskScheduler()->enterUnpreemptableSection();
}

//------------------------------------------------------------------------------------------------
// * UnpreemptableSection::~UnpreemptableSection
//
// This destructor has the side effect of exiting a critical section.
//------------------------------------------------------------------------------------------------

inline UnpreemptableSection::~UnpreemptableSection()
{
	TaskScheduler::getCurrentTaskScheduler()->exitUnpreemptableSection();
}

#endif // _UnpreemptableSection_h_
