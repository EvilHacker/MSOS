/*
 * heap3.c
 * Copyright: 1997 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 2001/04/16 20:14:44Z $
 * Revising $Author: marks $
 */

/* Code for a type (3) large block heap

The Heap3 algorithm has:
* The heap fragment structure:
        Alloced                 Free
        size                    size
        <user>                  2-3 tree node
                                        2-3 tree node
                                        <unused>
* Free blocks are maintained in two 2-3 trees. One tree orders them by
        address and is used for rapid block merging. The other tree orders them
        by size and is used for allocation.
* Blocks are added to the free trees when freed, and are merged at this time.
* The provided blocks are freed to the heap, and so automatically merge with
        adjacent free blocks.
* Provided blocks must be:
        A multiple of 4 bytes big
        At least 44 bytes big
        Not starting at address 0, not ending at address 0 and not containing address 0
* Allocation is done as best fit (lowest address in a tie-break).
* Block tops are always trimmed off if possible.
* The 2-3 tree nodes are:
                Branch node             Leaf node
                parent                  parent
                value                   value
                child0                  0
                child1                  value1
                child2                  value2
        The value field is always the least value for the tree at/below a given node.
        The parent field is required so that when a block is alloced its 2-3 tree nodes
        can be moved out of the way - this is also why the value0 field of leaf
        nodes is 0, so that leaves can be distinguished from branches.
        Within a free block the first 2-3 tree node is used only for the size tree
        and the second free block is used only for the address tree.

*/

#ifndef _HEAP3_H
#  include "heap3.h"
#endif

#ifndef PARANOID
#  define PARANOID 0
#endif

Heap3_FreeBlock *__Heap3_NodeFindLeastGT(Heap3_Tree *t, Heap3_FreeBlock *v);
void __Heap3_ClaimBlock(Heap3_Descriptor *h, Heap3_FreeBlock *blk);
void __Heap3_NodeRemove(Heap3_Tree *t, Heap3_FreeBlock *v);

/* <Heap3_NodeFree>
 * Release a Heap3 node to the free node list
 * 05-06-1997 JSR Written
 */
static inline void Heap3_NodeFree(Heap3_Tree *t, Heap3_Node *_n)
{
    Heap3_FreeNode *n = (Heap3_FreeNode *)_n;

    n->freed.inuse = 1;
    n->freed.link.prev = t->freenodes.prev;
    n->freed.link.next = (Heap3_Link *)&t->freenodes.marker;
    t->freenodes.prev->next = &n->freed.link;
    t->freenodes.prev = &n->freed.link;
}


/* <Heap3_NodeAlloc>
 * Allocate a node from the Heap3 node list
 * 05-06-1997 JSR Written
 */
static inline Heap3_Node *Heap3_NodeAlloc(Heap3_Tree *t)
{
    Heap3_FreeNode *n;

    /* This nasty piece of work adds the offset from the address of the freed.link inside
    a Heap_FreeNode to its containing Heap_FreeNode */
    n = ((Heap3_FreeNode *)((char *)(t->freenodes.next)-(char *)&((Heap3_FreeNode *)0)->freed.link));

    /* unlink the node */
    t->freenodes.next = n->freed.link.next;
    n->freed.link.next->prev = n->freed.link.prev;

    return &n->used;
}


/* <Heap3_UnitZero>
 * Zero the given unit
 * 26-03-1997 JSR Written
 */
static inline void Heap3_UnitZero(Heap3_Unit *u)
{
    u->value = 0;
    u->child = 0;
}


/* <Heap3_NodeIndexChanged>
 * Change the child node's index in the parent
 * 05-06-1997 JSR Written
 */
static inline void Heap3_NodeIndexChanged(Heap3_Node *n)
{
    Heap3_Node *parent;

    for ( parent = n->parent;
          parent != NULL && parent->u0.child == n;
          n = parent, parent = n->parent )
    {
        parent->value = n->value;
    }
}


/* <Heap3_SizeAdjust>
 * Adjust the block size to be the whole size of of the required allocation block
 * 06-06-1997 JSR Written
 */
static inline size_t Heap3_SizeAdjust(size_t size)
{
    /* Add allocated block overhead and round up to a multiple of 4 bytes */
    size = (size + sizeof(size_t) + 3) & ~3;
    if (size < sizeof(Heap3_FreeBlock))
    {
        size = sizeof(Heap3_FreeBlock);
    }

    return size;
}


