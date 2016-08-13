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

// Copyright (c) 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// ---
// Author: Craig Silverstein
//
// A dense hashtable is a particular implementation of
// a hashtable: one that is meant to minimize memory allocation.
// It does this by using an array to store all the data.  We
// steal a value from the key space to indicate "empty" array
// elements (ie indices where no item lives) and another to indicate
// "deleted" elements.
//
// To minimize allocation and pointer overhead, we use internal
// probing, in which the hashtable is a single table, and collisions
// are resolved by trying to insert again in another bucket.  The
// most cache-efficient internal probing schemes are linear probing
// (which suffers, alas, from clumping) and quadratic probing, which
// is what we implement by default.

// ---
// C adaptation (C) Copyright 2005 Björn Axelsson
// Original C++ code from http://sourceforge.net/projects/goog-sparsehash/
// Don't blame Google for everything ;-)

// You can change the following below:
// HT_OCCUPANCY_FLT      -- how full before we double size
// HT_EMPTY_FLT          -- how empty before we halve size
// HT_MIN_BUCKETS        -- default smallest bucket size
//
// How to decide what values to use?
// HT_EMPTY_FLT's default of .4 * OCCUPANCY_FLT, is probably good.
// HT_MIN_BUCKETS is probably unnecessary since you can specify
// (indirectly) the starting number of buckets at construct-time.
// For HT_OCCUPANCY_FLT, you can use this chart to try to trade-off
// expected lookup time to the space taken up.  By default, this
// code uses quadratic probing, though you can change it to linear
// via _JUMP below if you really want to.
//
// From http://www.augustana.ca/~mohrj/courses/1999.fall/csc210/lecture_notes/hashing.html
// NUMBER OF PROBES / LOOKUP       Successful            Unsuccessful
// Quadratic collision resolution   1 - ln(1-L) - L/2    1/(1-L) - L - ln(1-L)
// Linear collision resolution     [1+1/(1-L)]/2         [1+1/(1-L)2]/2
//
// -- HT_OCCUPANCY_FLT --         0.10  0.50  0.60  0.75  0.80  0.90  0.99
// QUADRATIC COLLISION RES.
//    probes/successful lookup    1.05  1.44  1.62  2.01  2.21  2.85  5.11
//    probes/unsuccessful lookup  1.11  2.19  2.82  4.64  5.81  11.4  103.6
// LINEAR COLLISION RES.
//    probes/successful lookup    1.06  1.5   1.75  2.5   3.0   5.5   50.5
//    probes/unsuccessful lookup  1.12  2.5   3.6   8.5   13.0  50.0  5000.0

#include "global.h"

// The probing method
// Linear probing
// #define JUMP_(key, num_probes)    ( 1 )
// Quadratic-ish probing
#define JUMP_(key, num_probes)    ( num_probes )

#define HASHTABLE_MIN_BUCKETS 8
// #define HASHTABLE_MIN_BUCKETS 32
#define HASHTABLE_OCCUPANCY_FLT 0.5
#define HASHTABLE_EMPTY_FLT (0.4 * HASHTABLE_OCCUPANCY_FLT)

// Private functions
static void hashtable_reset_thresholds(hashtable_t *const ht);
static void hashtable_resize(hashtable_t *const ht, const hashtable_size_t sz);
static hashtable_size_t hashtable_min_size(const hashtable_size_t num_elts,
        const hashtable_size_t min_buckets_wanted);
static void hashtable_maybe_shrink(hashtable_t *const ht);
static int hashtable_insert_noresize(hashtable_t *const ht,
        const hashtable_const_key_t key,
        const hashtable_value_t obj);
static void hashtable_find_position(const hashtable_t *const ht, const hashtable_const_key_t key,
        hashtable_size_t *const found_position, hashtable_size_t *const insert_position);

