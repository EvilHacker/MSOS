#include "BootTask.h"
#include "../../multitasking/sleep.h"
#include "../../CRC/Crc16Calculator.h"
#include "../../CRC/Crc32Calculator.h"
#include "../../Devices/getGeminiBoardRevision.h"
#if defined(__TARGET_CPU_ARM920T)
	#include "../../Mx1Devices/Mx1GpioOutput.h"
#endif
#if defined(__TARGET_CPU_SA_1100)
	#include "../../Sa1110Devices/Sa1110GpioOutput.h"
#endif

//------------------------------------------------------------------------------------------------
// * BootLoaderTask::BootLoaderTask
//
// Constructor.
//------------------------------------------------------------------------------------------------

FirmwareLoaderTask::FirmwareLoaderTask(BootTask * pBootTask) :
	Task(Task::normalPriority + 10, 20000),
	pFlash(Flash::getCurrentFlash()),
	pBootTask(pBootTask)
{
}

//------------------------------------------------------------------------------------------------
// * BootLoaderTask::main
//
// Main thread task.
//------------------------------------------------------------------------------------------------

void FirmwareLoaderTask::main()
{
	Bool shouldSleep = true;
	#if defined(__TARGET_CPU_ARM920T)

		// check if trigger is up
		Mx1GpioPin trigger(Mx1GpioPin::portD, 18);
		trigger.configureAsInput();
		if(trigger.getValue() != 0)
		{
			shouldSleep = false;
		}

		// check if reset by watchdog
		else if(*(volatile UInt *)(mx1RegistersBase + 0x1B800) == 2)
		{
			shouldSleep = false;
		}

	#endif
	
	if(shouldSleep)
	{
		// sleep for a couple of seconds
		pBootTask->initializeCommunication();
		sleepForSeconds(2);
	}
	
	// save firmware start address 
	const UInt32 runAddress = checkLoadFirmware();
	if (runAddress != 0)
	{
		// lock access mutex
		pBootTask->orderMutex.wait();
		
		// load firmware
		( (void (*)()) runAddress)();
	}
	else if (!shouldSleep)
	{
		pBootTask->initializeCommunication();
	}
}

//------------------------------------------------------------------------------------------------
// * BootLoaderTask::checkLoadFirmware
//
// Checks for and runs any firmware programmed in flash memory.
// Load firmware programmed in flash memory.
//------------------------------------------------------------------------------------------------

