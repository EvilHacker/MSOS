#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/Mutex.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/Mutex.h"
#endif