/* Notes on the equals(a,b) function usage:
 * 1. If a is empty_key or deleted_key, b is guaranteed to be empty_key or
 *     deleted key.
 *     (This can speed up some equality functions like string
 *     comparision with special empty/deleted values)
 * 2. b is always empty_key, deleted_key or a key already stored in the hash
 *     (b is never a "key" parameter from calls like find() or erase())
 */

/*
 * Creator / destructor
 */

/** Create a new hashtable.
 * @param hash_func key hashing function
 * @param equals_func key equality test function
 * @param empty_key key for empty buckets. Must not be part of normal keyspace.
 * @param deleted_key key for deleted buckets. Must not be part of normal keyspace.
 * @param num_buckets initial number of buckets in hashtable
 * @return pointer to an empty hashtable
 */
hashtable_t *hashtable_new(
        hashtable_size_t (*hash_func)(const hashtable_const_key_t),
        int (*equals_func)(const hashtable_const_key_t, const hashtable_const_key_t),
        hashtable_const_key_t empty_key,
        hashtable_const_key_t deleted_key,
        hashtable_size_t num_buckets
        )
{
    hashtable_size_t i;

    hashtable_t *ht = malloc(sizeof(hashtable_t));
    assert(ht);

    ht->hash = hash_func;
    ht->equals = equals_func;

    ht->num_elements = 0;
    ht->num_deleted = 0;
    ht->num_buckets = hashtable_min_size(0, num_buckets);
    ht->table = malloc(sizeof(struct hashtable_entry) * ht->num_buckets);
    assert(ht->table);

    hashtable_reset_thresholds(ht);

    ht->empty_key = empty_key;
    ht->deleted_key = deleted_key;

    for(i=0; i<ht->num_buckets; i++)
        ht->table[i].key = empty_key;

    return ht;
}

/** Delete the hashtable from memory.
 * This does not free any extra data associated with keys or values,
 * so make sure to free that before calling this function.
 * @param ht hashtable to delete. may be NULL
 */
void hashtable_delete(hashtable_t *ht)
{
    if(ht) {
        free(ht->table);
        ht->table = NULL; // To catch reuse errors
        free(ht);
    }
}

/*
 * Public insert/search/remove
 */

/** Remove all entries from the hashtable.
 * This does not free any extra data associated with keys or values,
 * so make sure to free that before calling this function.
 * @param ht hasthtable to erase
 */
void hashtable_clear(hashtable_t *ht)
{
    assert(NULL);
    // Not implemented (yet)
#if 0
  // It's always nice to be able to clear a table without deallocating it
  void clear() {
    num_buckets = min_size(0,0);          // our new size
    reset_thresholds();
    table = (value_type *) realloc(table, num_buckets * sizeof(*table));
    assert(table);
    set_empty(0, num_buckets);
    num_elements = 0;
    num_deleted = 0;
  }
#endif
}

/** find an entry in a hashtable.
 * @param ht hashtable to search in.
 * @param key to search for
 * @return value for key, or NULL if not found.
 */
hashtable_value_t hashtable_find(const hashtable_t *const ht, const hashtable_const_key_t key)
{
    hashtable_size_t found_position, insert_position;
    if((ht->num_elements - ht->num_deleted) == 0)
        return NULL;
    hashtable_find_position(ht, key, &found_position, &insert_position);
    if(found_position == HASHTABLE_ILLEGAL_BUCKET)
        return NULL;
    else
        return ht->table[found_position].value;
}

// TODO: possibly add a int hashtable_find2(hashtable_t *ht, const hashtable_const_key_t key, const hashtable_value_t *value)
// for keyspaces that include NULL

/** Insert a new entry into the hashtable.
 * @param ht hashtable to insert in.
 * @param key of entry to insert
 * @param obj value of entry to insert
 * @return 1 if the entry was inserted, 0 if key already existed
 */
int hashtable_insert(hashtable_t *const ht,
        const hashtable_const_key_t key, const hashtable_value_t obj)
{
    hashtable_resize_delta(ht, 1, 0); // Make room if needed

    return hashtable_insert_noresize(ht, key, obj);
}

