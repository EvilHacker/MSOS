#ifndef _multitaskingCommon_h_

#if defined(_MSC_VER) && defined(_M_ARM) && !defined(UNDER_CE) || defined(__ARMCC_VERSION)
	#define MSOS_MULTITASKING
#elif defined(WIN32)
	#define WIN32_MULTITASKING
#elif defined(UNDER_CE)
	#define WIN32_MULTITASKING
#else
	#error "unknown platform"
#endif

#endif //_multitaskingCommon_h_
