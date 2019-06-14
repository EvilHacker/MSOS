#ifndef _divideFraction_h_
#define _divideFraction_h_

#include "../cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * divideFraction
//
// Computes the value <numerator> / <denominator> where <numerator> < <denominator>.
// The result is expressed as a fixed-point integer with <numberOfBitsPrecision> decimal bits.
// The result is also rounded down.
// The return value will be between 0 and (1 << <numberOfBitsPrecision>) - 1.
// <numberOfBitsPrecision> must at least 1 and less than the number of bits in an <Integer> - 1.
//------------------------------------------------------------------------------------------------

template<class Integer>
Integer divideFraction(
	Integer numerator,
	Integer denominator,
	UInt numberOfBitsPrecision)
{
	Integer result = 0;

	do
	{
		result <<= 1;
		if((numerator <<= 1) >= denominator)
		{
			result |= 1;
			numerator -= denominator;
		}
	}
	while(--numberOfBitsPrecision > 0);

	return result;
}

#endif // _divideFraction_h_
