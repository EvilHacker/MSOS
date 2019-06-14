#include "Sa1110Timer.h"
#include "../Multitasking/TaskScheduler.h"
#include "Sa1110InterruptController.h"

//------------------------------------------------------------------------------------------------
// * Sa1110Timer::Sa1110Timer
//
// Constructor.
//------------------------------------------------------------------------------------------------

Sa1110Timer::Sa1110Timer()
{
	// add interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->addInterruptHandler(this);

	// initialize timer registers (disable matching and clear interrupts)
	getTimerRegisters()->interruptEnableRegister = 0x0;
	getTimerRegisters()->statusRegister = 0xf;

	// enable match register 0 interrupt on IRQ
	Sa1110InterruptController::getCurrentInterruptController()->setToIrq(26);
	Sa1110InterruptController::getCurrentInterruptController()->enable(26);
}

//------------------------------------------------------------------------------------------------
// * Sa1110Timer::~Sa1110Timer
//
// Destructor.
//------------------------------------------------------------------------------------------------

Sa1110Timer::~Sa1110Timer()
{
	// disable match register 0 interrupt
	Sa1110InterruptController::getCurrentInterruptController()->disable(26);

	// clean up timer registers (disable matching and clear interrupts)
	getTimerRegisters()->interruptEnableRegister = 0x0;
	getTimerRegisters()->statusRegister = 0xf;

	// remove interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->removeInterruptHandler(this);
}

//------------------------------------------------------------------------------------------------
// * Sa1110Timer::updateIntervals
//
// Handles a change in the list of intervals.
// Schedules an interrupt for the first time interval to expire.
//------------------------------------------------------------------------------------------------

void Sa1110Timer::updateIntervals()
{
	// check if there are any time intervals
	const TimeInterval *pFirstInterval = getFirstInterval();
	if(pFirstInterval == null)
	{
		// there are no time intervals, disable matching
		getTimerRegisters()->interruptEnableRegister = 0x0;
	}
	else
	{
		// match for the expiry time of the first interval
		getTimerRegisters()->matchRegisters[0] = pFirstInterval->getExpiryTime();
		getTimerRegisters()->interruptEnableRegister = 0x1;
	}
}

//------------------------------------------------------------------------------------------------
// * Sa1110Timer::handleInterrupt
//
// Handles interrupts.
//------------------------------------------------------------------------------------------------

Bool Sa1110Timer::handleInterrupt()
{
	// determine if this is a timer interrupt
	if(Sa1110InterruptController::getCurrentInterruptController()->isPending(26))
	{
		// acknowledge interrupt
		getTimerRegisters()->statusRegister = 0xf;

		// handle the change in time
		tick();
		return true;
	}

	// not a timer interrupt
	return false;
}
