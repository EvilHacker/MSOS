#ifndef _Mx1Timer_h_
#define _Mx1Timer_h_

#include "../Multitasking/Timer.h"
#include "../Multitasking/InterruptHandler.h"

//------------------------------------------------------------------------------------------------
// * class Mx1Timer
//
// MX1 general purpose timer.
//------------------------------------------------------------------------------------------------

class Mx1Timer : public Timer, private InterruptHandler
{
public:
	// constructor and destructor
	enum TimerNumber
	{
		timer1,
		timer2
	};
	enum ClockSource
	{
		perclk1 = 1,
		perclk1Div16 = 2,
		clk32 = 8,
		tin = 3
	};
	Mx1Timer(
		TimerNumber timerNumber,
		ClockSource clockSource,
		UInt divider,
		TimeValue frequency);
	~Mx1Timer();

	// querying
	inline TimeValue getFrequency() const;
	inline TimeValue convertSeconds(UInt seconds) const;
	inline TimeValue convertMilliseconds(UInt milliseconds) const;
	inline TimeValue convertMicroseconds(UInt microseconds) const;
	inline TimeValue getTime() const;

private:
	// interval change handling
	void updateIntervals();

	// interrupt handling
	Bool handleInterrupt();

	// types
	struct Mx1TimerRegisters
	{
		UInt tctl;
		UInt tprer;
		UInt tcmp;
		UInt tcr;
		UInt tcn;
		UInt tstat;
	};

	// timer register accessing
	inline const volatile Mx1TimerRegisters *getTimerRegisters() const;
	inline volatile Mx1TimerRegisters *getTimerRegisters();

	// representation
	volatile Mx1TimerRegisters *pTimerRegisters;
	UInt interruptNumber;
	TimeValue frequency;
};

//------------------------------------------------------------------------------------------------
// * Mx1Timer::getFrequency
//
// Returns the number of ticks per second made by this timer.
//------------------------------------------------------------------------------------------------

inline TimeValue Mx1Timer::getFrequency() const
{
	return frequency;
}

//------------------------------------------------------------------------------------------------
// * Mx1Timer::convertSeconds
//
// Converts <seconds> into clock ticks.
//------------------------------------------------------------------------------------------------

inline TimeValue Mx1Timer::convertSeconds(UInt seconds) const
{
	return frequency * seconds;
}

//------------------------------------------------------------------------------------------------
// * Mx1Timer::convertMilliseconds
//
// Converts <milliseconds> into clock ticks.
//------------------------------------------------------------------------------------------------

inline TimeValue Mx1Timer::convertMilliseconds(UInt milliseconds) const
{
	return frequency / 1000 * milliseconds;
}

//------------------------------------------------------------------------------------------------
// * Mx1Timer::convertMicroseconds
//
// Converts <microseconds> into clock ticks.
//------------------------------------------------------------------------------------------------

inline TimeValue Mx1Timer::convertMicroseconds(UInt microseconds) const
{
	return frequency / 1000000 * microseconds;
}

//------------------------------------------------------------------------------------------------
// * Mx1Timer::getTime
//
// Returns the current time.
//------------------------------------------------------------------------------------------------

inline TimeValue Mx1Timer::getTime() const
{
	// read timer counter register
	return getTimerRegisters()->tcn;
}

//------------------------------------------------------------------------------------------------
// * Mx1Timer::getTimerRegisters
//
// Returns the timer registers.
//------------------------------------------------------------------------------------------------

inline const volatile Mx1Timer::Mx1TimerRegisters *Mx1Timer::getTimerRegisters() const
{
	return pTimerRegisters;
}

//------------------------------------------------------------------------------------------------
// * Mx1Timer::getTimerRegisters
//
// Returns the timer registers.
//------------------------------------------------------------------------------------------------

inline volatile Mx1Timer::Mx1TimerRegisters *Mx1Timer::getTimerRegisters()
{
	return pTimerRegisters;
}

#endif // _Mx1Timer_h_
