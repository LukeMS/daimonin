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

    The author can be reached via e-mail to daimonin@nord-com.net
*/

/* benchmark_shstr.c 
 * Copyright (C) 2005 Björn Axelsson
 */

#include <global.h>
#if defined HAVE_CHECK && defined BUILD_BENCHMARKS
#include <check.h>
#include "benchmark.h"

#define MAX_WORDS 10000
static char *words[MAX_WORDS];
static int num_words;

static void read_words(void)
{
    FILE *f = fopen("words", "r");
    char buf[1024], *ptr;
    fail_if(f == NULL, "  No 'words' file found. Use for example http://homepages.gold.ac.uk/rachel/words.txt\n");

    if(f == NULL)
        return;

    num_words = 0;
    while(num_words < MAX_WORDS && fgets(buf, 1023, f)) {            
        ptr = malloc(strlen(buf)+1);
        strcpy(ptr, buf);
        words[num_words++] = ptr;
    }
}

static void dummy_teardown()
{
}

/* Test fixture. Since this clears the global list we
 * must run it as a checked fixture so later higher level
 * tests can run */
static void setup()
{
    init_hash_table();
}

/* This basically benchmarks searching and ref increasing */
START_TEST(shstr_benchmark_insert_1)
{
    int i,j;
    printf("\nBenchmarking %d x %d shstr insertions\n", benchmark_repetitions, num_words);

    timer_start();
    for(i=0; i<benchmark_repetitions; i++) {
        for(j=0; j<num_words; j++) {
            add_string(words[j]);
        }
    }
    timer_stop(num_words * benchmark_repetitions); 
}
END_TEST

/* This benchmarks table growth */
START_TEST(shstr_benchmark_insert_2)
{
    int i,j;
    printf("\nBenchmarking %d x %d shstr insertions, erasing after every iteration\n", benchmark_repetitions, num_words);

    timer_start();
    for(i=0; i<benchmark_repetitions; i++) {
        for(j=0; j<num_words; j++) {
            add_string(words[j]);
        }
        init_hash_table(); // Leaks memory like crazy
    }
    timer_stop(num_words * benchmark_repetitions); 
}
END_TEST

Suite *shstr_benchmark_suite(void)
{
  Suite *s = suite_create("Sharedstrings");
  TCase *tc_core = tcase_create("Core");

  tcase_add_unchecked_fixture(tc_core, read_words, dummy_teardown);
  tcase_add_checked_fixture(tc_core, setup, dummy_teardown);
  
  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, shstr_benchmark_insert_1);
  tcase_add_test(tc_core, shstr_benchmark_insert_2);
  tcase_set_timeout(tc_core, 0);

  return s;
}

#endif
