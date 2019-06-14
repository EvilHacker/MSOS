#ifndef _Crc16Calculator_h_
#define _Crc16Calculator_h_

#include "../cPrimitiveTypes.h"
#include "CrcCalculator.h"

typedef CrcCalculator<UInt16, 0x1021, maximumOfIntegerType(UInt16), 4> Crc16Calculator;
extern Crc16Calculator crc16Calculator;

#endif // _Crc16Calculator_h_
