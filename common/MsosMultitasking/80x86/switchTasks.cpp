#include "switchTasks.h"

//------------------------------------------------------------------------------------------------
// * switchTasks
//
// Switches tasks.
// The current task's stack pointer is saved in <*ppFromStack>.
// The new task's stack pointer is taken from <*ppToStack>.
//------------------------------------------------------------------------------------------------

#pragma optimize("g", off)
void switchTasks(void **ppFromStack, void *const *ppToStack)
{
	__asm__
	{
		// simulate an interrupt to interruptTaskSwitch
		pushfd
		call	interruptTaskSwitch
	
#if defined(__i386__)
		jmp		exit
#else	
	}

	// continue when we switch back to this task
	return;

	__asm__
	{
#endif

	interruptTaskSwitch:
		// save all registers of the current task
#if defined(__i386__)
		pushal
		sub		esp, 512 + 16
		mov		eax, esp
		and		eax, 0xFFFFFFF0
		fxsave	[eax + 16]
#else
		pushad
#endif

		// save the current task's stack pointer
		mov		ebx, [ppFromStack]
		mov		[ebx], esp

		// get the new task's stack pointer
		mov		ebx, [ppToStack]
		mov		esp, [ebx]

		// restore all registers of the new task
#if defined(__i386__)
		mov		eax, esp
		and		eax, 0xFFFFFFF0
		fxrstor	[eax + 16]
		add		esp, 512 + 16
		popal
#else
		popad
#endif

		// simulate a return from interrupt
		push	[esp + 4]
		popfd
		ret		4
	exit:
	}
}
#pragma optimize("", on)
