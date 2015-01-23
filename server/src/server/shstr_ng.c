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

#ifdef SS_STATISTICS
static struct statistics
{
    int                     calls;
    int                     hashed;
    int                     strcmps;
    int                     search;
    int                     linked;
} ladd_stats, add_stats, add_ref_stats, free_stats, find_stats, hash_stats;
#define GATHER(n) (++n)
#else /* !SS_STATISTICS */
#define GATHER(n)
#endif /* SS_STATISTICS */

/* define this will make the hash table more secure but
 * also somewhat slower - if no problems happens after
 * some testings (no bug messages from this module)
 * then this define should be disabled
 */
// #define SECURE_SHSTR_HASH

struct shared_string {
    uint32 refcount;
    shstr_t string[0];    /* Area for storing the actual string */
};

/* Our hashtable of shared strings */
static hashtable *shared_strings;

/* Initial number of buckets in the hashtable.
 * (Not very relevant, since it can grow) */
#define SHSTR_INITIAL_TABLE_SIZE 8192

/* Wrappers for statistics gathering */
#ifdef SS_STATISTICS
static struct statistics *s_stats;
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
#endif

/*
 * Initialises the hash-table used by the shared string library.
 */

void init_hash_table(void)
{
    shared_strings = string_hashtable_new(SHSTR_INITIAL_TABLE_SIZE);
#ifdef SS_STATISTICS
    shared_strings->hash = stats_string_hash;
    shared_strings->equals = stats_string_key_equals;
#endif
}

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
 * Description:
 *      This will add 'str' to the hash table. If there's no entry for this
 *      string, a copy will be allocated, and a pointer to that is returned.
 *      Only n characters will be added, and a NULL char will be added at the
 *      end of the string.
 *      This function is useful for adding parts of other buffers.
 * Return values:
 *      - pointer to string identical to str
 */

