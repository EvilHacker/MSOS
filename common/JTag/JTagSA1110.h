// JTagSA1110.h: interface for the JTagSA1110 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _JTagSA1110_h_
#define _JTagSA1110_h_

#include "JTagDevice.h"
#include "JTagSA1110Const.h"

//static const char * sDeviceId = "0101 1001001001100001 00000001001 1";	// JTAG ID-codes for the SA-1110
static const UInt32 deviceId = 0x59261013;

class JTagChain;
class JTagSA1110 : public JTagDevice 
{
public:
	JTagSA1110(JTagChain * pChain);
	virtual ~JTagSA1110() {};

	inline UInt getInputPin(int index);
	inline UInt getOutputPin(int index);

	UInt getMaximumDataRegisterLength() const { return 292; }
	UInt getInstructionRegisterLength() const { return 5; }

	inline UInt getIdcodeInstruction();
	inline UInt getIdcodeDataLength();
	inline UInt getBSCodeInstruction();
	inline UInt getExtestCodeInstruction();

	inline UInt getBSDataLength();
	inline UInt8 * getBSData();

	Bool compareDeviceId();

	void eraseFlushSector(UInt32 address);
	void writeWord(UInt address, UInt16 value);

	UInt16 readWord(UInt32 address);

private:
	// parent chain
	JTagChain * parentChain;

	enum IoType
	{
		ioRead,
		ioWrite,
		ioSetup,
		ioHold
	};

	UInt32 doIoPins(IoType type, UInt32 address, UInt32 value, Bool isReadBack);
	void writeFlashWord(UInt32 address, UInt16 value);
	inline UInt32 getDeviceId();
};

/////////////////////////////////////////////////////////////
//

inline UInt JTagSA1110::getIdcodeDataLength() 
{ 
	return 32; 
}

/////////////////////////////////////////////////////////////
//

inline UInt JTagSA1110::getIdcodeInstruction()
{ 
	return 6; 
}

/////////////////////////////////////////////////////////////
//

inline UInt JTagSA1110::getBSCodeInstruction() 
{ 
	return 1;
}

/////////////////////////////////////////////////////////////
//

inline UInt JTagSA1110::getExtestCodeInstruction() 
{ 
	return 0; 
}

/////////////////////////////////////////////////////////////
//

inline UInt JTagSA1110::getBSDataLength()
{
	return arrayDimension(pinState);
}

/////////////////////////////////////////////////////////////
//

inline UInt8 * JTagSA1110::getBSData()
{
	return (UInt8 *)pinState;
}

/////////////////////////////////////////////////////////////
//

inline UInt JTagSA1110::getInputPin(int index)
{
	return (dataPins[index]);
}

/////////////////////////////////////////////////////////////
//

inline UInt JTagSA1110::getOutputPin(int index)
{
	return (dataPins[index] - 1);
}

/////////////////////////////////////////////////////////////
//

inline UInt32 JTagSA1110::getDeviceId()
{ 
	return deviceId; 
}

#endif // !defined(_JTagSA1110_h_)
