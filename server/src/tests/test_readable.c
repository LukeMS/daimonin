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

/* test_readable.c 
 * Copyright (C) 2006 Björn Axelsson
 */

#include <global.h>
#if defined HAVE_CHECK && defined BUILD_UNIT_TESTS
#include <check.h>

static void setup()
{
    object_gc(); // Collect garbage
}

static void teardown()
{
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

Suite *readable_suite(void)
{
  Suite *s = suite_create("Readable");
  TCase *tc_core = tcase_create("Core");

  tcase_add_checked_fixture(tc_core, setup, teardown);
  
  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, readable_god_info_msg);

  return s;
}

#endif