/* <Heap3_TrimBlockTop>
 * Trim the top off the given claimed block
 * 06-06-1997 JSR Written
 */
static inline void Heap3_TrimBlockTop(Heap3_Descriptor *h, Heap3_FreeBlock *blk, size_t size)
{
    Heap3_FreeBlock *fragment;

    if (blk->size - size >= sizeof(Heap3_FreeBlock))
    {
        /* big enough to trim */
        fragment = (Heap3_FreeBlock *)((char *)blk + size);
        fragment->size = blk->size - size;
        blk->size = size;
        __Heap3_Free(h, &fragment->n0);
    }
}


#ifdef alloc_c

/* <Heap3_TreeInitialise>
 * Initialise a Heap3_Tree
 * 05-06-1997 JSR Written
 */
static void Heap3_TreeInitialise(Heap3_Tree *t,Heap3_Node *n,
                                 Heap3_Comparator *compare)
{
    t->compare = compare;
    t->root = n;
    t->freenodes.next = (Heap3_Link *)&t->freenodes.marker;
    t->freenodes.marker = 0;
    t->freenodes.prev = (Heap3_Link *)&t->freenodes.next;

    n->parent = 0;
    n->value = 0;
    Heap3_UnitZero( &n->u0 );
    Heap3_UnitZero( &n->u1 );
    Heap3_UnitZero( &n->u2 );
}


/* <Heap3_CompareSize>
 * Compare two Heap3_FreeBlocks by (size,address)
 * 05-06-1997 JSR Written
 */
static int Heap3_CompareSize(Heap3_FreeBlock const *a, Heap3_FreeBlock const *b)
{
    int res = (int)a->size - (int)b->size;
    if (res != 0)
    {
        return res;
    }

    return (int)a - (int)b;
}


/* <Heap3_CompareAddress>
 * Compare two Heap3_FreeBlocks by (address)
 * 05-06-1997 JSR Written
 */
static int Heap3_CompareAddress(Heap3_FreeBlock const *a, Heap3_FreeBlock const *b)
{
    return (int)a - (int)b;
}


/* <Heap3_Initialise>
 * Initialise the heap
 * 05-06-1997 JSR Written
 */
extern void __Heap3_Initialise(
    Heap3_Descriptor *h,
    int (*full)( void *, size_t ), void *fullparam,
    void (*broken)( void * ), void *brokenparam)
{
    h->full = full;
    h->fullparam = fullparam;
    h->broken = broken;
    h->brokenparam = brokenparam;
    h->startnodes.size = 0;
    Heap3_TreeInitialise( &h->free_bysize, &h->startnodes.n0.used, Heap3_CompareSize );
    Heap3_TreeInitialise( &h->free_byaddress, &h->startnodes.n1.used, Heap3_CompareAddress );
}


/* <Heap3_ProvideMemory>
 * Provide memory to the heap
 * 06-06-1997 JSR Written
 */
extern void __Heap3_ProvideMemory(Heap3_Descriptor *h, void *_blk, size_t size)
{
    Heap3_FreeBlock *blk = (Heap3_FreeBlock *)_blk;

    blk->size = size;
    __Heap3_Free( h, &blk->n0 );
}


/* <Heap3_NodeMergeCheck>
 * Merge/balance the given child if it needs it, and merge/balance
 * its parents if they need it as a consequence
 * 05-06-1997 JSR Written
 */
