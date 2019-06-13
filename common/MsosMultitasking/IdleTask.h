#ifndef _IdleTask_h_
#define _IdleTask_h_

#include "../cPrimitiveTypes.h"
#include "Task.h"

//------------------------------------------------------------------------------------------------
// * class IdleTask
//
// This task just runs at the lowest priority and loops forever doing nothing.
// It is run only when all other tasks are blocked perhaps waiting for an interrupt or timeout and
// there is nothing else usefull for the processor to do.
//------------------------------------------------------------------------------------------------

class IdleTask : public Task
{
public:
	// constructor
	inline IdleTask();

	#if defined(__TARGET_CPU_ARM920T)
		// controlling bus clock
		static inline void setBusClockDivider(UInt denominator);
	#endif

protected:
	// main entry point
	void main();

private:
	static inline UInt getDefaultStackSize();
	static UInt busClockDividerRegisterValue;
};

//------------------------------------------------------------------------------------------------
// * IdleTask::IdleTask
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline IdleTask::IdleTask() :
	Task(0, getDefaultStackSize())
{
}

//------------------------------------------------------------------------------------------------
// * IdleTask::setBusClockDivider
//
// Set the bus clock divider when the processor is idle.
// The <denominator> must be between 1 and 16.
//------------------------------------------------------------------------------------------------

#if defined(__TARGET_CPU_ARM920T)
inline void IdleTask::setBusClockDivider(UInt denominator)
{
	busClockDividerRegisterValue = (denominator - 1) << 10;
}
#endif

//------------------------------------------------------------------------------------------------
// * IdleTask::getDefaultStackSize
//
// Returns the default stack size for this task.
// This value will be relatively small because the task does make any function calls.
//------------------------------------------------------------------------------------------------

inline UInt IdleTask::getDefaultStackSize()
{
	// to do: calculate the smallest stack size possible, these are conservative estimates

	// the stack space used is dependent on the compiler settings and processor
	#if defined(_MSC_VER) && defined(_M_IX86)
		return 1024;
	#elif defined(_MSC_VER) && defined(_M_ARM)
		return 1024;
	#elif defined(__ARMCC_VERSION)
		return 1024;
	#elif defined(__i386__)
		return 2048;
	#elif defined(__x86_64__)
		return 3072;
	#elif defined(__ppc__)
		return 1024;
	#elif defined(__ppc64__)
		return 2048;
	#else
		#error "unknown platform"
	#endif
}

#endif // _IdleTask_h_
