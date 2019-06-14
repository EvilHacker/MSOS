#include "CheckPacket.h"
#include "../Crc/Crc16Calculator.h"

//------------------------------------------------------------------------------------------------
// * CheckPacket::CheckPacket
//
// Constructor
//------------------------------------------------------------------------------------------------

CheckPacket::CheckPacket()
{
	packetHeader.crcValue = 0;
	packetHeader.bitfields = 0;
}

//------------------------------------------------------------------------------------------------
// * CheckPacket::calculateCrcValue
//
// Calculates the CRC value of the packet.
//------------------------------------------------------------------------------------------------

UInt CheckPacket::calculateCrcValue() const
{
	return crc16Calculator.calculateCrc(
		&packetHeader.crcValue + 1,
		sizeof(CheckPacketHeader) - sizeof(packetHeader.crcValue) + getDataSize());
}
