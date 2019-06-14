#ifndef _UnidirectionalChannel_h_
#define _UnidirectionalChannel_h_

#include "../cPrimitiveTypes.h"
#include "Stream.h"

//------------------------------------------------------------------------------------------------
// * class UnidirectionalChannel
//
// Represents a unidirectional communication stream.
//------------------------------------------------------------------------------------------------

class UnidirectionalChannel : public Stream
{
public:
	virtual ~UnidirectionalChannel() {};

	// testing
	inline Bool isInError() const;

	// accessing
	inline UInt getChannelId() const;
	
protected:
	// representation
	UInt channelId;
	Bool inError;
};

//------------------------------------------------------------------------------------------------
// * UnidirectionalChannel::getChannelId
//
// Adds a channel to the packet layer.
//------------------------------------------------------------------------------------------------

inline UInt UnidirectionalChannel::getChannelId() const
{
	return channelId;
}

//------------------------------------------------------------------------------------------------
// * UnidirectionalChannel::isInError
//
// Tests whether an error condition has occurred.
//------------------------------------------------------------------------------------------------

inline Bool UnidirectionalChannel::isInError() const
{
	return inError;
}

#endif // _UnidirectionalChannel_h_
