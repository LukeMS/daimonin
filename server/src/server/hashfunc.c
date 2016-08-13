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

/*
 * A collection of useful hash functions
 * Please read the license notes of each functions, since not all are
 * GPL-compatible.
 *
 * Everything else in this file not otherwise marked is GPL and
 * (C) Copyright 2005-2006 Björn Axelsson
 */

#include "global.h"

#if defined NO_REDISTRIBUTION
/*
 * SuperFastHash
 * (C) Copyright 2004 by Paul Hsieh
 *
 * NOTE: This function's license is _probably_ not GPL compatible.
 * It is therefore not allowed to include it in binary redistributions
 * of any GPL software.
 * Another, GPL-compatible but slower algorithm is supplied below
 *
 * See http://www.azillionmonkeys.com/qed/hash.html
 *
 * The basic idea of this hash function is to walk the string 16 bits at
 * a time, mixing with a 32-bit internal state; most other hash functions
 * go only 8 bits at a time. This function does only a small amount of
 * mixing at each step, and then applies some final scrambling at the end.
 * [http://webkit.opendarwin.org/blog/?p=8]
 *
 * Paul Hsieh exposition license
 * The content of all text, figures, tables and displayed layout is copyrighted
 * by its author and owner Paul Hsieh unless specifically denoted otherwise.
 * Redistribution is limited to the following conditions:
 * * The redistributor must fully attribute the content's authorship and make
 *   a good faith effort to cite the original location of the original content.
 * * The content may not be modified via excerpt or otherwise with the
 *   exception of additional citations such as described above without prior
 *   consent of Paul Hsieh.
 * * The content may not be subject to a change in license without prior
 *   consent of Paul Hsieh.
 * * The content may be used for commercial purposes.
 *
 * Paul Hsieh derivative license
 * The derivative content includes raw computer source code, ideas, opinions,
 * and excerpts whose original source is covered under another license and
 * transformations of such derivatives. Note that mere excerpts by themselves
 * (with the exception of raw source code) are not considered derivative works
 * under this license. Use and redistribution is limited to the following
 * conditions:
 *
 * * One may not create a derivative work which, in any way, violates the
 *   Paul Hsieh exposition license described above on the original content.
 * * One may not apply a license to a derivative work that precludes anyone
 *   else from using and redistributing derivative content.
 * * One may not attribute any derivative content to authors not involved in
 *   the creation of the content, though an attribution to the author is not
 *   necessary.
 */

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(OS2) || defined(WINDOWS) || defined(WIN32) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16 *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((const uint8_t *)(d))[1] << UINT32_C(8))\
                      +((const uint8_t *)(d))[0])
#endif

uint32 generic_hash (const char *data, uint32 len) {
    uint32 hash=len, tmp;
    int rem;

//    if (len <= 0 || data == NULL) return 0;
    if (len == 0) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= data[sizeof (uint16)] << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += *data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 2;
    hash += hash >> 15;
    hash ^= hash << 10;

    return hash;
}

#else

/* Bob Jenkin's Hash Function
 * Not as fast as SuperFastHash (roughly 50% slower), but with a
 * more relaxed license
 */

#if 0
typedef  uint32 uint32;    /* unsigned 4-byte quantities */
typedef  unsigned char uint8;   /* unsigned 1-byte quantities */
#endif

#define hashsize(n) ((uint32)1<<(n))
#define hashmask(n) (hashsize(n)-1)

