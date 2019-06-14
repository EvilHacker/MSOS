
#ifndef _JTagGemini_h_
#define _JTagGemini_h_

#include "../multitasking/Task.h"
#include "../multitasking/Sleep.h"
#include "../JTag/JTagSA1110.h"
#include "../../Sa1110Devices/Sa1110GpioOutput.h"
#include "../../Sa1110Devices/Sa1110GpioButton.h"

typedef Sa1110GpioOutput GeminiGpioOutput;
typedef Sa1110GpioButton GeminiGpioButton;

#include "JTagChainGemini.h"

//------------------------------------------------------------------------------------------------
// * class HeartbeatTask
//
// Flashes LEDs.
//------------------------------------------------------------------------------------------------

class HeartbeatTask : public Task
{
public:
	// constructor
	HeartbeatTask(UInt sleepTime);

	enum DeviceStatus
	{
		idle = 0,
		programming = 1
	};

	DeviceStatus status;
	
private:
	// task entry point
	void main();
	UInt sleepTime;
};

//------------------------------------------------------------------------------------------------
// * class TriggerTask
//
// Process trigger commands
//------------------------------------------------------------------------------------------------

class JTagGemini : public Task
{
public:
	// constructor
	JTagGemini();
	
private:

	// error control
	void notifyError(Bool isInError);
	
	// task entry point
	void main();
};

#endif // !defined(_JTagGemini_h_)