UInt32 FirmwareLoaderTask::checkLoadFirmware()
{
	UInt32 magicNumber;
	UInt32 firmwareCrc;
	UInt32 firmwareLength;
	
	for(UInt32 startAddress = (flashBase + flashBlockSize); startAddress < flashLimit; startAddress += flashBlockSize)
	{
		// read block header
		UInt32 runAddress = startAddress;
		pFlash->read(&magicNumber, (void *)(runAddress), sizeof(magicNumber));
		runAddress += sizeof(magicNumber);
		
		if (magicNumber == magicFirmwareId)
		{
			pFlash->read(&firmwareCrc, (void *)(runAddress), sizeof(firmwareCrc));
			runAddress += sizeof(firmwareCrc);
			pFlash->read(&firmwareLength, (void *)(runAddress), sizeof(firmwareLength));
			runAddress += sizeof(firmwareLength);
			
			if ((startAddress + firmwareLength) < flashLimit)
			{
				if (firmwareCrc == crc32Calculator.calculateCrc((void *)runAddress, firmwareLength))
				{	
					// check processor type
					UInt32 firmwareTag;
					pFlash->read(&firmwareTag, (void *)(runAddress + 4), sizeof(firmwareTag));
					
					#if defined(__TARGET_CPU_ARM920T)
					if (firmwareTag == 0x0A009200)
					{
						//firmware start address
						return runAddress;
					}
					#endif
					
					#if defined(__TARGET_CPU_SA_1100)
					if (firmwareTag == 0x00A01100)
					{
						//firmware start address
						return runAddress;
					}
					#endif
					
					// firmware does not have required signature
					#if defined(__TARGET_CPU_ARM920T)
						Mx1GpioOutput buzzer(Mx1GpioPin::portA, 2, 1);
					#endif
					#if defined(__TARGET_CPU_SA_1100)
						Sa1110GpioOutput buzzer(8, 1);
					#endif
					
					// make a disaster
					Timer *pTimer = TaskScheduler::getCurrentTaskScheduler()->getTimer();
					
					UInt counter = 0;
					const UInt referenceDivider = 4700;
					const TimeValue period = pTimer->getFrequency() / referenceDivider;
					
					for (UInt r = 0; r < 16; r++)
					{
						if (counter < (referenceDivider >> 3))
							counter = referenceDivider;
							
						counter >>= 1;
						sleepForTicks(counter * period, pTimer);
						for (UInt c = 0; c < counter; c++)
						{
							// one pulse
							buzzer.turnOn();
							sleepForTicks(period / 2, pTimer);
							buzzer.turnOff();
							sleepForTicks((period + 1) / 2, pTimer);
						}
					}
				}
			}
			
			break;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------
// * BootTask::BootTask
//
// Constructor.
//------------------------------------------------------------------------------------------------

BootTask::BootTask() :
	pFlash(Flash::getCurrentFlash()),
	uartSynchronizer(this, Task::normalPriority, 20000)
	#if defined(USE_USB)
		, usbSynchronizer(this, Task::normalPriority, 20000)
	#endif
	#if defined(USE_USB2)
		, usb2Synchronizer(this, Task::normalPriority, 20000)
	#endif
{
	(new FirmwareLoaderTask(this))->resume();
}

//------------------------------------------------------------------------------------------------
// * BootTask::initializeCommunication
//
// Initialize UART or USB channels
//------------------------------------------------------------------------------------------------

void BootTask::initializeCommunication()
{
	#if defined(__TARGET_CPU_ARM920T)
		pUartPort = new Mx1UartPort(Mx1UartPort::port2);
		pUartPort->configure(38400, 8, 1, false, false);
		uartSynchronizer.resume();
	#endif
	
	#if defined(__TARGET_CPU_SA_1100)
		pUartPort = new Sa1110UartPort(Sa1110UartPort::port3);
		pUartPort->configure(38400, 8, 1, false, false);
		uartSynchronizer.resume();
	#endif

	#if defined(USE_USB) && defined(__TARGET_CPU_ARM920T)
		pUsbPort = new Mx1UsbPort();
		usbSynchronizer.resume();
	#endif

	#if defined(USE_USB) && defined(__TARGET_CPU_SA_1100)
		pUsbPort = new Sa1110UsbPort();
		usbSynchronizer.resume();
	#endif
	
	#if defined(USE_USB2) && defined(__TARGET_CPU_SA_1100)
	if (getGeminiBoardRevision() >= 2)
	{
		// clock pin
		Sa1110GpioPin memoryClockPin(26);
		memoryClockPin.setValue(0);
		
		// turns on the clock into the Vision Processor permanently
		UInt32 rtucr = *(volatile UInt32 *)(sa1110SystemControlBase + 0x30008);
		*(volatile UInt32 *)(sa1110SystemControlBase + 0x30008) = 
			((rtucr & ~0xE0000000) | 0x80000000);
		memoryClockPin.configureAsAlternateOutputFunction();
		
		pUsb2Port = new Usb2Port();
		usb2Synchronizer.resume();
	}
	#endif
}

//------------------------------------------------------------------------------------------------
// * BootTask::synchronizeUart
//
// Executes primitive commands sent over UART.
//------------------------------------------------------------------------------------------------

void BootTask::synchronizeUart()
{
	while (true)
	{
		yield();
		
		pUartPort->reset();
		if (pUartPort->isInError())
		{
			continue;
		}
		
		if (!sync(pUartPort))
		{
			continue;
		}
		
		// lock access mutex
		orderMutex.wait();
		pStream = pUartPort;
		
		processCommands();
		orderMutex.signal();
	}
}

//------------------------------------------------------------------------------------------------
// * BootTask::synchronizeUsb
//
// Executes primitive commands sent by over USB.
//------------------------------------------------------------------------------------------------

#if defined(USE_USB)
void BootTask::synchronizeUsb()
{
	while (true)
	{
		yield();
		
		pUsbPort->reset();
		if (pUsbPort->isInError())
		{
			continue;
		}
		
		if (!sync(pUsbPort))
		{
			continue;
		}
		
		// lock access mutex
		orderMutex.wait();
		pStream = pUsbPort;
		
		processCommands();
		orderMutex.signal();
	}
}
#endif

//------------------------------------------------------------------------------------------------
// * BootTask::synchronizeUsb2
//
// Executes primitive commands sent by over USB2.
//------------------------------------------------------------------------------------------------

#if defined(USE_USB2)
void BootTask::synchronizeUsb2()
{
	while (true)
	{
		yield();
		
		pUsb2Port->reset();
		if (pUsb2Port->isInError())
		{
			continue;
		}
		
		if (!sync(pUsb2Port))
		{
			continue;
		}
		
		// lock access mutex
		orderMutex.wait();
		pStream = pUsb2Port;
		
		processCommands();
		orderMutex.signal();
	}
}
#endif

//------------------------------------------------------------------------------------------------
// * Bool BootTask::sync
//
// Detects sync sequence from host software
//------------------------------------------------------------------------------------------------

Bool BootTask::sync(Stream * pPort)
{
	UInt syncCounter = 0;
	UInt timeOutCounter = 0;
	
	// polling data off the Receive FIFO
	while (true)
	{
		UInt8 testByte = 0;
		pPort->read(&testByte, sizeof(testByte));
		if (pPort->isInError())
		{
			break;
		}

		if (testByte == startSyncByte)
		{
			++syncCounter;
			if (syncCounter > 50)
			{
				testByte = stopSyncByte;
				pPort->write(&testByte, sizeof(testByte));
				testByte = 0;
				
				while (true)
				{
					pPort->read(&testByte, sizeof(testByte));
					if (pPort->isInError())
					{
						return false;
					}
					
					if (testByte == startSyncByte)
					{
						continue;
					}
					
					if (testByte == stopSyncByte)
					{
						return true;
					}
										
					return false;
				}
			}
		}
		else
		{
			syncCounter = 0;
			if (++timeOutCounter > 1000)
			{
				break;
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------------------------------
// * void BootTask::processCommands
//
// Process commands to manage firmware
//------------------------------------------------------------------------------------------------

void BootTask::processCommands()
{
	while (true)
	{
		UInt16 packLength, packCrc;
		pStream->read(&packCrc, sizeof(packCrc));
		pStream->read(&packLength, sizeof(packLength));
		
		if (pStream->isInError() || packLength > sizeof(dataBuffer))
		{
			break;
		}
		
		pStream->read(dataBuffer, packLength);
		if (pStream->isInError() || packCrc != crc16Calculator.calculateCrc(dataBuffer, packLength))
		{
			break;
		}
		
		UInt8 status = 1;
		switch (dataBuffer[0])
		{
			case eraseCommand:
				commandErase();
				break;
				
			case executeCommand: 
				commandExecute();
				break;
			
			case readFlashCommand:
				commandReadFlash();
				break;
			
			case readMemoryCommand:
				commandReadMemory();
				break;
			
			case writeFlashCommand:
				status = commandWriteFlash();
				break;
			
			case writeMemoryCommand:
				status = commandWriteMemory();
				break;
				
			case getBootBlockVersion:
				commandGetVersion();
				break;
		};
		
		// acknowlegement
		pStream->write(&status, sizeof(status));
	}
}

//------------------------------------------------------------------------------------------------
// * Bool BootTask::commandErase
//
// Erase memory from particular address
//------------------------------------------------------------------------------------------------

void BootTask::commandErase()
{
	UInt32 address;
	memoryCopy(&address, &dataBuffer[1], sizeof(address));
	pFlash->erase((void *)address);
}

//------------------------------------------------------------------------------------------------
// * Bool BootTask::commandExecute
//
// Executes code from particular address
//------------------------------------------------------------------------------------------------

void BootTask::commandExecute()
{
	UInt32 address;
	memoryCopy(&address, &dataBuffer[1], sizeof(address));
	((void (*)())address)();
}

//------------------------------------------------------------------------------------------------
// * Bool BootTask::commandReadFlash
//
// Read data from the flash
//------------------------------------------------------------------------------------------------

void BootTask::commandReadFlash()
{
	UInt32 address;
	UInt16 length;
	memoryCopy(&address, &dataBuffer[1], sizeof(address));
	memoryCopy(&length,  &dataBuffer[5], sizeof(length));
	
	length = minimum(sizeof(dataBuffer), length);
	pFlash->read(dataBuffer, (const void *)address, length);

	UInt16 dataCrc = crc16Calculator.calculateCrc(dataBuffer, length);
	pStream->write(&dataCrc, sizeof(dataCrc));
	pStream->write(&length, sizeof(length));
	pStream->write(dataBuffer, length);
}

//------------------------------------------------------------------------------------------------
// * Bool BootTask::commandWriteFlash
//
// Write data to the flash
//------------------------------------------------------------------------------------------------

UInt8 BootTask::commandWriteFlash()
{
	UInt32 address;
	UInt16 length;
	memoryCopy(&address, &dataBuffer[1], sizeof(address));
	memoryCopy(&length,  &dataBuffer[5], sizeof(length));
	
	if (length > sizeof(dataBuffer) - 7) //command + address + length
	{
		return 0;
	}

	pFlash->write((void *)address, (const void *)&dataBuffer[7], length);
	return 1;	
}

//------------------------------------------------------------------------------------------------
// * Bool BootTask::commandReadMemory
//
// Read arbitrary memory
//------------------------------------------------------------------------------------------------

void BootTask::commandReadMemory()
{
	UInt32 address;
	UInt16 length;
	UInt8  accessType;
	memoryCopy(&address, &dataBuffer[1], sizeof(address));
	memoryCopy(&length,  &dataBuffer[5], sizeof(length));
	memoryCopy(&accessType, &dataBuffer[7], sizeof(length));

	length = minimum(sizeof(dataBuffer), length);
	switch(accessType)
	{
		default:
		{
			typedef UInt8 AccessValue;
			arrayCopy(
				(AccessValue *)dataBuffer, 
				(AccessValue *)address,
				length / sizeof(AccessValue));
			break;
		}
		case 1:
		{
			typedef UInt16 AccessValue;
			arrayCopy(
				(AccessValue *)dataBuffer,
				(AccessValue *)address,
				length / sizeof(AccessValue));
			break;
		}
		case 2:
		{
			typedef UInt32 AccessValue;
			arrayCopy(
				(AccessValue *)dataBuffer,
				(AccessValue *)address,
				length / sizeof(AccessValue));
			break;
		}
	};

	UInt16 dataCrc = crc16Calculator.calculateCrc(dataBuffer, length);
	pStream->write(&dataCrc, sizeof(dataCrc));
	pStream->write(&length, sizeof(length));
	pStream->write(dataBuffer, length);
}

//------------------------------------------------------------------------------------------------
// * Bool BootTask::commandWriteMemory
//
// Write arbitrary memory
//------------------------------------------------------------------------------------------------

UInt8 BootTask::commandWriteMemory()
{
	UInt32 address;
	UInt16 length;
	UInt8  accessType;
	memoryCopy(&address, &dataBuffer[1], sizeof(address));
	memoryCopy(&length,  &dataBuffer[5], sizeof(length));
	memoryCopy(&accessType, &dataBuffer[7], sizeof(length));
	
	if (length > sizeof(dataBuffer) - 8) //command + address + length + accesstype
	{
		return 0;
	}

	switch(accessType)
	{
		default:
		{
			typedef UInt8 AccessValue;
			arrayCopy(
				(AccessValue *)address,
				(AccessValue *)&dataBuffer[8],
				length / sizeof(AccessValue));
			break;
		}
		case 1:
		{
			typedef UInt16 AccessValue;
			arrayCopy(
				(AccessValue *)address,
				(AccessValue *)&dataBuffer[8],
				length / sizeof(AccessValue));
			break;
		}
		case 2:
		{
			typedef UInt32 AccessValue;
			arrayCopy(
				(AccessValue *)address,
				(AccessValue *)&dataBuffer[8],
				length / sizeof(AccessValue));
			break;
		}
	};

	return 1;
}

//------------------------------------------------------------------------------------------------
// * Bool BootTask::commandGetVersion
//
// Command get version
//------------------------------------------------------------------------------------------------

void BootTask::commandGetVersion()
{
	static const Char versionData[] =
		"V10.0 "
		#if defined(__TARGET_CPU_ARM920T)
			"MX1"
		#endif
		#if defined(__TARGET_CPU_SA_1100)
			"SA1110"
		#endif
		" Build 1, " __TIME__ " " __DATE__;
	
	UInt16 versionLength = sizeof(versionData);
	UInt16 dataCrc = crc16Calculator.calculateCrc(versionData, versionLength);
	
	pStream->write(&dataCrc, sizeof(dataCrc));
	pStream->write(&versionLength, sizeof(versionLength));
	pStream->write(versionData, versionLength);
}