/** Delete single entry from hashtable.
 * @param ht hasthable to delete from.
 * @param key key of entry to delete.
 * @return 1 if the entry was deleted, 0 if key didn't exist
 */
int hashtable_erase(hashtable_t *const ht, const hashtable_const_key_t key)
{
    hashtable_size_t found_pos, insert_pos;

    hashtable_find_position(ht, key, &found_pos, &insert_pos);
    if(found_pos == HASHTABLE_ILLEGAL_BUCKET)
        return 0;
    else
    {
        assert(ht->num_deleted == 0 || !ht->equals(ht->table[found_pos].key, ht->deleted_key));
        ht->table[found_pos].key = ht->deleted_key;
        ht->num_deleted++;
        ht->consider_shrink = 1;
        return 1;
    }
}

/** Force a resize of the hashtable.
 * We'll let you resize a hashtable -- though this makes us copy and rehash all buckets.
 * When you resize, you say, "make it big enough for this many more elements"
 * This is also useful for forcing a resize after a bunch of erases
 * @param ht hashtable to resize
 * @param delta relative size change (or 0)
 * @param min_buckets_wanted minimum size wanted
 */
void hashtable_resize_delta(hashtable_t *const ht,
        const hashtable_size_t delta,
        const hashtable_size_t min_buckets_wanted)
{
    hashtable_size_t resize_to;

    if(ht->consider_shrink)
        hashtable_maybe_shrink(ht);
    if(ht->num_buckets > min_buckets_wanted &&
            (ht->num_elements + delta) <= ht->enlarge_threshold)
        return;

    resize_to = hashtable_min_size(ht->num_elements + delta, min_buckets_wanted);

    if(resize_to > ht->num_buckets)
        hashtable_resize(ht, resize_to);
}

/*
 * Public informative/statistical functions
 */

/** Get the number of probes needed to find the key.
 * @param ht hashtable to operate on.
 * @param key key to search for.
 * @return number of probes needed to find the key, or HASHTABLE_ILLEGAL_BUCKET if the key isn't in the table)
 */
hashtable_size_t hashtable_num_probes_needed(const hashtable_t *const ht, const hashtable_const_key_t key)
{
    hashtable_size_t num_probes = 0;              // how many times we've probed
    const hashtable_size_t bucket_count_minus_one = ht->num_buckets - 1;
    hashtable_size_t bucknum = ht->hash(key) & bucket_count_minus_one;
    while (num_probes < ht->num_buckets)
    {
        if(ht->equals(ht->table[bucknum].key, ht->empty_key))
            return HASHTABLE_ILLEGAL_BUCKET;
        else if (ht->equals(key, ht->table[bucknum].key))
            return num_probes + 1;

        num_probes++;
        bucknum = (bucknum + JUMP_(key, num_probes)) & bucket_count_minus_one;
    }
    return HASHTABLE_ILLEGAL_BUCKET;
}

/** Get an iterator for the hashtable.
 * The iterator will indicate the first available element of the
 * hashtable.
 *
 * Any iterator obtained before an insert or remove operation is
 * invalid after the operation. The result of accessing an invalid
 * iterator is undefined (but expect crashes).
 *
 * If the hashtable was empty, the returned iterator will be equal to
 * hashtable_iterator_last(ht).
 *
 * @param ht hasthable to get an iterator for. May be NULL.
 * @return an iterator indicating the first available element of ht,
 * or hashtable_iterator_last(ht) if none was found.
 */
hashtable_iterator_t hashtable_iterator(const hashtable_t *const ht)
{
    hashtable_iterator_t i;

    if(ht == NULL || ht->num_elements - ht->num_deleted == 0)
        return HASHTABLE_ITERATOR_END;

    for(i=0; i<ht->num_buckets; i++)
    {
        if(! ht->equals(ht->table[i].key, ht->empty_key) &&
                (ht->num_deleted == 0 || !ht->equals(ht->table[i].key, ht->deleted_key)))
            return i;
    }
    return HASHTABLE_ITERATOR_END;
}

