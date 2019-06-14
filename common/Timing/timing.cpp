#include "timing.h"

//------------------------------------------------------------------------------------------------
// * computeTimestampShift
//
// Computes the shift so that a TimeValue can represent at least 4095 seconds (more that 1 hour).
//------------------------------------------------------------------------------------------------

#if defined(_MSC_VER) && defined(_M_IX86)
static UInt computeTimestampShift()
{
	LARGE_INTEGER t;
	QueryPerformanceFrequency(&t);
	return constIntLog2(t.QuadPart >> (sizeof(TimeValue) * 8 - 14));
}
#endif

//------------------------------------------------------------------------------------------------
// * static variables
//------------------------------------------------------------------------------------------------

#if defined(_MSC_VER) && defined(_M_IX86)
	const UInt timestampShift = computeTimestampShift();
#endif
