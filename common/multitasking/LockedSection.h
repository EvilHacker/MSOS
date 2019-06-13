#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/LockedSection.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/LockedSection.h"
#endif
