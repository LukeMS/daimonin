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

/* benchmark_hashtable.c
 * Copyright (C) 2005 Björn Axelsson
 */

#include <global.h>
#if defined HAVE_CHECK && defined BUILD_BENCHMARKS
#include <check.h>
#include "benchmark.h"
#include "common_support.h"

START_TEST (benchmark_raw_hash)
{
   int i,j;
   printf("\nBenchmarking %d x %d hashes of known length\n", benchmark_repetitions, num_words);

   timer_start();
   for(i=0; i<benchmark_repetitions; i++) {
       for(j=0; j<num_words; j++) {
           generic_hash(words[j], word_lengths[j]);
       }
   }
   timer_stop(num_words * benchmark_repetitions);
}
END_TEST

START_TEST (benchmark_string_hash)
{
   int i,j;
   printf("\nBenchmarking %d x %d string hashings\n", benchmark_repetitions, num_words);

   timer_start();
   for(i=0; i<benchmark_repetitions; i++) {
       for(j=0; j<num_words; j++) {
           string_hash(words[j]);
       }
   }
   timer_stop(num_words * benchmark_repetitions);
}
END_TEST

START_TEST(benchmark_hashtable_insert)
{
   int i,j;
   hashtable_t *ht = string_hashtable_new(32);

   printf("\nBenchmarking %d x %d hashtable insertions\n", benchmark_repetitions, num_words);

   timer_start();
   for(i=0; i<benchmark_repetitions; i++) {
       for(j=0; j<num_words; j++) {
           hashtable_insert(ht, words[j], words[j]);
       }
   }
   timer_stop(num_words * benchmark_repetitions);
}
END_TEST

START_TEST (benchmark_hashtable_insert_erase)
{
   int i,j;
   hashtable_t *ht;

   printf("\nBenchmarking %d x %d hashtable insertions, erasing after every repetition\n", benchmark_repetitions, num_words);

   timer_start();
   for(i=0; i<benchmark_repetitions; i++) {
       /* TODO: replace with hashtable_erase when implemented */
       ht = string_hashtable_new(10000);

       for(j=0; j<num_words; j++) {
           hashtable_insert(ht, words[j], words[j]);
       }

       hashtable_delete(ht);
   }
   timer_stop(num_words * benchmark_repetitions);
}
END_TEST

START_TEST (benchmark_hashtable_find)
{
   int i,j;
   hashtable_t *ht = string_hashtable_new(32);

   printf("\nBenchmarking %d x %d hashtable finds\n", benchmark_repetitions, num_words);

   // Populate the hashtable
   for(j=0; j<num_words; j++) {
       hashtable_insert(ht, words[j], words[j]);
   }

   timer_start();
   for(i=0; i<benchmark_repetitions; i++) {
       for(j=0; j<num_words; j++) {
           hashtable_find(ht, words[j]);
       }
   }
   timer_stop(num_words * benchmark_repetitions);
}
END_TEST

/*
 * Comparative benchmarks. Helps finding out when using a hashtable
 * is better than a simple linked list
 */

/* This benchmark shows that iterating over a hashtable is about 5-10 times slower than
 * iterating a simple linked list when using string keys */
START_TEST (benchmark_hashtable_iterate)
{
   int i,j = 0;
   hashtable_t *ht = string_hashtable_new(32);
   hashtable_iterator_t curr;

   printf("\nBenchmarking %d x %d string hashtable iterator iterations\n", benchmark_repetitions, num_words);

   // Populate the hashtable
   for(j=0; j<num_words; j++) {
       hashtable_insert(ht, words[j], words[j]);
   }

   timer_start();
   for(i=0; i<benchmark_repetitions; i++) {
       for(curr = hashtable_iterator(ht);
               curr != hashtable_iterator_end(ht);
               curr = hashtable_iterator_next(ht, curr))
       {
           /* Need to do something that doesn't get optimized away */
           if(hashtable_iterator_value(ht, curr))
               j++;
       }
   }
   timer_stop(num_words * benchmark_repetitions);
}
END_TEST

START_TEST (benchmark_hashtable_iterate_2)
{
   int i,j = 0;
   hashtable_t *ht = pointer_hashtable_new(32);
   hashtable_iterator_t curr;

   printf("\nBenchmarking %d x %d pointer hashtable iterator iterations\n", benchmark_repetitions, num_words);

   // Populate the hashtable
   for(j=0; j<num_words; j++) {
       hashtable_insert(ht, words[j], words[j]);
   }

   timer_start();
   for(i=0; i<benchmark_repetitions; i++) {
       for(curr = hashtable_iterator(ht);
               curr != hashtable_iterator_end(ht);
               curr = hashtable_iterator_next(ht, curr))
       {
           /* Need to do something that doesn't get optimized away */
           if(hashtable_iterator_value(ht, curr))
               j++;
       }
   }
   timer_stop(num_words * benchmark_repetitions);
}
END_TEST

START_TEST (benchmark_linked_list_iterate)
{
    int i,j = 0;

    struct ll {
        struct ll * next;
        char *value;
    };

    struct ll *first = NULL, *curr;

    printf("\nBenchmarking %d x %d linked list iterations\n", benchmark_repetitions, num_words);

    /* Populate the list */
    for(j=0; j<num_words; j++) {
        curr = malloc(sizeof(struct ll));
        curr->next = first;
        curr->value = words[j];
        first = curr;
    }

   timer_start();
   for(i=0; i<benchmark_repetitions; i++)
   {
       for(curr = first; curr; curr = curr->next)
       {
           /* Need to do something that doesn't get optimized away */
           if(curr->value)
               j++;
       }
   }
   timer_stop(num_words * benchmark_repetitions);
}
END_TEST

