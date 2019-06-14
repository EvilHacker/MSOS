/*
 * heap6.h
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:46Z $
 * Revising $Author: marks $
 */

/* Header for a type (6) traced heap, based on any other sort of heap
*/
#ifdef _HEAP6_H
#  error "heap6.h has been included more than once"
#else
#define _HEAP6_H

#include "heapCommon.h"

#define HEAP6_BASETYPE 4

#if HEAP6_BASETYPE==1
#ifndef _HEAP1_H
#  include "heap1.h"
#endif
#define Heap6_BaseDescriptor    Heap1_Descriptor
#define Heap6_BaseInitialise    __Heap1_Initialise
#define Heap6_BaseProvideMemory __Heap1_ProvideMemory
#define Heap6_BaseAlloc         __Heap1_Alloc
#define Heap6_BaseFree          __Heap1_Free
#define Heap6_BaseRealloc       __Heap1_Realloc
#define Heap6_BaseStats         __Heap1_Stats

#elif HEAP6_BASETYPE==2
#ifndef _HEAP2_H
#  include "heap2.h"
#endif
#define Heap6_BaseDescriptor    Heap2_Descriptor
#define Heap6_BaseInitialise    __Heap2_Initialise
#define Heap6_BaseProvideMemory __Heap2_ProvideMemory
#define Heap6_BaseAlloc         __Heap2_Alloc
#define Heap6_BaseFree          __Heap2_Free
#define Heap6_BaseRealloc       __Heap2_Realloc
#define Heap6_BaseStats         __Heap2_Stats

#elif HEAP6_BASETYPE==3
#ifndef _HEAP3_H
#  include "heap3.h"
#endif
#define Heap6_BaseDescriptor    Heap3_Descriptor
#define Heap6_BaseInitialise    __Heap3_Initialise
#define Heap6_BaseProvideMemory __Heap3_ProvideMemory
#define Heap6_BaseAlloc         __Heap3_Alloc
#define Heap6_BaseFree          __Heap3_Free
#define Heap6_BaseRealloc       __Heap3_Realloc
#define Heap6_BaseStats         __Heap3_Stats

#elif HEAP6_BASETYPE==4
#ifndef _HEAP4_H
#  include "heap4.h"
#endif
#define Heap6_BaseDescriptor    Heap4_Descriptor
#define Heap6_BaseInitialise    __Heap4_Initialise
#define Heap6_BaseProvideMemory __Heap4_ProvideMemory
#define Heap6_BaseAlloc         __Heap4_Alloc
#define Heap6_BaseFree          __Heap4_Free
#define Heap6_BaseRealloc       __Heap4_Realloc
#define Heap6_BaseStats         __Heap4_Stats

#else
#  error "Bad value for HEAP6_BASETYPE"
#endif

typedef struct Heap6_AllocedBlock Heap6_AllocedBlock;

typedef struct Heap6_Descriptor
{
    Heap6_BaseDescriptor base;
    Heap6_AllocedBlock *next;
    Heap6_AllocedBlock *marker;
    Heap6_AllocedBlock *prev;
} Heap6_Descriptor;

extern void __Heap6_Initialise(Heap6_Descriptor *h, int (*full)(void *, size_t), void *fullparam, void (*broken)(void *), void *brokenparam);
extern void __Heap6_ProvideMemory(Heap6_Descriptor *h, void *blk, size_t size);
extern void *__Heap6_Alloc(Heap6_Descriptor *h, size_t size);
extern void __Heap6_Free(Heap6_Descriptor *h, void *blk);
extern void *__Heap6_Realloc(Heap6_Descriptor *h, void *_blk, size_t size);
extern void *__Heap6_Stats(int (*pr)(char const *format, ...), Heap6_Descriptor *h);
extern void *__Heap6_TraceAlloc(Heap6_Descriptor *h, size_t size, char const *f, int l);
#endif

/* End of file heap6.h */
