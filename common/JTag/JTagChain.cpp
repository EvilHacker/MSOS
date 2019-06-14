// JTagChain.cpp: implementation of the JTagChain class.
//
//////////////////////////////////////////////////////////////////////

#include "JTagChain.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

JTagChain::JTagChain()
{
	isInReset = false;
	isInIdle = false;
}

void JTagChain::addDevice(JTagDevice * pDevice)
{
	addFirst(pDevice);
}

void JTagChain::doReset()
{
	if (isInReset)
	{
		return;
	}

	// switch to reset state
	doInOut(1, 1, false);
	doInOut(1, 1, false);
	doInOut(1, 1, false);
	doInOut(1, 1, false);
	doInOut(1, 1, false);
	doInOut(1, 1, false);
	doInOut(1, 1, false);

	isInIdle = false;
	isInReset = true;
}

void JTagChain::doIdle()
{
	if (isInIdle)
	{
		return;
	}

	// switch to idle
	doInOut(1, 0, false);
	doInOut(1, 0, false);
	doInOut(1, 0, false);

	isInReset = false;
	isInIdle = true;

	// in command process
}

void JTagChain::doIrScan()
{
	doIdle();

	doInOut(1, 1, false);
	doInOut(1, 1, false);
	doInOut(1, 0, false);
	isInIdle = false;

	doFlush(false);
}

void JTagChain::doDrScan(Bool isReadBack)
{
	doIdle();

	doInOut(1, 1, false);
	doInOut(1, 0, false);
	isInIdle = false;

	doFlush(isReadBack);
}

void JTagChain::doFlush(Bool bReadBack)
{
	Link * pLink = getFirst();
	int tdo = 0;

	while (pLink)
	{
		JTagDevice * pDevice = (JTagDevice *)pLink;
		pDevice->initializeBufferIndexes();

		UInt bits = pDevice->getDataRegisterLength();
		for (UInt i = 0; i < (bits + 1); i++)
		{
			int tdi = 1;
			if (i > 0)
			{
				tdi = pDevice->getAndShiftWriteData();
				if (i == bits && pLink == getLast())
				{
					// Exit 1-xR
					doInOut(tdi, 1, false);
					continue;
				}
			}

			// Capture & Shift
			if (i != 0 || pLink == getFirst())
			{
				tdo = doInOut(tdi, 0, bReadBack);
			}

			if (bReadBack)
			{
				pDevice->setAndShiftReadData(tdo);
			}
		}

		// buffer padding
//		if (bits != bits / 8 * 8)
//		{
//			for (UInt j = 0; j < (8 - (bits - bits / 8 * 8)); j++)
//			{
//				pDevice->setAndShiftReadData(0);
//			}
//		}

		pLink = pLink->getNext();
	}

	// Update xR
	doInOut(1, 1, false);
}

Bool JTagChain::verifyDevices()
{
	// in the idle state
	Link * pLink = getFirst();
	while (pLink)
	{
		JTagDevice * pDevice = (JTagDevice *)pLink;
		UInt idcode = pDevice->getIdcodeInstruction();
		pDevice->setDataRegisterLength(pDevice->getInstructionRegisterLength());
		pDevice->setDataRegister(&idcode);
		pLink = pLink->getNext();
	}

	doIrScan();

	pLink = getFirst();
	while (pLink)
	{
		JTagDevice * pDevice = (JTagDevice *)pLink;
		pDevice->setDataRegisterLength(pDevice->getIdcodeDataLength());
		pDevice->setDataRegister(null);
		pLink = pLink->getNext();
	}

	doDrScan(true);

	pLink = getFirst();
	while (pLink)
	{
		JTagDevice * pDevice = (JTagDevice *)pLink;
		if (!pDevice->compareDeviceId())
		{
			return false;
		}

		pLink = pLink->getNext();
	}

	return true;
}

void JTagChain::preloadBScan()
{
	Link * pLink = getFirst();
	while (pLink)
	{
		JTagDevice * pDevice = (JTagDevice *)pLink;
		UInt idcode = pDevice->getBSCodeInstruction();
		pDevice->setDataRegisterLength(pDevice->getInstructionRegisterLength());
		pDevice->setDataRegister(&idcode);
		pLink = pLink->getNext();
	}

	doIrScan();
}

void JTagChain::doExtest()
{
	Link * pLink = getFirst();
	while (pLink)
	{
		JTagDevice * pDevice = (JTagDevice *)pLink;
		UInt idcode = pDevice->getExtestCodeInstruction();
		pDevice->setDataRegisterLength(pDevice->getInstructionRegisterLength());
		pDevice->setDataRegister(&idcode);
		pLink = pLink->getNext();
	}

	doIrScan();
}

void JTagChain::doBScan(Bool isReadBack)
{
	Link * pLink = getFirst();
	while (pLink)
	{
		JTagDevice * pDevice = (JTagDevice *)pLink;
		pDevice->setDataRegisterLength(pDevice->getBSDataLength());
		pDevice->setDataRegister(pDevice->getBSData(), true);
		pLink = pLink->getNext();
	}

	doDrScan(isReadBack);
}
