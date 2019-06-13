#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/MemberTask.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/MemberTask.h"
#endif
