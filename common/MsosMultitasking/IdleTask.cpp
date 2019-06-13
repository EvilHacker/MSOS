#include "TaskScheduler.h"
#include "UninterruptableSection.h"
#if defined(_MSC_VER) && defined(_M_ARM) || defined(__TARGET_CPU_SA_1100)
	#include "../Sa1110Devices/enterSa1110IdleMode.h"
#endif
#if defined(__TARGET_CPU_ARM920T)
	#include "../Mx1Devices/Mx1DeviceAddresses.h"
#endif
#include "IdleTask.h"

//------------------------------------------------------------------------------------------------
// * IdleTask::main
//
// This function sits in an infinite loop doing nothing and never returns.
//------------------------------------------------------------------------------------------------

void IdleTask::main()
{
	// infinite loop
	while(true)
	{
		#if defined(_MSC_VER) && defined(_M_ARM) || defined(__TARGET_CPU_SA_1100)
			// go into a power saving mode, we won't come out until an interrupt occurs
			UninterruptableSection criticalSection;
			enterSa1110IdleMode();
		#endif
		#if defined(__TARGET_CPU_ARM920T)
			// go into a power saving mode, we won't come out until an interrupt occurs
			UninterruptableSection criticalSection;

			// slow-down external bus, HCLK = BCLK = System PLL / <1 to 16>
			*(volatile UInt *)(mx1RegistersBase + 0x1B000) |= busClockDividerRegisterValue;

			asm
			{
				// fast bus clocking mode (GCLK = BCLK)
				mcr		p15, 0, 0, c15, c2, 2

				// wait for interrupt
				mcr		p15, 0, 0, c7, c0, 4

				// asynchronous clocking mode (GCLK = FCLK)
				mcr		p15, 0, 0, c15, c1, 2
			}

			// speed-up external bus, HCLK = BCLK = System PLL / 1
			*(volatile UInt *)(mx1RegistersBase + 0x1B000) &= ~0x00003C00;
		#endif
	}
}

//------------------------------------------------------------------------------------------------
// * IdleTask static variables
//------------------------------------------------------------------------------------------------

#if defined(__TARGET_CPU_ARM920T)
	UInt IdleTask::busClockDividerRegisterValue = (5 - 1) << 10;
#endif
