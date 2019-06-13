#include "TaskGroup.h"

#ifndef _TaskScheduler_h_
#define _TaskScheduler_h_

class Task;
#include "IdleTask.h"
#include "InterruptHandler.h"
class Timer;
#if defined(__TARGET_CPU_ARM920T)
	#include "../Mx1Devices/Mx1Timer.h"
#endif
#if defined(__TARGET_CPU_SA_1100)
	#include "../Sa1110Devices/Sa1110Timer.h"
#endif

//------------------------------------------------------------------------------------------------
// * class TaskScheduler
//
// Keeps track of running tasks and schedules them to run based on their priorities.
//------------------------------------------------------------------------------------------------

class TaskScheduler : public TaskGroup
{
public:
	// types
	typedef InterruptHandler::InterruptLevel InterruptLevel;

	// destructor
	virtual ~TaskScheduler();

	// querying
	inline static TaskScheduler *getCurrentTaskScheduler();
	inline Task *getCurrentTask() const;
	inline Timer *getTimer();
	inline InterruptHandler *getFirstInterruptHandler(InterruptLevel level);

	// testing
	static inline Bool isInitialized();

	// behaviour
	inline void start();

	// modifying interrupt handlers
	inline void addInterruptHandler(InterruptHandler *pHandler);
	inline void removeInterruptHandler(InterruptHandler *pHandler);

private:
	// constructor (private because the class has a singleton instance)
	TaskScheduler();

	// critical sections
	inline void enterUnpreemptableSection();
	inline void exitUnpreemptableSection();
	void enterUninterruptableSection();
	void exitUninterruptableSection();
	friend class UnpreemptableSection;
	friend class UninterruptableSection;

	// task scheduling
	void schedule();

	// interrupt handling
	void handleInterrupt(InterruptLevel level);
	#if defined(_MSC_VER) && defined(_M_ARM) || defined(__ARMCC_VERSION)
		struct ReturnFromInterruptInfo
		{
			void **ppSwitchFromTaskStackTop;
			void **ppSwitchToTaskStackTop;
		};
		__value_in_regs ReturnFromInterruptInfo returnFromInterrupt();
	#endif
	static SInt compareInterruptHandlers(
		const Link *pInterruptHandler1,
		const Link *pInterruptHandler2);

	// representation
	static TaskScheduler currentTaskScheduler;
	IdleTask idleTask;
	#if defined(__TARGET_CPU_ARM920T)
		static Mx1Timer timer;
	#endif
	#if defined(__TARGET_CPU_SA_1100)
		static Sa1110Timer timer;
	#endif
	Task *pCurrentTask;
	UInt unpreemptableSectionEntryCount;
	UInt uninterruptableSectionEntryCount;
	UInt interruptState;
	LinkedList interruptHandlerLists[InterruptHandler::numberOfInterruptLevels];
};

#include "Task.h"
#include "InterruptHandler.h"
#include "UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * TaskScheduler::getCurrentTaskScheduler
//
// Returns the task scheduler that is controlling the processor.
//------------------------------------------------------------------------------------------------

inline TaskScheduler *TaskScheduler::getCurrentTaskScheduler()
{
	return &currentTaskScheduler;
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::getCurrentTask
//
// Returns the task that is currently running.
//------------------------------------------------------------------------------------------------

inline Task *TaskScheduler::getCurrentTask() const
{
	return pCurrentTask;
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::getTimer
//
// Returns the default timer.
//------------------------------------------------------------------------------------------------

inline Timer *TaskScheduler::getTimer()
{
	#if defined(_MSC_VER) && defined(_M_ARM) || defined(__ARMCC_VERSION)
		return &timer;
	#else
		return null;
	#endif
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::getFirstInterruptHandler
//
// Returns the first interrupt handler.
//------------------------------------------------------------------------------------------------

inline InterruptHandler *TaskScheduler::getFirstInterruptHandler(InterruptLevel level)
{
	return (InterruptHandler *)interruptHandlerLists[level].getFirst();
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::isInitialized
//
// Tests whether the class has been initialized.
//------------------------------------------------------------------------------------------------

inline Bool TaskScheduler::isInitialized()
{
	return currentTaskScheduler.pCurrentTask != null;
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::start
//
// Starts the scheduler and multitasking.
// This function will not return to the caller, only Task objects will run.
// This function must be called only once.
//------------------------------------------------------------------------------------------------

inline void TaskScheduler::start()
{
	// enable preemption (the scheduler starts with preemption disabled)
	exitUnpreemptableSection();
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::addInterruptHandler
//
// Adds the specified <pHandler> to the list.
//------------------------------------------------------------------------------------------------

inline void TaskScheduler::addInterruptHandler(InterruptHandler *pHandler)
{
	UninterruptableSection criticalSection;
	interruptHandlerLists[pHandler->getLevel()].addSorted(pHandler, compareInterruptHandlers);
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::removeInterruptHandler
//
// Removes the specified <pHandler> from the list.
//------------------------------------------------------------------------------------------------

inline void TaskScheduler::removeInterruptHandler(InterruptHandler *pHandler)
{
	UninterruptableSection criticalSection;
	interruptHandlerLists[pHandler->getLevel()].remove(pHandler);
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::enterUnpreemptableSection
//
// Prevents task synchronizers and interrupt service routines from switching tasks.
// Interrupts are neither enabled nor disabled.
//------------------------------------------------------------------------------------------------

inline void TaskScheduler::enterUnpreemptableSection()
{
	++unpreemptableSectionEntryCount;
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::exitUnpreemptableSection
//
// Allows task synchronizers and interrupt service routines to switch tasks.
// Interrupts are neither enabled nor disabled.
//------------------------------------------------------------------------------------------------

inline void TaskScheduler::exitUnpreemptableSection()
{
	if(--unpreemptableSectionEntryCount == 0)
	{
		// this will cause another task to be scheduled
		UninterruptableSection criticalSection;
	}
}

#endif // _TaskScheduler_h_