/** Get the next element using an iterator.
 * The iterator will indicate the next available element of the
 * hashtable.
 * @see hashtable_iterator
 * @return an iterator indicating the next available element of ht,
 * or hashtable_iterator_last(ht) if none was found.
 */
hashtable_iterator_t hashtable_iterator_next(const hashtable_t *const ht, hashtable_iterator_t i)
{
    for(++i; i<ht->num_buckets; i++)
    {
        if(! ht->equals(ht->table[i].key, ht->empty_key) &&
                (ht->num_deleted == 0 || !ht->equals(ht->table[i].key, ht->deleted_key)))
            return i;
    }
    return HASHTABLE_ITERATOR_END;
}

/*
 * Private functions
 */

static void hashtable_reset_thresholds(hashtable_t *const ht)
{
    ht->enlarge_threshold = (hashtable_size_t)((double)ht->num_buckets * HASHTABLE_OCCUPANCY_FLT);
    ht->shrink_threshold = (hashtable_size_t) ((double)ht->num_buckets * HASHTABLE_EMPTY_FLT);
    ht->consider_shrink = 0;
}

/*
 * Note: storing the hash value in the hashtable_entry might seem like a good
 * idea, since rehashing e.g. 16384 strings might take a while.
 * The cost is of course memory, especially since some value types might want
 * to cache their hashvalue themselves anyways.
 * But, if the table is created big enough from the start we probably only have
 * to resize it a few times during its lifetime.
 */

/** resize ht to sz buckets.
 * Used to actually do the rehashing when we grow/shrink a hashtable
 * @param ht hasthable to resize and rehash
 * @param sz new number of buckets for ht.
 */
static void hashtable_resize(hashtable_t *const ht, const hashtable_size_t sz)
{
    struct hashtable_entry *new_table;
    hashtable_size_t i;
    hashtable_size_t bucket_count_minus_one;

    assert((sz & (sz-1)) == 0);       // sz should already be a power of two
    assert(sz >= hashtable_size(ht)); // we should never overshrink

    new_table = malloc(sizeof(struct hashtable_entry) * sz);
    assert(new_table);
    for(i=0; i<sz; i++)
        new_table[i].key = ht->empty_key;

    // We could use insert() here, but since we know there are
    // no duplicates and no deleted items, we can be more efficient
    bucket_count_minus_one = sz - 1;
    for(i=0; i<ht->num_buckets; i++)
    {
        hashtable_size_t num_probes = 0;     // how many times we've probed
        hashtable_size_t bucknum;

        if(ht->equals(ht->table[i].key, ht->empty_key) ||
                (ht->num_deleted > 0 && ht->equals(ht->table[i].key, ht->deleted_key)))
            continue;

        for (bucknum = ht->hash(ht->table[i].key) & bucket_count_minus_one;
                !ht->equals(new_table[bucknum].key, ht->empty_key); // not empty
                bucknum = (bucknum + JUMP_(key, num_probes)) & bucket_count_minus_one)
        {
            ++num_probes;
            assert(num_probes < sz); // or else the hashtable is full
        }
        new_table[bucknum].key = ht->table[i].key;
        new_table[bucknum].value = ht->table[i].value;
    }

    free(ht->table);
    ht->table = new_table;

    ht->num_buckets = sz;
    ht->num_elements -= ht->num_deleted;
    ht->num_deleted = 0;

    hashtable_reset_thresholds(ht);
}

/** This is the smallest size a hashtable can be without being too crowded.
 * @param num_elts wanted number of elements
 * @param min_buckets_wanted suggested minimum number of buckets
 */
