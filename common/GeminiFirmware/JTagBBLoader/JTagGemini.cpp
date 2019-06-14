
#include "JTagGemini.h"

//------------------------------------------------------------------------------------------------
// * HeartbeatTask::HeartbeatTask
//
// Constructor.
//------------------------------------------------------------------------------------------------

HeartbeatTask::HeartbeatTask(UInt sleepTime) :
	Task(Task::realtimePriority, 1000),
	status(HeartbeatTask::idle),
	sleepTime(sleepTime)
{
}

//------------------------------------------------------------------------------------------------
// * HeartbeatTask::main
//
// Flashes LEDs.
//------------------------------------------------------------------------------------------------

void HeartbeatTask::main()
{
	// flash LEDs on hardboard 2
	while(true)
	{
		if (status == HeartbeatTask::idle)
		{
			GeminiGpioOutput greenLed(7); 		// TMS
			GeminiGpioOutput redLed(6);			// TDI
			
			greenLed.turnOn();
			redLed.turnOn();
			
			sleepForMilliseconds(sleepTime);
	
			greenLed.turnOff();
			redLed.turnOff();
			
			sleepForMilliseconds(sleepTime);
		}
		else
		{
			sleepForMilliseconds(sleepTime / 20);
		}
	}
}

//------------------------------------------------------------------------------------------------
// * JTagGemini::JTagGemini
//
// Constructor.
//------------------------------------------------------------------------------------------------

JTagGemini::JTagGemini() :
Task(Task::realtimePriority, 2000)
{
}

//------------------------------------------------------------------------------------------------
// * JTagGemini::notifyStatus
//
// Constructor.
//------------------------------------------------------------------------------------------------

void JTagGemini::notifyError(Bool isInError)
{
	if (isInError)
	{
		// error
	}
	else
	{
		// success
	}
}

//------------------------------------------------------------------------------------------------
// * JTagGemini::main
//
// Process commands.
//------------------------------------------------------------------------------------------------

void JTagGemini::main()
{
	const UInt sleepTime = 200;
	
	GeminiGpioButton triggerButton(4, Task::realtimePriority + 3000);
	triggerButton.resume();
	
	HeartbeatTask controlTask(sleepTime);
	controlTask.resume();
	
	while (true)
	{
		controlTask.status = HeartbeatTask::idle;
		sleepForMilliseconds(sleepTime / 2);
		
		if (triggerButton.isPressed())
		{
			controlTask.status = HeartbeatTask::programming;
			sleepForMilliseconds(sleepTime * 3);
			
			JTagChainGemini chain;
			JTagSA1110 sa1110(&chain);
			
			// create and add more devices - in the same order as they are on the board
			chain.addDevice(&sa1110);
		
			// reset all devices in chain
			chain.doReset();
		
			// check 
			if (!chain.verifyDevices())
			{
				chain.doReset();
				// signal buzzer error
				notifyError(true);
				continue;
			}
			
			chain.preloadBScan();
			
			// do programming
			UInt32 sectorAddresses[] =
			{
				0x00000000,
				0x00010000,
				0x00020000,
				0x00030000,
				0x00040000,
				0x00050000,
				0x00060000,
				0x00070000,
				0x00080000,
				0x00090000,
				0x000A0000,
				0x000B0000,
				0x000C0000,
				0x000D0000,
				0x000E0000,
				0x000F0000,
				0x000F8000,
				0x000FA000,
				0x000FC000,
		
				// past the end of flash
				0x00100000
			};
			
			// check file size
			UInt32 programSize = chain.getProgramSize();
			if (programSize > sectorAddresses[arrayDimension(sectorAddresses) - 1])
			{
				chain.doReset();
				// signal buzzer error
				notifyError(true);
				continue;
			}

			// erase flush
			for (int i = 0; i < arrayDimension(sectorAddresses); i++)
			{
				if (programSize > sectorAddresses[i])
				{
					sa1110.eraseFlushSector(sectorAddresses[i]);
				}
			}
			
			Bool isInError = false;
	
			// write flush
			UInt runAddress = 0;
			const UInt startAddress = 0;
			while (runAddress < programSize)
			{
				UInt16 flushWord = chain.getFlushWord();
				sa1110.writeWord(startAddress + runAddress, flushWord);
				runAddress += sizeof(flushWord);
			}

			// test flush
			runAddress = 0;
			chain.rewindDataSource();
			while (runAddress < programSize)
			{
				UInt16 flushWord = chain.getFlushWord();
				sa1110.readWord(startAddress + runAddress);
				if (sa1110.readWord(startAddress + runAddress) != flushWord)
				{
					isInError = true;
					break;
				} 
		
				runAddress += sizeof(flushWord);
			}
			
			chain.doReset();
			if (isInError)
			{
				// signal buzzer error
				notifyError(true);
			}
			else
			{
				// signal buzzer success
				notifyError(false);
			}
		}

		sleepForMilliseconds(sleepTime / 10);
	}
}
