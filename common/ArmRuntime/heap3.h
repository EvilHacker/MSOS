/*
 * heap3.h
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:44Z $
 * Revising $Author: marks $
 */

/* Header for a type 3 large block heap
This sort of heap maintains two 2-3 trees referencing the free blocks.
One tree is sorted by (size,address) and is for finding blocks on allocation
and the other tree is sorted by (address) and is for merging on free.
Each free block has this format:
<size><tree node><tree node><unused bytes>
Two nodes per free block are needed to ensure sufficient nodes to generate
the two trees.
*/
#ifdef _HEAP3_H
 #error "heap3.h has been included more than once"
#else
#define _HEAP3_H

#include "heapCommon.h"

/*
These structures define a 2-way linked list where removing a link does not
require special handling for the list ends.
*/
typedef struct Heap3_Link Heap3_Link;
typedef struct Heap3_List Heap3_List;
typedef struct Heap3_Node Heap3_Node;
typedef struct Heap3_Tree Heap3_Tree;
typedef struct Heap3_FreeBlock Heap3_FreeBlock;

struct Heap3_Link
{
    Heap3_Link *next;
    Heap3_Link *prev;
};

struct Heap3_List
{
    Heap3_Link *next;
    Heap3_Link *marker;
    Heap3_Link *prev;
};

/*
These are the 2-3 tree structures.
*/
typedef union Heap3_Unit
{
    Heap3_FreeBlock *value;
    Heap3_Node *child;
} Heap3_Unit;

struct Heap3_Node
{
    Heap3_Node *parent;
    Heap3_FreeBlock *value;
    Heap3_Unit u0;  /* This is 0 for leaf nodes (needed for node mobility), relying on value for the value */
    Heap3_Unit u1;
    Heap3_Unit u2;
};

typedef int Heap3_Comparator(Heap3_FreeBlock const *, Heap3_FreeBlock const *);

struct Heap3_Tree
{
    Heap3_Comparator *compare;
    Heap3_Node *root;
    Heap3_List freenodes;
};

/*
This ensapsulation structure is used for 2-3 tree nodes on the 2-3 tree node
free list. When the free block is allocated any nodes in use inside it will
need allocation from the free node list and copying across. As each free
block supplies two 2-3 nodes there are guaranteed to be enough nodes for
this to succeed.
*/
typedef union Heap3_FreeNode
{
    struct
    {
        int inuse;      /* 1 => unused; other (even) value => used */
        Heap3_Link link;
    } freed;
    Heap3_Node used;
} Heap3_FreeNode;


struct Heap3_FreeBlock
{
    size_t size;
    Heap3_FreeNode n0;
    Heap3_FreeNode n1;
};
/* When these are set in size, that indicates that n0 and n1 are in use respectively */

typedef struct Heap3_Descriptor
{
    int (*full)(void *, size_t);
    void *fullparam;
    void (*broken)(void *);
    void *brokenparam;
    Heap3_FreeBlock startnodes;
    Heap3_Tree free_bysize;
    Heap3_Tree free_byaddress;
} Heap3_Descriptor;

#define inline __inline

extern void __Heap3_Initialise(Heap3_Descriptor *h, int (*full)(void *, size_t), void *fullparam, void (*broken)(void *), void *brokenparam);
extern void __Heap3_ProvideMemory(Heap3_Descriptor *h, void *blk, size_t size);
extern void *__Heap3_Alloc(Heap3_Descriptor *h, size_t size);
extern void __Heap3_Free(Heap3_Descriptor *h, void *blk);
extern void *__Heap3_Realloc(Heap3_Descriptor *h, void *_blk, size_t size);
extern void *__Heap3_Stats(int (*pr)(char const *format, ...), Heap3_Descriptor *h);
#define __Heap3_TraceAlloc(h,s,f,l) __Heap3_Alloc(h,s)
#endif

/* End of file heap3.h */

