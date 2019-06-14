#ifndef _heapCommon_h_
#define _heapCommon_h_

// define the type of heap to be used
#ifndef HEAPTYPE
	#define HEAPTYPE 1
#endif

// other 
#ifndef NULL
	#define NULL (0)
#endif
#ifndef alloc_c
	#define alloc_c
#endif
#ifndef free_c
	#define free_c
#endif
#ifndef extend_c
	#define extend_c
#endif
#ifndef size_t
	#define size_t unsigned int
#endif

#endif // _heapCommon_h_
