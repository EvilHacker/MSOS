#ifndef _80x86Interrupts_h_
#define _80x86Interrupts_h_

#include "../../cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * disableInterrupts
//------------------------------------------------------------------------------------------------

inline void disableInterrupts()
{
	__asm__
	{
		//cli
	}
}

//------------------------------------------------------------------------------------------------
// * enableInterrupts
//------------------------------------------------------------------------------------------------

inline void enableInterrupts()
{
	__asm__
	{
		//sti
	}
}

//------------------------------------------------------------------------------------------------
// * getInterruptState
//
// Returns the processor's status register value which includes
// interrupt enable flags and condition flags.
//------------------------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable : 4035)
inline UInt getInterruptState()
{
	__asm__
	{
		pushfd
		pop		eax
	}

	// return value is already in eax, no need for a return statement
}
#pragma warning(pop)

//------------------------------------------------------------------------------------------------
// * setInterruptState
//
// Modifies the processor's status register value which includes
// interrupt enable flags and condition flags.
//------------------------------------------------------------------------------------------------

inline void setInterruptState(UInt state)
{
	__asm__
	{
		push	state
		popfd
	}
}

#endif // _80x86Interrupts_h_
