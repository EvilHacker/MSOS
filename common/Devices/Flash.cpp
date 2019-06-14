#include "Flash.h"

//------------------------------------------------------------------------------------------------
// * Flash::~Flash
//
// Destructor.
//------------------------------------------------------------------------------------------------

Flash::~Flash()
{
}

//------------------------------------------------------------------------------------------------
// * Flash static variables
//------------------------------------------------------------------------------------------------

// Gemini board with 16-bit Atmel flash
#include "AtmelFlash.h"
#include "deviceAddresses.h"
static AtmelFlash<UInt16> flash((void *)flashBase);

// Gemini board with 16-bit Intel flash
//#include "IntelFlash.h"
//static IntelFlash<UInt16> flash;

// SA-1110 prototype board with 32-bit Intel flash (two 16-bit parts in parallel)
//#include "IntelFlash.h"
//static IntelFlash<UInt32> flash;

Flash *Flash::pCurrentFlash = &flash;
