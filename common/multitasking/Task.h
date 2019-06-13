#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/Task.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/Task.h"
#endif
