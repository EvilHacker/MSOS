
#include "JTagChainGemini.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

JTagChainGemini::JTagChainGemini() :
	pFlash(Flash::getCurrentFlash()),
	tckPin(8), tmsPin(7), tdiPin(6), tdoPin(9)
{
	rewindDataSource();
	tckPin.setValue(0);
	tmsPin.setValue(0);
	tdiPin.setValue(0);
	
	tckPin.configureAsOutput();
	tmsPin.configureAsOutput();
	tdiPin.configureAsOutput();
	
	tdoPin.configureAsInput();
}

JTagChainGemini::~JTagChainGemini()
{
	tckPin.configureAsInput();
	tmsPin.configureAsInput();
	tdiPin.configureAsInput();
}

void JTagChainGemini::rewindDataSource()
{
	flushPointer = 0;
}

UInt32 JTagChainGemini::getProgramSize()
{
	UInt32 maxBootBlockAddress = 0x00010000;
	UInt32 testDWord = maxUInt32;
	UInt32 backOffset = 0;

	while (backOffset <= (maxBootBlockAddress - sizeof(testDWord)))
	{
		backOffset += sizeof(testDWord);
		
		UInt32 runAddress = flashBase + maxBootBlockAddress - backOffset;
		pFlash->read(&testDWord, (void *)(runAddress), sizeof(testDWord));
		
		if (testDWord != maxUInt32)
		{
			return ((runAddress - flashBase) + sizeof(testDWord));
		}
	}
	
	return 0;
}

UInt16 JTagChainGemini::getFlushWord()
{
	UInt16 flushWord;
	pFlash->read(&flushWord, (void *)(flashBase + flushPointer), sizeof(flushWord));
	flushPointer += sizeof(flushWord);
	return flushWord;
}