/* See how hashtables of different sizes compare against linked lists */
/* It turns out that for string keys (strcmp() comparision) hashtable beats
 * linear search at any size >= 8 elements! */
START_TEST (benchmark_hashtable_sizes)
{
    int power;
    printf("\n");

    for(power = 3; (1 << power) < num_words; power++)
    {
        int i,j = 0;
        hashtable_t *ht = string_hashtable_new(8);

        // Populate the hashtable
        for(j=0; j<(1<<power); j++) {
            hashtable_insert(ht, words[j], words[j]);
        }

        printf("Benchmarking %d x %d hashtable string find with table size %d\n", benchmark_repetitions, num_words, hashtable_size(ht));

        timer_start();
        for(i=0; i<benchmark_repetitions; i++)
        {
            for(j=0; j<num_words; j++)
            {
                hashtable_find(ht, words[j]);
            }
        }
        timer_stop(num_words * benchmark_repetitions);

        hashtable_delete(ht);
    }
}
END_TEST

START_TEST (benchmark_linked_list_sizes)
{
    int power;
    struct ll {
        struct ll * next;
        char *value;
    };


    printf("\n");

    for(power = 3; power < 8; power++)
    {
        int i,j = 0;
        struct ll *first = NULL, *curr, *next;

        /* Populate the list */
        for(j=0; j<(1 << power); j++) {
            curr = malloc(sizeof(struct ll));
            curr->next = first;
            curr->value = words[j];
            first = curr;
        }

        printf("Benchmarking %d x %d linear string find with list size %d\n", benchmark_repetitions, num_words, 1 << power);

        timer_start();
        for(i=0; i<benchmark_repetitions; i++)
        {
            for(j=0; j<num_words; j++)
            {
                for(curr = first; curr; curr = curr->next)
                    if(strcmp(curr->value, words[j]) == 0)
                        break;
            }
        }
        timer_stop(num_words * benchmark_repetitions);

        for(curr = first; curr; curr = next)
        {
            next = curr->next;
            free(curr);
        }
    }
}
END_TEST

/* See how hashtables of different sizes compare against linked lists */
/* It turns out that for simple keys (pointer comparision) hashtable beats
 * linear search when size >= 64 elements */
START_TEST (benchmark_hashtable_sizes_2)
{
    int power;
    printf("\n");

    for(power = 3; (1 << power) < num_words; power++)
    {
        int i,j = 0;
        hashtable_t *ht = pointer_hashtable_new(8);

        // Populate the hashtable
        for(j=0; j<(1<<power); j++) {
            hashtable_insert(ht, words[j], words[j]);
        }

        printf("Benchmarking %d x %d hashtable pointer find with table size %d\n", benchmark_repetitions, num_words, hashtable_size(ht));

        timer_start();
        for(i=0; i<benchmark_repetitions; i++)
        {
            for(j=0; j<num_words; j++)
            {
                hashtable_find(ht, words[j]);
            }
        }
        timer_stop(num_words * benchmark_repetitions);

        hashtable_delete(ht);
    }
}
END_TEST

START_TEST (benchmark_linked_list_sizes_2)
{
    int power;
    struct ll {
        struct ll * next;
        char *value;
    };

    printf("\n");

    for(power = 3; power < 8; power++)
    {
        int i,j = 0;
        struct ll *first = NULL, *curr, *next;

        /* Populate the list */
        for(j=0; j<(1 << power); j++) {
            curr = malloc(sizeof(struct ll));
            curr->next = first;
            curr->value = words[j];
            first = curr;
        }

        printf("Benchmarking %d x %d linear pointer find with list size %d\n", benchmark_repetitions, num_words, 1 << power);

        timer_start();
        for(i=0; i<benchmark_repetitions; i++)
        {
            for(j=0; j<num_words; j++)
            {
                for(curr = first; curr; curr = curr->next)
                    if(curr->value == words[j])
                        break;
            }
        }
        timer_stop(num_words * benchmark_repetitions);

        for(curr = first; curr; curr = next)
        {
            next = curr->next;
            free(curr);
        }
    }
}
END_TEST

Suite *hashtable_benchmark_suite(void)
{
  Suite *s = suite_create("hash(table)");
  TCase *tc_core = tcase_create("Core");

  tcase_add_unchecked_fixture(tc_core, read_words, dummy_teardown);

  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, benchmark_raw_hash);
  tcase_add_test(tc_core, benchmark_string_hash);
  tcase_add_test(tc_core, benchmark_hashtable_insert);
  tcase_add_test(tc_core, benchmark_hashtable_insert_erase);
  tcase_add_test(tc_core, benchmark_hashtable_find);
  tcase_add_test(tc_core, benchmark_hashtable_iterate);
  tcase_add_test(tc_core, benchmark_hashtable_iterate_2);
  tcase_add_test(tc_core, benchmark_linked_list_iterate);
  tcase_add_test(tc_core, benchmark_hashtable_sizes);
  tcase_add_test(tc_core, benchmark_linked_list_sizes);
  tcase_add_test(tc_core, benchmark_hashtable_sizes_2);
  tcase_add_test(tc_core, benchmark_linked_list_sizes_2);
  tcase_set_timeout(tc_core, 0);

  return s;
}
#endif
