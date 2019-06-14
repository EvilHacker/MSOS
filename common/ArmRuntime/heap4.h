/*
 * heap4.h
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:45Z $
 * Revising $Author: marks $
 */

/* Header for a type (4) small block heap based on a large block heap
*/
#ifdef _HEAP4_H
 #error "heap4.h has been included more than once"
#else
#define _HEAP4_H

#include "heapCommon.h"

#ifndef HEAP_BASETYPE
#  define HEAP4_BASETYPE 3
#endif

#if HEAP4_BASETYPE==1
#ifndef _HEAP1A_H
#  include "heap1.h"
#endif
#define Heap4_BaseDescriptor    Heap1_Descriptor
#define Heap4_BaseInitialise    __Heap1_Initialise
#define Heap4_BaseProvideMemory __Heap1_ProvideMemory
#define Heap4_BaseAlloc         __Heap1_Alloc
#define Heap4_BaseFree          __Heap1_Free
#define Heap4_BaseRealloc       __Heap1_Realloc
#define Heap4_BaseStats         __Heap1_Stats

#elif HEAP4_BASETYPE==2
#ifndef _HEAP2_H
# include "heap2.h"
#endif
#define Heap4_BaseDescriptor    Heap2_Descriptor
#define Heap4_BaseInitialise    __Heap2_Initialise
#define Heap4_BaseProvideMemory __Heap2_ProvideMemory
#define Heap4_BaseAlloc         __Heap2_Alloc
#define Heap4_BaseFree          __Heap2_Free
#define Heap4_BaseRealloc       __Heap2_Realloc
#define Heap4_BaseStats         __Heap2_Stats

#elif HEAP4_BASETYPE==3
#ifndef _HEAP3_H
# include "heap3.h"
#endif
#define Heap4_BaseDescriptor    Heap3_Descriptor
#define Heap4_BaseInitialise    __Heap3_Initialise
#define Heap4_BaseProvideMemory __Heap3_ProvideMemory
#define Heap4_BaseAlloc         __Heap3_Alloc
#define Heap4_BaseFree          __Heap3_Free
#define Heap4_BaseRealloc       __Heap3_Realloc
#define Heap4_BaseStats         __Heap3_Stats

#else
#  error "Bad value for HEAP4_BASETYPE"
#endif

typedef struct Heap4_FreeBlock
{
    size_t size;
    struct Heap4_FreeBlock *next;
} Heap4_FreeBlock;
#define Heap4_Marker 2

typedef struct Heap4_Link
{
    struct Heap4_Link *next;
    struct Heap4_Link *prev;
} Heap4_Link;

typedef struct Heap4_ListHeader
{
    Heap4_Link *next;
    Heap4_Link *marker;
    Heap4_Link *prev;
} Heap4_ListHeader;

typedef struct Heap4_BucketChain
{
    Heap4_ListHeader used;
    Heap4_ListHeader empty;
} Heap4_BucketChain;

typedef struct Heap4_BucketHeader
{
    Heap4_Link link;        /* this must remain at the header's start */
    Heap4_FreeBlock *free;
    int usedcount;
    Heap4_BucketChain *ch;
} Heap4_BucketHeader;

/* Note there must be at least as many bucket descriptions in Heap4.c as this */
#define HEAP4_BUCKETCOUNT 12

typedef struct Heap4_Descriptor
{
    Heap4_BaseDescriptor base;
    int (*full)(void *, size_t);
    void *fullparam;
    void (*broken)(void *);
    void *brokenparam;
    Heap4_BucketChain array[HEAP4_BUCKETCOUNT];
} Heap4_Descriptor;

extern void __Heap4_Initialise(Heap4_Descriptor *h, int (*full)(void *, size_t), void *fullparam, void (*broken)(void *), void *brokenparam);
extern void __Heap4_ProvideMemory(Heap4_Descriptor *h, void *blk, size_t size);
extern void *__Heap4_Alloc(Heap4_Descriptor *h, size_t size);
extern void __Heap4_Free(Heap4_Descriptor *h, void *blk);
extern void *__Heap4_Realloc(Heap4_Descriptor *h, void *_blk, size_t size);
extern void *__Heap4_Stats(int (*pr)(char const *format, ...), Heap4_Descriptor *h);
#define __Heap4_TraceAlloc(h,s,f,l) __Heap4_Alloc(h,s)
#endif

/* End of file heap4.h */
