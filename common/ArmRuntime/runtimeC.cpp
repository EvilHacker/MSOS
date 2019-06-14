#include "cPrimitiveTypes.h"
#include "runtime.h"
#include "../multitasking/Mutex.h"
#include "../multitasking/LockedSection.h"
#include "../Devices/WatchdogSection.h"
#if defined(__TARGET_CPU_SA_1100)
	#include "../Sa1110Devices/Sa1110GpioOutput.h"
#endif
#if defined(__TARGET_CPU_ARM920T)
	#include "../Mx1Devices/Mx1GpioOutput.h"
#endif
extern "C"
{
	#include "heap.h"
}

Heap_Descriptor *pGlobalHeap;
Mutex heapMutex;

//------------------------------------------------------------------------------------------------
// * initializeHeap
//
// Initializes the heap.
//------------------------------------------------------------------------------------------------

void initializeHeap(UInt heapBaseAddress, UInt heapLimitAddress)
{
	pGlobalHeap = (Heap_Descriptor *)heapBaseAddress;
	Heap_Initialise(pGlobalHeap, null, null, null, null);
	Heap_InitMemory(pGlobalHeap, pGlobalHeap + 1,
		heapLimitAddress - heapBaseAddress - sizeof(Heap_Descriptor));
}

//------------------------------------------------------------------------------------------------
// * operator new
//
// Allocates memory from the heap.
//------------------------------------------------------------------------------------------------

void *operator new(UInt size)
{
	if(size == 0)
	{
		return null;
	}
	else
	{
		void *pMemory;

		// check if the RTOS has started
		if(TaskScheduler::isInitialized())
		{
			// RTOS is running, allocate within a lock
			LockedSection heapLock(heapMutex);
			pMemory = Heap_Alloc(pGlobalHeap, size);
		}
		else
		{
			// RTOS not yet started, allocate without locking
			pMemory = Heap_Alloc(pGlobalHeap, size);
		}

		if(pMemory == null)
		{
			// not enough memory to allocate
			handleFatalError();
		}

		return pMemory;
	}
}

//------------------------------------------------------------------------------------------------
// * operator delete
//
// Deallocates memory in the heap.
//------------------------------------------------------------------------------------------------

void operator delete(void *pMemory)
{
	if(pMemory != null)
	{
		// check if the RTOS has started
		if(TaskScheduler::isInitialized())
		{
			// RTOS is running, deallocate within a lock
			LockedSection heapLock(heapMutex);
			Heap_Free(pGlobalHeap, pMemory);
		}
		else
		{
			// RTOS not yet started, deallocate without locking
			Heap_Free(pGlobalHeap, pMemory);
		}
	}
}

//------------------------------------------------------------------------------------------------
// * malloc
//
// Allocates memory from the heap.
//------------------------------------------------------------------------------------------------

extern "C" void *malloc(UInt size)
{
	return operator new(size);
}

//------------------------------------------------------------------------------------------------
// * free
//
// Deallocates memory in the heap.
//------------------------------------------------------------------------------------------------

extern "C" void free(void *pMemory)
{
	operator delete(pMemory);
}

//------------------------------------------------------------------------------------------------
// * operator new[]
//
// Allocates memory from the heap.
//------------------------------------------------------------------------------------------------

void *operator new[](UInt size)
{
	return operator new(size);
}

//------------------------------------------------------------------------------------------------
// * operator delete[]
//
// Deallocates memory in the heap.
//------------------------------------------------------------------------------------------------

void operator delete[](void *pMemory)
{
	operator delete(pMemory);
}

//------------------------------------------------------------------------------------------------
// * __pvfn
//
// Pure virtual function.
//------------------------------------------------------------------------------------------------

void __pvfn()
{
	handleFatalError();
}

//------------------------------------------------------------------------------------------------
// * handleFatalError
//
// Handle an unrecoverable error condition.
//------------------------------------------------------------------------------------------------

void handleFatalError()
{
	// reset the device after 3 seconds
	WatchdogSection watchdogSection(3000);

	#if defined(__TARGET_CPU_SA_1100)
		// LEDs on StrongARM Gemini device
		Sa1110GpioOutput greenLed(7);
		Sa1110GpioOutput redLed(6);
	#endif

	#if defined(__TARGET_CPU_ARM920T)
		// LEDs on DragonBall MX1 Gemini device
		Mx1GpioOutput greenLed(Mx1GpioOutput::portD, 16);
		Mx1GpioOutput redLed(Mx1GpioOutput::portD, 15);
	#endif

	// flash some LEDs
	while(true)
	{
		// on
		greenLed.turnOn();
		redLed.turnOn();
		for(volatile UInt i = 0; i < 1100000; ++i);

		// off
		greenLed.turnOff();
		redLed.turnOff();
		for(volatile UInt i = 0; i < 1000000; ++i);
	}
}

//------------------------------------------------------------------------------------------------
// * __rt_errno_addr
//
// C library support.
//------------------------------------------------------------------------------------------------

static Int errno = 0;
extern "C" Int *__rt_errno_addr()
{
	return &errno;
}

//------------------------------------------------------------------------------------------------
// * __set_errno
//
// C library support.
//------------------------------------------------------------------------------------------------

extern "C" void __set_errno(Int newErrno)
{
	errno = newErrno;
}

//------------------------------------------------------------------------------------------------
// * __rt_fp_status_addr
//
// Floating-point library support.
//------------------------------------------------------------------------------------------------

extern "C" UInt *__rt_fp_status_addr()
{
	static UInt floatingPointStatus = 0;
	return &floatingPointStatus;
}

//------------------------------------------------------------------------------------------------
// * _fp_trap
//
// Floating-point library support.
//------------------------------------------------------------------------------------------------

extern "C" void _fp_trap()
{
	handleFatalError();
}

// all other runtime functions taken from c_a__un.l and cpprt_a_mun.l using the following command lines:
// armar -x c_a__un.l rt_memclr_w.o rt_memcpy_w.o rt_memcpy.o rt_memmove_w.o rt_memmove.o rt_memset.o uwrite4.o uread4.o
// armar -x c_a__un.l ll* _ll_*
// armar -x cpprt_a_mun.l vecctor.o vecdtor.o vecdtor2.o vc1c.o vc1ci.o vc2.o
// armar -x c_a__un.l strcpy.o atoi.o abs.o strncpy.o memcmp.o strlen.o strcat.o tolower.o strcmp.o strncmp.o postlj.o strtol.o _strtoul.o _chval.o lc_ctype_c.o rt_ctype_table.o libspace.o
