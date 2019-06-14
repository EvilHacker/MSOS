#ifndef _timing_h_
#define _timing_h_

#include "../cPrimitiveTypes.h"
#include "../Multitasking/TimeValue.h"
#if defined(_MSC_VER) && defined(_M_IX86)
	#include "../arithmetic/constIntLog2.h"
	#include <windows.h>
#elif defined(_WIN32_WCE)
	#include <windows.h>
#elif defined(_MSC_VER) && defined(_M_ARM) || defined(__ARMCC_VERSION)
	#include "../Multitasking/TaskScheduler.h"
#endif


//------------------------------------------------------------------------------------------------
// * static variables
//------------------------------------------------------------------------------------------------

#if defined(_MSC_VER) && defined(_M_IX86)
	extern const UInt timestampShift;
#endif


//------------------------------------------------------------------------------------------------
// * timestamp
//
// Returns the current time relative to an unknown starting time.
// The highest resolution timer available will be used.
//------------------------------------------------------------------------------------------------

inline TimeValue timestamp()
{
	#if defined(_MSC_VER) && defined(_M_IX86)
		// use Windows API function
		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);
		return (TimeValue)(t.QuadPart >> timestampShift);
	#elif defined(_WIN32_WCE)
		// use Windows API function
		return GetTickCount();
	#elif defined(_MSC_VER) && defined(_M_ARM) || defined(__ARMCC_VERSION)
		// use MSOS function
		return TaskScheduler::getCurrentTaskScheduler()->getTimer()->getTime();
	#endif
}

//------------------------------------------------------------------------------------------------
// * getTimerFrequency
//
// Returns the frequency of a high resolution timer.
//------------------------------------------------------------------------------------------------

inline TimeValue getTimerFrequency()
{
	#if defined(_MSC_VER) && defined(_M_IX86)
		// use Windows API function
		LARGE_INTEGER t;
		QueryPerformanceFrequency(&t);
		return (TimeValue)(t.QuadPart >> timestampShift);
	#elif defined(_WIN32_WCE)
		// use Windows API function
		return 1000;
	#elif defined(_MSC_VER) && defined(_M_ARM) || defined(__ARMCC_VERSION)
		// use MSOS function
		return TaskScheduler::getCurrentTaskScheduler()->getTimer()->getFrequency();
	#endif
}

#endif // _timing_h_
