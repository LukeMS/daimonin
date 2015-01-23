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

/* test_readable.c
 * Copyright (C) 2006 Björn Axelsson
 */

#include <global.h>
#if defined HAVE_CHECK && defined BUILD_UNIT_TESTS
#include <check.h>
#include "common_support.h"

static void setup()
{
    prepare_memleak_detection();
}

/* This used to return NULL if get_rand_god() failed. Lets try it a few times */
START_TEST (readable_god_info_msg)
{
    int i;
    for(i=0; i<10; i++)
        fail_if(god_info_msg(12, 1024) == NULL, "god_info_msg returned NULL");

    /* Force a failure */
    first_god = NULL;
    fail_if(god_info_msg(12, 1024) == NULL, "god_info_msg returned NULL");
}
END_TEST

/* Track down and test for memleaks in tailor_readable_book() */
START_TEST (readable_memleak)
{
    object_t * book = arch_to_object(find_archetype("book"));
    tailor_readable_ob(book, 5);

    fail_if(memleak_detected(), "_possible_ memory leak detected");
}
END_TEST

/* Track down and test for memleaks in god_info_msg() */
START_TEST (readable_god_info_msg_memleak)
{
    char *foo = god_info_msg(1, 4095);
    fail_if(memleak_detected(), "_possible_ memory leak detected");
}
END_TEST

/* Track down and test for memleaks in change_book() */
START_TEST (readable_change_book_memleak)
{
    object_t * book = arch_to_object(find_archetype("book"));
    FREE_AND_COPY_HASH(book->msg, "I am a book message");
    change_book(book, 5);

    fail_if(memleak_detected(), "_possible_ memory leak detected");
}
END_TEST

Suite *readable_suite(void)
{
  Suite *s = suite_create("Readable");
  TCase *tc_core = tcase_create("Core");

  tcase_add_checked_fixture(tc_core, setup, dummy_teardown);

  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, readable_god_info_msg);
  tcase_add_test(tc_core, readable_god_info_msg_memleak);
  tcase_add_test(tc_core, readable_change_book_memleak);
  tcase_add_test(tc_core, readable_memleak);

  return s;
}

#endif