static inline void Heap3_NodeMergeCheck(Heap3_Tree *t, Heap3_Node *c)
{
    Heap3_Node *n;
    Heap3_Node *other;
    int notleaf = 0;

    for (; c->u1.value == NULL; c = n, notleaf = 1)
    {
        n = c->parent;

        if (n == NULL)
        {
            /* top of tree reached */
            if (c->u0.child != NULL)
            {
                /* More than 1 level left - reduce by one level */
                t->root = c->u0.child;
                t->root->parent = NULL;
                Heap3_NodeFree(t, c);
            }
            return;
        }

        if (n->u0.child == c)
        {
            other = n->u1.child;
            if (other->u2.value != NULL)
            {
                /* Balance u1 into u0 */
                if (notleaf)
                {
                    c->u1 = other->u0;
                    c->u1.child->parent = c;
                    other->u0 = other->u1;
                    other->value = other->u0.child->value;
                }
                else
                {
                    c->u1.value = other->value;
                    other->value = other->u1.value;
                }
                other->u1 = other->u2;
                Heap3_UnitZero(&other->u2);
                return;
            }
            else
            {
                /* Merge u1 into u0 */
                if (notleaf)
                {
                    c->u1 = other->u0;
                    c->u1.child->parent = c;
                    c->u2 = other->u1;
                    c->u2.child->parent = c;
                }
                else
                {
                    c->u1.value = other->value;
                    c->u2 = other->u1;
                }
                Heap3_NodeFree(t, n->u1.child);
                n->u1 = n->u2;
                Heap3_UnitZero(&n->u2);
            }
        }
        else if (n->u1.child == c)
        {
            other = n->u0.child;
            if (other->u2.value != NULL)
            {
                /* balance u0 into u1 */
                if (notleaf)
                {
                    c->u1 = c->u0;
                    c->u0 = other->u2;
                    c->u0.child->parent = c;
                    c->value = c->u0.child->value;
                }
                else
                {
                    c->u1.value = c->value;
                    c->value = other->u2.value;
                }
                Heap3_UnitZero(&n->u0.child->u2);
                return;
            }
            else
            {
                /* Merge u1 into u0 */
                if (notleaf)
                {
                    other->u2 = c->u0;
                    other->u2.child->parent = other;
                }
                else
                {
                    other->u2.value = c->value;
                }
                n->u1 = n->u2;
                Heap3_UnitZero(&n->u2);
                Heap3_NodeFree(t, c);
            }
        }
        else
        {
            other = n->u1.child;
            if (other->u2.value != NULL)
            {
                /* Balance u1 into u2 */
                if (notleaf)
                {
                    c->u1 = c->u0;
                    c->u0 = other->u2;
                    c->u0.child->parent = c;
                    c->value = c->u0.child->value;
                }
                else
                {
                    c->u1.value = c->value;
                    c->value = other->u2.value;
                }
                Heap3_UnitZero(&other->u2);
            }
            else
            {
                /* Merge u2 into u1 */
                if (notleaf)
                {
                    other->u2 = c->u0;
                    other->u2.child->parent = other;
                }
                else
                {
                    other->u2.value = c->value;
                }
                Heap3_UnitZero(&n->u2);
                Heap3_NodeFree(t, c);
            }
            return;
        }
    }
}


/* <Heap3_NodeRemove>
 * At a given node remove the value
 * 05-06-1997 JSR Written
 */
void __Heap3_NodeRemove(Heap3_Tree *t, Heap3_FreeBlock *v)
{
    Heap3_Node *n;
    int cmp;
    Heap3_Comparator *compare = t->compare;

    for (n = t->root; n->u0.child != NULL;)
    {
        /* Note, the n < u0 case should never happen (in theory, and in practice) */
        if (n->u1.child == NULL || compare( v, n->u1.child->value) < 0)
        {
            /* n < u1 */
            n = n->u0.child;
        }
        else if (n->u2.child == NULL || compare(v, n->u2.child->value) < 0)
        {
            /* u1 <= n < u2 */
            n = n->u1.child;
        }
        else
        {
            /* u2 <= n */
            n = n->u2.child;
        }
    }

    /* Several cases aren't special-cased, but they can't ever happen */
    cmp = n->u1.value ? compare(v, n->u1.value) : -1;
    if (cmp > 0)
    {
        /* otherwise n = u2 */
        Heap3_UnitZero(&n->u2);
        return;
    }
    else if (cmp == 0)
    {
        /* n = u1 */
        n->u1 = n->u2;
        Heap3_UnitZero(&n->u2);
    }
    else
    {
        /* n < u1, hence n = u0 */
        n->value = n->u1.value;
        n->u1 = n->u2;
        Heap3_UnitZero(&n->u2);
        Heap3_NodeIndexChanged(n);
    }

    Heap3_NodeMergeCheck(t, n);
}


/* <Heap3_NodeFindLeastGT>
 * Find the given index in the node
 * 06-06-1997 JSR Written
 */
