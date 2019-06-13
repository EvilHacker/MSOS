#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/TaskSynchronizer.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/TaskSynchronizer.h"
#endif
