/*
 * heap1a.h
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:43Z $
 * Revising $Author: marks $
 */

/* Header for a type (1) large block heap
This sort of heap maintains a linked-list of free blocks of memory.
Each free block has this format:
<size><link><(size-8) unused bytes (multiple of 4)>
<link>=0 indicates end of list
This list is maintained in memory address order.
*/
#ifdef _HEAP1_H
#  error "heap1.h has been included more than once"
#else
#  define _HEAP1_H

#include "heapCommon.h"

typedef struct Heap1_FreeBlock
{
    size_t size;
    struct Heap1_FreeBlock *next;
} Heap1_FreeBlock;

typedef struct Heap1_Descriptor
{
    int (*full)(void *, size_t);
    void *fullparam;
    void (*broken)(void *);
    void *brokenparam;
    Heap1_FreeBlock freechain;
    Heap1_FreeBlock *lastprev;
} Heap1_Descriptor;

#define inline __inline

extern void __Heap1_Initialise(Heap1_Descriptor *h, int (*full)(void *, size_t), void *fullparam, void (*broken)(void *), void *brokenparam);
extern void __Heap1_InitMemory(Heap1_Descriptor *h, void *blk, size_t size);
extern void __Heap1_ProvideMemory(Heap1_Descriptor *h, void *blk, size_t size);
extern void *__Heap1_Alloc(Heap1_Descriptor *h, size_t size);
extern void __Heap1_Free(Heap1_Descriptor *h, void *blk);
extern void *__Heap1_Realloc(Heap1_Descriptor *h, void *_blk, size_t size);
extern void *__Heap1_Stats(int (*pr)(char const *format, ...), Heap1_Descriptor *h);
#define __Heap1_TraceAlloc(h,s,f,l) __Heap1_Alloc(h,s)

#endif

/* End of file heap1.h */
