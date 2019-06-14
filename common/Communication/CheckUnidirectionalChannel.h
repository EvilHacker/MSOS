#ifndef _CheckUnidirectionalChannel_h_
#define _CheckUnidirectionalChannel_h_

#include "../cPrimitiveTypes.h"
#include "../multitasking/IntertaskQueue.h"
#include "UnidirectionalChannel.h"
#include "CheckPacket.h"
#include "CheckPacketLayer.h"

//------------------------------------------------------------------------------------------------
// * class CheckUnidirectionalChannel
//
// Represents a unidirectional communication stream.
//------------------------------------------------------------------------------------------------

class CheckUnidirectionalChannel : public UnidirectionalChannel
{
public:
	// constructor and destructor
	CheckUnidirectionalChannel(CheckPacketLayer &packetLayer, UInt channelId);			
	virtual ~CheckUnidirectionalChannel();

	// error related
	void forceError();
	void reset();

	// packet operations
	void receivePacket(CheckPacket *pPacket);
	void recordPacket(CheckPacket *pPacket);
	void resendAllPackets();
	void freeAllPackets();

protected:
	// packet operations
	void sendPacket(
		CheckPacket *pPacket,
		UInt dataSize);
	void freePacketsInHistory();
	inline CheckPacket *getFreePacket();
	inline void freePacket(CheckPacket *pPacket);

	// representation
	enum { packetPipelineDepth = 2 };
	CheckPacketLayer &packetLayer;
	IntertaskPointerQueue<CheckPacket> receiveQueue;
	IntertaskPointerQueue<CheckPacket> historyQueue;
	UInt sequenceNumber : CheckPacket::numberOfSequenceBits;
	UInt numberOfPacketsToFreeFromHistory;
	CheckPacket *pWorkingPacket;
};

//------------------------------------------------------------------------------------------------
// * CheckUnidirectionalChannel::getFreePacket
//
// Gets a free packet from the packet layer.
//------------------------------------------------------------------------------------------------

inline CheckPacket *CheckUnidirectionalChannel::getFreePacket()
{
	return packetLayer.getFreePacket();
}

//------------------------------------------------------------------------------------------------
// * CheckUnidirectionalChannel::freePacket
//
// Frees <pPacket> by giving it back to the packet layer.
//------------------------------------------------------------------------------------------------

inline void CheckUnidirectionalChannel::freePacket(CheckPacket *pPacket)
{
	packetLayer.freePacket(pPacket);
}

#endif // _UnidirectionalChannel_h_
