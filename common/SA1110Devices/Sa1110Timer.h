#ifndef _Sa1110Timer_h_
#define _Sa1110Timer_h_

#include "../Multitasking/Timer.h"
#include "../Multitasking/InterruptHandler.h"

//------------------------------------------------------------------------------------------------
// * class Sa1110Timer
//
// StrongARM-1110 OS timer.
//------------------------------------------------------------------------------------------------

class Sa1110Timer : public Timer, private InterruptHandler
{
public:
	// constructor and destructor
	Sa1110Timer();
	~Sa1110Timer();

	// querying
	inline TimeValue getFrequency() const;
	inline TimeValue convertSeconds(UInt seconds) const;
	inline TimeValue convertMilliseconds(UInt milliseconds) const;
	inline TimeValue convertMicroseconds(UInt milliseconds) const;
	inline TimeValue getTime() const;

private:
	// interval change handling
	void updateIntervals();

	// interrupt handling
	Bool handleInterrupt();

	// types
	struct Sa1110TimerRegisters
	{
		UInt matchRegisters[4];
		volatile UInt counterRegister;
		volatile UInt statusRegister;
		UInt watchdogEnableRegister;
		UInt interruptEnableRegister;
	};

	// timer register accessing
	inline const Sa1110TimerRegisters *getTimerRegisters() const;
	inline Sa1110TimerRegisters *getTimerRegisters();
};

#include "Sa1110DeviceAddresses.h"

//------------------------------------------------------------------------------------------------
// * Sa1110Timer::getFrequency
//
// Returns the number of ticks per second made by this timer.
//------------------------------------------------------------------------------------------------

inline TimeValue Sa1110Timer::getFrequency() const
{
	return 3686400;
}

//------------------------------------------------------------------------------------------------
// * Sa1110Timer::convertSeconds
//
// Converts <seconds> into clock ticks.
//------------------------------------------------------------------------------------------------

inline TimeValue Sa1110Timer::convertSeconds(UInt seconds) const
{
	return 3686400 * seconds;
}

//------------------------------------------------------------------------------------------------
// * Sa1110Timer::convertMilliseconds
//
// Converts <milliseconds> into clock ticks.
//------------------------------------------------------------------------------------------------

inline TimeValue Sa1110Timer::convertMilliseconds(UInt milliseconds) const
{
	return 3686400 / 1000 * milliseconds;
}

//------------------------------------------------------------------------------------------------
// * Sa1110Timer::convertMicroseconds
//
// Converts <microseconds> into clock ticks.
//------------------------------------------------------------------------------------------------

inline TimeValue Sa1110Timer::convertMicroseconds(UInt microseconds) const
{
	return microseconds * (3686400 >> 6) / (1000000 >> 6);
}

//------------------------------------------------------------------------------------------------
// * Sa1110Timer::getTime
//
// Returns the current time.
//------------------------------------------------------------------------------------------------

inline TimeValue Sa1110Timer::getTime() const
{
	// read timer counter register
	return getTimerRegisters()->counterRegister;
}

//------------------------------------------------------------------------------------------------
// * Sa1110Timer::getTimerRegisters
//
// Returns the timer registers.
//------------------------------------------------------------------------------------------------

inline const Sa1110Timer::Sa1110TimerRegisters *Sa1110Timer::getTimerRegisters() const
{
	return (const Sa1110TimerRegisters *)sa1110SystemControlBase;
}

//------------------------------------------------------------------------------------------------
// * Sa1110Timer::getTimerRegisters
//
// Returns the timer registers.
//------------------------------------------------------------------------------------------------

inline Sa1110Timer::Sa1110TimerRegisters *Sa1110Timer::getTimerRegisters()
{
	return (Sa1110TimerRegisters *)sa1110SystemControlBase;
}

#endif // _Sa1110Timer_h_
