#include "TaskScheduler.h"
#include "IdleTask.h"
#include "interrupts.h"

#if (defined(_MSC_VER) && defined(_M_IX86)) || defined(__i386__)
	// Intel 80x86 32-bit processor
	#include "80x86/switchTasks.h"
#elif defined(_MSC_VER) && defined(_M_ARM) || defined(__ARMCC_VERSION)
	// ARM processor
	#include "arm/exceptionHandlers.h"
	#include "arm/switchTasks.h"
	#define SWITCH_TASKS_USING_SWI
	//#define SWITCH_TASKS_USING_IRQ
	//#define SWITCH_TASKS_USING_FIQ
#else
	#error "unknown platform"
#endif

//------------------------------------------------------------------------------------------------
// * TaskScheduler::TaskScheduler
//
// Constructor.
// The scheduler will start with preemption disabled.
// The user must call the start() function once to begin scheduling.
//------------------------------------------------------------------------------------------------

TaskScheduler::TaskScheduler()
{
	// initialize instance variables
	pCurrentTask = null;
	unpreemptableSectionEntryCount = 1;
	uninterruptableSectionEntryCount = 0;

	// add an idle task
	addTask(&idleTask);

	// install exception handlers
	#if (defined(_MSC_VER) && defined(_M_IX86)) || defined(__i386__)
		// Intel 80x86 32-bit processor

		// interrupt handlers and timer not currently implemented
	#elif defined(_MSC_VER) && defined(_M_ARM) || defined(__ARMCC_VERSION)
		// ARM processor

		// replace IRQ and FIQ handlers
		oldIrqHandler = getExceptionHandler(irqVectorIndex);
		setExceptionHandler(irqVectorIndex, &handleIrq);
		oldFiqHandler = getExceptionHandler(fiqVectorIndex);
		setExceptionHandler(fiqVectorIndex, &handleFiq);

		#if defined(SWITCH_TASKS_USING_SWI)
			// chain into software interrupt exception handler
			oldSoftwareInterruptHandler = getExceptionHandler(softwareInterruptVectorIndex);
			setExceptionHandler(softwareInterruptVectorIndex, &handleSoftwareInterrupt);
		#endif
	#else
		#error "unknown platform"
	#endif
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::~TaskScheduler
//
// Destructor.
//------------------------------------------------------------------------------------------------

TaskScheduler::~TaskScheduler()
{
	#if (defined(_MSC_VER) && defined(_M_IX86)) || defined(__i386__)
		// Intel 80x86 32-bit processor

	#elif defined(_MSC_VER) && defined(_M_ARM) || defined(__ARMCC_VERSION)
		// ARM processor

		// uninstall exception handlers
		setExceptionHandler(irqVectorIndex, oldIrqHandler);
		setExceptionHandler(fiqVectorIndex, oldFiqHandler);
		#if defined(SWITCH_TASKS_USING_SWI)
			setExceptionHandler(softwareInterruptVectorIndex, oldSoftwareInterruptHandler);
		#endif
	#else
		#error "unknown platform"
	#endif
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::enterUninterruptableSection
//
// Disables interrupts.
//------------------------------------------------------------------------------------------------

void TaskScheduler::enterUninterruptableSection()
{
	if(uninterruptableSectionEntryCount == 0)
	{
		const UInt savedInterruptState = getInterruptState();
		disableInterrupts();
		interruptState = savedInterruptState;
	}
	++uninterruptableSectionEntryCount;
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::exitUninterruptableSection
//
// Enables interrupts.
//------------------------------------------------------------------------------------------------

void TaskScheduler::exitUninterruptableSection()
{
	if(--uninterruptableSectionEntryCount == 0)
	{
		const UInt savedInterruptState = interruptState;
		schedule();
		setInterruptState(savedInterruptState);
	}
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::schedule
//
// Switches to the highest priority task that is ready to run.
//------------------------------------------------------------------------------------------------

void TaskScheduler::schedule()
{
	// check if preemption is disabled
	if(unpreemptableSectionEntryCount != 0)
	{
		// do not switch tasks when preemption is disabled
		return;
	}

	#if defined(SWITCH_TASKS_USING_SWI)
		asm
		{
			swi		0
		}
	#elif defined(SWITCH_TASKS_USING_IRQ)
		simulateIrq();
	#elif defined(SWITCH_TASKS_USING_FIQ)
		simulateFiq();
	#else
		// check if the currently running task is not the highest priority task
		if(getCurrentTask() != getFirstTask())
		{
			if(getCurrentTask() == null)
			{
				// start the very first task
				void *pCurrentTaskStackTop;
				switchTasks(&pCurrentTaskStackTop, &(pCurrentTask = getFirstTask())->pStackTop);
			}
			else
			{
				// switch from the current task to the highest priority task
				void **ppCurrentTaskStackTop = &getCurrentTask()->pStackTop;
				switchTasks(ppCurrentTaskStackTop, &(pCurrentTask = getFirstTask())->pStackTop);
			}
		}
	#endif
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::handleInterrupt
//
// Handles an interrupt.
// Must be called from within an interrupt service routine.
//------------------------------------------------------------------------------------------------

void TaskScheduler::handleInterrupt(InterruptLevel level)
{
	// do not switch tasks during an interrupt
	++unpreemptableSectionEntryCount;

	// check if the compiler does or does not support exceptions
	#if !defined(__ARMCC_VERSION)
		try
	#endif
		{
			// iterate over all interrupt handlers
			InterruptHandler *pHandler = getFirstInterruptHandler(level);
			while(pHandler != null)
			{
				// call the handler
				if(pHandler->handleInterrupt())
				{
					// stop if this interrupt was handled
					break;
				}
		
				// next handler
				pHandler = pHandler->getNextHandler();
			}
		}
	#if !defined(__ARMCC_VERSION)
		catch(...)
		{
			// ignore unhandled exceptions
		}
	#endif

	--unpreemptableSectionEntryCount;
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::compareInterruptHandlers
//
// Compares the two interrupt handlers by their priority.
//------------------------------------------------------------------------------------------------

SInt TaskScheduler::compareInterruptHandlers(
	const Link *pInterruptHandler1,
	const Link *pInterruptHandler2)
{
	return (SInt)((InterruptHandler *)pInterruptHandler2)->getPriority() + 1
		- (SInt)((InterruptHandler *)pInterruptHandler1)->getPriority();
}

//------------------------------------------------------------------------------------------------
// * TaskScheduler::returnFromInterrupt
//
// Helper function for returning from an interrupt service routine.
// The Assembly caller will actually do the switch based on the return value.
// Must be called from within an interrupt service routine.
//------------------------------------------------------------------------------------------------

#if defined(_MSC_VER) && defined(_M_ARM) || defined(__ARMCC_VERSION)
	__value_in_regs TaskScheduler::ReturnFromInterruptInfo TaskScheduler::returnFromInterrupt()
	{
		ReturnFromInterruptInfo info;

		// check if preemption is enabled and
		// check if the currently running task is not the highest priority task
		if(unpreemptableSectionEntryCount == 0
			&& getCurrentTask() != getFirstTask())
		{
			// indicate stack of task being switched from
			if(getCurrentTask() == null)
			{
				// no current task
				static void *pDummy;
				info.ppSwitchFromTaskStackTop = &pDummy;
			}
			else
			{
				// stack of current task
				info.ppSwitchFromTaskStackTop = &getCurrentTask()->pStackTop;
			}

			// indicate stack of task being switched to
			info.ppSwitchToTaskStackTop = &getFirstTask()->pStackTop;

			// change the current task
			pCurrentTask = getFirstTask();
		}
		else
		{
			// indicate that no task switch should occur
			info.ppSwitchFromTaskStackTop = null;
			info.ppSwitchToTaskStackTop = null;
		}

		return info;
	}
#endif

//------------------------------------------------------------------------------------------------
// * TaskScheduler static variables
//------------------------------------------------------------------------------------------------

TaskScheduler TaskScheduler::currentTaskScheduler;
#if defined(__TARGET_CPU_ARM920T)
	// create a 4.000MHz clock (assuming System PLL is 96.000MHz and PERCLK1 divider is one of 12,8,6,4,3,2,1)
	#include "../Mx1Devices/Mx1DeviceAddresses.h"
	Mx1Timer TaskScheduler::timer(
		Mx1Timer::timer1,
		Mx1Timer::perclk1,
		96000000 / 4000000 / ((*(volatile UInt *)(mx1RegistersBase + 0x1B020) & 0xF) + 1),
		4000000);
#endif
#if defined(__TARGET_CPU_SA_1100)
	Sa1110Timer TaskScheduler::timer;
#endif
