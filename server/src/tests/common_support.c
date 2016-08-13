/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2006 Michael Toennies

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

    The author can be reached via e-mail to info@daimonin.org
*/

/* common_support.c
 * Copyright (C) 2006 Björn Axelsson
 */

#include <global.h>

#if defined BUILD_BENCHMARKS || defined BUILD_UNIT_TESTS
#include "common_support.h"

#define MAX_WORDS 10000
char *words[MAX_WORDS];
int num_words;
int word_lengths[MAX_WORDS];

void read_words(void)
{
    FILE *f = fopen(WORDS_FILE, "r");
    char buf[1024], *ptr;
    num_words = 0;
    if(f) {
        while(num_words < MAX_WORDS && fgets(buf, 1023, f)) {
            ptr = malloc(strlen(buf)+1);
            strcpy(ptr, buf);
            word_lengths[num_words] = strlen(ptr);
            words[num_words++] = ptr;
        }

        /*assert(strcmp(words[0], words[1]) != 0);
        assert(strlen(words[0]) > 0);
        assert(strlen(words[1]) > 0); */

        fclose(f);
    } else {
        fprintf(stderr, "!  No \"%s\" words file found. Generating %d random words\n", WORDS_FILE, MAX_WORDS);
        fprintf(stderr, "   You can use for example http://homepages.gold.ac.uk/rachel/words.txt\n");

        int i,j;
        for(i=0; i<MAX_WORDS; i++)
        {
            word_lengths[i] = rand()%20+5;
            words[i] = malloc(word_lengths[i]+1);
            words[i][word_lengths[i]] = '\0';
            for(j=0; j<word_lengths[i]; j++)
                words[i][j] = rand()%26 + 'a';
        }
        num_words = MAX_WORDS;
    }
}

/* Dump all non-free objects in memory */
void dump_objects()
{
#ifdef MEMPOOL_TRACKING
    struct puddle_info *puddle = pool_object->first_puddle_info;
    int i;
    object_t *obj;
    while(puddle)
    {
        for(i=0; i<pool_object->expand_size; i++)
        {
            obj = MEM_USERDATA((char *)puddle->first_chunk + i * (sizeof(struct mempool_chunk) + pool_object->chunksize));
            if(! OBJECT_FREE(obj))
                LOG(llevDebug, "obj '%s'(%d)(%s) #=%d\n", STRING_OBJ_NAME(obj),  obj->count,
                        QUERY_FLAG(obj, FLAG_REMOVED) ? "removed" : "in use",
                        obj->nrof);
        }
        puddle = puddle->next;
    }
#endif
}

/* Dump all non-free treasurelist tweaks in memory */
void dump_treasurelist_tweaks()
{
#ifdef MEMPOOL_TRACKING
    struct puddle_info *puddle = pool_tlist_tweak->first_puddle_info;
    struct mempool * pool = pool_tlist_tweak;
    int i;
    tlist_tweak *tweak;
    while(puddle)
    {
        for(i=0; i<pool->expand_size; i++)
        {
            tweak = MEM_USERDATA((char *)puddle->first_chunk + i * (sizeof(struct mempool_chunk) + pool->chunksize));
            if(! CHUNK_FREE(tweak))
                LOG(llevDebug, "tweak '%s'\n", STRING_SAFE(tweak->name));
        }
        puddle = puddle->next;
    }
#endif
}

/* Dump all non-free objlinks in memory */
void dump_objlinks()
{
#ifdef MEMPOOL_TRACKING
    struct mempool * pool = pool_objectlink;
    struct puddle_info *puddle = pool->first_puddle_info;
    int i;
    objectlink_t *objlink;
    while(puddle)
    {
        for(i=0; i<pool->expand_size; i++)
        {
            objlink = MEM_USERDATA((char *)puddle->first_chunk + i * (sizeof(struct mempool_chunk) + pool->chunksize));
            if(! CHUNK_FREE(objlink))
                switch(objlink->flags & 0xff)
                {
                    case OBJLNK_FLAG_OB:
                        LOG(llevDebug, "object objlink: flags 0x%04x, obj: freed %s, name %s, count %d\n", objlink->flags, OBJECT_FREE(objlink->objlink.ob)?"yes":"no",STRING_OBJ_NAME(objlink->objlink.ob), objlink->objlink.ob->count);
                        break;
                    default:
                        LOG(llevDebug, "objlink flags = 0x%04x\n", objlink->flags);
                }
        }
        puddle = puddle->next;
    }
#endif
}


/* Shallowly dump the inventory of an object_t */
void dump_inventory(object_t *op)
{
    object_t *tmp;
    LOG(llevDebug, "inventory of %s:\n", STRING_OBJ_NAME(op));
    for(tmp = op->inv; tmp; tmp = tmp->below)
    {
        LOG(llevDebug, "    '%s'(%d)(%s) #=%d\n", STRING_OBJ_NAME(tmp),  tmp->count,
                QUERY_FLAG(tmp, FLAG_REMOVED) ? "removed" : "in use",
                tmp->nrof);
    }
}

/* Useful functions for detecting memory leaks
 * (string reference leaks, object leaks etc)
 */

void prepare_memleak_detection()
{
}

int memleak_detected()
{
    int refs2, entries2, links2;
    int i, k, mempool_leak = 0;

    cleanup_without_exit();
    shstr_get_totals(&entries2, &refs2, &links2);

    for(i=0; i<nrof_mempools; i++)
        for (k = 0; k < MEMPOOL_NROF_FREELISTS; k++)
            if (mempools[i] != pool_puddle
                    && mempools[i]->nrof_allocated[k] != mempools[i]->nrof_free[k])
                mempool_leak = 1;
    if(mempool_leak)
    {
        LOG(llevDebug, "At least one mempool wasn't completely emptied:\n");
        dump_mempool_statistics(NULL, NULL, NULL);
        return -1;
    }

    if(0 != pool_object->nrof_allocated[0] - pool_object->nrof_free[0])
    {
        LOG(llevDebug, "Some objects not returned (diff = %d)\n", (pool_object->nrof_allocated[0] - pool_object->nrof_free[0]));
        dump_objects();
        return -1;
    }

    if(0 != entries2)
    {
        LOG(llevDebug, "Some string(s) not freed (diff = %d)\n", entries2);
        return -1;
    }

    /* adjust stringrefs for refs stored in other places than
     * objects (might need extending! */
    if(0 != refs2)
    {
        LOG(llevDebug, "Some string(s) not dereferenced (diff = %d)\n", refs2);
        return -1;
    }


    return 0;
}

#endif
