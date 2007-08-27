/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001 Michael Toennies
    Memory Management Routines Copyright (C) 2004 Björn Axelsson

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

/*
 * Memory management functions for memory pools
 */

#include <global.h>
#ifndef WIN32 /* ---win32 exclude headers */
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#endif /* win32 */

#ifdef DEBUG_MEMPOOL_OBJECT_TRACKING
#define MEMPOOL_OBJECT_FLAG_FREE 1
#define MEMPOOL_OBJECT_FLAG_USED 2
static struct mempool_chunk    *used_object_list        = NULL; /* for debugging only! */
static uint32                   chunk_tracking_id       = 1;
#endif

/* The removedlist is not ended by NULL, but by a pointer to the end_marker */
struct mempool_chunk            end_marker; /* only used as an end marker for the lists */

/*
 * The Life Cycle of an Object:
 *

 - expand_mempool(): Allocated from system memory and put into the freelist of the object pool.
 - get_object():     Removed from freelist & put into removedlist (since it is not inserted anywhere yet).
 - insert_ob_in_(map/ob)(): Filled with data & inserted into (any) environment
 ... end of timestep
 - object_gc():      Removed from removedlist, but not freed (since it sits in an env).
 ...
 - remove_ob():      Removed from environment
 - Sits in removedlist until the end of this server timestep
 ... end of timestep
 - object_gc():      Freed and moved to freelist
 (attrsets are freed and given back to their respective pools too).
*/


/** Basic pooling memory management system **/

/*
 * A pool definition in the mempools[] array and an entry in the mempool id enum
 * is needed for each type of struct we want to use the pooling memory management for.
 */

int nrof_mempools = 0;
struct mempool *mempools[MAX_NROF_MEMPOOLS];

#ifdef MEMPOOL_TRACKING
struct mempool *pool_puddle;
#endif

struct mempool *pool_object, *pool_player, *pool_map_bfs,
    *pool_path_segment, *pool_mob_data, *pool_mob_knownobj,
    *pool_mob_behaviourset, *pool_mob_behaviour, *pool_mob_behaviourparam,
    *pool_objectlink, *pool_gmasters, *pool_bannode, *pool_tlist_tweak,
    *pool_cmd_buf16,*pool_cmd_buf32,*pool_cmd_buf64,
    *pool_cmd_buf128,*pool_cmd_buf256,*pool_cmd_buf1024,*pool_cmd_buf4096
#ifdef USE_CHANNELS
    ,*pool_player_channel
#endif
    ;

/* Return the exponent exp needed to round n up to the nearest power of two, so that
 * (1 << exp) >= n and (1 << (exp -1)) < n */
uint32 nearest_pow_two_exp(uint32 n)
{
    /* Lookup table generated with:
     *  perl -e 'for($n=0; $n<=64; $n++) {for($i=0; (1 << $i) < $n; $i++) {} print "$i,";}'
     */
    static const uint32 exp_lookup[65]  =
    {
        0, 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
    };
    uint32              i;
    if (n <= 64)
        return exp_lookup[n];
    for (i = 7; (uint32) (1 << i) < n; i++)
        ;
    return i;
}

/* The mempool system never frees memory back to the system, but is extremely efficient
 * when it comes to allocating and returning pool chunks. Always use the get_poolchunk()
 * and return_poolchunk() functions for getting and returning memory chunks. expand_mempool() is
 * used internally.
 *
 * Be careful if you want to use the internal chunk or pool data, its semantics and
 * format might change in the future.
 */

/* TODO: implement =) */
void free_mempool(struct mempool *pool)
{
}

