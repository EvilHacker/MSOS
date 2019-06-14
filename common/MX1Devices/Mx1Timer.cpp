#include "Mx1Timer.h"
#include "Mx1InterruptController.h"
#include "deviceAddresses.h"
#include "../Multitasking/TaskScheduler.h"

//------------------------------------------------------------------------------------------------
// * Mx1Timer::Mx1Timer
//
// Constructor.
//------------------------------------------------------------------------------------------------

Mx1Timer::Mx1Timer(
	TimerNumber timerNumber,
	ClockSource clockSource,
	UInt divider,
	TimeValue frequency) :
		frequency(frequency)
{
	// timer1 or timer2 specific initialization
	switch(timerNumber)
	{
		default:
		case timer1:
		{
			pTimerRegisters = (Mx1TimerRegisters *)(mx1RegistersBase + 0x2000);
			interruptNumber = 59;
			break;
		}
		case timer2:
		{
			pTimerRegisters = (Mx1TimerRegisters *)(mx1RegistersBase + 0x3000);
			interruptNumber = 58;

			// initialize TMR2OUT pin
			{
				UninterruptableSection criticalSection;
				*(volatile UInt *)(mx1RegistersBase + 0x1C320) &= ~(1u << 31);
				*(volatile UInt *)(mx1RegistersBase + 0x1C338) &= ~(1u << 31);
			}
			break;
		}
	}

	// initialize TIN pin
	if(clockSource == tin)
	{
		UninterruptableSection criticalSection;
		*(volatile UInt *)(mx1RegistersBase + 0x1C020) &= ~(1u << 1);
		*(volatile UInt *)(mx1RegistersBase + 0x1C038) &= ~(1u << 1);
	}

	// initialize timer registers
	getTimerRegisters()->tprer = divider - 1;
	getTimerRegisters()->tctl = 0x00000101 | (clockSource << 1);

	// add interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->addInterruptHandler(this);

	// enable interrupt on IRQ
	Mx1InterruptController::getCurrentInterruptController()->setToIrq(interruptNumber);
	Mx1InterruptController::getCurrentInterruptController()->enable(interruptNumber);
}

//------------------------------------------------------------------------------------------------
// * Mx1Timer::~Mx1Timer
//
// Destructor.
//------------------------------------------------------------------------------------------------

Mx1Timer::~Mx1Timer()
{
	// disable interrupt
	Mx1InterruptController::getCurrentInterruptController()->disable(interruptNumber);

	// clean up timer registers
	getTimerRegisters()->tctl = 0x00000100;

	// remove interrupt handler
	TaskScheduler::getCurrentTaskScheduler()->removeInterruptHandler(this);
}

//------------------------------------------------------------------------------------------------
// * Mx1Timer::updateIntervals
//
// Handles a change in the list of intervals.
// Schedules an interrupt for the first time interval to expire.
//------------------------------------------------------------------------------------------------

void Mx1Timer::updateIntervals()
{
	// check if there are any time intervals
	const TimeInterval *pFirstInterval = getFirstInterval();
	if(pFirstInterval == null)
	{
		// there are no time intervals, disable matching
		getTimerRegisters()->tctl &= ~0x10;
	}
	else
	{
		// match for the expiry time of the first interval
		getTimerRegisters()->tcmp = pFirstInterval->getExpiryTime();
		getTimerRegisters()->tctl |= 0x10;
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1Timer::handleInterrupt
//
// Handles interrupts.
//------------------------------------------------------------------------------------------------

Bool Mx1Timer::handleInterrupt()
{
	// determine if this is a timer interrupt
	if((getTimerRegisters()->tstat & 0x1) != 0)
	{
		// acknowledge interrupt
		getTimerRegisters()->tstat = 0;

		// handle the change in time
		tick();

		return true;
	}

	// not a timer interrupt
	return false;
}
