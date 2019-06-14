#ifndef _CheckPacket_h_
#define _CheckPacket_h_

#include "../cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * class CheckPacket
//
// Represents a piece of data trasmitted or received over a communication link.
//------------------------------------------------------------------------------------------------

class CheckPacket
{
public:
	CheckPacket();
	
	// types
	struct CheckPacketHeader
	{
		UInt16 crcValue;
		UInt16 bitfields;
	};
	
	enum { numberOfSequenceBits = 2 };

	// testing
	inline Bool isCrcValid() const;

	// querying
	inline UInt getMaximumPacketDataSize() const;

	// accessing
	inline const CheckPacketHeader *getPacketHeader() const;
	inline CheckPacketHeader *getPacketHeader();
	inline const void *getPacketData() const;
	inline void *getPacketData();

	// accessing header fields
	inline UInt getConnectionNumber() const;
	inline void setConnectionNumber(UInt connectionNumber);
	inline UInt getChannelId() const;
	inline void setChannelId(UInt channelId);
	inline UInt getSequenceNumber() const;
	inline void setSequenceNumber(UInt sequenceNumber);
	inline UInt getDataSize() const;
	inline void setDataSize(UInt dataSize);
	inline UInt getCrcValue() const;
	inline void setCrcValue(UInt crc);
	inline void setCrcValue();

	// CRC calculation
	UInt calculateCrcValue() const;

private:
	
	// representation
	UInt connectionNumber;
	enum { maximumPacketDataSize = 1016 };
	
	// packet contents
	CheckPacketHeader packetHeader;
	UInt8 packetData[maximumPacketDataSize];
};

//------------------------------------------------------------------------------------------------
// * CheckPacket::isCrcValid
//
// Test whether the data is contains no errors.
//------------------------------------------------------------------------------------------------

inline Bool CheckPacket::isCrcValid() const
{
	return packetHeader.crcValue == calculateCrcValue();
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::getMaximumPacketDataSize
//
// Returns the maximum amount of data that can be contained within this packet.
//------------------------------------------------------------------------------------------------

inline UInt CheckPacket::getMaximumPacketDataSize() const
{
	return maximumPacketDataSize;
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::getConnectionNumber
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline UInt CheckPacket::getConnectionNumber() const
{
	return connectionNumber;
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::setConnectionNumber
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void CheckPacket::setConnectionNumber(UInt connectionNumber)
{
	this->connectionNumber = connectionNumber;
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::getChannelId
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline UInt CheckPacket::getChannelId() const
{
	return packetHeader.bitfields & ((UInt16)0xfu);
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::setChannelId
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void CheckPacket::setChannelId(UInt channelId)
{
	packetHeader.bitfields = (packetHeader.bitfields & ~((UInt16)0xfu)) | channelId;
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::getSequenceNumber
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline UInt CheckPacket::getSequenceNumber() const
{
	return (packetHeader.bitfields >> 4) & ((UInt16)0x03u);
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::setSequenceNumber
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void CheckPacket::setSequenceNumber(UInt sequenceNumber)
{
	packetHeader.bitfields = (packetHeader.bitfields) & ~(((UInt16)0x03u) << 4) | (sequenceNumber << 4);
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::getDataSize
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline UInt CheckPacket::getDataSize() const
{
	return (packetHeader.bitfields) >> 6;
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::setDataSize
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void CheckPacket::setDataSize(UInt dataSize)
{
	packetHeader.bitfields = (packetHeader.bitfields) & ~(((UInt16)0x3FFu) << 6) | (dataSize << 6);
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::getCrcValue
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline UInt CheckPacket::getCrcValue() const
{
	return packetHeader.crcValue;
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::setCrcValue
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void CheckPacket::setCrcValue(UInt crc)
{
	packetHeader.crcValue = crc;
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::setCrcValue
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void CheckPacket::setCrcValue()
{
	packetHeader.crcValue = calculateCrcValue();
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::getPacketHeader
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline const CheckPacket::CheckPacketHeader *CheckPacket::getPacketHeader() const
{
	return &packetHeader;
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::getPacketHeader
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline CheckPacket::CheckPacketHeader *CheckPacket::getPacketHeader()
{
	return &packetHeader;
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::getPacketData
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline const void *CheckPacket::getPacketData() const
{
	return packetData;
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::getPacketData
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void *CheckPacket::getPacketData()
{
	return packetData;
}

#endif // _CheckPacket_h_
