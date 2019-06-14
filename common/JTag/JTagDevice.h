// JTagDevice.h: interface for the JTagDevice class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _JTagDevice_h_
#define _JTagDevice_h_


#ifdef WIN32
#include "..\Win32Primitives\Primitives_w32.h"
#else
#include "cPrimitiveTypes.h"
#include "LinkedList.h"
#endif

static UInt8 zero = 0;
class JTagDevice : public Link  
{
public:
	JTagDevice() {};
	virtual ~JTagDevice() {};

public:

	inline virtual UInt8 * getBypassData() { return &zero; };
	inline virtual UInt getBypassDataLength() const { return 1; };
	inline virtual UInt getBypassInstruction() const { return (UInt)~0; };

	inline void setDataRegisterLength(const UInt bits);
	inline UInt getDataRegisterLength();

	inline virtual UInt getIdcodeInstruction();						// read Id instruction
	inline virtual UInt getIdcodeDataLength();
	inline virtual UInt getBSCodeInstruction();
	inline virtual UInt getExtestCodeInstruction();

	inline virtual UInt getBSDataLength();
	inline virtual UInt8 * getBSData();

	inline void setAndShiftReadData(int tdo);
	inline int getAndShiftWriteData();

	inline const UInt8 * getDataRegister();
	inline void initializeBufferIndexes();

	// device specific overrides
	virtual UInt getMaximumDataRegisterLength() const = 0;
	virtual UInt getInstructionRegisterLength() const = 0;

	virtual Bool compareDeviceId();

	void setDataRegister(void * data, Bool isBytes = false);

protected:

	UInt dataRegisterLength;

	UInt indexRead;
	UInt indexWrite;

	UInt8 pDataBitsRead[300];
	UInt8 pDataBitsWrite[300];
};

/////////////////////////////////////////////////
//

inline UInt JTagDevice::getDataRegisterLength() 
{ 
	return dataRegisterLength; 
}

/////////////////////////////////////////////////
//

inline void JTagDevice::setDataRegisterLength(const UInt bits)
{ 
	dataRegisterLength = bits; 
}

/////////////////////////////////////////////////
//

inline const UInt8 * JTagDevice::getDataRegister()
{
	return pDataBitsRead;
}

/////////////////////////////////////////////////
//

inline UInt JTagDevice::getIdcodeInstruction()
{
	return getBypassInstruction();
}

/////////////////////////////////////////////////
//

inline UInt JTagDevice::getIdcodeDataLength()
{
	return getBypassDataLength();
}

/////////////////////////////////////////////////
//

inline UInt JTagDevice::getBSCodeInstruction()
{
	return getBypassInstruction();
}

/////////////////////////////////////////////////
//

inline UInt JTagDevice::getBSDataLength()
{
	return getBypassDataLength();
}

/////////////////////////////////////////////////
//

inline UInt8 * JTagDevice::getBSData()
{
	return getBypassData();
}

/////////////////////////////////////////////////
//

inline UInt JTagDevice::getExtestCodeInstruction()
{
	return getBypassInstruction();
}

/////////////////////////////////////////////////
//

inline void JTagDevice::initializeBufferIndexes() 
{ 
	indexRead = 0; 
}

/////////////////////////////////////////////////
//

inline int JTagDevice::getAndShiftWriteData()
{
//	UInt8 outBit = 0;
//	int bytes = (dataRegisterLength + 7) / 8;
//	for (int i = bytes - 1; i >= 0; i--)
//	{
//		UInt8 b = pDataBitsWrite[i]&1;
//		pDataBitsWrite[i] >>= 1;
//		pDataBitsWrite[i] |= (outBit << 7);
//		outBit = b;
//	}
//
//	return outBit;

	return pDataBitsWrite[indexWrite++];
}

/////////////////////////////////////////////////
//

inline void JTagDevice::setAndShiftReadData(int tdo)
{
//	UInt8 inBit = tdo;
//	UInt bytes = (dataRegisterLength + 7) / 8;
//	for (int i = bytes - 1; i >= 0; i--)
//	{
//		UInt8 b = pDataBitsRead[i]&1;
//		pDataBitsRead[i] >>= 1;
//		pDataBitsRead[i] |= (inBit << 7);
//		inBit = b;
//	}

	pDataBitsRead[indexRead++] = (UInt8)tdo;
}

#endif // !defined(_JTagDevice_h_)
