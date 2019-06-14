#ifndef _absoluteValue_h_
#define _absoluteValue_h_

//------------------------------------------------------------------------------------------------
// * absoluteValue
//
// Returns the the absolute value of <n> (|n|).
//------------------------------------------------------------------------------------------------

template<class Number>
inline Number absoluteValue(Number n)
{
	if(n < 0)
	{
		return -n;
	}
	else
	{
		return n;
	}
}

#endif // _absoluteValue_h_
