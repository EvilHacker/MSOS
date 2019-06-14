#ifndef _NocheckPacket_h_
#define _NocheckPacket_h_

#include "../cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * class NocheckPacket
//
// Represents a piece of data trasmitted or received over a communication link.
//------------------------------------------------------------------------------------------------

class NocheckPacket
{
public:
	NocheckPacket();
	
	// types
	struct NocheckPacketHeader
	{
		UInt32 bitfields;
	};
	
	enum { numberOfSequenceBits = 2 };

	// accessing
	inline const NocheckPacketHeader *getPacketHeader() const;
	inline NocheckPacketHeader *getPacketHeader();
	inline void setPacketData(const void * pData);
	inline const void *getPacketData() const;
	inline void *getPacketData();

	// accessing header fields
	inline UInt getChannelId() const;
	inline void setChannelId(UInt channelId);
	inline UInt getSequenceNumber() const;
	inline void setSequenceNumber(UInt sequenceNumber);
	inline UInt getDataSize() const;
	inline void setDataSize(UInt dataSize);

private:
	
	// packet contents
	NocheckPacketHeader packetHeader;
	UInt8 *packetData;
};

//------------------------------------------------------------------------------------------------
// * NocheckPacket::getChannelId
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline UInt NocheckPacket::getChannelId() const
{
	return packetHeader.bitfields & ((UInt32)0xfu);
}

//------------------------------------------------------------------------------------------------
// * NocheckPacket::setChannelId
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void NocheckPacket::setChannelId(UInt channelId)
{
	packetHeader.bitfields = (packetHeader.bitfields & ~((UInt32)0xfu)) | channelId;
}

//------------------------------------------------------------------------------------------------
// * NocheckPacket::getSequenceNumber
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline UInt NocheckPacket::getSequenceNumber() const
{
	return (packetHeader.bitfields >> 4) & ((UInt32)0x03u);
}

//------------------------------------------------------------------------------------------------
// * NocheckPacket::setSequenceNumber
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void NocheckPacket::setSequenceNumber(UInt sequenceNumber)
{
	packetHeader.bitfields = (packetHeader.bitfields) & ~(((UInt32)0x03u) << 4) | (sequenceNumber << 4);
}

//------------------------------------------------------------------------------------------------
// * NocheckPacket::getDataSize
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline UInt NocheckPacket::getDataSize() const
{
	return (packetHeader.bitfields) >> 6;
}

//------------------------------------------------------------------------------------------------
// * NocheckPacket::setDataSize
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void NocheckPacket::setDataSize(UInt dataSize)
{
	packetHeader.bitfields = (packetHeader.bitfields) & ~(((UInt32)0x3FFFFFFu) << 6) | (dataSize << 6);
}

//------------------------------------------------------------------------------------------------
// * NocheckPacket::getPacketHeader
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline const NocheckPacket::NocheckPacketHeader *NocheckPacket::getPacketHeader() const
{
	return &packetHeader;
}

//------------------------------------------------------------------------------------------------
// * NocheckPacket::getPacketHeader
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline NocheckPacket::NocheckPacketHeader *NocheckPacket::getPacketHeader()
{
	return &packetHeader;
}

//------------------------------------------------------------------------------------------------
// * NocheckPacket::getPacketData
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline const void *NocheckPacket::getPacketData() const
{
	return packetData;
}

//------------------------------------------------------------------------------------------------
// * NocheckPacket::getPacketData
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void *NocheckPacket::getPacketData()
{
	return packetData;
}

//------------------------------------------------------------------------------------------------
// * NocheckPacket::getPacketData
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void NocheckPacket::setPacketData(const void * pData)
{
	packetData = (UInt8 *)pData;
}

#endif // _FastPacket_h_