struct mempool *create_mempool(const char *description, uint32 expand, uint32 size,
        uint32 flags, chunk_initialisator initialisator, chunk_deinitialisator deinitialisator,
        chunk_constructor constructor, chunk_destructor destructor)
{
    int i;
    struct mempool *pool;

    if(nrof_mempools >= MAX_NROF_MEMPOOLS)
        LOG(llevError, "Too many memory pools registered. Please increase the MAX_NROF_MEMPOOLS constant in mempools.h\n");

    pool = calloc(1, sizeof(struct mempool));

    mempools[nrof_mempools] = pool;

    pool->chunk_description = description;
    pool->expand_size = expand;
    pool->chunksize = size;
    pool->flags = flags;
    pool->initialisator = initialisator;
    pool->deinitialisator = deinitialisator;
    pool->constructor = constructor;
    pool->destructor = destructor;

    for (i = 0; i < MEMPOOL_NROF_FREELISTS; i++)
    {
        pool->freelist[i] = &end_marker;
        pool->nrof_free[i] = 0;
        pool->nrof_allocated[i] = 0;
    }

#ifdef MEMPOOL_TRACKING
    pool->first_puddle_info = NULL;
#endif

    nrof_mempools++;

    return pool;
}

/* Initialize the mempools lists and related data structures */
void init_mempools()
{
#ifdef MEMPOOL_TRACKING
    pool_puddle = create_mempool("puddles", 10, sizeof(struct puddle_info), MEMPOOL_ALLOW_FREEING, NULL, NULL, NULL, NULL);
#endif
    pool_object = create_mempool("objects", OBJECT_EXPAND, sizeof(object), 0, NULL, NULL,
            (chunk_constructor) initialize_object, (chunk_destructor) destroy_object);
    pool_player = create_mempool("players", 25, sizeof(player), MEMPOOL_BYPASS_POOLS, NULL, NULL, NULL, NULL);
    pool_map_bfs= create_mempool("map BFS nodes", 16, sizeof(struct mapsearch_node), 0, NULL, NULL, NULL, NULL);
    pool_path_segment= create_mempool("path segments", 500, sizeof(struct path_segment), 0, NULL, NULL, NULL, NULL);
    pool_mob_data= create_mempool("mob brains", 100, sizeof(struct mobdata), 0, NULL, NULL, NULL, NULL);
    pool_mob_knownobj=  create_mempool("mob known objects", 100, sizeof(struct mob_known_obj), 0, NULL, NULL, NULL, NULL);
    pool_mob_behaviourset = create_mempool("mob behaviour sets", 100, sizeof(struct mob_behaviourset), 0, NULL, NULL, NULL, NULL);
    pool_mob_behaviour = create_mempool("mob behaviours", 100, sizeof(struct mob_behaviour), 0, NULL, NULL, NULL, NULL);
    pool_mob_behaviourparam = create_mempool("mob behaviour parameter", 100, sizeof(struct mob_behaviour_param), 0, NULL, NULL, NULL, NULL);
    pool_objectlink = create_mempool("object links", 500, sizeof(objectlink), 0, NULL, NULL, NULL, NULL);

    pool_gmasters = create_mempool("gmaster entries", 10, sizeof(gmaster_struct), 0, NULL, NULL, NULL, NULL);
    pool_bannode = create_mempool("ban node entries", 25, sizeof(struct ban_struct), 0, NULL, NULL, NULL, NULL);

    pool_tlist_tweak = create_mempool("treasure list tweak", 100, sizeof(tlist_tweak), 0, NULL, NULL, NULL, NULL);

    /* for testing purpose, we get everytime only 1 buffer more */
    pool_cmd_buf16 = create_mempool("command buffer 16b", 1, sizeof(command_struct), 0,
            (chunk_initialisator) initialize_command_buffer16, NULL, NULL, NULL);
    pool_cmd_buf32 = create_mempool("command buffer 32b", 1, sizeof(command_struct), 0,
            (chunk_initialisator) initialize_command_buffer32, NULL, NULL, NULL);
    pool_cmd_buf64 = create_mempool("command buffer 64b", 1, sizeof(command_struct), 0,
            (chunk_initialisator) initialize_command_buffer64, NULL, NULL, NULL);
    pool_cmd_buf128 = create_mempool("command buffer 128b", 1, sizeof(command_struct), 0,
            (chunk_initialisator) initialize_command_buffer128, NULL, NULL, NULL);
    pool_cmd_buf256 = create_mempool("command buffer 256b", 1, sizeof(command_struct), 0,
            (chunk_initialisator) initialize_command_buffer256, NULL, NULL, NULL);
    pool_cmd_buf1024 = create_mempool("command buffer 1024b", 1, sizeof(command_struct), 0,
            (chunk_initialisator) initialize_command_buffer1024, NULL, NULL, NULL);
    pool_cmd_buf4096 = create_mempool("command buffer 4096b", 1, sizeof(command_struct), 0,
            (chunk_initialisator) initialize_command_buffer4096, NULL, NULL, NULL);
#ifdef USE_CHANNELS
    pool_player_channel = create_mempool("player_channel", 25, sizeof(struct player_channel), 0, NULL, NULL, NULL, NULL);
#endif

    /* Initialize end-of-list pointers and a few other values*/
    removed_objects = &end_marker;

    /* Set up container for "loose" objects */
    initialize_object(&void_container);
    void_container.type = TYPE_VOID_CONTAINER;
    FREE_AND_COPY_HASH(void_container.name, "<void container>");
}