Heap3_FreeBlock *__Heap3_NodeFindLeastGT(Heap3_Tree *t, Heap3_FreeBlock *v)
{
    Heap3_Node *n;
    Heap3_Comparator *compare = t->compare;
    Heap3_FreeBlock *leastgt = NULL;

    for (n = t->root; n->u0.child != NULL;)
    {
        if (n->u1.child == NULL || compare(n->u1.child->value, v) > 0)
        {
            /* v < u1 */
            if (n->u0.child == NULL || compare(n->u0.child->value, v) > 0)
            {
                /* v < u0 */
                return n->u0.child->value;
            }
            else
            {
                /* u0 <= v < u1 */
                leastgt = n->u1.child->value;
                n = n->u0.child;
            }
        }
        else
        {
            /* v >= u1 */
            if (n->u2.child == NULL || compare(n->u2.child->value, v) > 0)
            {
                /* u1 <= v < u2 */
                if (n->u2.child != NULL)
                {
                    leastgt = n->u2.child->value;
                }
                n = n->u1.child;
            }
            else
            {
                /* u2 <= v */
                n = n->u2.child;
            }
        }
    }

    if (n->u1.value == NULL || compare(n->u1.value, v) > 0)
    {
        /* v < u1 */
        if (n->value == NULL || compare(n->value, v) > 0)
        {
            /* v < u0 */
            return n->value;
        }
        else
        {
            /* u0 <= v < u1 */
            return n->u1.value;
        }
    }
    else
    {
        /* v >= u1 */
        if (n->u2.value == NULL || compare(n->u2.value, v) > 0)
        {
            /* u1 <= v < u2 */
            if (n->u2.value != NULL)
            {
                return n->u2.value;
            }
        }
    }

    return leastgt;
}


/* <Heap3_ClaimFreeNode>
 * Remove the given node from the free chain
 * 06-06-1997 JSR Written
 */
static inline void Heap3_ClaimFreeNode(Heap3_FreeNode *n)
{
    n->freed.link.prev->next = n->freed.link.next;
    n->freed.link.next->prev = n->freed.link.prev;
}
        

/* <Heap3_MoveUsedNode>
 * Move the given used node's contents to a different node
 * 06-06-1997 JSR Written
 */
static inline void Heap3_MoveUsedNode(Heap3_Tree *t, Heap3_FreeNode *n)
{
    Heap3_Node *newnode = Heap3_NodeAlloc(t);
    *newnode = n->used;

    /* Change the node's children's parent pointer to point to the new node */
    if (newnode->u0.child != NULL)
    {
        /* Has some children */
        newnode->u0.child->parent = newnode;
        if (newnode->u1.child != NULL)
        {
            newnode->u1.child->parent = newnode;
            if (newnode->u2.child != NULL)
            {
                newnode->u2.child->parent = newnode;
            }
        }
    }

    /* Change the node's parent's child point to point to the new node */
    if (newnode->parent != NULL)
    {
        if (newnode->parent->u0.child == &n->used)
        {
            newnode->parent->u0.child = newnode;
        }
        else if (newnode->parent->u1.child == &n->used)
        {
            newnode->parent->u1.child = newnode;
        }
        else
        {
            newnode->parent->u2.child = newnode;
        }
    }
    else
    {
        t->root = newnode;
    }
}


/* <Heap3_ClaimBlock>
 * Remove the given free block from the free trees
 * 06-06-1997 JSR Written
 */
void __Heap3_ClaimBlock(Heap3_Descriptor *h, Heap3_FreeBlock *blk)
{
    /* Remove entries for this block from trees */
    __Heap3_NodeRemove(&h->free_bysize, blk);
    __Heap3_NodeRemove(&h->free_byaddress, blk);

    /* Remove unused tree nodes from free list */
    if (blk->n0.freed.inuse == 1)
    {
        Heap3_ClaimFreeNode(&blk->n0);
    }
    if (blk->n1.freed.inuse == 1)
    {
        Heap3_ClaimFreeNode(&blk->n1);
    }

    /* Move used nodes to other nodes */
    if (blk->n0.freed.inuse != 1)
    {
        Heap3_MoveUsedNode(&h->free_bysize, &blk->n0);
    }
    if (blk->n1.freed.inuse != 1)
    {
        Heap3_MoveUsedNode(&h->free_byaddress, &blk->n1);
    }
}


/* <Heap3_Alloc>
 * Allocate some memory from the heap
 * 06-06-1997 JSR Written
 */
