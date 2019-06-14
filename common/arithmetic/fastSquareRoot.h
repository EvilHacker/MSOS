#ifndef _fastSquareRoot_h_
#define _fastSquareRoot_h_

//------------------------------------------------------------------------------------------------
// * fastSquareRoot (Number) type must be unsigned
//
// Returns the square root of <n>.
//------------------------------------------------------------------------------------------------

inline UInt32 fastSquareRoot(UInt32 n)
{
	const UInt32 base = 0x40000000;
	UInt32 t, b, c = 0;
	for (b = base; b != 0; b >>= 2)
	{
		t = c + b;
		c >>= 1;
		if (n >= t)
		{
			n -= t;
			c += b;
		}
	}

	return c;
}

#endif // _fastSquareRoot_h_
