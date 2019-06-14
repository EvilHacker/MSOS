// JTagChain.h: interface for the JTagChain class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _JTagChain_h_
#define _JTagChain_h_

#include "JTagDevice.h"

class JTagChain : public LinkedList  
{
public:
	JTagChain();
	virtual ~JTagChain() {};

	// overrides
	inline virtual int doInOut(int tdi, int tms, Bool isReadPort) = 0;
	virtual UInt32 getProgramSize() = 0;
	virtual UInt16 getFlushWord() = 0;

	// functionality
	void addDevice(JTagDevice * pDevice);
	Bool verifyDevices();
	void preloadBScan();
	void eraseFlush();

	void doIdle();
	void doReset();
	void doExtest();
	void doBScan(Bool isReadBack);

protected:

	void doIrScan();
	void doDrScan(Bool isReadBack);
	void doFlush(Bool isReadBack = true);

	// internal state
	Bool isInReset;
	Bool isInIdle;
};

#endif // !defined(_JTagChain_h_)
