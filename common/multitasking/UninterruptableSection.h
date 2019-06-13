#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/UninterruptableSection.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/UninterruptableSection.h"
#endif
