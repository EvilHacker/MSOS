#ifndef _AtmelFlash_h_
#define _AtmelFlash_h_

#include "CommonFlash.h"
#include "../multitasking/LockedSection.h"

//------------------------------------------------------------------------------------------------
// * class AtmelFlash
//
// Provides an interface to Flash memory.
//------------------------------------------------------------------------------------------------

template<class FlashWord>
class AtmelFlash : public CommonFlash<FlashWord>
{
public:
	// constructor
	AtmelFlash(void *pFlashBase);
	
	// flash operations
	void erase(void *destination);
		
private:
	// flash operations
	void writeWord(void *destination, FlashWord value);

	// representation
	volatile FlashWord *pFlashBase;
};

//------------------------------------------------------------------------------------------------
// * AtmelFlash::AtmelFlash
//
// Constructor.
//------------------------------------------------------------------------------------------------

template<class FlashWord>
AtmelFlash<FlashWord>::AtmelFlash(void *pFlashBase)
{
	this->pFlashBase = (FlashWord *)pFlashBase;
}

//------------------------------------------------------------------------------------------------
// * AtmelFlash::write
//
// Programs one word in flash.
//------------------------------------------------------------------------------------------------

template<class FlashWord>
void AtmelFlash<FlashWord>::writeWord(void *destination, FlashWord value)
{
	LockedSection flashLock(flashMutex);

	volatile FlashWord *pFlash = (volatile FlashWord *)destination;

	pFlashBase[0x5555] = 0xAA;
	pFlashBase[0x2AAA] = 0x55;
	pFlashBase[0x5555] = 0xA0;
	*pFlash = value;

	while(*pFlash != value);
}

//------------------------------------------------------------------------------------------------
// * AtmelFlash::erase
//
// Erases the entire flash block containing the specified <destination> address.
//------------------------------------------------------------------------------------------------

template<class FlashWord>
void AtmelFlash<FlashWord>::erase(void *destination)
{
	LockedSection flashLock(flashMutex);

	volatile FlashWord *pFlash = (volatile FlashWord *)destination;

	pFlashBase[0x5555] = 0xAA;
	pFlashBase[0x2AAA] = 0x55;
	pFlashBase[0x5555] = 0x80;

	pFlashBase[0x5555] = 0xAA;
	pFlashBase[0x2AAA] = 0x55;
	*pFlash = 0x30;

	while(*pFlash != maximumOfIntegerType(FlashWord));
}

#endif // _Flash_h_
