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

/* test_holy.c
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

START_TEST (holy_get_rand_god)
{
    int i;
    for(i=0; i<1000; i++)
        fail_if(get_rand_god() == NULL, "get_rand_god() failed");
}
END_TEST

Suite *holy_suite(void)
{
  Suite *s = suite_create("Holy");
  TCase *tc_core = tcase_create("Core");

  tcase_add_checked_fixture(tc_core, setup, teardown);

  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, holy_get_rand_god);

  return s;
}

#endif
