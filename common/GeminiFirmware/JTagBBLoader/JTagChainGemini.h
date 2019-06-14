
#ifndef _JTagChainGemini_h_
#define _JTagChainGemini_h_

#include "..\JTag\JTagChain.h"
#include "../devices/Flash.h"
#include "../multitasking/Task.h"
#include "../devices/deviceAddresses.h"
#include "../../Sa1110Devices/Sa1110GpioPin.h"
#include "../../Sa1110Devices/Sa1110GpioOutput.h"

typedef Sa1110GpioPin GeminiGpioPin;
typedef Sa1110GpioOutput GeminiGpioOutput;

class JTagChainGemini : public JTagChain
{
public:
	JTagChainGemini();
	virtual ~JTagChainGemini();

	// overrides
	inline int doInOut(int tdi, int tms, Bool isReadPort);
	void rewindDataSource();

	UInt32 getProgramSize();
	UInt16 getFlushWord();

private:
	
	// wait in nanoseconds
	inline void sleep(int ns);

	UInt32 flushPointer;
	Flash * pFlash;
	
	GeminiGpioPin tckPin;
	GeminiGpioPin tmsPin;
	GeminiGpioPin	tdiPin;
	GeminiGpioPin tdoPin;
};

//------------------------------------------------------------------------------------------------
// * JTagChainGemini::doInOut
//
// Gemini GPIO io
//------------------------------------------------------------------------------------------------

inline int JTagChainGemini::doInOut(int tdi, int tms, Bool isReadPort)
{
//	GPIO 8 Pin 14 and TCK JTAG 3 Pin 
//	GPIO 7 Pin 12 and TMS JTAG 6 Pin
//	GPIO 6 Pin 10 and TDI JTAG 5 Pin 
//	GPIO 9 Pin 16 and TDI JTAG 8 Pin 

	tckPin.setValue(0);				// TCK low
	tmsPin.setValue(tms);			// set TMS
	tdiPin.setValue(tdi);			// set TDI
	
//	sleep(50);						// wait TCK low
	
	tckPin.setValue(1);				// TCK high
//	sleep(50);
	
	tckPin.setValue(0);				// TCK low
//	sleep(40);

	if(isReadPort)
	{
		return tdoPin.getValue();	// get TDO data
	}
	
	return -1;
}

//------------------------------------------------------------------------------------------------
// * JTagChainGemini::sleep
//
// Delay
//------------------------------------------------------------------------------------------------

inline void JTagChainGemini::sleep(int ns)
{
	asm 
	{
		nop
	}
}

#endif // !defined(_JTagChainGemini_h_)
