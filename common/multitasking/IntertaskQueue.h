#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/IntertaskQueue.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/IntertaskQueue.h"
#endif
