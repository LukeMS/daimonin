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

/* shstr_ng.c
 *
 * Based on the shstr.c API by Kjetil T. Homme, Oslo 1992.
 * This is a simple shared strings package with a simple interface.
 * Now using the generic hashtable implementation
 *
 * Copyright (C) 2006 Björn Axelsson (gecko)
 */

#include "global.h"

static struct statistics
{
    int calls;
    int hashed;
    int strcmps;
    int search;
    int linked;
} ladd_stats, add_stats, add_ref_stats, free_stats, find_stats, hash_stats;

static struct statistics *s_stats;

#define GATHER(n) (++n)

static int stats_string_key_equals(const hashtable_const_key_t key1, const hashtable_const_key_t key2)
{
    GATHER(s_stats->strcmps);
    return string_key_equals(key1, key2);
}

static hashtable_size_t stats_string_hash(const hashtable_const_key_t key)
{
    GATHER(s_stats->hashed);
    return string_hash(key);
}

struct shared_string {
    uint32 refcount;
    shstr_t string[0];    /* Area for storing the actual string */
};

/* Our hashtable of shared strings */
static hashtable *shared_strings;

/*
 * Allocates and initialises a new shared_string structure,
 * containing the string str.
 */

static struct shared_string *new_shared_string(const char *str, const int n)
{
    struct shared_string  *ss;

    /* Allocate room for a struct which can hold str. Note
     * that some bytes for the string are already allocated in the
     * shared_string struct.
     */
    ss = (struct shared_string *) malloc(sizeof(struct shared_string) + n + 1);
    ss->refcount = 1;
    /*LOG(llevDebug,"SS: >%s< #%d - new\n",str,ss->refcount);*/
    memcpy((char *)ss->string, str, n);
    ((char *)ss->string)[n] = '\0'; /* We aren't guaranteed to be given a
                             0-terminated string */

    return ss;
}

/* Hacked wrappers for inserting strings without terminating null char */
static int s_newstringlength;
static const char *s_newstring;
static int lstring_key_equals(const hashtable_const_key_t key1, const hashtable_const_key_t key2)
{
    register char __res = 1, *k1 = (char *)key1, *k2 = (char *)key2;
    register signed int l = (key1 == s_newstring || key2 == s_newstring ? s_newstringlength : -1);

    GATHER(ladd_stats.strcmps);
    // Try to find a quick answer (see guarantee given about equals() use in hashtable.c)
    if(key2 == HASH_EMPTY_KEY)
        return key1 == HASH_EMPTY_KEY;
    else if(key2 == HASH_DELETED_KEY)
        return key1 == HASH_DELETED_KEY;

    // Fast implementation adapted from Linux kernel source (sys/lib/string.c)
    // Copyright (C) 1991, 1992  Linus Torvalds

    while (l) {
        if (!(__res = (*k1 == *k2++)) || !*k1++)
            break;
        --l;
    }
    return __res;
}

static hashtable_size_t lstring_hash(const hashtable_const_key_t key)
{
    GATHER(ladd_stats.hashed);
    return generic_hash(key, key == s_newstring ? s_newstringlength : (int)strlen(key));
}

/*
 * Initialises the hash-table used by the shared string library.
 */

void shstr_init(void)
{
    /* This is the initial number of buckets in the hashtable. */
    shared_strings = string_hashtable_new(8192);
    shared_strings->hash = stats_string_hash;
    shared_strings->equals = stats_string_key_equals;
}

/*
 * Description:
 *      This will add 'str' to the hash table. If there's no entry for this
 *      string, a copy will be allocated, and a pointer to that is returned.
 * Return values:
 *      - pointer to string identical to str
 */

