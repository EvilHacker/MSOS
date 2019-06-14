// JTagSA1110.cpp: implementation of the JTagSA1110 class.
//
//////////////////////////////////////////////////////////////////////

#include "JTagSA1110.h"
#include "JTagChain.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

JTagSA1110::JTagSA1110(JTagChain * pChain) : parentChain(pChain)
{
}

Bool JTagSA1110::compareDeviceId()
{
//	for (UInt i = 0; i < sizeof(id); i++)
//	{
//		((UInt8 *)&id)[i] = ((UInt8 *)getDataRegister())[i];
//	}

	UInt32 id = 0;
	for (int i = 0; i < 32; i++)
	{
		id |= ((UInt32)(getDataRegister()[i]) << i);
	}

	if ((id << 4) == (getDeviceId() << 4))
	{
		return true;
	}

	return false;
}

void JTagSA1110::writeFlashWord(UInt32 address, UInt16 value)
{
	doIoPins(ioSetup, address, value, false);
	doIoPins(ioWrite, address, value, false);
//	doIoPins(ioHold, address, value, false);
}

UInt16 JTagSA1110::readWord(UInt32 address)
{
	return (UInt16)doIoPins(ioRead, address, 0, true);
}

void JTagSA1110::writeWord(UInt address, UInt16 value)
{
	writeFlashWord(0x5555 * 2, 0xAA);
	writeFlashWord(0x2AAA * 2, 0x55);
	writeFlashWord(0x5555 * 2, 0xA0);
	writeFlashWord(address, value);

	// polling
	readWord(address);
	while (readWord(address) != value);
}

void JTagSA1110::eraseFlushSector(UInt32 address)
{
	// magic numbers to erase Atmel flush
	writeFlashWord(0x5555 * 2, 0xAA); 
	writeFlashWord(0x2AAA * 2, 0x55); 
	writeFlashWord(0x5555 * 2, 0x80); 
	writeFlashWord(0x5555 * 2, 0xAA); 
	writeFlashWord(0x2AAA * 2, 0x55); 
	writeFlashWord(address, 0x30);

	// polling
	readWord(address);
	while (readWord(address) != 0xFFFF);
}

UInt32 JTagSA1110::doIoPins(IoType type, UInt32 address, UInt32 value, Bool isReadBack)
{
	// Preset SA-1110 pins to default values
	pinState[nCs0Out] = 1;
	pinState[nCs1Out] = 1;
	pinState[nCs2Out] = 1;
	pinState[nCs3Out] = 1;
	pinState[nCs4Out] = 1;
	pinState[nCs5Out] = 1;

	pinState[nWeOut] = 1;
	pinState[nOeOut] = 0;
	pinState[rdnWrOut] = 0;

	pinState[d310En] = 1;

	for (int i = 0; i < 26; i++)
	{
		pinState[i + 28] = (char)((address >> i) & 1u);	// set address 0 thru 25
	}

	if (type == ioRead)
	{
		pinState[rdnWrOut] = 1;
		switch (address >> 27)
		{
		case 0:	{ pinState[nCs0Out] = 0; break; }
		case 1: { pinState[nCs1Out] = 0; break; }
		case 2: { pinState[nCs2Out] = 0; break; }
		case 3: { pinState[nCs3Out] = 0; break; }
		case 4: { pinState[nCs4Out] = 0; break; }
		case 5: { pinState[nCs5Out] = 0; break; }
		}
	}

	if (type == ioWrite)
	{
		pinState[nWeOut] = 0;
		pinState[nOeOut] = 1;
		pinState[d310En] = 0;	// switch data pins to drive
		switch (address >> 27)	// set CS pin for corresponding address
		{
		case 0: { pinState[nCs0Out] = 0; break; }
		case 1: { pinState[nCs1Out] = 0; break; }
		case 2: { pinState[nCs2Out] = 0; break; }
		case 3: { pinState[nCs3Out] = 0; break; }
		case 4: { pinState[nCs4Out] = 0; break; }
		case 5: { pinState[nCs5Out] = 0; break; }
		}

		for (int i = 0; i < 32; i++)
		{
			pinState[getInputPin(i)] = (char)((value >> i) & 1u);	// set data pins
		}
	}
	
	if (type == ioSetup || type == ioHold)	// just like a write except WE, WE needs setup time
	{
		pinState[nOeOut] = 1;
		pinState[d310En] = 0;

		for (int i = 0; i < 32; i++)
		{
			pinState[getInputPin(i)] = (char)((value >> i) & 1u);	// set data pins
		}
	}

	parentChain->doBScan(isReadBack);

	UInt32 pinsOut = 0;
	for(int j = 0; j < 32; j++)	// convert serial data to single unsigned short
	{
		pinsOut |= (UInt32)((getDataRegister()[getOutputPin(j)]) << j);
	}

	parentChain->doExtest();
	return pinsOut;
}