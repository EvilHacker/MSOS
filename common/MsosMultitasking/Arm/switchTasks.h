#ifndef _switchTasks_h_
#define _switchTasks_h_

extern "C"
{
	void switchTasks(void **ppFromStack, void *const *ppToStack);
}

#endif // _switchTasks_h_
