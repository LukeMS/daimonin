// C adaptation (C) Copyright 2005 Björn Axelsson
// See hashtable.c for original Google copyright and license
// Don't blame Google for everything ;-)

#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#ifdef WIN32
#include <win32.h>
#include <stddef.h>
#define SIZEOF_VOID_P 4
#ifndef MIN_GW
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef uint32_t ub4;    /* unsigned 4-byte quantities */
typedef unsigned _int64 uint64_t;
#else
#include <stdint.h>
#endif
#else
#include <stdint.h>
#include <autoconf.h>
#endif

#include <string.h>
#include <stdlib.h>

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

typedef struct hashtable_s
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
} hashtable;

/* Public functions */
hashtable *hashtable_new(
        hashtable_size_t (*hash_func)(const hashtable_const_key_t),
        int (*equals_func)(const hashtable_const_key_t, const hashtable_const_key_t),
        hashtable_const_key_t deleted_key,
        hashtable_const_key_t empty_key,
        hashtable_size_t num_buckets
        );
void hashtable_delete(hashtable *ht);

void hashtable_clear(hashtable *ht);
hashtable_value_t hashtable_find(const hashtable *const ht, const hashtable_const_key_t key);
int hashtable_insert(hashtable *const ht,
        const hashtable_const_key_t key, const hashtable_value_t obj);
int hashtable_erase(hashtable *const ht, const hashtable_const_key_t key);

static inline hashtable_size_t hashtable_size(const hashtable *const ht)
{
    return ht->num_elements - ht->num_deleted;
}

void hashtable_resize_delta(hashtable *const ht,
     const hashtable_size_t delta,
     const hashtable_size_t min_buckets_wanted);

hashtable_size_t hashtable_num_probes_needed(const hashtable *const ht, const hashtable_const_key_t key);

/* Hashtable iterators are useful, but 5-10 times slower than a simple
 * linked list iteration. Avoid in time-critical areas */
/* NOTE: the iterators assume a const table while iterating. */
hashtable_iterator_t hashtable_iterator(const hashtable *const ht);
hashtable_iterator_t hashtable_iterator_next(const hashtable *const ht, hashtable_iterator_t i);

static inline hashtable_const_key_t hashtable_iterator_key(const hashtable *const ht, const hashtable_iterator_t i)
{
    return i == HASHTABLE_ITERATOR_END ? NULL : ht->table[i].key;
}
static inline hashtable_value_t hashtable_iterator_value(const hashtable *const ht, const hashtable_iterator_t i)
{
    return i == HASHTABLE_ITERATOR_END ? NULL : ht->table[i].value;
}
#define hashtable_iterator_end(x) HASHTABLE_ILLEGAL_BUCKET
#endif
