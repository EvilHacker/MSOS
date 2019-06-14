#ifndef _Stream_h_
#define _Stream_h_

#include "../cPrimitiveTypes.h"
#include "../multitasking/TimeValue.h"

//------------------------------------------------------------------------------------------------
// * Stream
//
// Abstract base class for byte streams.
//------------------------------------------------------------------------------------------------

class Stream
{
public:
	inline Stream();
	virtual ~Stream() {};

	// testing
	virtual Bool isInError() const = 0;

	// streaming
	virtual UInt read(void *pDestination, UInt length);
	virtual UInt read(void *pDestination, UInt length, TimeValue timeout) = 0;
	virtual UInt write(const void *pSource, UInt length);
	virtual UInt write(const void *pSource, UInt length, TimeValue timeout) = 0;
	virtual void flush() = 0;

	// timeout
	inline TimeValue getDefaultTimeout() const;
	inline void setDefaultTimeout(TimeValue timeout);

	// error related
	virtual void forceError() = 0;
	virtual void reset() = 0;

protected:
	// representation
	TimeValue defaultTimeout;
};

//------------------------------------------------------------------------------------------------
// * Stream::Stream
//
// Constructor.
//------------------------------------------------------------------------------------------------

inline Stream::Stream()
{
	defaultTimeout = infiniteTime;
}

//------------------------------------------------------------------------------------------------
// * Stream::getDefaultTimeout
//
// Get the default timeout for reads and write on this stream.
//------------------------------------------------------------------------------------------------

inline TimeValue Stream::getDefaultTimeout() const
{
	return defaultTimeout;
}

//------------------------------------------------------------------------------------------------
// * Stream::setDefaultTimeout
//
// Set the default timeout for reads and write on this stream.
//------------------------------------------------------------------------------------------------

inline void Stream::setDefaultTimeout(TimeValue timeout)
{
	defaultTimeout = timeout;
}

#endif // _Stream_h_
