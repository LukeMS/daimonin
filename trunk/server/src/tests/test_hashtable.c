/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2005 Michael Toennies

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

/* test_hashtable.c
 * Copyright (C) 2005 Björn Axelsson
 */

#include <global.h>
#if defined HAVE_CHECK && defined BUILD_UNIT_TESTS
#include <check.h>
#include "common_support.h"

START_TEST(hashtable_basic_operations)
{
    char **ptr;
    char *strings1[] = { "hello", "world", "this", "is", "working", NULL };

    hashtable_t *ht = string_hashtable_new(32);

    for(ptr = strings1; *ptr; ptr++)
        hashtable_insert(ht, *ptr, *ptr);
    fail_unless(hashtable_size(ht) == 5, "Size didn't increase after insert");
    for(ptr = strings1; *ptr; ptr++)
        fail_unless(hashtable_find(ht, *ptr) == *ptr, "Couldn't find inserted strings");

    fail_unless(hashtable_erase(ht, "hello"), "Couldn't erase inserted string");
    fail_if(hashtable_erase(ht, "does not exist"), "Could erase non-inserted string");
    fail_if(hashtable_erase(ht, "hello"), "Could erase string twice");
    fail_unless(hashtable_find(ht, "hello") == NULL, "string still in hash after erasing");
    for(ptr = strings1 + 1; *ptr; ptr++)
        fail_unless(hashtable_find(ht, *ptr) == *ptr, "erased unrelated string");

    fail_unless(hashtable_size(ht) == 4, "Incorrect size after erase");

    hashtable_delete(ht);
}
END_TEST

/* Make sure hash table actually shrinks when elements are deleted */
/* This test is a little shaky since it depends on the word list not
 * containing duplicate words and not being to small */
START_TEST(hashtable_shrinkage)
{
    int j;
    int maxbuckets;
    hashtable_t *ht = string_hashtable_new(32);

    for(j=0; j<num_words; j++) {
        hashtable_insert(ht, words[j], words[j]);
        fail_unless(ht->num_buckets >= j, "Hashtable didn't expand");
    }

    /* Test shrinkage through resize_delta */
    maxbuckets = ht->num_buckets;
    fail_unless(hashtable_size(ht) >= num_words, "More words in list than in hashtable. Duplicate words?");
    fail_unless(ht->num_buckets >= num_words, "Hashtable didn't expand");

    /* Remove half the objects. When we go below ~6550 objects the hash should be shrinkable */

    for(j=0; j<num_words/2; j++)
        hashtable_erase(ht, words[j]);

    fail_unless(ht->num_buckets == maxbuckets, "Hashtable shrunk too early");
    hashtable_resize_delta(ht, 0,0); /* Force shrinkage */
    fail_unless(ht->num_buckets < maxbuckets, "Hashtable didn't shrink (expected < %d buckets. Got %d)", maxbuckets, ht->num_buckets);

    /* Test shrinkage through insert */
    maxbuckets = ht->num_buckets;

    /* Remove the rest */
    for(j=num_words/2; j<num_words; j++)
        hashtable_erase(ht, words[j]);

    fail_unless(ht->num_buckets == maxbuckets, "Hashtable shrunk too early");
    hashtable_insert(ht, "test1234","test1234"); /* indirect shrinkage */

    fail_unless(ht->num_buckets < maxbuckets, "Hashtable didn't shrink (expected < %d buckets. Got %d)", maxbuckets, ht->num_buckets);
}
END_TEST

START_TEST (hashtable_iterators)
{
    int j;
    hashtable_t *ht = string_hashtable_new(32);
    hashtable_iterator_t i;
    int count = 0;

    for(j=0; j<num_words; j++)
        hashtable_insert(ht, words[j], (void *)j);

    /* Iterate through the hash table and make sure we can
     * find every word in the wordlist */
    for(i=hashtable_iterator(ht);
            i != hashtable_iterator_end(ht);
            i = hashtable_iterator_next(ht, i))
    {
        int foundit = 0;
        for(j=0; j<num_words; j++) {
            if(strcmp(hashtable_iterator_key(ht, i), words[j]) == 0) {
                hashtable_iterator_value(ht, i) == (void *)j;
                foundit = 1;
                count++;
                break;
            }
        }
        fail_unless(foundit, "String \"%s\" in hashtable but not in wordlist", hashtable_iterator_key(ht, i));
    }

    fail_unless(count == num_words && num_words == hashtable_size(ht), "Couldn't iterate over all words in wordlist");
}
END_TEST

START_TEST (hashtable_efficiency)
{
    hashtable_t *ht = string_hashtable_new(32);
    int j, probesum= 0;

    for(j=0; j<num_words; j++)
        hashtable_insert(ht, words[j], (void *)j);

    for(j=0; j<num_words; j++)
        probesum += hashtable_num_probes_needed(ht, words[j]);

    fail_unless(probesum >= num_words, "Finding words need less probes than words?");

    fail_if((double)probesum / (double)num_words > 3.0, "Find needs more than 3.0 probes per average!");

    // fprintf(stderr, "  Total number of probes: %d. Average per key: %0.2f\n", probesum, (double)probesum / (double)num_words);
}
END_TEST

Suite *hashtable_suite(void)
{
  Suite *s = suite_create("hashtable");
  TCase *tc_core = tcase_create("Core");

  tcase_add_unchecked_fixture(tc_core, read_words, dummy_teardown);

  suite_add_tcase (s, tc_core);

  tcase_add_test(tc_core, hashtable_basic_operations);
  tcase_add_test(tc_core, hashtable_shrinkage);
  tcase_add_test(tc_core, hashtable_efficiency);
  tcase_set_timeout(tc_core, 10); /* This test might need some time on slow machines */
  tcase_add_test(tc_core, hashtable_iterators);

  return s;
}

#endif
