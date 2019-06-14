#ifndef _IntelFlash_h_
#define _IntelFlash_h_

#include "../cPrimitiveTypes.h"
#include "CommonFlash.h"
#include "../multitasking/LockedSection.h"

//------------------------------------------------------------------------------------------------
// * class IntelFlash
//
// Provides an interface to Flash memory.
//------------------------------------------------------------------------------------------------

template<class FlashWord>
class IntelFlash : public CommonFlash<FlashWord>
{
public:
	// flash operations
	void erase(void *address);
		
private:
	// flash operations
	void writeWord(void *destination, FlashWord value);

	// commands
	static const FlashWord programCommand = 0x00100010 & maximumOfIntegerType(FlashWord);
	static const FlashWord eraseCommand = 0x00200020 & maximumOfIntegerType(FlashWord);
	static const FlashWord verifyCommand = 0x00D000D0 & maximumOfIntegerType(FlashWord);
	static const FlashWord readCommand = 0x00FF00FF & maximumOfIntegerType(FlashWord);

	// masks
	static const FlashWord readyStatusMask = 0x00800080 & maximumOfIntegerType(FlashWord);
};

//------------------------------------------------------------------------------------------------
// * IntelFlash::write
//
// Programs one word in flash.
//------------------------------------------------------------------------------------------------

template<class FlashWord>
void IntelFlash<FlashWord>::writeWord(void *destination, FlashWord value)
{
	LockedSection flashLock(flashMutex);

	volatile FlashWord *pFlash = (volatile FlashWord *)destination;

	// issue single word program command
	*pFlash = programCommand;
	
	// write value
	*pFlash = value;
	
	// wait for operation to complete
	while((*pFlash & readyStatusMask) != readyStatusMask);

	// put flash in read mode
	*pFlash = readCommand;
}

//------------------------------------------------------------------------------------------------
// * IntelFlash::erase
//
// Erases the entire flash block containing the specified <destination> address.
//------------------------------------------------------------------------------------------------

template<class FlashWord>
void IntelFlash<FlashWord>::erase(void *destination)
{
	LockedSection flashLock(flashMutex);

	volatile FlashWord *pFlash = (volatile FlashWord *)destination;
	
	// issue single block erase command
	*pFlash = eraseCommand;
	
	// confirm command
	*pFlash = verifyCommand;
	
	// wait for operation to complete
	while((*pFlash & readyStatusMask) != readyStatusMask);

	// put flash in read mode
	*pFlash = readCommand;
}

#endif // _Flash_h_
