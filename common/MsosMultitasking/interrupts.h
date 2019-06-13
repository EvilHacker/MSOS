#ifndef _interrupts_h_
#define _interrupts_h_

#if defined(_MSC_VER) && defined(_M_IX86) || defined(__i386__)

#include "80x86/80x86Interrupts.h"

#else

#include "../cPrimitiveTypes.h"
extern "C"
{
	void disableInterrupts();
	void enableInterrupts();
	UInt getInterruptState();
	void setInterruptState(UInt state);
}

#endif

#endif // _interrupts_h_
