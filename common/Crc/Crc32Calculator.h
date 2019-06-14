#ifndef _Crc32Calculator_h_
#define _Crc32Calculator_h_

#include "../cPrimitiveTypes.h"
#include "CrcCalculator.h"

typedef CrcCalculator<UInt32, 0x04C11DB7, maximumOfIntegerType(UInt32), 8> Crc32Calculator;
extern Crc32Calculator crc32Calculator;

#endif // _Crc32Calculator_h_