shstr_t *shstr_add_string(const char *str)
{
    struct shared_string *ss;

    GATHER(add_stats.calls);

    /* Should really core dump here, since functions should not be calling
     * shstr_add_string with a null parameter.  But this will prevent a few
     * core dumps.
     */
    if (str == NULL)
    {
        LOG(llevBug, "BUG: shstr_add_string(): try to add null string to hash table\n");
        return NULL;
    }

    s_stats = &add_stats;
    GATHER(add_stats.search);

    /* Unfortunately, this means two probes in the case of
     * strings that weren't in the hashtable already. */
    if((ss = hashtable_find(shared_strings, str)))
    {
        ss->refcount++;
    } else {
        GATHER(add_stats.search);
        ss = new_shared_string(str, strlen(str));
        hashtable_insert(shared_strings, ss->string, ss);
    }

    return ss->string;
}

/*
 * Description:
 *      This will add 'str' to the hash table. If there's no entry for this
 *      string, a copy will be allocated, and a pointer to that is returned.
 *      Only n characters will be added, and a NULL char will be added at the
 *      end of the string.
 *      This function is useful for adding parts of other buffers.
 * Return values:
 *      - pointer to string identical to str
 */

shstr_t *shstr_add_lstring(const char *str, int n)
{
    struct shared_string  *ss;

    GATHER(ladd_stats.calls);

    /* Should really core dump here, since functions should not be calling
     * shstr_add_string with a null parameter.  But this might prevent a few
     * core dumps.
     */
    if (str == NULL)
    {
        LOG(llevBug, "BUG: shstr_add_string(): try to add null string to hash table\n");
        return NULL;
    }

    /* TODO: this got very ugly and probably not efficient... */

    /* hack the hashtable functions for strings of known length */
    shared_strings->hash = lstring_hash;
    shared_strings->equals = lstring_key_equals;
    s_newstringlength = n;
    s_newstring = str;

    /* Unfortunately, this means two probes in the case of
     * strings that weren't in the hashtable already. */
    GATHER(ladd_stats.search);
    if((ss = hashtable_find(shared_strings, str)))
    {
        ss->refcount++;
    } else {
        GATHER(ladd_stats.search);
        ss = new_shared_string(str, n);
        hashtable_insert(shared_strings, ss->string, ss);
    }

    /* Restore hashing functions */
    shared_strings->hash = stats_string_hash;
    shared_strings->equals = stats_string_key_equals;

    return ss->string;
}

/*
 * Description:
 *      This will increase the refcount of the string str, which *must*
 *      have been returned from a previous shstr_add_string().
 * Return values:
 *      - str
 */
shstr_t * shstr_add_refcount(shstr_t* str)
{
#ifdef DEBUG_SHSTR
    const char *tmp_str = shstr_find(str);
    if (!str || str != tmp_str)
    {
        LOG(llevBug, "BUG: shstr_add_refcount(shared_string)(): tried to get refcount of an invalid string! >%s<\n", str ? str : ">>NULL<<");
        return NULL;
    }
#endif

    GATHER(add_ref_stats.calls);
    ++(SS(str)->refcount);
    /*LOG(llevDebug,"SS: >%s< #%d addref\n", str,SS(str)->refcount);*/
    return str;
}

/*
 * Description:
 *      This will return the refcount of the string str, which *must*
 *      have been returned from a previous shstr_add_string().
 * Return values:
 *      - length
 */

int shstr_query_refcount(shstr_t *str)
{
    return SS(str)->refcount;
}

/*
 * Description:
 *      This will see if str is in the hash table, and return the address
 *      of that string if it exists.
 * Return values:
 *      - pointer to identical string or NULL
 */

const char *shstr_find(const char *str)
{
    struct shared_string  *ss;

    GATHER(find_stats.calls);
    GATHER(find_stats.search);
    s_stats = &find_stats;

    if((ss = hashtable_find(shared_strings, str)))
        return ss->string;
    else
        return NULL;
}

/*
 * Description:
 *     This will reduce the refcount, and if it has reached 0, str will
 *     be freed.
 * Return values:
 *     None
 */

