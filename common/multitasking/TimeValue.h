#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/TimeValue.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/TimeValue.h"
#endif
