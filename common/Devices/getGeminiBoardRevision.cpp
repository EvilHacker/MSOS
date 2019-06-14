#include "getGeminiBoardRevision.h"
#if defined(__TARGET_CPU_ARM920T)
	#include "../Mx1Devices/Mx1GpioPin.h"
#endif
#if defined(__TARGET_CPU_SA_1100)
	#include "../Sa1110Devices/Sa1110GpioPin.h"
#endif

//------------------------------------------------------------------------------------------------
// getGeminiBoardRevision
//
// Reads the board revision.
//------------------------------------------------------------------------------------------------

UInt getGeminiBoardRevision()
{
	#if defined(__TARGET_CPU_ARM920T)
		Mx1GpioPin rev0(Mx1GpioPin::portD, 19);
		Mx1GpioPin rev1(Mx1GpioPin::portD, 20);
		Mx1GpioPin rev2(Mx1GpioPin::portD, 21);
	#endif
	#if defined(__TARGET_CPU_SA_1100)
		Sa1110GpioPin rev0(16);
		Sa1110GpioPin rev1(17);
		Sa1110GpioPin rev2(18);
	#endif
	
	rev0.configureAsInput();
	rev1.configureAsInput();
	rev2.configureAsInput();

	return 
		(rev0.getValue() << 0) | 
		(rev1.getValue() << 1) | 
		(rev2.getValue() << 2);
}

