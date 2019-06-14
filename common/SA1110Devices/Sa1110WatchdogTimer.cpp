#if !defined(INCLUDE_DEBUGGER)

#include "Sa1110WatchdogTimer.h"
#include "../devices/WatchdogSection.h"
#include "../Collections/LinkedList.h"
#include "../multitasking/LockedSection.h"
#include "../multitasking/sleep.h"

//------------------------------------------------------------------------------------------------
// * Sa1110WatchdogTimer::Sa1110WatchdogTimer
//
// Constructor.
//------------------------------------------------------------------------------------------------

Sa1110WatchdogTimer::Sa1110WatchdogTimer() :
	Task(Task::realtimePriority, 10000)
{
	// start kicking task
	resume();

	// initialize first time
	update();

	// enabled the watchdog
	writeRegister(ower, 1);
}

//------------------------------------------------------------------------------------------------
// * Sa1110WatchdogTimer::~Sa1110WatchdogTimer
//
// Destructor.
//------------------------------------------------------------------------------------------------

Sa1110WatchdogTimer::~Sa1110WatchdogTimer()
{
	// the watchdog cannot be disabled and should never be destroyed
}

//------------------------------------------------------------------------------------------------
// * Sa1110WatchdogTimer::addSection
//
// Adds a watchdog section.
//------------------------------------------------------------------------------------------------

void Sa1110WatchdogTimer::addSection(WatchdogSection *pSection)
{
	LockedSection sectionsListLock(sectionsListMutex);

	// add the section sorted by the end time
	sectionsList.addSorted(pSection, compareWatchdogSections);

	// check if this section was added first in the list
	if(pSection == sectionsList.getFirst())
	{
		update();
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110WatchdogTimer::removeSection
//
// Removes a watchdog section.
//------------------------------------------------------------------------------------------------

void Sa1110WatchdogTimer::removeSection(WatchdogSection *pSection)
{
	LockedSection sectionsListLock(sectionsListMutex);

	// check if this section is first in the list
	if(pSection == sectionsList.getFirst())
	{
		// remove the first section and update timer
		sectionsList.removeFirst();
		update();
	}
	else
	{
		// remove this section,
		// we do not need to update because the first section did not change
		sectionsList.remove(pSection);
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110WatchdogTimer::update
//
// Update the state of the watchdog timer with new sections list.
//------------------------------------------------------------------------------------------------

void Sa1110WatchdogTimer::update()
{
	LockedSection sectionsListLock(sectionsListMutex);

	// set the watchdog match register
	const TimeValue watchdogMatchTime = sectionsList.isEmpty()
		? readRegister(oscr) + resetTime
		: ((WatchdogSection *)sectionsList.getFirst())->getEndTime();
	writeRegister(osmr3, watchdogMatchTime);

	// check if the watchdog has expired already
	if(compareTimes(readRegister(oscr), watchdogMatchTime) >= 0)
	{
		// soft reset
		writeRegister(rsrr, 1);
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110WatchdogTimer::main
//
// Task to kick the watchdog timer periodically.
//------------------------------------------------------------------------------------------------

void Sa1110WatchdogTimer::main()
{
	while(true)
	{
		sleepForTicks(kickTime, TaskScheduler::getCurrentTaskScheduler()->getTimer());
		update();
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110WatchdogTimer::compareWatchdogSections
//
// Compares the end times of two watchdog sections.
//------------------------------------------------------------------------------------------------

SInt Sa1110WatchdogTimer::compareWatchdogSections(
	const Link *pSection1,
	const Link *pSection2)
{
	return compareTimes(
		((WatchdogSection *)pSection1)->getEndTime(),
		((WatchdogSection *)pSection2)->getEndTime());
}

//------------------------------------------------------------------------------------------------
// * Sa1110WatchdogTimer static variables
//------------------------------------------------------------------------------------------------

Sa1110WatchdogTimer Sa1110WatchdogTimer::currentWatchdogTimer;

#endif // !defined(INCLUDE_DEBUGGER)
