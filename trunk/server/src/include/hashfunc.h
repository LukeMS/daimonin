#ifndef _HASHFUNC_H_
#define _HASHFUNC_H_

#include "hashtable.h"
#ifndef WIN32
#include "autoconf.h"
#endif

/* Useful special values for pointer keys. See string_key_equals for
 * how they can be used to speed up the equals functions */
#define HASH_EMPTY_KEY ((void *)0)
#define HASH_DELETED_KEY ((void *)~0)

uint32_t generic_hash (const char * data, uint32_t len);

hashtable *string_hashtable_new(hashtable_size_t num_buckets);
hashtable *pointer_hashtable_new(hashtable_size_t num_buckets);

hashtable_size_t string_hash(const hashtable_const_key_t key);
int string_key_equals(const hashtable_const_key_t key1, const hashtable_const_key_t key2);

hashtable_size_t int32_hash(const hashtable_const_key_t key_store);
int int32_key_equals(const hashtable_const_key_t key1, const hashtable_const_key_t key2);

hashtable_size_t int64_hash(const hashtable_const_key_t key_ptr);
int int64_key_equals(const hashtable_const_key_t key1, const hashtable_const_key_t key2);

#if SIZEOF_VOID_P == 4
#define pointer_hash int32_hash
#define pointer_key_equals int32_key_equals
#elif SIZEOF_VOID_P == 8
#define pointer_hash int64_hash
#define pointer_key_equals int64_key_equals
#else
#error Unknown pointer size
#endif

#endif
