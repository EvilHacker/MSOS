#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/IntertaskBroadcastEvent.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/IntertaskBroadcastEvent.h"
#endif
