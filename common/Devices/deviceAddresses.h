#ifndef _deviceAddresses_h_
#define _deviceAddresses_h_

#if defined(__TARGET_CPU_ARM920T)
	#include "../Mx1Devices/Mx1DeviceAddresses.h"
#endif

#if defined(__TARGET_CPU_SA_1100)
	#include "../Sa1110Devices/Sa1110DeviceAddresses.h"
#endif

#endif // _deviceAddresses_h_