extern void *__Heap3_Alloc(Heap3_Descriptor *h, size_t size)
{
    Heap3_FreeBlock tester;
    Heap3_FreeBlock *found;

    size = Heap3_SizeAdjust(size);
    tester.size = size-1;

    do
    {
        found = __Heap3_NodeFindLeastGT(&h->free_bysize, &tester);
        if (found)
        {
#if PARANOID
            if (found->size < size)
            {
                /* Definitely not OK */
                h->broken(h->brokenparam);
                return 0;
            }
#endif
            __Heap3_ClaimBlock(h, found);
            Heap3_TrimBlockTop(h, found, size);
            return &found->n0;
        }
    } while (h->full != NULL && h->full(h->fullparam, size));

    return 0;
}

#endif

#ifdef free_c


/* <Heap3_NodeInsert>
 * Insert the given v into the given node
 * 05-06-1997 JSR Written
 * 27-06-1997 JSR Made non-recursive
 */
static inline void Heap3_NodeInsert(Heap3_Tree *t, Heap3_Node *n, Heap3_FreeBlock *v)
{
    Heap3_Node *newnode;
    Heap3_Node *newroot;
    Heap3_Node *parent;
    Heap3_FreeBlock *v1;
    void *cv;
    int pos;
    int notleaf;
    Heap3_Comparator *compare = t->compare;

    if (n->u1.value == NULL || compare(n->u1.value, v) > 0)
    {
        /* u1 > v */
        if (n->value == NULL || compare(n->value, v) > 0)
        {
            /* u0 > v */
            pos = 1;
            /* Turn it into an insert of the old u0 value after u0 */
            v1 = n->value;
            n->value = v;
            Heap3_NodeIndexChanged(n);
            v = v1;
        }
        else
        {
            /* u0 <= v < u1 */
            pos = 1;
        }
    }
    else
    {
        /* u1 <= v */
        if (n->u2.value == NULL || compare(n->u2.value, v) > 0)
        {
            /* u1 <= v < u2 */
            pos = 2;
        }
        else
        {
            /* u2 <= v */
            pos = 3;
        }
    }

    cv = v;
    for (notleaf = 0;; notleaf = 1)
    {
        if (n->u2.value == NULL)
        {
            /* Insert */
            if (pos == 1)
            {
                n->u2 = n->u1;
                n->u1.value = (Heap3_FreeBlock *)cv;
            }
            else
            {
                n->u2.value = (Heap3_FreeBlock *)cv;
            }

            return;
        }

        /* Split */
        newnode = Heap3_NodeAlloc(t);
        Heap3_UnitZero(&newnode->u2);
        newnode->parent = n->parent;

        switch (pos)
        {
        case 1:
            if (notleaf)
            {
                newnode->u0 = n->u1;
                newnode->value = n->u1.child->value;
            }
            else
            {
                newnode->u0.value = NULL;
                newnode->value = n->u1.value;
            }
            newnode->u1 = n->u2;
            n->u1.value = (Heap3_FreeBlock *)cv;
            break;
        case 2:
            if (notleaf)
            {
                newnode->u0.child = (Heap3_Node *)cv;
                newnode->value = newnode->u0.child->value;
            }
            else
            {
                newnode->u0.value = NULL;
                newnode->value = (Heap3_FreeBlock *)cv;
            }
            newnode->u1 = n->u2;
            break;
        case 3:
            if (notleaf)
            {
                newnode->u0 = n->u2;
                newnode->value = newnode->u0.child->value;
            }
            else
            {
                newnode->u0.value = NULL;
                newnode->value = n->u2.value;
            }
            newnode->u1.value = (Heap3_FreeBlock *)cv;
            break;
        }
        if (notleaf)
        {
            newnode->u0.child->parent = newnode;
            newnode->u1.child->parent = newnode;
        }
        Heap3_UnitZero(&n->u2);

        parent = n->parent;
        if (parent == NULL)
        {
            /* Reached root, so generate a new root to hold the split old root */
            newroot = Heap3_NodeAlloc(t);
            newroot->value = n->value;
            newroot->u0.child = n;
            newroot->u1.child = newnode;
            Heap3_UnitZero(&newroot->u2);
            newroot->parent = NULL;
            t->root = newroot;
            n->parent = newnode->parent = newroot;

            return;
        }

        if (parent->u0.child == n)
        {
            pos = 1;
        }
        else if (parent->u1.child == n)
        {
            pos = 2;
        }
        else
        {
            pos = 3;
        }
        n = parent;
        cv = newnode;
    }
}