static hashtable_size_t hashtable_min_size(
        const hashtable_size_t num_elts,
        const hashtable_size_t min_buckets_wanted)
{
    hashtable_size_t sz = HASHTABLE_MIN_BUCKETS;             // min buckets allowed
    while ( sz < min_buckets_wanted || num_elts >= sz * HASHTABLE_OCCUPANCY_FLT )
        sz *= 2;
    return sz;
}

/** Used after a string of deletes */
static void hashtable_maybe_shrink(hashtable_t *const ht)
{
    assert(ht->num_elements >= ht->num_deleted);
    assert((ht->num_buckets & (ht->num_buckets-1)) == 0); // is a power of two
    assert(ht->num_buckets >= HASHTABLE_MIN_BUCKETS);

    if ( (ht->num_elements-ht->num_deleted) <= ht->shrink_threshold &&
         ht->num_buckets > HASHTABLE_MIN_BUCKETS )
    {
        hashtable_size_t sz = ht->num_buckets / 2;    // find how much we should shrink
        while ( sz > HASHTABLE_MIN_BUCKETS &&
                (ht->num_elements - ht->num_deleted) <= sz * HASHTABLE_EMPTY_FLT )
            sz /= 2;                            // stay a power of 2

        hashtable_resize(ht, sz);
    }
    ht->consider_shrink = 0;                // because we just considered it
}

/** Returns a pair of positions.
 * Note: because of deletions where-to-insert is not trivial: it's the
 * first deleted bucket we see, as long as we don't find the key later
 *
 * @param ht hashtable to search/insert in
 * @param key key to search for/insert
 * @param found_position where the object is, or ILLEGAL_BUCKET if object is not found
 * @param insert_position where it would go if you wanted to insert it, or ILLEGAL_BUCKET if it was already in the table.
 */
static void hashtable_find_position(const hashtable_t *const ht, const hashtable_const_key_t key,
        hashtable_size_t *const found_position, hashtable_size_t *const insert_position)
{
    hashtable_size_t num_probes = 0;              // how many times we've probed
    const hashtable_size_t bucket_count_minus_one = ht->num_buckets - 1;
    hashtable_size_t bucknum = ht->hash(key) & bucket_count_minus_one;
    *insert_position = HASHTABLE_ILLEGAL_BUCKET;
    while (1)
    {
        if(ht->equals(ht->table[bucknum].key, ht->empty_key))
        {
            *found_position = HASHTABLE_ILLEGAL_BUCKET;
            if(*insert_position == HASHTABLE_ILLEGAL_BUCKET)
                *insert_position = bucknum;
            return;
        }
        else if (ht->num_deleted > 0 && ht->equals(ht->table[bucknum].key, ht->deleted_key))
        {
            if(*insert_position == HASHTABLE_ILLEGAL_BUCKET)
                *insert_position = bucknum;
        }
        else if (ht->equals(key, ht->table[bucknum].key))
        {
            *insert_position = HASHTABLE_ILLEGAL_BUCKET;
            *found_position = bucknum;
            return;
        }

        num_probes++;
        bucknum = (bucknum + JUMP_(key, num_probes)) & bucket_count_minus_one;
        assert(num_probes < ht->num_buckets); // don't probe too many times!
    }
}

/** If you know the hashtable is big enough to hold obj, use this routine */
static int hashtable_insert_noresize(
        hashtable_t *const ht,
        const hashtable_const_key_t key,
        const hashtable_value_t obj)
{
    hashtable_size_t found_position, insert_position;
    hashtable_find_position(ht, key, &found_position, &insert_position);
    if(found_position != HASHTABLE_ILLEGAL_BUCKET)
        return 0;
    else
    {
        if(ht->equals(ht->table[insert_position].key, ht->deleted_key))
            ht->num_deleted--;
        else
            ht->num_elements++;
        ht->table[insert_position].value = obj;
        ht->table[insert_position].key = key;
        return 1;
    }
}

#undef JUMP_
