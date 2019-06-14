/*
 * heap2.h
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:44Z $
 * Revising $Author: marks $
 */

/* Header for a type (1a) large block heap
This sort of heap maintains a linked-list of free blocks of memory.
Each free block has this format:
<size><link><(size-8) unused bytes (multiple of 4)>
<link>=0 indicates end of list
This list is maintained in memory address order.
*/
#ifdef _HEAP2_H
#  error "heap2.h has been included more than once"
#else
#define _HEAP2_H

#include "heapCommon.h"

typedef struct Heap2_ProvidedBlock
{
    size_t size;
    struct Heap2_ProvidedBlock *next;
} Heap2_ProvidedBlock;

typedef struct Heap2_FreeBlock
{
    size_t size;    /* Size is always a multiple of 4, so also use bottom bit to indicate freeness */
    struct Heap2_FreeBlock *next;
} Heap2_FreeBlock;
#define Heap2_FreeMark 1

typedef struct Heap2_Descriptor
{
    int (*full)(void *, size_t);
    void *fullparam;
    void (*broken)(void *);
    void *brokenparam;
    Heap2_ProvidedBlock *providedchain;
    Heap2_FreeBlock *freechain;
} Heap2_Descriptor;

#define inline __inline

extern void __Heap2_Initialise(Heap2_Descriptor *h, int (*full)(void *, size_t), void *fullparam, void (*broken)(void *), void *brokenparam);
extern void __Heap2_ProvideMemory(Heap2_Descriptor *h, void *blk, size_t size);
extern void *__Heap2_Alloc(Heap2_Descriptor *h, size_t size);
extern void __Heap2_Free(Heap2_Descriptor *h, void *blk);
extern void *__Heap2_Realloc(Heap2_Descriptor *h, void *_blk, size_t size);
extern void *__Heap2_Stats(int (*pr)(char const *format, ...), Heap2_Descriptor *h);
#define __Heap2_TraceAlloc(h,s,f,l) __Heap2_Alloc(h,s)
#endif

/* End of file heap2.h */
