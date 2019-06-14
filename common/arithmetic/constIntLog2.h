#ifndef _constIntLog2_h_
#define _constIntLog2_h_

#include "../cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * constIntLog2
//
// Returns floor(log2(<x>)), where <x> is an integer.
// This is equivalent to getting the index of the most significant bit of <x>.
// For a constant value of <x>, the result of this function will be computed at compile time.
//------------------------------------------------------------------------------------------------

template<class T>
inline UInt constIntLog2(T x)
{
	x >>= 1;

	if(x <= 0)
	{
		return 0;
	}

	return constIntLog2(x) + 1;
}

#endif // _constIntLog2_h_
