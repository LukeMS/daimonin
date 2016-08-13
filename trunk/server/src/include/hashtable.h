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

    The author can be reached via e-mail to info@daimonin.org
*/

// C adaptation (C) Copyright 2005 Björn Axelsson
// See hashtable.c for original Google copyright and license
// Don't blame Google for everything ;-)

#ifndef __HASHTABLE_H
#define __HASHTABLE_H

typedef size_t hashtable_size_t;
typedef void * hashtable_key_t;
typedef const void *hashtable_const_key_t;
typedef void * hashtable_value_t;
typedef hashtable_size_t hashtable_iterator_t;

#define HASHTABLE_ILLEGAL_BUCKET ((hashtable_size_t)-1)
#define HASHTABLE_ITERATOR_END HASHTABLE_ILLEGAL_BUCKET

struct hashtable_entry
{
    hashtable_const_key_t key;
    hashtable_value_t value;
};

struct hashtable_t
{
    hashtable_size_t (*hash)(const hashtable_const_key_t);
    int (*equals)(const hashtable_const_key_t, const hashtable_const_key_t);

    struct hashtable_entry *table;
    hashtable_size_t num_elements;
    hashtable_size_t num_deleted;
    hashtable_size_t num_buckets;

    hashtable_size_t shrink_threshold;
    hashtable_size_t enlarge_threshold;
    int consider_shrink;

    hashtable_const_key_t deleted_key;
    hashtable_const_key_t empty_key;
};

/* Public functions */
hashtable_t *hashtable_new(
        hashtable_size_t (*hash_func)(const hashtable_const_key_t),
        int (*equals_func)(const hashtable_const_key_t, const hashtable_const_key_t),
        hashtable_const_key_t deleted_key,
        hashtable_const_key_t empty_key,
        hashtable_size_t num_buckets
        );
void hashtable_delete(hashtable_t *ht);

void hashtable_clear(hashtable_t *ht);
hashtable_value_t hashtable_find(const hashtable_t *const ht, const hashtable_const_key_t key);
int hashtable_insert(hashtable_t *const ht,
        const hashtable_const_key_t key, const hashtable_value_t obj);
int hashtable_erase(hashtable_t *const ht, const hashtable_const_key_t key);

static inline hashtable_size_t hashtable_size(const hashtable_t *const ht)
{
    return ht->num_elements - ht->num_deleted;
}

void hashtable_resize_delta(hashtable_t *const ht,
     const hashtable_size_t delta,
     const hashtable_size_t min_buckets_wanted);

hashtable_size_t hashtable_num_probes_needed(const hashtable_t *const ht, const hashtable_const_key_t key);

/* Hashtable iterators are useful, but 5-10 times slower than a simple
 * linked list iteration. Avoid in time-critical areas */
/* NOTE: the iterators assume a const table while iterating. */
hashtable_iterator_t hashtable_iterator(const hashtable_t *const ht);
hashtable_iterator_t hashtable_iterator_next(const hashtable_t *const ht, hashtable_iterator_t i);

static inline hashtable_const_key_t hashtable_iterator_key(const hashtable_t *const ht, const hashtable_iterator_t i)
{
    return i == HASHTABLE_ITERATOR_END ? NULL : ht->table[i].key;
}
static inline hashtable_value_t hashtable_iterator_value(const hashtable_t *const ht, const hashtable_iterator_t i)
{
    return i == HASHTABLE_ITERATOR_END ? NULL : ht->table[i].value;
}
#define hashtable_iterator_end(x) HASHTABLE_ILLEGAL_BUCKET

#endif /* ifndef __HASHTABLE_H */
