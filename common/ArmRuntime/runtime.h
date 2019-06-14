#ifndef _runtime_h_
#define _runtime_h_

#include "cPrimitiveTypes.h"

extern "C"
{
	void initializeHeap(UInt heapBaseAddress, UInt heapLimitAddress);
	void handleFatalError();
}

#endif // _runtime_h_
