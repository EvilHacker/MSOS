#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/Timer.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/Timer.h"
#endif