void cleanup_mempools()
{
    LOG(llevDebug, "Freeing global mempool data\n");
    FREE_AND_CLEAR_HASH(void_container.name);
}

/*
 * Expands the memory pool with MEMPOOL_EXPAND new chunks. All new chunks
 * are put into the pool's freelist for future use.
 * expand_mempool is only meant to be used from get_poolchunk().
 *
 * arraysize_exp is the exponent for the array size, for example 3 for
 * arrays of length 8 (2^3 = 8)
 */
static void expand_mempool(struct mempool *pool, uint32 arraysize_exp)
{
    uint32                  i;
    struct mempool_chunk   *first, *ptr;
    int                     chunksize_real;
    int                     nrof_arrays;

    if (pool->nrof_free[arraysize_exp] > 0)
        LOG(llevBug, "BUG: expand_mempool called with chunks still available in pool\n");

    nrof_arrays = pool->expand_size >> arraysize_exp;

    if (nrof_arrays == 0)
    {
        LOG(llevDebug, "WARNING: expand_mempool called with too big array size for its expand_size\n");
        nrof_arrays = 1;
    }

    chunksize_real = sizeof(struct mempool_chunk) + (pool->chunksize << arraysize_exp);
    first = (struct mempool_chunk *) calloc(1,nrof_arrays * chunksize_real);

    if (first == NULL)
        LOG(llevError, "ERROR: expand_mempool(): Out Of Memory.\n");

    pool->freelist[arraysize_exp] = first;
    pool->nrof_allocated[arraysize_exp] += nrof_arrays;
    pool->nrof_free[arraysize_exp] = nrof_arrays;

    /* Set up the linked list */
    ptr = first;
    for (i = 0; (int) i < nrof_arrays - 1; i++)
    {
#ifdef DEBUG_MEMPOOL_OBJECT_TRACKING
        ptr->obj_next = ptr->obj_prev = 0; /* secure */
        ptr->pool = pool;
        ptr->id = chunk_tracking_id++; /* this is a real, unique object id  allows tracking beyond get/free objects */
        ptr->flags |= MEMPOOL_OBJECT_FLAG_FREE;
#endif
        if (pool->initialisator )
            pool->initialisator(MEM_USERDATA(ptr));

        ptr = ptr->next = (struct mempool_chunk *) (((char *) ptr) + chunksize_real);
    }

    /* and the last element */
    ptr->next = &end_marker;
    if (pool->initialisator )
        pool->initialisator(MEM_USERDATA(ptr));
#ifdef DEBUG_MEMPOOL_OBJECT_TRACKING
    ptr->obj_next = ptr->obj_prev = 0; /* secure */
    ptr->pool = pool;
    ptr->id = chunk_tracking_id++; /* this is a real, unique object id  allows tracking beyond get/free objects */
    ptr->flags |= MEMPOOL_OBJECT_FLAG_FREE;
#endif

#ifdef MEMPOOL_TRACKING
    /* Track the allocation of puddles? */
    {
        struct puddle_info *p   = get_poolchunk(pool_puddle);
        p->first_chunk = first;
        p->next = pool->first_puddle_info;
        pool->first_puddle_info = p;
    }
#endif
}