shstr_t *add_lstring(const char *str, int n)
{
    struct shared_string  *ss;

    GATHER(ladd_stats.calls);

    /* Should really core dump here, since functions should not be calling
     * add_string with a null parameter.  But this might prevent a few
     * core dumps.
     */
    if (str == NULL)
    {
        LOG(llevBug, "BUG: add_string(): try to add null string to hash table\n");
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
#ifdef SS_STATISTICS
    shared_strings->hash = stats_string_hash;
    shared_strings->equals = stats_string_key_equals;
#else
    shared_strings->hash = string_hash;
    shared_strings->equals = string_key_equals;
#endif

    return ss->string;
}

/*
 * Description:
 *      This will add 'str' to the hash table. If there's no entry for this
 *      string, a copy will be allocated, and a pointer to that is returned.
 * Return values:
 *      - pointer to string identical to str
 */

shstr_t *add_string(const char *str)
{
    struct shared_string *ss;

    GATHER(add_stats.calls);

    /* Should really core dump here, since functions should not be calling
     * add_string with a null parameter.  But this will prevent a few
     * core dumps.
     */
    if (str == NULL)
    {
        LOG(llevBug, "BUG: add_string(): try to add null string to hash table\n");
        return NULL;
    }

#ifdef SS_STATISTICS
    s_stats = &add_stats;
#endif

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
 *      This will return the refcount of the string str, which *must*
 *      have been returned from a previous add_string().
 * Return values:
 *      - length
 */

int query_refcount(shstr_t *str)
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

const char *find_string(const char *str)
{
    struct shared_string  *ss;

    GATHER(find_stats.calls);
    GATHER(find_stats.search);

#ifdef SS_STATISTICS
    s_stats = &find_stats;
#endif

    if((ss = hashtable_find(shared_strings, str)))
        return ss->string;
    else
        return NULL;
}

/*
 * Description:
 *      This will increase the refcount of the string str, which *must*
 *      have been returned from a previous add_string().
 * Return values:
 *      - str
 */
shstr_t * add_refcount(shstr_t* str)
{
#ifdef SECURE_SHSTR_HASH
    const char *tmp_str = find_string(str);
    if (!str || str != tmp_str)
    {
        LOG(llevBug, "BUG: add_refcount(shared_string)(): tried to get refcount of an invalid string! >%s<\n", str ? str : ">>NULL<<");
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
 *     This will reduce the refcount, and if it has reached 0, str will
 *     be freed.
 * Return values:
 *     None
 */

void free_string_shared(shstr_t *str)
{
    struct shared_string  *ss;

    /* we check str is in the hash table - if not, something
     * is VERY wrong here. We can also be sure, that SS(str) is
     * a safe operation - except we really messed something
     * strange up inside the hash table. This will check for
     * free, wrong or non SS() objects.
     */
#ifdef SECURE_SHSTR_HASH
    const char     *tmp_str = find_string(str);
    if (!str || str != tmp_str)
    {
        LOG(llevBug, "BUG: free_string_shared(): tried to free a invalid string! >%s<\n", str ? str : ">>NULL<<");
        return;
    }
#endif

    GATHER(free_stats.calls);

    ss = SS(str);
    --ss->refcount;
    if (ss->refcount == 0) {
#ifdef SS_STATISTICS
        s_stats = &free_stats;
#endif
        GATHER(free_stats.search);
        hashtable_erase(shared_strings, str);
        free(ss);
    }
    /*
        else
            LOG(llevDebug,"SS: >%s< #%d dec\n", str,ss->refcount& ~TOPBIT);
    */
}

/*
 * Description:
 *      The routines will gather statistics if SS_STATISTICS is defined.
 *      A call to this function will cause the statistics to be dumped
 *      into the string msg, which must be large.
 * Return values:
 *      pointer to msg
 */
char * ss_dump_statistics(char *msg)
{
    char    line[126];
    msg[0] = '\0';

#ifdef SS_STATISTICS
    sprintf(msg, "%-13s   %6s %6s %6s %6s %6s\n", "hashtable  :", "calls", "hashed", "strcmp", "search", "linked");
    sprintf(line, "%-13s %6d %6d %6d %6d %6d\n", "add_string:", add_stats.calls, add_stats.hashed, add_stats.strcmps,
            add_stats.search, add_stats.linked);
    strcat(msg, line);
    sprintf(line, "%-13s %6d %6d %6d %6d %6d\n", "ladd_string:", ladd_stats.calls, ladd_stats.hashed, ladd_stats.strcmps,
            ladd_stats.search, ladd_stats.linked);
    strcat(msg, line);
    sprintf(line, "%-13s %6d %6d %6d %6d %6d\n", "find_string:", find_stats.calls, find_stats.hashed,
            find_stats.strcmps, find_stats.search, find_stats.linked);
    strcat(msg, line);
    sprintf(line, "%-13s %6d\n", "add_refcount:", add_ref_stats.calls);
    strcat(msg, line);
    sprintf(line, "%-13s %6d\n", "free_string:", free_stats.calls);
    strcat(msg, line);
    sprintf(line, "%-13s %6d", "hashstr:", hash_stats.calls);
    strcat(msg, line);
#endif

    return msg;
}

static void ss_find_totals(int *entries, int *refs, int *links, int what)
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
        int probes = hashtable_num_probes_needed(shared_strings, ss->string);

        *refs += ss->refcount;
        *links += probes;
        if(what & SS_DUMP_TOTALS)
            LOG(llevSystem, "%4d -- %4d refs '%s' (%d probes)\n", (int)i, (int)(ss->refcount), ss->string, probes);
    }
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
void ss_get_totals(int *entries, int *refs, int *links)
{
    ss_find_totals(entries, refs, links, 0);
}

/*
 * Description:
 *      If (what & SS_DUMP_TOTALS) return a string which
 *      says how many entries etc. there are in the table.
 * Return values:
 *      - a string or NULL
 */
char * ss_dump_table(int what)
{
    static char totals[80];
    int         entries = 0, refs = 0, links = 0;

    ss_find_totals(&entries, &refs, &links, what);

    sprintf(totals, "%d entries, %d refs, %d links.", entries, refs, links);

    return totals;
}
