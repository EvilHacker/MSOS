// JTagDevice.cpp: implementation of the JTagDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "JTagDevice.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*
UInt JTagDevice::getBit(const UInt8 * ptr, int index)
{
	if (ptr[index / 8] & (1u << (index - index / 8 * 8)))
	{
		return 1;
	}

	return 0;
}
*/

Bool JTagDevice::compareDeviceId()
{
	return true;
}

void JTagDevice::setDataRegister(void * data, Bool isBytes)
{
	indexWrite = 0;
	if (data == null)
	{
		for (UInt j = 0; j < dataRegisterLength; j++)
		{
			pDataBitsWrite[j] = 0;
		}
	}
	else
	{
		if (isBytes)
		{
			for (UInt i = 0; i < dataRegisterLength; i++)
			{
//				if (((UInt8 *)data)[i] > 0)
//				{
//					int shift = i - i / 8 * 8;
//					pDataBitsWrite[i / 8] |= (1u << shift);
//				}

				pDataBitsWrite[i] = ((UInt8 *)data)[i];
			}
		}
		else
		{
//			for (UInt ii = 0; ii < (dataRegisterLength + 7) / 8; ii++)
//			{
//				pDataBitsWrite[ii] = ((UInt8 *)data)[ii];
//			}

			for (UInt i = 0; i < dataRegisterLength; i++)
			{
				if (((UInt8 *)data)[i >> 3] & (1u << (i - ((i >> 3) << 3))))
				{
					pDataBitsWrite[i] = 1;
				}
				else
				{
					pDataBitsWrite[i] = 0;
				}
			}
		}
	}
}
