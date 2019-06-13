#ifndef _exceptionHandlers_h_
#define _exceptionHandlers_h_

#include "../../cPrimitiveTypes.h"

// types
typedef void (*ExceptionHandler)();
enum ExceptionVectorIndex
{
	resetVectorIndex,
	undefinedInstructionVectorIndex,
	softwareInterruptVectorIndex,
	instructionAccessAbortVectorIndex,
	dataAccessAbortVectorIndex,
	reservedVectorIndex,
	irqVectorIndex,
	fiqVectorIndex
};

// implemented in C++ file "exceptionHandlersC.s"
void setExceptionHandler(ExceptionVectorIndex vectorIndex, ExceptionHandler handler);

// implemented in Assembly file "exceptionHandlersAsm.s"
extern "C" ExceptionHandler oldResetHandler;
extern "C" ExceptionHandler oldSoftwareInterruptHandler;
extern "C" ExceptionHandler oldUndefinedInstructionHandler;
extern "C" ExceptionHandler oldInstructionAccessAbortHandler;
extern "C" ExceptionHandler oldDataAccessAbortHandler;
extern "C" ExceptionHandler oldIrqHandler;
extern "C" ExceptionHandler oldFiqHandler;
extern "C" void handleReset();
extern "C" void handleSoftwareInterrupt();
extern "C" void handleUndefinedInstruction();
extern "C" void handleInstructionAccessAbort();
extern "C" void handleDataAccessAbort();
extern "C" void handleIrq();
extern "C" void handleFiq();
extern "C" void simulateIrq();
extern "C" void simulateFiq();

//------------------------------------------------------------------------------------------------
// * getExceptionHandler
//
// Returns the exception handler in the vector table at <vectorIndex>.
//------------------------------------------------------------------------------------------------

inline ExceptionHandler getExceptionHandler(ExceptionVectorIndex vectorIndex)
{
	// return an entry in the virtual vector table
	return ((ExceptionHandler *)0x20)[vectorIndex];
}

#endif // _exceptionHandlers_h_
