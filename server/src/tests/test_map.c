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

/* test_map.c 
 * Copyright (C) 2005 Björn Axelsson
 */

#include <global.h>
#if defined HAVE_CHECK && defined BUILD_UNIT_TESTS
#include <check.h>
#include "common_support.h"

static void setup()
{
    object_gc(); // Collect garbage
    prepare_memleak_detection();
}

/* Test map loading and swapping out, and make sure
 * everything is freed after the swapping */
START_TEST (map_loading)
{
    const char *path = add_string("/dev/unit_tests/test_maploader");
    mapstruct *map;
    

    fail_unless(has_been_loaded_sh(path) == NULL, "Map already loaded");
    
    map = ready_map_name(NULL, path, MAP_STATUS_MULTI, NULL);
    fail_unless(has_been_loaded_sh(path) != NULL, "Map not loaded");
    fail_unless(map != NULL, "Couldn't load %s", path);
    fail_unless(strcmp(map->path, path) == 0, "Wierd path");
    fail_unless(map->path == path, "non-shared path");
    fail_if(strcmp(map->name, "Testmap") != 0, "Not the testmap");
    
    delete_map(map);
    fail_unless(has_been_loaded_sh(path) == NULL, "Map still loaded");
    FREE_ONLY_HASH(path);
    
    fail_if(memleak_detected(), "Memory leak detected");
}
END_TEST

Suite *map_suite(void)
{
  Suite *s = suite_create("Maps");
  TCase *tc_core = tcase_create("Core");

  tcase_add_checked_fixture(tc_core, setup, dummy_teardown);
  
  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, map_loading);

  return s;
}

#endif
