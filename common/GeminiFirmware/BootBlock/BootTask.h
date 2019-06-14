#ifndef _BootTask_h_
#define _BootTask_h_

#include "../../cPrimitiveTypes.h"
#include "../../pointerArithmetic.h" 
#include "../../memoryUtilities.h"

#include "../../multitasking/Task.h"
#include "../../multitasking/TaskScheduler.h"
#include "../../multitasking/Mutex.h"
#include "../../devices/deviceAddresses.h"
#include "../../devices/Flash.h"
#if defined(__TARGET_CPU_ARM920T)
	#include "../../Mx1Devices/Mx1UartPort.h"
	#if defined(USE_USB)
		#include "../../Mx1Devices/Mx1UsbPort.h"
	#endif
#endif
#if defined(__TARGET_CPU_SA_1100)
	#include "../../Sa1110Devices/Sa1110UartPort.h"
	#if defined(USE_USB)
		#include "../../Sa1110Devices/Sa1110UsbPort.h"
	#endif
#endif
#if defined(USE_USB2)
	#include "../../Devices/Usb2Port.h"
#endif
#include "../../crc/CrcCalculator.h" 
#include "../../multitasking/MemberTask.h"

//------------------------------------------------------------------------------------------------
// * class BootTask
//
// Checks for and runs any firmware programmed in flash memory.
// Executes primitive commands sent by host.
//------------------------------------------------------------------------------------------------

class BootTask
{
	friend class FirmwareLoaderTask;
	
public:
	// constructor
	BootTask();

private:
	// task entry point
	void initializeCommunication();
	Bool sync(Stream * pPort);
	void processCommands();
	
	void commandErase();
	void commandExecute();
	void commandReadFlash();
	void commandReadMemory();
	UInt8 commandWriteFlash();
	UInt8 commandWriteMemory();
	void commandGetVersion();
	
	// variables
	Mutex orderMutex;
	Flash * pFlash;
	Stream * pStream;

	// communication
	#if defined(__TARGET_CPU_ARM920T)
		Mx1UartPort * pUartPort;
	#endif
	#if defined(__TARGET_CPU_SA_1100)
		Sa1110UartPort * pUartPort;
	#endif
	#if defined(USE_USB) && defined(__TARGET_CPU_ARM920T)
		Mx1UsbPort * pUsbPort;
	#endif
	#if defined(USE_USB) && defined(__TARGET_CPU_SA_1100)
		Sa1110UsbPort * pUsbPort;
	#endif
	#if defined(USE_USB2)
		Usb2Port * pUsb2Port;
	#endif
	
	void synchronizeUart();
	MemberTask(UartPortSynchronizer, BootTask, synchronizeUart) uartSynchronizer;

	#if defined(USE_USB)
		void synchronizeUsb();
		MemberTask(UsbPortSynchronizer, BootTask, synchronizeUsb) usbSynchronizer;
	#endif

	#if defined(USE_USB2)
		void synchronizeUsb2();
		MemberTask(Usb2PortSynchronizer, BootTask, synchronizeUsb2) usb2Synchronizer;
	#endif

	// data buffer
	UInt8 dataBuffer[1024];
	
	// constants
	static const UInt8 stopSyncByte = 0x69;
	static const UInt8 startSyncByte = 0x96;
	
	enum bootCommands
	{
		eraseCommand = 1,
		executeCommand = 2,
		readFlashCommand = 3,
		writeFlashCommand = 4,
		readMemoryCommand = 5,
		writeMemoryCommand = 6,
		getBootBlockVersion = 7
	};
};

//------------------------------------------------------------------------------------------------
// * class FirmwareLoaderTask
//
// Load firmware programmed in flash memory.
//------------------------------------------------------------------------------------------------

class FirmwareLoaderTask : public Task
{
public:
	// constructor
	FirmwareLoaderTask(BootTask * pBootTask);
	
private:
	// task entry point
	void main();
	
	// return firmware address
	UInt32 checkLoadFirmware();
	
	// variables
	Flash * pFlash;
	BootTask * pBootTask;
	
public:

	static const UInt32 flashBlockSize = 0x10000;
	static const UInt32 magicFirmwareId = 0xC301070B;
};

#endif // _BootTask_h_
