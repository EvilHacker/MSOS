#include "Task.h"
#include "TaskBlocker.h"
#include "IntertaskEvent.h"
#include "UninterruptableSection.h"
#include "interrupts.h"
#include "../pointerArithmetic.h"
#include "../memoryUtilities.h"
#if defined(INCLUDE_DEBUGGER)
	#include "arm/RemoteDebuggerAgent.h"
#endif

//------------------------------------------------------------------------------------------------
// * Task::Task
//
// Constructs a new task given its <priority> level and <stackSize> in bytes.
//------------------------------------------------------------------------------------------------

Task::Task(UInt priority, UInt stackSize)
{
	// initialize instance variables
	this->priority = priority;
	pStack = new UInt8[stackSize];
	pStackTop = addToPointer(pStack, stackSize);
	suspendCount = 1;
	pGroup = getScheduler();

	// define the task entry point
	typedef void (Task::*EntryFunction)();
	const EntryFunction entryFunction = &Task::entry;

	// setup the stack for a suspended task,
	// when this task is resumed it will begin executing the entry() function
	// with this task as the receiver (this pointer) and no other arguments
	#if defined(_MSC_VER) && defined(_M_IX86)
		// Intel 80x86 32-bit processor

		// setup the stack as if an interrupt had occurred
		{
			// status = same as the current task's status
			pStackTop = subtractFromPointer(pStackTop, sizeof(UInt));
			*(UInt *)pStackTop = getInterruptState();
			// return address = entry() function
			pStackTop = subtractFromPointer(pStackTop, sizeof(void *));
			*(void **)pStackTop = *(void **)&entryFunction;
		}

		// simulate the "pushad" instruction
		{
			// eax unused
			pStackTop = subtractFromPointer(pStackTop, sizeof(UInt));
			*(UInt *)pStackTop = 0;
			// ecx = receiver (this pointer)
			pStackTop = subtractFromPointer(pStackTop, sizeof(Task *));
			*(Task **)pStackTop = this;
			// edx unused
			pStackTop = subtractFromPointer(pStackTop, sizeof(UInt));
			*(UInt *)pStackTop = 0;
			// ebx unused
			pStackTop = subtractFromPointer(pStackTop, sizeof(UInt));
			*(UInt *)pStackTop = 0;
			// esp unused
			pStackTop = subtractFromPointer(pStackTop, sizeof(UInt));
			*(UInt *)pStackTop = 0;
			// ebp unused
			pStackTop = subtractFromPointer(pStackTop, sizeof(UInt));
			*(UInt *)pStackTop = 0;
			// esi unused
			pStackTop = subtractFromPointer(pStackTop, sizeof(UInt));
			*(UInt *)pStackTop = 0;
			// edi unused
			pStackTop = subtractFromPointer(pStackTop, sizeof(UInt));
			*(UInt *)pStackTop = 0;
		}
	#elif defined(__i386__)
		struct InitialStackLayout
		{
			// processor state
			UInt fxsaveAlignment2[3];
			UInt8 fxsave[512];
			UInt fxsaveAlignment1[1];
			UInt edi;
			UInt esi;
			UInt ebp;
			UInt esp;
			UInt ebx;
			UInt edx;
			UInt ecx;
			UInt eax;
			UInt eip;
			UInt status;

			// stack setup for call to entry function
			UInt callerIp; // linkage area
			Task *thisPointerForEntryFunction; // 1 stack argument
			UInt stackArgumentAlignment[3]; // align stack arguments to 16 bytes
		};
	
		// push all zeros on the stack
		pStackTop = subtractFromPointer(pStackTop, sizeof(InitialStackLayout));
		memorySet(pStackTop, 0, sizeof(InitialStackLayout));

		// set all other values
		((InitialStackLayout *)pStackTop)->thisPointerForEntryFunction = this;
		((InitialStackLayout *)pStackTop)->status = getInterruptState();
		((InitialStackLayout *)pStackTop)->eip = *(UInt *)&entryFunction;
	
	#elif defined(_MSC_VER) && defined(_M_ARM) || defined(__ARMCC_VERSION)
		// ARM processor

		// push registers on the stack in the following order:
		// pc(r15), lr(r14), r12, r11, r10, r9, r8, r7, r6, r5, r4, r3, r2, r1, r0, cpsr
		{
			// pc = entry() function
			pStackTop = subtractFromPointer(pStackTop, sizeof(EntryFunction));
			*(EntryFunction *)pStackTop = entryFunction;
			// lr, r12, r11, r10, r9, r8, r7, r6, r5, r4, r3, r2, r1 are unused
			for(UInt registerNumber = 13; registerNumber > 0; --registerNumber)
			{
				pStackTop = subtractFromPointer(pStackTop, sizeof(UInt));
				*(UInt *)pStackTop = 0;
			}
			// r0 = receiver (this pointer)
			pStackTop = subtractFromPointer(pStackTop, sizeof(Task *));
			*(Task **)pStackTop = this;
			// cpsr = same as current task's cpsr with interrupts enabled
			pStackTop = subtractFromPointer(pStackTop, sizeof(UInt));
			*(UInt *)pStackTop = getInterruptState() & ~0xC0;
		}
	#else
		#error "unknown platform"
	#endif

	#if defined(INCLUDE_DEBUGGER)
		// register this task with the debugger
		RemoteDebuggerAgent *pDebugger = RemoteDebuggerAgent::getCurrentRemoteDebuggerAgent();
		if(pDebugger != null)
		{
			pDebugger->registerTask(this);
		}
	#endif
}