/* Get a chunk from the selected pool. The pool will be expanded if necessary. */
void * get_poolchunk_array_real(struct mempool *pool, uint32 arraysize_exp)
{
    struct mempool_chunk   *new_obj;

    if (pool->flags & MEMPOOL_BYPASS_POOLS)
    {
        new_obj = calloc(1, sizeof(struct mempool_chunk) + (pool->chunksize << arraysize_exp));
        pool->nrof_allocated[arraysize_exp]++;
    }
    else
    {
        if (pool->nrof_free[arraysize_exp] == 0)
            expand_mempool(pool, arraysize_exp);
        new_obj = pool->freelist[arraysize_exp];
        pool->freelist[arraysize_exp] = new_obj->next;
        pool->nrof_free[arraysize_exp]--;
    }

    new_obj->next = NULL;

    if (pool->constructor)
        pool->constructor(MEM_USERDATA(new_obj));

#ifdef DEBUG_MEMPOOL_OBJECT_TRACKING
    /* that should never happens! */
    if (new_obj->obj_prev || new_obj->obj_next)
    {
        LOG(llevDebug, "WARNING:DEBUG_OBJ::get_poolchunk() object >%d< is in used_object list!!\n", new_obj->id);
    }

    /* put it in front of the used object list */
    new_obj->obj_next = used_object_list;
    if (new_obj->obj_next)
        new_obj->obj_next->obj_prev = new_obj;
    used_object_list = new_obj;
    new_obj->flags &= ~MEMPOOL_OBJECT_FLAG_FREE;
    new_obj->flags |= MEMPOOL_OBJECT_FLAG_USED;
#endif

    return MEM_USERDATA(new_obj);
}

/* Return a chunk to the selected pool. Don't return memory to the wrong pool!
 * Returned memory will be reused, so be careful about those stale pointers */
void return_poolchunk_array_real(void *data, uint32 arraysize_exp, struct mempool *pool)
{
    struct mempool_chunk   *old = MEM_POOLDATA(data);

    if (CHUNK_FREE(data))
    {
        /* When this happens we can choose to ignore it and return or stop and
         * make finding the original error easier. */
#ifdef PRODUCTION_SYSTEM
        LOG(llevBug, "BUG: return_poolchunk on already free chunk (pool \"%s\")\n",
            pool->chunk_description);
        return; /* Ignore the problem */
#else
        LOG(llevError, "ERROR: return_poolchunk on already free chunk (pool \"%s\")\n",
            pool->chunk_description);
        /* llevError terminates the server */
#endif
    }

#ifdef DEBUG_MEMPOOL_OBJECT_TRACKING
    if (old->obj_next)
        old->obj_next->obj_prev = old->obj_prev;
    if (old->obj_prev)
        old->obj_prev->obj_next = old->obj_next;
    else
        used_object_list = old->obj_next;

    old->obj_next = old->obj_prev = 0; /* secure */
    old->flags &= ~MEMPOOL_OBJECT_FLAG_USED;
    old->flags |= MEMPOOL_OBJECT_FLAG_FREE;
#endif

    if (pool->destructor)
        pool->destructor(data);

    if (pool->flags & MEMPOOL_BYPASS_POOLS)
    {
        if (pool->deinitialisator )
            pool->deinitialisator(MEM_USERDATA(old));
        free(old);
        pool->nrof_allocated[arraysize_exp]--;
    }
    else
    {
        old->next = pool->freelist[arraysize_exp];
        pool->freelist[arraysize_exp] = old;
        pool->nrof_free[arraysize_exp]++;
    }
}

/** Gather mempool statistics and write details to the log and the give player.
 * @param player to send detailed info to (optional)
 * @param sum_used total number of bytes actively in use from mempools (OUTPUT)
 * @param sum_alloc total number of bytes allocated by the mempool system (OUTPUT)
 */
