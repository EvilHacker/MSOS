#ifndef _CrcCalculator_h_
#define _CrcCalculator_h_

#include "../cPrimitiveTypes.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1400) // if Visual Studio 2005
	#pragma warning(disable: 4333) // warning C4333: '>>' : right shift by too large amount, data loss
#endif

//------------------------------------------------------------------------------------------------
// * class CrcCalculator
//
// This class implements a Cyclic Redundancy Code algorithm.
// The <log2TableSize> template argument must one of 1, 2, 4, 8 (4 or 8 are best).
// There are a limited number of <polynomial>s that provide good error detection.
// Here are some good instances.
// CrcCalculator<UInt16, 0x1021, maximumOfIntegerType(UInt16)>
// CrcCalculator<UInt16, 0x8021, maximumOfIntegerType(UInt16)>
// CrcCalculator<UInt16, 0x8408, maximumOfIntegerType(UInt16)>
// CrcCalculator<UInt32, 0x04C11DB7, maximumOfIntegerType(UInt32)>
//------------------------------------------------------------------------------------------------

template<class CrcValue, CrcValue polynomial, CrcValue initialValue, UInt log2TableSize = 4>
class CrcCalculator
{
public:
	// types
	typedef CrcValue CrcValue;

	// constructor
	CrcCalculator();

	// CRC calculation
	CrcValue calculateCrc(const void *pData, UInt length, CrcValue crc = initialValue) const;

private:
	// representation
	CrcValue crcTable[(1 << log2TableSize)];
};

//------------------------------------------------------------------------------------------------
// * CrcCalculator::CrcCalculator
//
// Constructor.
//------------------------------------------------------------------------------------------------

template<class CrcValue, CrcValue polynomial, CrcValue initialValue, UInt log2TableSize>
CrcCalculator<CrcValue, polynomial, initialValue, log2TableSize>::CrcCalculator()
{
	// initialize the table
	for(UInt i = 0; i < arrayDimension(crcTable); ++i)
	{
		CrcValue tableEntry = i << (8 * sizeof(CrcValue) - log2TableSize);
		for(UInt j = 0; j < log2TableSize; ++j)
		{
			if((tableEntry & (1 << (8 * sizeof(CrcValue) - 1))) != 0)
			{
				tableEntry = (tableEntry << 1) ^ polynomial;
			}
			else
			{
				tableEntry <<= 1;
			}
		}
		crcTable[i] = tableEntry;
	}
}

//------------------------------------------------------------------------------------------------
// * CrcCalculator::calculateCrc
//
// Calculates a CRC value given some data.
// The <crc> arguments is the initial value if a new CRC calculation is intended.
// The CRC calculation of a sequence of data items can be chained together by specifying the
// CRC calculated from a previous buffer as the <crc> argument of the next buffer.
//------------------------------------------------------------------------------------------------

template<class CrcValue, CrcValue polynomial, CrcValue initialValue, UInt log2TableSize>
CrcValue CrcCalculator<CrcValue, polynomial, initialValue, log2TableSize>::calculateCrc(
	const void *pData, UInt length, CrcValue crc) const
{
	// iterate over the data one byte at a time
	while((SInt)length-- > 0)
	{
		// get one byte of data
		const UInt8 byte = *(UInt8 *)pData;

		// advance to the next data byte
		pData = (UInt8 *)pData + 1;

		// select algorithm based on table size
		// this constant condition will be resolved at compile time
		if(log2TableSize == 8)
		{
			crc = crcTable[(crc >> (8 * sizeof(CrcValue) - log2TableSize)) ^ byte] ^ crc << log2TableSize;
		}
		else if(log2TableSize == 4)
		{
			crc = crcTable[(crc >> (8 * sizeof(CrcValue) - log2TableSize)) ^ (byte >> log2TableSize)] ^ crc << log2TableSize;
			crc = crcTable[(crc >> (8 * sizeof(CrcValue) - log2TableSize)) ^ (byte & ((1 << log2TableSize) - 1))] ^ crc << log2TableSize;
		}
		else if(log2TableSize == 2)
		{
			crc = crcTable[(crc >> (8 * sizeof(CrcValue) - log2TableSize)) ^ (byte >> (3 * log2TableSize))] ^ crc << log2TableSize;
			crc = crcTable[(crc >> (8 * sizeof(CrcValue) - log2TableSize)) ^ (byte >> (2 * log2TableSize)) & ((1 << log2TableSize) - 1)] ^ crc << log2TableSize;
			crc = crcTable[(crc >> (8 * sizeof(CrcValue) - log2TableSize)) ^ (byte >> (1 * log2TableSize)) & ((1 << log2TableSize) - 1)] ^ crc << log2TableSize;
			crc = crcTable[(crc >> (8 * sizeof(CrcValue) - log2TableSize)) ^ byte & ((1 << log2TableSize) - 1)] ^ crc << log2TableSize;
		}
		else
		{
			// this is a generic algorithm for any table size
			SInt shift = 8 * sizeof(byte);
			while((shift -= log2TableSize) >= 0)
			{
				crc = crcTable[(crc >> (8 * sizeof(CrcValue) - log2TableSize)) ^ (byte >> shift) & ((1 << log2TableSize) - 1)] ^ crc << log2TableSize;
			}
		}
	}

	return crc;
}

#endif // _CrcCalculator_h_
