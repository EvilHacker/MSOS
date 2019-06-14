#ifndef _NocheckUnidirectionalChannel_h_
#define _NocheckUnidirectionalChannel_h_

#include "../cPrimitiveTypes.h"
#include "../multitasking/IntertaskQueue.h"
#include "UnidirectionalChannel.h"
#include "NocheckPacket.h"
#include "NocheckPacketLayer.h"

//------------------------------------------------------------------------------------------------
// * class NocheckUnidirectionalChannel
//
// Represents a unidirectional communication stream.
//------------------------------------------------------------------------------------------------

class NocheckUnidirectionalChannel : public UnidirectionalChannel
{
public:
	// constructor and destructor
	NocheckUnidirectionalChannel(NocheckPacketLayer &packetLayer, UInt channelId);			
	virtual ~NocheckUnidirectionalChannel();

	// error related
	void forceError();
	void reset();

	// packet operations
	NocheckPacket * waitPacket();
	void releasePacket(NocheckPacket * pPacket);

protected:
	// packet operations
	void receivePacket(NocheckPacket *pPacket, TimeValue timeout);
	void sendPacket(NocheckPacket *pPacket);

	NocheckPacketLayer &packetLayer;
	IntertaskPointerQueue<NocheckPacket> releaseQueue;
	IntertaskPointerQueue<NocheckPacket> waitQueue;
	NocheckPacket workingPacket;
};

#endif // _NocheckUnidirectionalChannel_h_