void dump_mempool_statistics(object *op, int *sum_used, int *sum_alloc)
{
    int j, k;

    for (j = 0; j < nrof_mempools; j++)
    {
        for (k = 0; k < MEMPOOL_NROF_FREELISTS; k++)
        {
            if (mempools[j]->nrof_allocated[k] > 0)
            {
                int ob_used = mempools[j]->nrof_allocated[k] - mempools[j]->  nrof_free[k],
                    ob_free = mempools[j]->nrof_free[k];
                int mem_used = ob_used*((mempools[j]->chunksize << k) + sizeof(struct mempool_chunk));
                int mem_free = ob_free*((mempools[j]->chunksize << k) + sizeof(struct mempool_chunk));

                sprintf(errmsg, "%4d used (%4d free) %s[%3d]: %d (%d)", ob_used, ob_free, mempools[j]->chunk_description,
                        1 << k, mem_used, mem_free);
                if(op)
                    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
                LOG(llevSystem, "%s\n", errmsg);
                if(sum_used)
                    *sum_used += mem_used;
                if(sum_alloc)
                    *sum_alloc += mem_used + mem_free;
            }
        }
    }
}

#ifdef DEBUG_MEMPOOL_OBJECT_TRACKING

/* this is time consuming DEBUG only
 * function. Mainly, it checks the different memory parts
 * and controls they are was they are - if a object claims its
 * in a inventory we check the inventory - same for map.
 * If we have detached but not deleted a object - we will find it here.
 */
void check_use_object_list(void)
{
    struct mempool_chunk   *chunk;

    for (chunk = used_object_list; chunk; chunk = chunk->obj_next)
    {
#ifdef MEMPOOL_TRACKING
        if (chunk->pool == pool_puddle) /* ignore for now */
        {
        }
        else
        #endif
        if (chunk->pool == pool_object)
        {
            object *tmp2, *tmp = MEM_USERDATA(chunk);

            /*LOG(llevDebug,"DEBUG_OBJ:: object >%s< (%d)\n",  query_name(tmp), chunk->id);*/

            if (QUERY_FLAG(tmp, FLAG_REMOVED))
                LOG(llevDebug, "VOID:DEBUG_OBJ:: object >%s< (%d) has removed flag set!\n", query_name(tmp), chunk->id);

            if (tmp->map) /* we are on a map */
            {
                if (tmp->map->in_memory != MAP_IN_MEMORY)
                    LOG(llevDebug, "BUG:DEBUG_OBJ:: object >%s< (%d) has invalid map! >%d<!\n", query_name(tmp),
                        tmp->map->name ? tmp->map->name : "NONE", chunk->id);
                else
                {
                    for (tmp2 = get_map_ob(tmp->map, tmp->x, tmp->y); tmp2; tmp2 = tmp2->above)
                    {
                        if (tmp2 == tmp)
                            goto goto_object_found;
                    }

                    LOG(llevDebug, "BUG:DEBUG_OBJ:: object >%s< (%d) has invalid map! >%d<!\n", query_name(tmp),
                        tmp->map->name ? tmp->map->name : "NONE", chunk->id);
                }
            }
            else if (tmp->env)
            {
                /* object claims to be here... lets check it IS here */
                for (tmp2 = tmp->env->inv; tmp2; tmp2 = tmp2->below)
                {
                    if (tmp2 == tmp)
                        goto goto_object_found;
                }

                LOG(llevDebug, "BUG:DEBUG_OBJ:: object >%s< (%d) has invalid env >%d<!\n", query_name(tmp),
                    query_name(tmp->env), chunk->id);
            }
            else /* where we are ? */
            {
                LOG(llevDebug, "BUG:DEBUG_OBJ:: object >%s< (%d) has no env/map\n", query_name(tmp), chunk->id);
            }
        }
        else if (chunk->pool == pool_player)
        {
            player *tmp = MEM_USERDATA(chunk);

            /*LOG(llevDebug,"DEBUG_OBJ:: player >%s< (%d)\n",  tmp->ob?query_name(tmp->ob):"NONE", chunk->id);*/
        }
        else
        {
            LOG(llevDebug, "BUG:DEBUG_OBJ: wrong pool ID! (%s - %d)", chunk->pool->chunk_description, chunk->id);
        }
        goto_object_found:;
    }
}
#endif

#ifdef MEMPOOL_TRACKING

