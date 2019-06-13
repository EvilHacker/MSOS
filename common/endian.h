#ifndef _endian_h_
#define _endian_h_

#include "cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * isLittleEndianProcessor
//
// This is a constant function which test if the target processor is little endian.
//------------------------------------------------------------------------------------------------

inline Bool isLittleEndianProcessor()
{
	#if defined(__MWERKS__)
		return __option(little_endian);
	#elif defined(_MSC_VER) && defined(_M_IX86)
		return true;
	#elif defined(_MSC_VER) && defined(_M_ARM)
		return true;
	#elif defined(__ARMCC_VERSION)
		#if defined(__BIG_ENDIAN)
			return false;
		#else
			return true;
		#endif
	#else
		#error "unknown architecture"
	#endif
}

//------------------------------------------------------------------------------------------------
// * isBigEndianProcessor
//
// This is a constant function which test if the target processor is big endian.
//------------------------------------------------------------------------------------------------

inline Bool isBigEndianProcessor()
{
	return !isLittleEndianProcessor();
}

//------------------------------------------------------------------------------------------------
// * changeEndianOfInt8
//
// Returns a <value> with its bytes reversed (changes its endian).
//------------------------------------------------------------------------------------------------

inline UInt8 changeEndianOfInt8(UInt8 value)
{
	return value;
}

//------------------------------------------------------------------------------------------------
// * changeEndianOfInt16
//
// Returns a <value> with its bytes reversed (changes its endian).
//------------------------------------------------------------------------------------------------

inline UInt16 changeEndianOfInt16(UInt16 value)
{
	UInt16 result;
	((UInt8 *)&result)[0] = ((UInt8 *)&value)[1];
	((UInt8 *)&result)[1] = ((UInt8 *)&value)[0];
	return result;
}

//------------------------------------------------------------------------------------------------
// * changeEndianOfInt32
//
// Returns a <value> with its bytes reversed (changes its endian).
//------------------------------------------------------------------------------------------------

inline UInt32 changeEndianOfInt32(UInt32 value)
{
	UInt32 result;
	((UInt8 *)&result)[0] = ((UInt8 *)&value)[3];
	((UInt8 *)&result)[1] = ((UInt8 *)&value)[2];
	((UInt8 *)&result)[2] = ((UInt8 *)&value)[1];
	((UInt8 *)&result)[3] = ((UInt8 *)&value)[0];
	return result;
}

//------------------------------------------------------------------------------------------------
// * changeEndianOfInt64
//
// Returns a <value> with its bytes reversed (changes its endian).
//------------------------------------------------------------------------------------------------

inline UInt64 changeEndianOfInt64(UInt64 value)
{
	UInt64 result;
	((UInt8 *)&result)[0] = ((UInt8 *)&value)[7];
	((UInt8 *)&result)[1] = ((UInt8 *)&value)[6];
	((UInt8 *)&result)[2] = ((UInt8 *)&value)[5];
	((UInt8 *)&result)[3] = ((UInt8 *)&value)[4];
	((UInt8 *)&result)[4] = ((UInt8 *)&value)[3];
	((UInt8 *)&result)[5] = ((UInt8 *)&value)[2];
	((UInt8 *)&result)[6] = ((UInt8 *)&value)[1];
	((UInt8 *)&result)[7] = ((UInt8 *)&value)[0];
	return result;
}

//------------------------------------------------------------------------------------------------
// * changeEndian
//
// Returns a <value> with its bytes reversed (changes its endian).
//------------------------------------------------------------------------------------------------

template<class T>
inline T changeEndian(T value)
{
	if(sizeof(value) <= 1)
	{
		return value;
	}
	else if(sizeof(value) == 2)
	{
		const UInt16 result = changeEndianOfInt16(*(UInt16 *)&value);
		return *(T *)&result;
	}
	else if(sizeof(value) == 4)
	{
		const UInt32 result = changeEndianOfInt32(*(UInt32 *)&value);
		return *(T *)&result;
	}
	else if(sizeof(value) == 8)
	{
		const UInt64 result = changeEndianOfInt64(*(UInt64 *)&value);
		return *(T *)&result;
	}
	else
	{
		T result;

		Int index = sizeof(T) - 1;
		do
		{
			((UInt8 *)&result)[index] = ((UInt8 *)&value)[sizeof(T) - 1 - index];
		}
		while(--index >= 0);

		return result;
	}
}

//------------------------------------------------------------------------------------------------
// * littleEndian
//
// This function can
// - convert a little endian <value> into the target processor's endian, or
// - convert a <value> in the target processor's endian into little endian.
//------------------------------------------------------------------------------------------------

template<class T>
inline T littleEndian(T value)
{
	if(isLittleEndianProcessor())
	{
		return value;
	}
	else
	{
		return changeEndian(value);
	}
}

//------------------------------------------------------------------------------------------------
// * bigEndian
//
// This function can
// - convert a big endian <value> into the target processor's endian, or
// - convert a <value> in the target processor's endian into big endian.
//------------------------------------------------------------------------------------------------

template<class T>
inline T bigEndian(T value)
{
	if(isBigEndianProcessor())
	{
		return value;
	}
	else
	{
		return changeEndian(value);
	}
}

#endif // _endian_h_