/*
--------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.
For every delta with one or two bits set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
* If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
* If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
mix() was built out of 36 single-cycle latency instructions in a
  structure that could supported 2x parallelism, like so:
      a -= b;
      a -= c; x = (c>>13);
      b -= c; a ^= x;
      b -= a; x = (a<<8);
      c -= a; b ^= x;
      c -= b; x = (b>>13);
      ...
  Unfortunately, superscalar Pentiums and Sparcs can't take advantage
  of that parallelism.  They've also turned some of those single-cycle
  latency instructions into multi-cycle latency instructions.  Still,
  this is the fastest good hash I could find.  There were about 2^^68
  to choose from.  I only looked at a billion or so.
--------------------------------------------------------------------
*/
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  len     : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 6*len+35 instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (uint8 **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

uint32 generic_hash (const char *k, uint32 length)
{
//    static uint32 initval = 0xdeadbeef; // Just a semi-random initialization
    register uint32 a,b,c, len;

    /* Set up the internal state */
    len = length;
    a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
    c = 0xdeadbeef; // An arbitrary number
//    c = initval;         /* the previous hash value */

    /*---------------------------------------- handle most of the key */
    while (len >= 12)
    {
        a += (k[0] +((uint32)k[1]<<8) +((uint32)k[2]<<16) +((uint32)k[3]<<24));
        b += (k[4] +((uint32)k[5]<<8) +((uint32)k[6]<<16) +((uint32)k[7]<<24));
        c += (k[8] +((uint32)k[9]<<8) +((uint32)k[10]<<16)+((uint32)k[11]<<24));
        mix(a,b,c);
        k += 12; len -= 12;
    }

    /*------------------------------------- handle the last 11 bytes */
    c += length;
    switch(len)              /* all the case statements fall through */
    {
        case 11: c+=((uint32)k[10]<<24);
        case 10: c+=((uint32)k[9]<<16);
        case 9 : c+=((uint32)k[8]<<8);
                 /* the first byte of c is reserved for the length */
        case 8 : b+=((uint32)k[7]<<24);
        case 7 : b+=((uint32)k[6]<<16);
        case 6 : b+=((uint32)k[5]<<8);
        case 5 : b+=k[4];
        case 4 : a+=((uint32)k[3]<<24);
        case 3 : a+=((uint32)k[2]<<16);
        case 2 : a+=((uint32)k[1]<<8);
        case 1 : a+=k[0];
                 /* case 0: nothing left to add */
    }
    mix(a,b,c);
    /*-------------------------------------------- report the result */
//    initval = c; // Initval for next run
    return c;
}
#endif

/* Convenience function to get a hash table with string keys */
hashtable_t *string_hashtable_new(hashtable_size_t num_buckets)
{
   return hashtable_new(string_hash, string_key_equals, HASH_EMPTY_KEY, HASH_DELETED_KEY, num_buckets);
}

/*
 * Wrapper for string hashing using the above algorithm.
 *
 * Hopefully, at least the strlen() call is inlined
 *
 * this is limited to generating 32-bit hashes, but that should be ok
 * as long as we don't have more than 2^31 entries in the hash table
 *
 * Unfortunately about 1/5 of the time of this function is the strlen() call
 */
hashtable_size_t string_hash(const hashtable_const_key_t key)
{
    return generic_hash(key, strlen((const char *)key));
}

/* Key equality for strings */
int string_key_equals(const hashtable_const_key_t key1, const hashtable_const_key_t key2)
{
    register char __res, *k1 = (char *)key1, *k2 = (char *)key2;

    // Try to find a quick answer (see guarantee given about equals() use in hashtable.c)
    if(key2 == HASH_EMPTY_KEY)
        return key1 == HASH_EMPTY_KEY;
    else if(key2 == HASH_DELETED_KEY)
        return key1 == HASH_DELETED_KEY;

    // Fast strcmp() implementation adapted from Linux kernel source (sys/lib/string.c)
    // Copyright (C) 1991, 1992  Linus Torvalds

    if (!k1 || !k2)
    {
        LOG(llevBug, ">>string_key_equals(): k1 is %s, k2 is %s", STRING_SAFE(k1), STRING_SAFE(k2));
    }

    while (1) {
        if (!(__res = (*k1 == *k2++)) || !*k1++)
            break;
    }
    return __res;
}

/* 32-bit hash by Thomas Wang
 * http://www.concentric.net/~Ttwang/tech/inthash.htm
 *
 * assumes that the 32 bit value was stored directly in the key
 * (i.e. not pointed to)
 *
 * this is limited to generating 32-bit hashes, but that should be ok
 * as long as we don't have more than 2^31 entries in the hash table
 */
hashtable_size_t int32_hash(const hashtable_const_key_t key_store)
{
#ifdef WIN32
    uint32 key = (uint32)key_store;
#else /* LINUX and others. */
# if SIZEOF_VOID_P == 4
    uint32 key = (uint32)key_store;
# elif SIZEOF_VOID_P == 8
    uint32 key = (uint32)((uint64)key_store);
# endif
#endif
    key += ~(key << 15);
    key ^=  (key >> 10);
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11);
    key ^=  (key >> 16);
    return (hashtable_size_t)key;
}

/* Key equality for 32 bit keys (actually any directly stored keys) */
int int32_key_equals(const hashtable_const_key_t key1, const hashtable_const_key_t key2)
{
    return key1 == key2;
}

/* 64-bit hash by Thomas Wang
 * Assumes that the key points to something 64-bit wide
 */
hashtable_size_t int64_hash(const hashtable_const_key_t ptr)
{
    uint64 key = (uint64)ptr;

    key += ~(key << 32);
    key ^= (key >> 22);
    key += ~(key << 13);
    key ^= (key >> 8);
    key += (key << 3);
    key ^= (key >> 15);
    key += ~(key << 27);
    key ^= (key >> 31);
    return (hashtable_size_t)key;
}

/* Key equality for 64 bit keys */
int int64_key_equals(const hashtable_const_key_t key1, const hashtable_const_key_t key2)
{
    return *(uint64 *)key1 == *(uint64 *)key2;
}

/* Convenience function to get a hash table with pointer keys */
hashtable_t *pointer_hashtable_new(hashtable_size_t num_buckets)
{
    static int empty = 0, deleted = 0; /* Anchors for deleted/empty keys */

    return hashtable_new(pointer_hash, pointer_key_equals, &empty, &deleted, num_buckets);
}