/* A Linked-List Memory Sort
 * by Philip J. Erdelsky <pje@efgh.com>
 * http://www.alumni.caltech.edu/~pje/
 * (Public Domain)
 *
 * The function sort_linked_list() will sort virtually any kind of singly-linked list, using a comparison
 * function supplied by the calling program. It has several advantages over qsort().
 *
 * The function sorts only singly linked lists. If a list is doubly linked, the backward pointers can be
 * restored after the sort by a few lines of code.
 *
 * Each element of a linked list to be sorted must contain, as its first members, one or more pointers.
 * One of the pointers, which must be in the same relative position in each element, is a pointer to the
 * next element. This pointer is <end_marker> (usually NULL) in the last element.
 *
 * The index is the position of this pointer in each element.
 * It is 0 for the first pointer, 1 for the second pointer, etc.
 *
 * Let n = compare(p,q,pointer) be a comparison function that compares two elements p and q as follows:
 * void *pointer;  user-defined pointer passed to compare() by linked_list_sort()
 * int n;          result of comparing *p and *q
 *                      >0 if *p is to be after *q in sorted order
 *                      <0 if *p is to be before *q in sorted order
 *                       0 if the order of *p and *q is irrelevant
 *
 *
 * The fourth argument (pointer) is passed to compare() without change. It can be an invaluable feature if
 * two or more comparison methods share a substantial amount of code and differ only in one or more parameter
 * values.
 *
 * The last argument (pcount) is of type (unsigned long *).
 * If it is not NULL, then *pcount is set equal to the number of records in the list.
 *
 * It is permissible to sort an empty list. If first == end_marker, the returned value will also be end_marker.
 */
void * sort_singly_linked_list(void *p, unsigned index, int (*compare) (void *, void *, void *), void *pointer,
                               unsigned long *pcount, void *end_marker)
{
    unsigned base;
    unsigned long block_size;

    struct record
    {
        struct record  *next[1];
        /* other members not directly accessed by this function */
    };

    struct tape
    {
        struct record  *first, *last;
        unsigned long   count;
    } tape[4];

    /* Distribute the records alternately to tape[0] and tape[1]. */

    tape[0].count = tape[1].count = 0L;
    tape[0].first = NULL;
    base = 0;
    while (p != end_marker)
    {
        struct record  *next    = ((struct record *) p)->next[index];
        ((struct record *) p)->next[index] = tape[base].first;
        tape[base].first = ((struct record *) p);
        tape[base].count++;
        p = next;
        base ^= 1;
    }

    /* If the list is empty or contains only a single record, then */
    /* tape[1].count == 0L and this part is vacuous.               */

    for (base = 0, block_size = 1L; tape[base + 1].count != 0L; base ^= 2, block_size <<= 1)
    {
        int             dest;
        struct tape    *tape0, *tape1;
        tape0 = tape + base;
        tape1 = tape + base + 1;
        dest = base ^ 2;
        tape[dest].count = tape[dest + 1].count = 0;
        for (; tape0->count != 0; dest ^= 1)
        {
            unsigned long n0, n1;
            struct tape    *output_tape = tape + dest;
            n0 = n1 = block_size;
            while (1)
            {
                struct record  *chosen_record;
                struct tape    *chosen_tape;
                if (n0 == 0 || tape0->count == 0)
                {
                    if (n1 == 0 || tape1->count == 0)
                        break;
                    chosen_tape = tape1;
                    n1--;
                }
                else if (n1 == 0 || tape1->count == 0)
                {
                    chosen_tape = tape0;
                    n0--;
                }
                else if ((*compare) (tape0->first, tape1->first, pointer) > 0)
                {
                    chosen_tape = tape1;
                    n1--;
                }
                else
                {
                    chosen_tape = tape0;
                    n0--;
                }
                chosen_tape->count--;
                chosen_record = chosen_tape->first;
                chosen_tape->first = chosen_record->next[index];
                if (output_tape->count == 0)
                    output_tape->first = chosen_record;
                else
                    output_tape->last->next[index] = chosen_record;
                output_tape->last = chosen_record;
                output_tape->count++;
            }
        }
    }

    if (tape[base].count > 1L)
        tape[base].last->next[index] = end_marker;
    if (pcount != NULL)
        *pcount = tape[base].count;
    return tape[base].first;
}

