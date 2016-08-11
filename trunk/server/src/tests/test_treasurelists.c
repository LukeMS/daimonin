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

/* test_treasurelists.c
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

static void teardown()
{
}

/* Test map loading and swapping out, and make sure
 * everything is freed after the swapping */
START_TEST (treasurelist_memleak)
{
    const char *path = shstr_add_string("/dev/unit_tests/test_treasurelists");
    map_t *map;

    map = ready_map_name(path, NULL, MAP_STATUS_MULTI, NULL);

/*
    dump_inventory(locate_beacon(shstr_find("beacon1"))->env);
    dump_inventory(locate_beacon(shstr_find("beacon2"))->env);
    dump_inventory(locate_beacon(shstr_find("beacon3"))->env);
    dump_inventory(locate_beacon(shstr_find("beacon4"))->env);
    dump_inventory(locate_beacon(shstr_find("beacon5"))->env);
    dump_inventory(locate_beacon(shstr_find("beacon6"))->env);
    dump_inventory(locate_beacon(shstr_find("beacon7"))->env);
    dump_inventory(locate_beacon(shstr_find("beacon8"))->env);
*/
    delete_map(map);
    SHSTR_FREE(path);

    fail_if(memleak_detected(), "Memory leak detected");
}
END_TEST

Suite *treasurelists_suite(void)
{
  Suite *s = suite_create("Treasurelists");
  TCase *tc_core = tcase_create("Core");

  tcase_add_checked_fixture(tc_core, setup, teardown);

  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, treasurelist_memleak);

  return s;
}

#endif
