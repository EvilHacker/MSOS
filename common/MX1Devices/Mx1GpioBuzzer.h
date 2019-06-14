#ifndef _Mx1GpioBuzzer_h_
#define _Mx1GpioBuzzer_h_

#include "../cPrimitiveTypes.h"
#include "../multitasking/TaskScheduler.h"
#include "Mx1GpioPin.h"

//------------------------------------------------------------------------------------------------
// * class Mx1GpioBuzzer
//
// Used to Generates tones via a speaker connected to a GPIO pin.
//------------------------------------------------------------------------------------------------

class Mx1GpioBuzzer
{
public:
	// constructor and destructor
	Mx1GpioBuzzer(
		Mx1GpioPin::Port portNumber,
		UInt pinNumber);
	~Mx1GpioBuzzer();

	// querying
	inline Timer *getReferenceTimer() const;
	inline TimeValue getReferenceFrequency() const;

	// behaviour
	void generateTone(UInt duration, UInt period);

private:
	// representation
	Mx1GpioPin outputPin;

	// pin values
	enum
	{
		lowValue = 0,
		highValue = 1 - lowValue
	};
};

//------------------------------------------------------------------------------------------------
// * Mx1GpioBuzzer::getReferenceTimer
//
// Returns the reference clock frequency from which all generated frequencies are derived from.
//------------------------------------------------------------------------------------------------

inline Timer *Mx1GpioBuzzer::getReferenceTimer() const
{
	return TaskScheduler::getCurrentTaskScheduler()->getTimer();
}

//------------------------------------------------------------------------------------------------
// * Mx1GpioBuzzer::getReferenceFrequency
//
// Returns the reference clock frequency from which all generated frequencies are derived from.
//------------------------------------------------------------------------------------------------

inline TimeValue Mx1GpioBuzzer::getReferenceFrequency() const
{
	return TaskScheduler::getCurrentTaskScheduler()->getTimer()->getFrequency();
}

#endif // _Mx1GpioBuzzer_h_
