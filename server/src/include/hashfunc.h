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

#ifndef __HASHFUNC_H
#define __HASHFUNC_H

/* Useful special values for pointer keys. See string_key_equals for
 * how they can be used to speed up the equals functions */
#define HASH_EMPTY_KEY ((void *)0)
#define HASH_DELETED_KEY ((void *)~0)

uint32 generic_hash (const char * data, uint32 len);

hashtable_t *string_hashtable_new(hashtable_size_t num_buckets);
hashtable_t *pointer_hashtable_new(hashtable_size_t num_buckets);

hashtable_size_t string_hash(const hashtable_const_key_t key);
int string_key_equals(const hashtable_const_key_t key1, const hashtable_const_key_t key2);

hashtable_size_t int32_hash(const hashtable_const_key_t key_store);
int int32_key_equals(const hashtable_const_key_t key1, const hashtable_const_key_t key2);

hashtable_size_t int64_hash(const hashtable_const_key_t key_ptr);
int int64_key_equals(const hashtable_const_key_t key1, const hashtable_const_key_t key2);

#ifdef WIN32
# define pointer_hash int32_hash
# define pointer_key_equals int32_key_equals
#else /* LINUX and others. */
# if SIZEOF_VOID_P == 4
#  define pointer_hash int32_hash
#  define pointer_key_equals int32_key_equals
# elif SIZEOF_VOID_P == 8
#  define pointer_hash int64_hash
/* Note: this might look wrong, but should always be int32_key_equals.
 * int64_key_equals dereferences the key as a pointer to a 64bit value */
#  define pointer_key_equals int32_key_equals
# else
#  error Unknown pointer size
# endif
#endif

#endif /* ifndef __HASHFUNC_H */