/* <Heap3_NodeAdd>
 * At a given node add the value
 * 05-06-1997 JSR Written
 */
static void Heap3_NodeAdd(Heap3_Tree *t, Heap3_FreeBlock *v)
{
    Heap3_Node *n;
    Heap3_Comparator *compare = t->compare;

    for (n = t->root; n->u0.child != NULL;)
    {
        if (n->u1.child == NULL || compare(v, n->u1.child->value) < 0)
        {
            /* n < u1 */
            n = n->u0.child;
        }
        else if (n->u2.child == NULL || compare(v, n->u2.child->value) < 0)
        {
            /* u1 <= n < u2 */
            n = n->u1.child;
        }
        else
        {
            /* u2 <= n */
            n = n->u2.child;
        }
    }

    Heap3_NodeInsert(t, n, v);
}


/* <Heap3_NodeFindGreatestLT>
 * Find the given index in the node
 * 06-06-1997 JSR Written
 */
static Heap3_FreeBlock *Heap3_NodeFindGreatestLT(Heap3_Tree *t, Heap3_FreeBlock *v)
{
    Heap3_Comparator *compare = t->compare;
    Heap3_Node *n = t->root;

    for ( ; n->u0.child != NULL; )
    {
        if (n->u1.child == NULL || compare( n->u1.child->value, v) >= 0)
        {
            /* u1 >= v */
            if (n->u0.child == NULL || compare( n->u0.child->value, v) >= 0)
            {
                /* u0 >= v */
                return NULL;
            }
            else
            {
                /* u0 < v <= u1 */
                n = n->u0.child;
            }
        }
        else
        {
            /* u1 < v */
            if (n->u2.child == NULL || compare(n->u2.child->value, v) >= 0)
            {
                /* u2 < v <= u2 */
                n = n->u1.child;
            }
            else
            {
                /* u2 < v */
                n = n->u2.child;
            }
        }
    }

    /* Now at leaf level */
    if (n->u1.value == NULL || compare(n->u1.value, v) >= 0)
    {
        /* u1 >= v */
        if (n->value == NULL || compare(n->value, v) >= 0)
        {
            /* u0 >= v */
            return NULL;
        }
        else
        {
            /* u0 < v <= u1 */
            return n->u0.value;
        }
    }
    else
    {
        /* u1 < v */
        if (n->u2.value == NULL || compare(n->u2.value, v) >= 0)
        {
            /* u2 < v <= u2 */
            return n->u1.value;
        }
        else
        {
            /* u2 < v */
            return n->u2.value;
        }
    }
}


/* <Heap3_Free>
 * Free a node to a type 3 heap
 * 06-06-1997 JSR Written
 */
extern void __Heap3_Free(Heap3_Descriptor *h, void *_blk)
{
    Heap3_FreeBlock *blk = (Heap3_FreeBlock *)((char *)_blk - sizeof(size_t));
    Heap3_FreeBlock *merger;

    /* See if we have a free block after us to merge with */
    merger = __Heap3_NodeFindLeastGT(&h->free_byaddress, blk);
    if ((Heap3_FreeBlock *)((char *)blk + blk->size) == merger)
    {
        /* merge with block above us */
        __Heap3_ClaimBlock(h, merger);
        blk->size += merger->size;
    }

    /* See if we have a free block before us to merge with */
    merger = Heap3_NodeFindGreatestLT(&h->free_byaddress, blk);
    if (merger != NULL && (Heap3_FreeBlock *)((char *)merger + merger->size) == blk)
    {
        /* Merge with block before us */

        __Heap3_NodeRemove(&h->free_bysize, merger);
        merger->size += blk->size;
        Heap3_NodeAdd(&h->free_bysize, merger);
    }
    else
    {
        /* New free block */

        /* Give the two tree nodes to the tree node free list */
        Heap3_NodeFree(&h->free_bysize, &blk->n0.used);
        Heap3_NodeFree(&h->free_byaddress, &blk->n1.used);

        Heap3_NodeAdd(&h->free_byaddress, blk);
        Heap3_NodeAdd(&h->free_bysize, blk);
    }
}

#endif

#ifdef realloc_c

/* <Heap3_Realloc>
 * Re-allocate the given block, ensuring the newly allocated block has the same
 * min(newsize,oldsize) bytes at its start
 * 06-06-1997 JSR Written
 */
