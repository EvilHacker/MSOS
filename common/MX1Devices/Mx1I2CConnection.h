#ifndef _Mx1I2cConnection_h_
#define _Mx1I2cConnection_h_

#include "../../cPrimitiveTypes.h"
#include "../../Communication/Stream.h"

//------------------------------------------------------------------------------------------------
// * class Mx1I2cConnection
//
// Provides a Stream interface to a particular address on an I2C port.
//------------------------------------------------------------------------------------------------

class Mx1I2cConnection :
	public Stream
{
public:
	// constructor
	inline Mx1I2cConnection(Mx1I2cPort &port, UInt address);

	// testing
	inline Bool isInError() const;

	// streaming
	inline UInt read(void *pDestination, UInt length);
	inline UInt read(void *pDestination, UInt length, TimeValue timeout);
	inline UInt write(const void *pSource, UInt length);
	inline UInt write(const void *pSource, UInt length, TimeValue timeout);
	inline void flush();

	// error related
	inline void forceError();
	inline void reset();

	// i2c connection specific
	inline UInt getAddress() const;
	inline void setAddress(const UInt address);

private:
	// representation
	Mx1I2cPort *pPort;
	UInt address;
	Bool inError;
};

//------------------------------------------------------------------------------------------------
// * Mx1I2cConnection::Mx1I2cConnection
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline Mx1I2cConnection::Mx1I2cConnection(Mx1I2cPort &port, UInt address) :
	pPort(&port),
	address(address),
	inError(false)
{
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cConnection::isInError
//
// Tests whether an error condition has occurred.
//------------------------------------------------------------------------------------------------

inline Bool Mx1I2cConnection::isInError() const
{
	return inError;
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cConnection::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

inline UInt Mx1I2cConnection::read(void *pDestination, UInt length)
{
	if(inError)
	{
		return 0;
	}
	else
	{
		const UInt actualLength = pPort->read(address, pDestination, length, infiniteTime);
		if(length != actualLength)
		{
			inError = true;
		}
		return actualLength;
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cConnection::read
//
// Read data from the stream.
//------------------------------------------------------------------------------------------------

inline UInt Mx1I2cConnection::read(void *pDestination, UInt length, TimeValue timeout)
{
	if(inError)
	{
		return 0;
	}
	else
	{
		const UInt actualLength = pPort->read(address, pDestination, length, timeout);
		if(length != actualLength)
		{
			inError = true;
		}
		return actualLength;
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cConnection::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

inline UInt Mx1I2cConnection::write(const void *pSource, UInt length)
{
	if(inError)
	{
		return 0;
	}
	else
	{
		const UInt actualLength = pPort->write(address, pSource, length, infiniteTime);
		if(length != actualLength)
		{
			inError = true;
		}
		return actualLength;
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cConnection::write
//
// Write data to the stream.
//------------------------------------------------------------------------------------------------

inline UInt Mx1I2cConnection::write(const void *pSource, UInt length, TimeValue timeout)
{
	if(inError)
	{
		return 0;
	}
	else
	{
		const UInt actualLength = pPort->write(address, pSource, length, timeout);
		if(length != actualLength)
		{
			inError = true;
		}
		return actualLength;
	}
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cConnection::flush
//
// Sends any buffered data.
//------------------------------------------------------------------------------------------------

inline void Mx1I2cConnection::flush()
{
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cConnection::forceError
//
// Force an error condition.
//------------------------------------------------------------------------------------------------

inline void Mx1I2cConnection::forceError()
{
	inError = true;
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cConnection::reset
//
// Clear an error condition.
//------------------------------------------------------------------------------------------------

inline void Mx1I2cConnection::reset()
{
	inError = false;
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cConnection::getAddress
//
// Return I2C address.
//------------------------------------------------------------------------------------------------

inline UInt Mx1I2cConnection::getAddress() const
{
	return address;
}

//------------------------------------------------------------------------------------------------
// * Mx1I2cConnection::setAddress
//
// Set I2C address.
//------------------------------------------------------------------------------------------------

inline void Mx1I2cConnection::setAddress(const UInt address)
{
	this->address = address;
}

#endif // _Mx1I2cConnection_h_
