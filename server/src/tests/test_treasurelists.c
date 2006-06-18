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

/* test_treasurelists.c 
 * Copyright (C) 2006 Bj�rn Axelsson
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
    const char *path = add_string("/dev/unit_tests/test_treasurelists");
    mapstruct *map;
    
    map = ready_map_name(path, 0);

/*    
    dump_inventory(locate_beacon(find_string("beacon1"))->env);
    dump_inventory(locate_beacon(find_string("beacon2"))->env);
    dump_inventory(locate_beacon(find_string("beacon3"))->env);
    dump_inventory(locate_beacon(find_string("beacon4"))->env);
    dump_inventory(locate_beacon(find_string("beacon5"))->env);
    dump_inventory(locate_beacon(find_string("beacon6"))->env);
    dump_inventory(locate_beacon(find_string("beacon7"))->env);
    dump_inventory(locate_beacon(find_string("beacon8"))->env);
*/    
    delete_map(map);
    FREE_ONLY_HASH(path);
    
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
