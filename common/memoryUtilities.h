#ifndef _memoryUtilities_h_
#define _memoryUtilities_h_

#include "cPrimitiveTypes.h"

#if defined(USE_ANSI_MEMORY_UTILITIES)
	extern "C" void *memcpy(void *, const void *, UInt);
	extern "C" void *memset(void *, Int, UInt);
#endif

//------------------------------------------------------------------------------------------------
// * arrayCopy
//
// Copies <length> elements from the <source> array to the <destination> array.
//------------------------------------------------------------------------------------------------

template<class T>
inline void arrayCopy(T *destination, const T *source, UInt length)
{
	while(length > 0)
	{
		*(destination++) = *(source++);
		--length;
	}
}

//------------------------------------------------------------------------------------------------
// * memoryCopy
//
// Copies <lengthInBytes> bytes from the <source> buffer to the <destination> buffer.
//------------------------------------------------------------------------------------------------

inline void memoryCopy(void *destination, const void *source, UInt lengthInBytes)
{
	#if defined(USE_ANSI_MEMORY_UTILITIES)
		memcpy(destination, source, lengthInBytes);
	#else
		arrayCopy((UInt8 *)destination, (const UInt8 *)source, lengthInBytes);
	#endif
}

//------------------------------------------------------------------------------------------------
// * arraySet
//
// Copies the contents of *<pValue> into the first <length> elements of the <destination> array.
//------------------------------------------------------------------------------------------------

template<class T>
inline void arraySet(T *destination, const T *pValue, UInt length)
{
	while(length > 0)
	{
		*(destination++) = *pValue;
		--length;
	}
}

//------------------------------------------------------------------------------------------------
// * arraySet
//
// Copies <value> into the first <length> elements of the <destination> array.
//------------------------------------------------------------------------------------------------

template<class T>
inline void arraySet(T *destination, const T value, UInt length)
{
	while(length > 0)
	{
		*(destination++) = value;
		--length;
	}
}

//------------------------------------------------------------------------------------------------
// * memorySet
//
// Copies <value> into the first <lengthInBytes> bytes of the <destination> buffer.
//------------------------------------------------------------------------------------------------

inline void memorySet(void *destination, UInt8 value, UInt lengthInBytes)
{
	#if defined(USE_ANSI_MEMORY_UTILITIES)
		memset(destination, value, lengthInBytes);
	#else
		arraySet((UInt8 *)destination, value, lengthInBytes);
	#endif
}

//------------------------------------------------------------------------------------------------
// * memorySet
//
// Writes zeros into the first <lengthInBytes> bytes of the <destination> buffer.
//------------------------------------------------------------------------------------------------

inline void memoryZero(void *destination, UInt lengthInBytes)
{
	#if defined(USE_ANSI_MEMORY_UTILITIES)
		memset(destination, 0, lengthInBytes);
	#else
		arraySet((UInt8 *)destination, (UInt8)0, lengthInBytes);
	#endif
}

//------------------------------------------------------------------------------------------------
// * arrayCompare
//
// Lexicographicaly compares two arrays <source1> and <source2> of the same <length>.
// -1 is returned if source1 < source2.
//  0 is returned if source1 = source2.
//  1 is returned if source1 > source2.
//------------------------------------------------------------------------------------------------

template<class T>
SInt arrayCompare(const T *source1, const T *source2, UInt length)
{
	while((SInt)--length >= 0)
	{
		// compare
		if(*source1 != *source2)
		{
			// check which is greater
			if(*source1 > *source2)
			{
				// source1 > source2
				return 1;
			}
			else
			{
				// source1 < source2
				return -1;
			}
		}

		// advance to the next element
		++source1;
		++source2;
	}

	// no differences encounterred
	return 0;
}

//------------------------------------------------------------------------------------------------
// * findElementInArray
//
// Find the smallest index within the array <pArray> of length <arrayLength> where an
// <element> can be found.
// If the <element> is not found, the function returns -1.
//------------------------------------------------------------------------------------------------

template<class T>
SInt findElementInArray(const T *pArray, UInt arrayLength, const T element)
{
	// iterate over all elements in the array
	for(UInt index = 0; index < arrayLength; ++index)
	{
		// check if the element exists at this index
		if(*pArray++ == element)
		{
			// we found the element, return the index
			return index;
		}
	}

	// the element was not found
	return -1;
}

//------------------------------------------------------------------------------------------------
// * findSubArray
//
// Find the smallest index within the array <pArray> of length <arrayLength> where
// the sub-sequence <pSubArray> of length <subArrayLength> can be found.
// If the sub-array is not found, the function returns -1.
//------------------------------------------------------------------------------------------------

template<class T>
SInt findSubArray(const T *pArray, UInt arrayLength, const T *pSubArray, UInt subArrayLength)
{
	// check if the sub-array is empty
	if(subArrayLength == 0)
	{
		return 0;
	}

	// try to match the sub-array at all indexes
	UInt index = 0;
	while(subArrayLength <= arrayLength - index)
	{
		// check if the sub-array matches at this index
		if(arrayCompare(pArray + index, pSubArray, subArrayLength) == 0)
		{
			return index;
		}
		
		// advance to next index
		++index;
	}

	// sub-array was not found
	return -1;
}

//------------------------------------------------------------------------------------------------
// * missalignedLoad
//
// Loads and return a value from an <address> which may not be aligned on a multiple of its size.
// This function is intended to be used with C primitive types or structures of similar size.
//------------------------------------------------------------------------------------------------

template<class T>
inline T missalignedLoad(const T *address)
{
	// check for a processor that supports missaliged memory access
	#if defined(_x86_) || defined(_PowerPc_) || defined(_68K_)
		// load directly from the address
		return *address;
	#else
		// the memoryCopy will break down the operation into multiple loads (if needed)
		T value;
		memoryCopy(&value, address, sizeof(T));
		return value;
	#endif
}

//------------------------------------------------------------------------------------------------
// * missalignedLoad
//
// Stores a <value> to an <address> which may not be aligned on a multiple of its size.
// This function is intended to be used with C primitive types or structures of similar size.
//------------------------------------------------------------------------------------------------

template<class T>
inline void missalignedStore(T *address, const T value)
{
	// check for a processor that supports missaliged memory access
	#if defined(_x86_) || defined(_PowerPc_) || defined(_68K_)
		// store directly to the address
		*address = value;
	#else
		// the memoryCopy will break down the operation into multiple stores (if needed)
		memoryCopy(address, &value, sizeof(T));
	#endif
}

#endif // _memoryUtilities_h_
