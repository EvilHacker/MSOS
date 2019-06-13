#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/IntertaskEvent.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/IntertaskEvent.h"
#endif
