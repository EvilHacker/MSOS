#include "multitaskingCommon.h"

#if defined(MSOS_MULTITASKING)
	#include "../MsosMultitasking/IntertaskCondition.h"
#endif
#if defined(WIN32_MULTITASKING)
	#include "../Win32Multitasking/IntertaskCondition.h"
#endif