void shstr_free(shstr_t *str)
{
    struct shared_string  *ss;

    /* Lets not make a big song and dance when passed NULL. Means freeing an
     * already free'd (and NULL'd) shstr is valid (if poiintless), a la free()
     * for malloc'd pointers. */
    if (!str)
    {
        return;
    }

    /* we check str is in the hash table - if not, something
     * is VERY wrong here. We can also be sure, that SS(str) is
     * a safe operation - except we really messed something
     * strange up inside the hash table. This will check for
     * free, wrong or non SS() objects.
     */
#ifdef DEBUG_SHSTR
    const char     *tmp_str = shstr_find(str);
    if (!str || str != tmp_str)
    {
        LOG(llevBug, "BUG: shstr_free(): tried to free a invalid string! >%s<\n", str ? str : ">>NULL<<");
        return;
    }
#endif

    GATHER(free_stats.calls);

    ss = SS(str);
    --ss->refcount;
    if (ss->refcount == 0) {
        s_stats = &free_stats;
        GATHER(free_stats.search);
        hashtable_erase(shared_strings, str);
        free(ss);
    }
    /*
        else
            LOG(llevDebug,"SS: >%s< #%d dec\n", str,ss->refcount& ~TOPBIT);
    */
}

/** Returns the number of unique string entries,
 * the total number of references used and the total number
 * of links in the hash table.
 *
 * A large "refs" number indicates either that the memory savings
 * of shared strings are good, or a reference leak.
 *
 * A "links" value much larger than "entries" indicates a too small
 * table size and/or a bad hashing function.
 */
void shstr_get_totals(int *entries, int *refs, int *links)
{
    hashtable_iterator_t i;

    *entries = hashtable_size((void *)shared_strings);

    *refs = 0;
    *links = 0;

    for (i = hashtable_iterator(shared_strings);
        i != hashtable_iterator_end(shared_strings);
        i = hashtable_iterator_next(shared_strings, i))
    {
        struct shared_string *ss = hashtable_iterator_value(shared_strings, i);
        int                   probes = hashtable_num_probes_needed(shared_strings, ss->string);

        *refs += ss->refcount;
        *links += probes;
    }
}

/* shstr_command_dump() logs some statistics about the current shstr usage. */
/* TODO: I'm not convinced this is particularly useful. I think the original
 * author just liked statistics. Still, here it is. Also, what's the deal with
 * shstr_get_totals()? Is this not just a way of totting up the entire hash
 * table, as opposed to the statistics struct which keeps running counts? Might
 * remove it.
 *
 * -- Smacky 20160812 */
int shstr_command_dump(object_t *op, char *params)
{
    char buf[LARGE_BUF];
    int  entries = 0,
         refs = 0,
         links = 0;

    shstr_get_totals(&entries, &refs, &links);
    ndi(NDI_UNIQUE, 0, op, "Shared string statistics dumped to logs!");
    LOG(llevInfo, "SHSTR STATISTICS DUMP\n");
    LOG(llevInfo, "        CALLS  HASHED STRCMP SEARCH LINKED\n");
    LOG(llevInfo, " ADD  : %6d %6d %6d %6d %6d\n", add_stats.calls, add_stats.hashed, add_stats.strcmps, add_stats.search, add_stats.linked);
    LOG(llevInfo, " LADD : %6d %6d %6d %6d %6d\n", ladd_stats.calls, ladd_stats.hashed, ladd_stats.strcmps, ladd_stats.search, ladd_stats.linked);
    LOG(llevInfo, " FIND : %6d %6d %6d %6d %6d\n", find_stats.calls, find_stats.hashed, find_stats.strcmps, find_stats.search, find_stats.linked);
    LOG(llevInfo, " REF  : %6d\n", add_ref_stats.calls);
    LOG(llevInfo, " FREE : %6d\n", free_stats.calls);
    LOG(llevInfo, " HASH : %6d\n", hash_stats.calls);
    LOG(llevInfo, "[ %d entries ] [ %d refs ] [ %d links ]", entries, refs, links);
    return COMMANDS_RTN_VAL_OK_SILENT;
}
