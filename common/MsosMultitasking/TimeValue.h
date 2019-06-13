#ifndef _TimeValue_h_
#define _TimeValue_h_

#include "../cPrimitiveTypes.h"

typedef SInt TimeValue;
const TimeValue infiniteTime = minimumOfIntegerType(TimeValue);

//------------------------------------------------------------------------------------------------
// * compareTimes
//
// Compares two time values.
// The function returns -, 0, + if the arguments are <, =, > each other.
// Works even if values wrap around, however, the logical difference between the two values can't
// be more than the maximum value representable by the TimeValue type.
// Logically, |time1 - time2| < maximumOfIntegerType(TimeValue).
//------------------------------------------------------------------------------------------------

inline SInt compareTimes(TimeValue time1, TimeValue time2)
{
	return time1 - time2;
}

#endif // _TimeValue_h_
