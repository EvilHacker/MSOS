#if !defined(INCLUDE_DEBUGGER)

#include "Mx1WatchdogTimer.h"
#include "Mx1InterruptController.h"
#include "../devices/WatchdogSection.h"
#include "../Collections/LinkedList.h"
#include "../multitasking/LockedSection.h"
#include "../multitasking/UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * Mx1WatchdogTimer::Mx1WatchdogTimer
//
// Constructor.
//------------------------------------------------------------------------------------------------

Mx1WatchdogTimer::Mx1WatchdogTimer()
{
}

//------------------------------------------------------------------------------------------------
// * Mx1WatchdogTimer::~Mx1WatchdogTimer
//
// Destructor.
//------------------------------------------------------------------------------------------------

Mx1WatchdogTimer::~Mx1WatchdogTimer()
{
	// disable the watchdog
	writeRegister(wcr, 0x0002);
}

//------------------------------------------------------------------------------------------------
// * Mx1WatchdogTimer::addSection
//
// Adds a watchdog section.
//------------------------------------------------------------------------------------------------

void Mx1WatchdogTimer::addSection(WatchdogSection *pSection)
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
// * Mx1WatchdogTimer::removeSection
//
// Removes a watchdog section.
//------------------------------------------------------------------------------------------------

void Mx1WatchdogTimer::removeSection(WatchdogSection *pSection)
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
// * Mx1WatchdogTimer::update
//
// Update the state of the watchdog timer with new sections list.
//------------------------------------------------------------------------------------------------

void Mx1WatchdogTimer::update()
{
	LockedSection sectionsListLock(sectionsListMutex);

	// disable the watchdog
	writeRegister(wcr, 0x0002);

	// check if there is some watchdog section
	if(!sectionsList.isEmpty())
	{
		const Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer();
		const TimeValue halfSecond = pTimer->getFrequency() / 2;
		const WatchdogSection *pSection = (WatchdogSection *)sectionsList.getFirst();
		const SInt counter = (pSection->getEndTime() - pTimer->getTime() + halfSecond - 1) / halfSecond;
		const UInt controlRegister = maximum(1, minimum(counter, 63)) << 8;

		// set the watchdog counter
		writeRegister(wcr, controlRegister | 0x0002);

		// enable the watchdog
		writeRegister(wcr, controlRegister | 0x0003);

		// check if the section has already expired
		if(counter <= 0)
		{
			// stop and wait for system to reset itself
			UninterruptableSection criticalSection;
			while(true);
		}
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1WatchdogTimer::compareWatchdogSections
//
// Compares the end times of two watchdog sections.
//------------------------------------------------------------------------------------------------

SInt Mx1WatchdogTimer::compareWatchdogSections(
	const Link *pSection1,
	const Link *pSection2)
{
	return compareTimes(
		((WatchdogSection *)pSection1)->getEndTime(),
		((WatchdogSection *)pSection2)->getEndTime());
}

//------------------------------------------------------------------------------------------------
// * Mx1WatchdogTimer static variables
//------------------------------------------------------------------------------------------------

Mx1WatchdogTimer Mx1WatchdogTimer::currentWatchdogTimer;

#endif // !defined(INCLUDE_DEBUGGER)
