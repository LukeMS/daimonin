/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001 Michael Toennies

    A split from Crossfire, a Multiplayer game for X-windows.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The author can be reached via e-mail to daimonin@nord-com.net
*/

#ifndef MEMPOOL_H
#define MEMPOOL_H

/* Pooling memory management stuff */

#define MEMPOOL_TRACKING /* Enable tracking/freeing of mempools ? */

/*#define MEMPOOL_OBJECT_TRACKING*/ /* enable a global list of *all* objects
                                    * we have allocated. We can browse them to
                                    * control & debug them. WARNING: Enabling this
                                    * feature will slow down the server *EXTREMLY* and should
                                    * only be done in real debug runs
                                    */

/* Minimalistic memory management data for a single chunk of memory 
 * It is (currently) up to the application to keep track of which pool 
 * it belongs to.
 */
struct mempool_chunk
{
    /* This struct must always be padded for longword alignment of the data coming behind it.
     * Not a problem as long as we only keep a single pointer here, but be careful
     * if adding more data. */
    struct mempool_chunk   *next; /* Used for the free list and the removed list. NULL if this
                                     memory chunk has been allocated and is in use */
#ifdef MEMPOOL_OBJECT_TRACKING
    struct mempool_chunk   *obj_prev; /* for debug only */
    struct mempool_chunk   *obj_next; /* for debug only */
    uint32                  flags;  
    uint32                  pool_id; /* to what mpool is this memory part related? */
    uint32                  id;   /* the REAL unique ID number */
#endif
};

typedef void (* chunk_constructor) (void *ptr);     /* Optional constructor to be called when expanding */
typedef void (* chunk_destructor) (void *ptr);      /* Optional destructor to be called when freeing */

/* Definitions used for array handling */
#define MEMPOOL_NROF_FREELISTS 8              
#define MEMPOOL_MAX_ARRAYSIZE (1 << MEMPOOL_NROF_FREELISTS) /* = 256 if NROF_FREELISTS == 8 */

/* Data for a single memory pool */
struct mempool
{
    /* Fields that need need declaration */
    char                   *chunk_description;            /* Description of chunks. Mostly for debugging */
    uint32                  expand_size;                 /* How many chunks to allocate at each expansion */
    uint32                  chunksize;                   /* size of chunks, excluding sizeof(mempool_chunk) and padding */
    uint32                  flags;                       /* Special handling flags. See definitions below */
    chunk_constructor       constructor;      /* Optional constructor to be called when getting chunks */
    chunk_destructor        destructor;        /* Optional destructor to be called when returning chunks */

    /* Runtime fields */
    struct mempool_chunk   *freelist[MEMPOOL_NROF_FREELISTS];   /* First free chunk */    
    uint32                  nrof_free[MEMPOOL_NROF_FREELISTS], nrof_allocated[MEMPOOL_NROF_FREELISTS]; /* List size counters */
#ifdef MEMPOOL_TRACKING
    struct puddle_info     *first_puddle_info; /* List of puddles used for mempool tracking */
#endif
};

#ifdef MEMPOOL_TRACKING
struct puddle_info
{
    struct puddle_info     *next;
    struct mempool_chunk   *first_chunk;

    /* Local freelist only for this puddle. Temporary used when freeing memory*/
    struct mempool_chunk   *first_free, *last_free;
    uint32                  nrof_free;
};
#endif

typedef enum
{
#ifdef MEMPOOL_TRACKING
    POOL_PUDDLE,
#endif    
    POOL_OBJECT,
    POOL_PLAYER,
    POOL_MAP_BFS,
    POOL_PATHSEGMENT,
    POOL_MOBDATA,
    POOL_MOB_KNOWN_OBJ,
    POOL_BEHAVIOURSET,
    POOL_BEHAVIOUR,
    POOL_BEHAVIOUR_PARAM,
    POOL_OBJECT_LINK,
    NROF_MEMPOOLS
}                            mempool_id;

/* Get the memory management struct for a chunk of memory */
#define MEM_POOLDATA(ptr) (((struct mempool_chunk *)(ptr)) - 1)
/* Get the actual user data area from a mempool reference */
#define MEM_USERDATA(ptr) ((void *)(((struct mempool_chunk *)(ptr)) + 1))
/* Check that a chunk of memory is in the free (or removed for objects) list */
#define CHUNK_FREE(ptr) (MEM_POOLDATA(ptr)->next != NULL)

#define MEMPOOL_ALLOW_FREEING 1 /* Allow puddles from this pool to be freed */
#define MEMPOOL_BYPASS_POOLS  2 /* Don't use pooling, but only malloc/free instead */

extern struct mempool       mempools[];
extern struct mempool_chunk end_marker; /* only used as an end marker for the lists */

#define get_poolchunk(_pool_) get_poolchunk_array_real((_pool_), 0)
#define get_poolarray(_pool_, _arraysize_) get_poolchunk_array_real((_pool_), nearest_pow_two_exp(_arraysize_))

#define return_poolchunk(_data_, _pool_) \
    return_poolchunk_array_real((_data_), 0, (_pool_))
#define return_poolarray(_data_, _arraysize_, _pool_) \
    return_poolchunk_array_real((_data_), nearest_pow_two_exp(_arraysize_), (_pool_))

#endif
