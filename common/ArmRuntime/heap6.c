/*
 * heap6.c
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:46Z $
 * Revising $Author: marks $
 */

/* Code for a type (6) traced heap based on any sort of heap.

The traced heap allocates more than requested, using the extra bytes to
stored a char * and a int (notionally source file and line).
*/
#ifndef _HEAP6_H
 #include "heap6.h"
#endif

#ifndef PARANOID
 #define PARANOID 0
#endif

struct Heap6_AllocedBlock
{
    Heap6_AllocedBlock *next;
    Heap6_AllocedBlock *prev;
    char const *f;
    int l;
};

#ifdef extend_c

/* <Heap6_Initialise>
 * Initialise the type 6 heap
 * 13-06-1997 JSR Written
 */
extern void __Heap6_Initialise(
    Heap6_Descriptor *h,
    int (*full)(void *, size_t), void *fullparam,
    void (*broken)(void *), void *brokenparam)
{
    Heap6_BaseInitialise(&h->base, full, fullparam, broken, brokenparam);

    /* Initialise empty list */
    h->next = (Heap6_AllocedBlock *)&h->marker;
    h->marker = 0;
    h->prev = (Heap6_AllocedBlock *)&h->next;
}


/* <Heap6_ProvideMemory>
 * Provide memory to the heap
 * 13-06-1997 JSR Written
 */
extern void __Heap6_ProvideMemory(Heap6_Descriptor *h, void *_blk, size_t size)
{
    /* Just give the memory to our base heap */
    Heap6_BaseProvideMemory(&h->base, _blk, size);
}

#endif

#ifdef alloc_c

/* <Heap6_Alloc>
 * Allocate some memory from the type 6 heap
 * 13-06-1997 JSR Written
 */
extern void *__Heap6_Alloc(Heap6_Descriptor *h, size_t size)
{
    return __Heap6_TraceAlloc(h, size, 0, 0);
}


/* <Heap6_TraceAlloc>
 * Allocate some memory from the type 6 heap
 * 13-06-1997 JSR Written
 */
extern void *__Heap6_TraceAlloc(Heap6_Descriptor *h, size_t size, char const *f, int l)
{
    Heap6_AllocedBlock *blk = (Heap6_AllocedBlock *)Heap6_BaseAlloc(&h->base, size + sizeof(Heap6_AllocedBlock));

    if (blk == NULL)
    {
        return blk;
    }

    /* fill in details */
    blk->f = f;
    blk->l = l;

    /* Add to list head */
    blk->next = h->next;
    blk->prev = (Heap6_AllocedBlock *)&h->next;
    blk->next->prev = blk->prev->next = blk;

    return blk+1;
}

#endif

#ifdef free_c

/* <Heap6_Free>
 * Free some memory back to the heap
 * 13-06-1997 JSR Written
 */
extern void __Heap6_Free(Heap6_Descriptor *h, void *_blk)
{
    Heap6_AllocedBlock *blk = (Heap6_AllocedBlock *)_blk - 1;

    /* Unlink from list */
    blk->prev->next = blk->next;
    blk->next->prev = blk->prev;    

    Heap6_BaseFree(&h->base, blk);
}

#endif

#ifdef realloc_c

/* <Heap6_Realloc>
 * Re-allocate the given block, ensuring the newly allocated block has the same
 * min(newsize,oldsize) bytes at its start
 * 13-06-1997 JSR Written
 */
extern void *__Heap6_Realloc(Heap6_Descriptor *h, void *_blk, size_t size)
{
    Heap6_AllocedBlock *blk = (Heap6_AllocedBlock *)_blk - 1;
    Heap6_AllocedBlock *newblk;

    /* Unlink from list */
    blk->prev->next = blk->next;
    blk->next->prev = blk->prev;    

    newblk = (Heap6_AllocedBlock *)Heap6_BaseRealloc(&h->base, blk, size + sizeof(Heap6_AllocedBlock));
    if (newblk != NULL)
    {
        /* Add to list head */
        newblk->next = h->next;
        newblk->prev = (Heap6_AllocedBlock *)&h->next;
        newblk->next->prev = newblk->prev->next = newblk;

        return newblk+1;
    }

    return 0;
}

#endif

#ifdef stats_c

/* <Heap6_Stats>
 * Print stats on the given heap
 * 13-06-1997 JSR Written
 */
extern void *__Heap6_Stats(int (*dprint)(char const *format, ...), Heap6_Descriptor *h)
{
    Heap6_AllocedBlock *rover;

    if (h->next->next != NULL)
    {
        dprint("Alloc list not empty\n");
        for (rover = h->next; rover->next != NULL; rover = rover->next)
        {
            dprint("%p f=%s l=%d\n", rover+1, rover->f, rover->l);
        }
    }

    return Heap6_BaseStats(dprint, &h->base);
}

#endif

/* End of file heap6.c */