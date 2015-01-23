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

/* test_shstr.c
 * Copyright (C) 2005 Björn Axelsson
 */

#include <global.h>
#if defined HAVE_CHECK && defined BUILD_UNIT_TESTS
#include <check.h>

/* Test fixture. Since this clears the global list we
 * must run it as a checked fixture so later higher level
 * tests can run */
static void setup()
{
    init_hash_table();
}

static void teardown()
{
}

/* Test that we can clear the table */
START_TEST (shstr_t_init)
{
    int entries, refs, links;
    ss_get_totals(&entries, &refs, &links);
    fail_unless(entries == 0 && refs == 0 && links == 0, "hashtable not empty");
}
END_TEST

/* Test that strings can be inserted and are shared */
START_TEST (shstr_t_add_string)
{
    int entries, refs, links;
    const char *a, *b;

    a = add_string("hello");
    b = add_string("world");
    ss_get_totals(&entries, &refs, &links);
    fail_unless(entries == 2 && refs == 2, "Insertion failed");
    fail_unless(strcmp(a, "hello") == 0, "String was modified");
    fail_unless(strcmp(b, "world") == 0, "String was modified");

    a = add_string("hello");
    b = add_string("world");
    ss_get_totals(&entries, &refs, &links);
    fail_unless(entries == 2 && refs == 4, "Strings are not shared");
    fail_unless(strcmp(a, "hello") == 0, "String was modified");
    fail_unless(strcmp(b, "world") == 0, "String was modified");
}
END_TEST

/* Test that add_lstring really heeds the length parameter */
START_TEST (shstr_t_add_lstring)
{
    int entries, refs, links;
    const char *a, *b;

    a = add_lstring("hello foobar", 5);
    b = add_lstring("world snafu", 5);
    ss_get_totals(&entries, &refs, &links);
    fail_unless(entries == 2 && refs == 2, "Insertion failed");
    fail_unless(strcmp(a, "hello") == 0, "String was modified");
    fail_unless(strcmp(b, "world") == 0, "String was modified");

    a = add_lstring("hello foobar", 5);
    b = add_lstring("world snafu", 5);
    ss_get_totals(&entries, &refs, &links);
    fail_unless(entries == 2 && refs == 4, "Strings are not shared (%d!=2 || %d!=4)", entries, refs);
    fail_unless(strcmp(a, "hello") == 0, "String was modified");
    fail_unless(strcmp(b, "world") == 0, "String was modified");
}
END_TEST

/* Test that query_refcount works */
START_TEST (shstr_t_query_refcount)
{
    const char *a, *b;

    a = add_string("hello");
    b = add_string("world");
    fail_unless(query_refcount(a) == 1, "String refcount mismatches");
    fail_unless(query_refcount(b) == 1, "String refcount mismatches");

    a = add_string("hello");
    b = add_string("world");
    fail_unless(query_refcount(a) == 2, "String refcount mismatches");
    fail_unless(query_refcount(b) == 2, "String refcount mismatches");
}
END_TEST

/* Test that find_string works */
START_TEST (shstr_t_find_string)
{
    const char *a, *b;

    a = add_string("hello");
    b = add_string("world");

    fail_unless(find_string("hello") == a, "find_string failed");
    fail_unless(find_string("world") == b, "find_string failed");
}
END_TEST

/* Test that add_refcount works */
START_TEST (shstr_t_add_refcount)
{
    int entries, refs, links;
    const char *a, *b;

    a = add_string("hello");
    b = add_string("world");

    a = add_refcount(a);
    b = add_refcount(b);
    fail_unless(query_refcount(a) == 2, "String refcount mismatches");
    fail_unless(query_refcount(b) == 2, "String refcount mismatches");
    ss_get_totals(&entries, &refs, &links);
    fail_unless(entries == 2 && refs == 4, "Strings are not shared");
}
END_TEST

/* Test that free_string_shared works */
START_TEST (shstr_t_free_string_shared)
{
    int entries, refs, links;
    const char *a, *b;

    a = add_string("hello");
    b = add_string("world");
    a = add_string("hello");

    ss_get_totals(&entries, &refs, &links);
    fail_unless(entries == 2 && refs == 3, "Strings are not shared");

    free_string_shared(a);
    ss_get_totals(&entries, &refs, &links);
    fail_unless(entries == 2 && refs == 2, "Strings are not unrefed");

    free_string_shared(b);
    ss_get_totals(&entries, &refs, &links);
    fail_unless(entries == 1 && refs == 1, "Strings are not unrefed");

    fail_if(find_string("world"), "String still findable");
    fail_unless(find_string("hello") != NULL, "String not findable");
}
END_TEST

Suite *shstr_suite(void)
{
  Suite *s = suite_create("Sharedstrings");
  TCase *tc_core = tcase_create("Core");

  tcase_add_checked_fixture(tc_core, setup, teardown);

  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, shstr_init);
  tcase_add_test(tc_core, shstr_add_string);
  tcase_add_test(tc_core, shstr_add_lstring);
  tcase_add_test(tc_core, shstr_query_refcount);
  tcase_add_test(tc_core, shstr_find_string);
  tcase_add_test(tc_core, shstr_add_refcount);
  tcase_add_test(tc_core, shstr_free_string_shared);

  return s;
}

#endif