/* Comparision function for sort_singly_linked_list */
static int sort_puddle_by_nrof_free(void *a, void *b, void *args)
{
    if (((struct puddle_info *) a)->nrof_free < ((struct puddle_info *) b)->nrof_free)
        return -1;
    else if (((struct puddle_info *) a)->nrof_free > ((struct puddle_info *) b)->nrof_free)
        return 1;
    else
        return 0;
}

/*
 * Go through the freelists and free puddles with no used chunks.
 * This function is quite slow and dangerous to call.
 * The idea is that it should be called occasionally when CPU usage is low
 *
 * Complexity of this function is O(N (M log M)) where
 * N is number of pools and M is number of puddles in pool
 */
void free_empty_puddles(struct mempool *pool)
{
    /* TODO: Gecko: there's no support for arrays here... I might add it later */
#if 0
    int chunksize_real = sizeof(struct mempool_chunk) + mempools[pool].chunksize;
    int freed = 0;

    struct mempool_chunk *last_free, *chunk;
    struct puddle_info *puddle, *next_puddle;

    if(mempools[pool].flags & MEMPOOL_BYPASS_POOLS)
        return;

    /* Free empty puddles and setup puddle-local freelists */
    for(puddle = mempools[pool].first_puddle_info, mempools[pool].first_puddle_info = NULL;
            puddle != NULL; puddle = next_puddle) {
      uint32 ii;
      next_puddle = puddle->next;

      /* Count free chunks in puddle, and set up a local freelist */
      puddle->first_free = puddle->last_free=NULL;
      puddle->nrof_free = 0;
      for(ii=0; ii<mempools[pool].expand_size; ii++) {
        chunk = (struct mempool_chunk *)((char *)(puddle->first_chunk) + chunksize_real * ii);
        /* Find free chunks. (Notice special case for objects here. Yuck!) */
        if((pool == POOL_OBJECT && OBJECT_FREE((object *)MEM_USERDATA(chunk))) ||
                (pool != POOL_OBJECT && CHUNK_FREE((object *)MEM_USERDATA(chunk)))) {
            if(puddle->nrof_free == 0) {
                puddle->first_free = chunk;
                puddle->last_free = chunk;
                chunk->next = NULL;
            } else {
                chunk->next = puddle->first_free;
                puddle->first_free = chunk;
            }

            puddle->nrof_free ++;
        }
      }

      /* Can we actually free this puddle? */
      if(puddle->nrof_free == mempools[pool].expand_size) {
          /* Yup. Forget about it. */
          free(puddle->first_chunk);
          return_poolchunk(puddle, POOL_PUDDLE);
          mempools[pool].nrof_free -= mempools[pool].expand_size;
          freed++;
      } else {
          /* Nope keep this puddle: put it back into the tracking list */
          puddle->next = mempools[pool].first_puddle_info;
          mempools[pool].first_puddle_info = puddle;
      }
    }

    /* Sort the puddles by amount of free chunks. It will let us set up the freelist so that
     * the chunks from the fullest puddles are used first.
     * This should (hopefully) help us free some of the lesser-used puddles earlier. */
    mempools[pool].first_puddle_info = sort_singly_linked_list(mempools[pool].first_puddle_info, 0, sort_puddle_by_nrof_free, NULL, NULL, NULL);

    /* Finally: restore the global freelist */
    mempools[pool].first_free = &end_marker;
    last_free = &end_marker;
    LOG(llevDebug,"%s free in puddles: ", mempools[pool].chunk_description);
    for(puddle = mempools[pool].first_puddle_info; puddle != NULL; puddle = puddle->next) {
        if(puddle->nrof_free > 0) {
            if(mempools[pool].first_free == &end_marker)
                mempools[pool].first_free = puddle->first_free;
            else
                last_free->next = puddle->first_free;
            puddle->last_free->next = &end_marker;
            last_free = puddle->last_free;
        }

        LOG(llevDebug,"%d ", puddle->nrof_free);
    }
    LOG(llevDebug,"\n");

    LOG(llevInfo,"Freed %d %s puddles\n", freed, mempools[pool].chunk_description);
#endif
    LOG(llevInfo, "Memory recovery temporarily disabled\n");
}
#endif