extern void *__Heap3_Realloc(Heap3_Descriptor *h, void *_blk, size_t size)
{
    Heap3_FreeBlock *blk = (Heap3_FreeBlock *)((char *)_blk - sizeof( size_t ));
    Heap3_FreeBlock *blk2;

    size = Heap3_SizeAdjust(size);

    if (size <= blk->size)
    {
        /* No change or shrink */
        Heap3_TrimBlockTop(h, blk, size);
        return &blk->n0;
    }

    /* Enlarge */

    blk2 = __Heap3_NodeFindLeastGT(&h->free_byaddress, blk);
    if (blk2 == (Heap3_FreeBlock *)((char *)blk + blk->size)
        && blk->size + blk2->size >= size)
    {
        /* Merge is good */
        __Heap3_ClaimBlock(h, blk2);
        blk->size += blk2->size;
        Heap3_TrimBlockTop(h, blk, size);
        return &blk->n0;
    }

    /* No free block of sufficient size follows blk, so do an alloc,copy,free sequence */
    blk2 = (Heap3_FreeBlock *)__Heap3_Alloc(h, size - sizeof(size_t));
    if (blk2 != NULL)
    {
        memcpy(blk2, &blk->n0, blk->size - sizeof(size_t));
        __Heap3_Free(h, &blk->n0);
    }

    return blk2;
}

#endif

#ifdef stats_c

/* <Heap3_NodeEnumerate>
 * Call the given function for each value
 * 06-06-1997 JSR Written
 * 30-06-1997 JSR Addapted to compact 2-3 nodes
 */
static void Heap3_NodeEnumerate(
    void (*enumerate)(void *, Heap3_FreeBlock *), void *param,
    Heap3_Node *n)
{
    if (n->u0.child != NULL)
    {
        Heap3_NodeEnumerate(enumerate, param, n->u0.child);
        if (n->u1.child != NULL)
        {
            Heap3_NodeEnumerate(enumerate, param, n->u1.child);
            if (n->u2.child != NULL)
            {
                Heap3_NodeEnumerate(enumerate, param, n->u2.child);
            }
        }
    }
    else if (n->value != NULL)
    {
        enumerate(param, n->value);
        if (n->u1.value != NULL)
        {
            enumerate(param, n->u1.value);
            if (n->u2.value != NULL)
            {
                enumerate(param, n->u2.value);
            }
        }
    }
}


typedef struct Heap3_StatsBlk
{
    char *hwm;
    size_t totsize;
    int numblocks;
    int sizecount[32];
} Heap3_StatsBlk;


/* <Heap3_AccumulateBlock>
 * Accumulate one block into the statistics
 * 06-06-1997 JSR Written
 */
static void Heap3_AccumulateBlock(void *param, Heap3_FreeBlock *blk)
{
    Heap3_StatsBlk *stats = (Heap3_StatsBlk *)param;
    int i;
    size_t size;

    if ((char *)blk > stats->hwm)
    {
        stats->hwm = (char *)blk;
    }
    stats->numblocks += 1;
    size = blk->size;
    stats->totsize += size;
    for (i = 0; i < 32; i++)
    {
        size >>= 1;
        if (size == 0)
        {
            break;
        }
    }
    stats->sizecount[i]++;
}


/* <Heap3_Stats>
 * Print stats on the given heap
 * 06-06-1997 JSR Written
 */
extern void *__Heap3_Stats(int (*dprint)(char const *format, ...), Heap3_Descriptor *h)
{
    Heap3_StatsBlk stats;
    int i;

    stats.hwm = 0;
    stats.totsize = 0;
    stats.numblocks = 0;

    for (i = 0; i < 32; i++)
    {
        stats.sizecount[i] = 0;
    }

    Heap3_NodeEnumerate( Heap3_AccumulateBlock, &stats, h->free_byaddress.root );

    dprint("%d bytes in %d free blocks (avge size %d)\n",
           stats.totsize, stats.numblocks, stats.totsize/(stats.numblocks==0?1:stats.numblocks));
    for (i = 0; i < 32; i++)
    {
        if (stats.sizecount[i] != 0)
        {
            dprint( "%d blocks 2^%d+1 to 2^%d\n", stats.sizecount[i], i-1, i );
        }
    }

    return stats.hwm;
}

#endif

/* End of file heap3.c */
