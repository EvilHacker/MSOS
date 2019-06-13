#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/Semaphore.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/Semaphore.h"
#endif