//------------------------------------------------------------------------------------------------
// * Task::~Task
//
// Destructor.
//------------------------------------------------------------------------------------------------

Task::~Task()
{
	#if defined(INCLUDE_DEBUGGER)
		// unregister this task with the debugger
		RemoteDebuggerAgent *pDebugger = RemoteDebuggerAgent::getCurrentRemoteDebuggerAgent();
		if(pDebugger != null)
		{
			pDebugger->unregisterTask(this);
		}
	#endif

	// make sure this task is not running
	suspend();

	delete[] (UInt8 *)pStack;
}

//------------------------------------------------------------------------------------------------
// * Task::setPriority
//
// Changes the <priority> level of this task.
//------------------------------------------------------------------------------------------------

void Task::setPriority(UInt priority)
{
	UninterruptableSection criticalSection;
	const Bool wasSuspended = isSuspended();
	if(!wasSuspended)
	{
		pGroup->removeTask(this);
	}
	this->priority = priority;
	if(!wasSuspended)
	{
		pGroup->addTask(this);
	}
}

//------------------------------------------------------------------------------------------------
// * Task::suspend
//
// Removes this task from the scheduler or blocker it is blocked on
// so that it will no longer run.
//------------------------------------------------------------------------------------------------

void Task::suspend()
{
	UninterruptableSection criticalSection;
	const Bool toBeSuspended = suspendCount == 0;
	if(toBeSuspended)
	{
		pGroup->suspendTask(this);
	}
	++suspendCount;
	if(toBeSuspended)
	{
		pGroup->removeTask(this);
	}
}

//------------------------------------------------------------------------------------------------
// * Task::resume
//
// Adds this task to the scheduler or blocker it was blocked on so that it can run.
//------------------------------------------------------------------------------------------------

void Task::resume()
{
	UninterruptableSection criticalSection;
	const Bool toBeResumed = suspendCount == 1;
	if(toBeResumed)
	{
		pGroup->resumeTask(this);
	}
	--suspendCount;
	if(toBeResumed)
	{
		pGroup->addTask(this);
	}
}

//------------------------------------------------------------------------------------------------
// * Task::yield
//
// Allows another task of greater or equal priority to run.
//------------------------------------------------------------------------------------------------

void Task::yield()
{
	UninterruptableSection criticalSection;
	if(!isSuspended())
	{
		pGroup->yieldTask(this);
	}
}

//------------------------------------------------------------------------------------------------
// * Task::blockOn
//
// Removes this task from the scheduler so that it will no longer run and adds
// the task to the <pTaskBlocker> so that the task can be unBlock()ed later.
// This method should only be called by TaskBlockers which control the
// blocking and unblocking of Tasks.
//------------------------------------------------------------------------------------------------

void Task::blockOn(TaskBlocker *pTaskBlocker)
{
	UninterruptableSection criticalSection;
	const Bool wasSuspended = isSuspended();
	if(!wasSuspended)
	{
		pGroup->removeTask(this);
	}
	pGroup = pTaskBlocker;
	if(!wasSuspended)
	{
		pGroup->addTask(this);
	}
}

//------------------------------------------------------------------------------------------------
// * Task::unblock
//
// Adds this task to the scheduler so that it will be able to run.
// This task will be dissassociated from the TaskBlocker it was blocked on.
// This method should only be called by TaskBlockers which control the
// blocking and unblocking of Tasks.
//------------------------------------------------------------------------------------------------

void Task::unblock()
{
	UninterruptableSection criticalSection;
	const Bool wasSuspended = isSuspended();
	if(!wasSuspended)
	{
		pGroup->removeTask(this);
	}
	pGroup = getScheduler();
	if(!wasSuspended)
	{
		pGroup->addTask(this);
	}
}

//------------------------------------------------------------------------------------------------
// * Task::entry
//
// This is the hidden entry point of a task.
// This function calls the user definable main() function and performs any cleanup required
// when the main() function completes.
//------------------------------------------------------------------------------------------------

void Task::entry()
{
	// check if the compiler does or does not support exceptions
	#if !defined(__ARMCC_VERSION)
		try
	#endif
		{
			// call the user defined main function
			main();
		}
	#if !defined(__ARMCC_VERSION)
		catch(...)
		{
			// ignore unhandled exceptions
		}
	#endif

	do
	{
		// remove this task from the scheduler and switch to another task
		suspend();
	}
	// loop just in case this task is resumed by another task
	while(true);
}
