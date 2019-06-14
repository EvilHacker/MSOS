#ifndef _Mx1InterruptController_h_
#define _Mx1InterruptController_h_

#include "../cPrimitiveTypes.h"
#include "Mx1DeviceAddresses.h"

//------------------------------------------------------------------------------------------------
// * class Mx1InterruptController
//
// Interface to the MX1 Interrupt Controller.
//------------------------------------------------------------------------------------------------

class Mx1InterruptController
{
public:
	// accessing
	inline static Mx1InterruptController *getCurrentInterruptController();

	// testing
	inline Bool isPending(UInt interruptNumber) const;

	// modifying
	inline void setToIrq(UInt interruptNumber);
	inline void setToFiq(UInt interruptNumber);
	inline void enable(UInt interruptNumber);
	inline void disable(UInt interruptNumber);

private:
	// constructor
	inline Mx1InterruptController();

	// registers
	struct Mx1InterruptControllerRegisters
	{
		UInt intcntl;
		UInt nimask;
		UInt intennum;
		UInt intdisnum;
		UInt intenableh;
		UInt intenablel;
		UInt inttypeh;
		UInt inttypel;
		UInt nipriority7;
		UInt nipriority6;
		UInt nipriority5;
		UInt nipriority4;
		UInt nipriority3;
		UInt nipriority2;
		UInt nipriority1;
		UInt nipriority0;
		UInt nivecsr;
		UInt fivecsr;
		UInt intsrch;
		UInt intsrcl;
		UInt intfrch;
		UInt intfrcl;
		UInt nipndh;
		UInt nipndl;
		UInt fipndh;
		UInt fipndl;
	};

	// interrupt controller register accessing
	inline const volatile Mx1InterruptControllerRegisters *getInterruptControllerRegisters() const;
	inline volatile Mx1InterruptControllerRegisters *getInterruptControllerRegisters();

	// singleton
	static Mx1InterruptController currentInterruptController;
};

#include "../Multitasking/UninterruptableSection.h"

//------------------------------------------------------------------------------------------------
// * Mx1InterruptController::Mx1InterruptController
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline Mx1InterruptController::Mx1InterruptController()
{
	getInterruptControllerRegisters()->nimask = 0;
	getInterruptControllerRegisters()->nipriority7 = ~0;
	getInterruptControllerRegisters()->nipriority6 = ~0;
	getInterruptControllerRegisters()->nipriority5 = ~0;
	getInterruptControllerRegisters()->nipriority4 = ~0;
	getInterruptControllerRegisters()->nipriority3 = ~0;
	getInterruptControllerRegisters()->nipriority2 = ~0;
	getInterruptControllerRegisters()->nipriority1 = ~0;
	getInterruptControllerRegisters()->nipriority0 = ~0;
}

//------------------------------------------------------------------------------------------------
// * Mx1InterruptController::getCurrentMx1InterruptController
//
// Returns the singleton instance.
//------------------------------------------------------------------------------------------------

inline Mx1InterruptController *Mx1InterruptController::getCurrentInterruptController()
{
	return &currentInterruptController;
}

//------------------------------------------------------------------------------------------------
// * Mx1InterruptController::isPending
//
// Tests whether a particular interrupt is pending.
//------------------------------------------------------------------------------------------------

inline Bool Mx1InterruptController::isPending(UInt interruptNumber) const
{
	return ((*(&getInterruptControllerRegisters()->intsrcl - (interruptNumber >> 5))
		>> (interruptNumber & 0x1F)) & 1) != 0;
}

//------------------------------------------------------------------------------------------------
// * Mx1InterruptController::enable
//
// Enables a particular interrupt.
//------------------------------------------------------------------------------------------------

inline void Mx1InterruptController::enable(UInt interruptNumber)
{
	getInterruptControllerRegisters()->intennum = interruptNumber;
}

//------------------------------------------------------------------------------------------------
// * Mx1InterruptController::disable
//
// Disables a particular interrupt.
//------------------------------------------------------------------------------------------------

inline void Mx1InterruptController::disable(UInt interruptNumber)
{
	getInterruptControllerRegisters()->intdisnum = interruptNumber;
}

//------------------------------------------------------------------------------------------------
// * Mx1InterruptController::setToIrq
//
// Sets the level of a particular interrupt to IRQ.
//------------------------------------------------------------------------------------------------

inline void Mx1InterruptController::setToIrq(UInt interruptNumber)
{
	// debug
	getInterruptControllerRegisters()->nimask = 0;
	getInterruptControllerRegisters()->nipriority7 = ~0;
	getInterruptControllerRegisters()->nipriority6 = ~0;
	getInterruptControllerRegisters()->nipriority5 = ~0;
	getInterruptControllerRegisters()->nipriority4 = ~0;
	getInterruptControllerRegisters()->nipriority3 = ~0;
	getInterruptControllerRegisters()->nipriority2 = ~0;
	getInterruptControllerRegisters()->nipriority1 = ~0;
	getInterruptControllerRegisters()->nipriority0 = ~0;

	UninterruptableSection criticalSection;
	*(&getInterruptControllerRegisters()->inttypel - (interruptNumber >> 5))
		&= ~(1 << (interruptNumber & 0x1F));
}

//------------------------------------------------------------------------------------------------
// * Mx1InterruptController::setToFiq
//
// Sets the level of a particular interrupt to FIQ.
//------------------------------------------------------------------------------------------------

inline void Mx1InterruptController::setToFiq(UInt interruptNumber)
{
	UninterruptableSection criticalSection;
	*(&getInterruptControllerRegisters()->inttypel - (interruptNumber >> 5))
		|= (1 << (interruptNumber & 0x1F));
}

//------------------------------------------------------------------------------------------------
// * Mx1InterruptController::getInterruptControllerRegisters
//
// Returns the interrupt controller registers.
//------------------------------------------------------------------------------------------------

inline const volatile Mx1InterruptController::Mx1InterruptControllerRegisters *Mx1InterruptController::getInterruptControllerRegisters() const
{
	return (Mx1InterruptControllerRegisters *)(mx1RegistersBase + 0x23000);
}

//------------------------------------------------------------------------------------------------
// * Mx1Timer::getInterruptControllerRegisters
//
// Returns the interrupt controller registers.
//------------------------------------------------------------------------------------------------

inline volatile Mx1InterruptController::Mx1InterruptControllerRegisters *Mx1InterruptController::getInterruptControllerRegisters()
{
	return (Mx1InterruptControllerRegisters *)(mx1RegistersBase + 0x23000);
}

#endif // _Mx1InterruptController_h_
