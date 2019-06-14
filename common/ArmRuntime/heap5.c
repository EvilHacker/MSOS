/*
 * heap5.c
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:45Z $
 * Revising $Author: marks $
 */

/* Code for a type (5) guarded heap based on any sort of heap.

The guarded heap allocates more than requested, using the extra bytes to place
guard patterns around the requested amount.
*/
#ifndef _HEAP5_H
#  include "heap5.h"
#endif

#ifndef PARANOID
 #define PARANOID 0
#endif

typedef struct Heap5_Marker
{
    unsigned char val[8];
} Heap5_Marker;

typedef struct Heap5_AllocedBlock
{
    Heap5_Marker *endmark;
    Heap5_Marker startmark;
} Heap5_AllocedBlock;

#define Heap5_Extra (sizeof(Heap5_AllocedBlock) + sizeof(Heap5_Marker))

void __Heap5_MarkBlock(Heap5_AllocedBlock *blk, size_t size);
void __Heap5_CheckBlock(Heap5_Descriptor *h, Heap5_AllocedBlock *blk);

/* A bunch of random numbers to use as a start and end marker */
static Heap5_Marker const Heap5_MarkerValue =
{
    { 0x87, 0x37, 0x1c, 0x9c, 0xa5, 0x62, 0xe9, 0x78 }
};

#ifdef extend_c

/* <Heap5_Initialise>
 * Initialise the type 4 heap
 * 13-06-1997 JSR Written
 */
extern void __Heap5_Initialise(
    Heap5_Descriptor *h,
    int (*full)(void *, size_t), void *fullparam,
    void (*broken)(void *), void *brokenparam)
{
    Heap5_BaseInitialise(&h->base, full, fullparam, broken, brokenparam);
    h->broken = broken;
    h->brokenparam = brokenparam;
}


/* <Heap5_ProvideMemory>
 * Provide memory to the heap
 * 13-06-1997 JSR Written
 */
extern void __Heap5_ProvideMemory(Heap5_Descriptor *h, void *_blk, size_t size)
{
    /* Just give the memory to our base heap */
    Heap5_BaseProvideMemory(&h->base, _blk, size);
}

#endif

#ifdef alloc_c

/* <Heap5_MarkBlock>
 * Mark the given allocated block
 * 13-06-1997 JSR Written
 */
void __Heap5_MarkBlock(Heap5_AllocedBlock *blk, size_t size)
{
    int i;

    blk->startmark = Heap5_MarkerValue;
    blk->endmark = (Heap5_Marker *)((char *)(blk+1) + size);
    for (i = 0; i < sizeof(Heap5_Marker); i++)
    {
        blk->endmark->val[i] = Heap5_MarkerValue.val[i];
    }
}


/* <Heap5_Alloc>
 * Allocate some memory from the type 5 heap
 * 13-06-1997 JSR Written
 */
extern void *__Heap5_Alloc(Heap5_Descriptor *h, size_t size)
{
    Heap5_AllocedBlock *blk = (Heap5_AllocedBlock *)Heap5_BaseAlloc(&h->base, size + Heap5_Extra);

    if (blk == NULL)
    {
        return blk;
    }

    __Heap5_MarkBlock(blk, size);

    return blk+1;
}


/* <Heap5_TraceAlloc>
 * Allocate some memory from the type 5 heap
 * 13-06-1997 JSR Written
 */
extern void *__Heap5_TraceAlloc(Heap5_Descriptor *h, size_t size, char const *f, int l)
{
    Heap5_AllocedBlock *blk = (Heap5_AllocedBlock *)Heap5_BaseTraceAlloc(&h->base, size + Heap5_Extra, f, l);

    if (blk == NULL)
    {
        return blk;
    }

    __Heap5_MarkBlock(blk, size);

    return blk+1;
}

#endif

#ifdef free_c

/* <Heap5_CheckBlock>
 * Check the given block is correctly marked
 * 13-06-1997 JSR Written
 */
void __Heap5_CheckBlock(Heap5_Descriptor *h, Heap5_AllocedBlock *blk)
{
    int i;
    int broken = 0;

    /* Check start mark */
    for (i = 0; i < sizeof(Heap5_Marker); i++)
    {
        if (blk->startmark.val[i] != Heap5_MarkerValue.val[i])
        {
            broken = 1;
            break;
        }
    }

    /* Check end mark */
    for (i = 0; i < sizeof(Heap5_Marker); i++)
    {
        if (blk->endmark->val[i] != Heap5_MarkerValue.val[i])
        {
            broken = 1;
            break;
        }
    }

    /* Phone up if either is busted */
    if (broken)
    {
        h->broken(h->brokenparam);
    }
}


/* <Heap5_Free>
 * Free some memory back to the heap
 * 13-06-1997 JSR Written
 */
extern void __Heap5_Free(Heap5_Descriptor *h, void *_blk)
{
    Heap5_AllocedBlock *blk = (Heap5_AllocedBlock *)_blk - 1;

    __Heap5_CheckBlock(h, blk);

    Heap5_BaseFree(&h->base, blk);
}

#endif

#ifdef realloc_c

/* <Heap5_Realloc>
 * Re-allocate the given block, ensuring the newly allocated block has the same
 * min(newsize,oldsize) bytes at its start
 * 13-06-1997 JSR Written
 */
extern void *__Heap5_Realloc(Heap5_Descriptor *h, void *_blk, size_t size)
{
    Heap5_AllocedBlock *blk = (Heap5_AllocedBlock *)_blk - 1;
    Heap5_AllocedBlock *newblk;

    __Heap5_CheckBlock(h, blk);
    newblk = (Heap5_AllocedBlock *)Heap5_BaseRealloc(&h->base, blk, size + Heap5_Extra);
    if (newblk != NULL)
    {
        __Heap5_MarkBlock(newblk, size);
        return newblk+1;
    }

    return 0;
}

#endif

#ifdef stats_c

/* <Heap5_Stats>
 * Print stats on the given heap
 * 13-06-1997 JSR Written
 */
extern void *__Heap5_Stats(int (*dprint)(char const *format, ...), Heap5_Descriptor *h)
{
    return Heap5_BaseStats(dprint, &h->base);
}

#endif

/* End of file heap5.c */
