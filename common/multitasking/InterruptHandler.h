#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/InterruptHandler.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/InterruptHandler.h"
#endif
