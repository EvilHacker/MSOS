#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/TimeInterval.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/TimeInterval.h"
#endif