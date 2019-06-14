/*
 * heap5.h
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:46Z $
 * Revising $Author: marks $
 */

/* Header for a type (5) guarded heap, based on any other sort of heap
*/
#ifdef _HEAP5_H
#  error "heap5.h has been included more than once"
#else
#define _HEAP5_H

#include "heapCommon.h"

#ifndef HEAP_BASETYPE
#  define HEAP5_BASETYPE 5
#endif

#if HEAP5_BASETYPE==1
#ifndef _HEAP1_H
#  include "heap1.h"
#endif
#define Heap5_BaseDescriptor    Heap1_Descriptor
#define Heap5_BaseInitialise    __Heap1_Initialise
#define Heap5_BaseProvideMemory __Heap1_ProvideMemory
#define Heap5_BaseAlloc         __Heap1_Alloc
#define Heap5_BaseFree          __Heap1_Free
#define Heap5_BaseRealloc       __Heap1_Realloc
#define Heap5_BaseStats         __Heap1_Stats
#define Heap5_BaseTraceAlloc    __Heap1_TraceAlloc

#elif HEAP5_BASETYPE==2
#ifndef _HEAP2_H
#  include "heap2.h"
#endif
#define Heap5_BaseDescriptor    Heap2_Descriptor
#define Heap5_BaseInitialise    __Heap2_Initialise
#define Heap5_BaseProvideMemory __Heap2_ProvideMemory
#define Heap5_BaseAlloc         __Heap2_Alloc
#define Heap5_BaseFree          __Heap2_Free
#define Heap5_BaseRealloc       __Heap2_Realloc
#define Heap5_BaseStats         __Heap2_Stats
#define Heap5_BaseTraceAlloc    __Heap2_TraceAlloc

#elif HEAP5_BASETYPE==3
#ifndef _HEAP3_H
#  include "heap3.h"
#endif
#define Heap5_BaseDescriptor    Heap3_Descriptor
#define Heap5_BaseInitialise    __Heap3_Initialise
#define Heap5_BaseProvideMemory __Heap3_ProvideMemory
#define Heap5_BaseAlloc         __Heap3_Alloc
#define Heap5_BaseFree          __Heap3_Free
#define Heap5_BaseRealloc       __Heap3_Realloc
#define Heap5_BaseStats         __Heap3_Stats
#define Heap5_BaseTraceAlloc    __Heap3_TraceAlloc

#elif HEAP5_BASETYPE==4
#ifndef _HEAP4_H
#  include "heap4.h"
#endif
#define Heap5_BaseDescriptor    Heap4_Descriptor
#define Heap5_BaseInitialise    __Heap4_Initialise
#define Heap5_BaseProvideMemory __Heap4_ProvideMemory
#define Heap5_BaseAlloc         __Heap4_Alloc
#define Heap5_BaseFree          __Heap4_Free
#define Heap5_BaseRealloc       __Heap4_Realloc
#define Heap5_BaseStats         __Heap4_Stats
#define Heap5_BaseTraceAlloc    __Heap4_TraceAlloc

#elif HEAP5_BASETYPE==5
#ifndef _HEAP6_H
#  include "heap6.h"
#endif
#define Heap5_BaseDescriptor    Heap6_Descriptor
#define Heap5_BaseInitialise    __Heap6_Initialise
#define Heap5_BaseProvideMemory __Heap6_ProvideMemory
#define Heap5_BaseAlloc         __Heap6_Alloc
#define Heap5_BaseFree          __Heap6_Free
#define Heap5_BaseRealloc       __Heap6_Realloc
#define Heap5_BaseStats         __Heap6_Stats
#define Heap5_BaseTraceAlloc    __Heap6_TraceAlloc

#else
#  error "Bad value for HEAP5_BASETYPE"
#endif

typedef struct Heap5_Descriptor
{
    Heap5_BaseDescriptor base;
    void (*broken)(void *);
    void *brokenparam;
} Heap5_Descriptor;

extern void __Heap5_Initialise(Heap5_Descriptor *h, int (*full)(void *, size_t), void *fullparam, void (*broken)(void *), void *brokenparam);
extern void __Heap5_ProvideMemory(Heap5_Descriptor *h, void *blk, size_t size);
extern void *__Heap5_Alloc(Heap5_Descriptor *h, size_t size);
extern void __Heap5_Free(Heap5_Descriptor *h, void *blk);
extern void *__Heap5_Realloc(Heap5_Descriptor *h, void *_blk, size_t size);
extern void *__Heap5_Stats(int (*pr)(char const *format, ...), Heap5_Descriptor *h);
extern void *__Heap5_TraceAlloc(Heap5_Descriptor *h, size_t size, char const *f, int l);
#endif

/* End of file heap5.h */
