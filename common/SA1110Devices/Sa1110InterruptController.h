#ifndef _Sa1110InterruptController_h_
#define _Sa1110InterruptController_h_

#include "../cPrimitiveTypes.h"
#include "../multitasking/UninterruptableSection.h"
#include "Sa1110DeviceAddresses.h"

//------------------------------------------------------------------------------------------------
// * class Sa1110InterruptController
//
// Interface to the SA-1110 Interrupt Controller.
//------------------------------------------------------------------------------------------------

class Sa1110InterruptController
{
public:
	// accessing
	inline static Sa1110InterruptController *getCurrentInterruptController();

	// testing
	inline Bool isPending(UInt interruptNumber) const;

	// modifying
	inline void setToIrq(UInt interruptNumber);
	inline void setToFiq(UInt interruptNumber);
	inline void enable(UInt interruptNumber);
	inline void disable(UInt interruptNumber);

private:
	// constructor
	inline Sa1110InterruptController();

	// registers
	enum RegisterAddress
	{
		icmr = sa1110SystemControlBase + 0x50004, // mask register
		iclr = sa1110SystemControlBase + 0x50008, // level register
		iccr = sa1110SystemControlBase + 0x5000C, // control register
		icpr = sa1110SystemControlBase + 0x50020, // pending register (read only)
		icip = sa1110SystemControlBase + 0x50000, // IRQ pending register (read only)
		icfp = sa1110SystemControlBase + 0x50010  // FIQ pending register (read only)
	};

	// singleton
	static Sa1110InterruptController currentInterruptController;
};

//------------------------------------------------------------------------------------------------
// * Sa1110InterruptController::Sa1110InterruptController
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline Sa1110InterruptController::Sa1110InterruptController()
{
}

//------------------------------------------------------------------------------------------------
// * Sa1110InterruptController::getCurrentInterruptController
//
// Returns the singleton instance.
//------------------------------------------------------------------------------------------------

inline Sa1110InterruptController *Sa1110InterruptController::getCurrentInterruptController()
{
	return &currentInterruptController;
}

//------------------------------------------------------------------------------------------------
// * Sa1110InterruptController::isPending
//
// Tests whether a particular interrupt is pending.
//------------------------------------------------------------------------------------------------

inline Bool Sa1110InterruptController::isPending(UInt interruptNumber) const
{
	return ((*(volatile UInt *)icpr >> interruptNumber) & 1) != 0;
}

//------------------------------------------------------------------------------------------------
// * Sa1110InterruptController::setToIrq
//
// Sets the level of a particular interrupt to IRQ.
//------------------------------------------------------------------------------------------------

inline void Sa1110InterruptController::setToIrq(UInt interruptNumber)
{
	const UInt mask = 1 << interruptNumber;

	UninterruptableSection criticalSection;
	*(volatile UInt *)iclr &= ~mask;
}

//------------------------------------------------------------------------------------------------
// * Sa1110InterruptController::setToFiq
//
// Sets the level of a particular interrupt to FIQ.
//------------------------------------------------------------------------------------------------

inline void Sa1110InterruptController::setToFiq(UInt interruptNumber)
{
	const UInt mask = 1 << interruptNumber;

	UninterruptableSection criticalSection;
	*(volatile UInt *)icmr |= mask;
}

//------------------------------------------------------------------------------------------------
// * Sa1110InterruptController::enable
//
// Enables a particular interrupt.
//------------------------------------------------------------------------------------------------

inline void Sa1110InterruptController::enable(UInt interruptNumber)
{
	const UInt mask = 1 << interruptNumber;

	UninterruptableSection criticalSection;
	*(volatile UInt *)icmr |= mask;
}

//------------------------------------------------------------------------------------------------
// * Sa1110InterruptController::disable
//
// Disables a particular interrupt.
//------------------------------------------------------------------------------------------------

inline void Sa1110InterruptController::disable(UInt interruptNumber)
{
	const UInt mask = 1 << interruptNumber;

	UninterruptableSection criticalSection;
	*(volatile UInt *)icmr &= ~mask;
}

#endif // _Sa1110InterruptController_h_
