#include "common/cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * main
//------------------------------------------------------------------------------------------------

Int main()
{
	// run a simple multitasking test
	extern void rtosTest();
	rtosTest();

	// we will never get here
	return 0;
}
