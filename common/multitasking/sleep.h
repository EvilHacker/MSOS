#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/sleep.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/sleep.h"
#endif
