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

/* test_teleporter.c 
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

START_TEST (simple_teleporter_without_path)
{
    shstr *path = add_string("/dev/unit_tests/test_teleporter");
    mapstruct *map = ready_map_name(path, path, MAP_STATUS_MULTI, NULL);

    object *lever = locate_beacon(find_string("lever_1"))->env;
    object *apple = locate_beacon(find_string("apple_1"))->env;
 
    fail_unless(apple->x == 1 && apple->y == 1, "Apple initially in incorrect spot");
    fail_unless(apple->map == map, "Apple initially in incorrect map");

    /* Switch the lever and make sure the inverse is true */
    manual_apply(lever, lever, 0);
    
    fail_if(apple->x == 1 && apple->y == 1 && apple->map == map, "Apple wasn't teleported");
    fail_unless(apple->x == 0 && apple->y == 0, "Apple teleported to incorrect spot");
    fail_unless(apple->map == map, "Apple teleported to incorrect map");
}
END_TEST

START_TEST (bad_teleporter_without_path)
{
    shstr *path = add_string("/dev/unit_tests/test_teleporter");
    mapstruct *map = ready_map_name(path, path, MAP_STATUS_MULTI, NULL);

    object *lever = locate_beacon(find_string("lever_1"))->env;
    object *apple = locate_beacon(find_string("apple_1"))->env;
    object *teleport = apple->below ? apple->below : apple->above;
 
    fail_unless(apple->x == 1 && apple->y == 1, "Apple initially in incorrect spot");
    fail_unless(apple->map == map, "Apple initially in incorrect map");

    /* Set telportation location to point outside map */
    teleport->stats.hp = 100;
    teleport->stats.sp = 100;

    /* Switch the lever and make sure the inverse is true */
    manual_apply(lever, lever, 0);
    
    fail_unless(apple->x == 1 && apple->y == 1 && apple->map == map, "Apple was teleported with bad teleporter");
    fail_unless(QUERY_FLAG(teleport, FLAG_REMOVED), "Bad teleporter wasn't removed");
}
END_TEST

Suite *teleporter_suite(void)
{
  Suite *s = suite_create("Teleporter");
  TCase *tc_core = tcase_create("Core");

  tcase_add_checked_fixture(tc_core, setup, teardown);
  
  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, simple_teleporter_without_path);
  tcase_add_test(tc_core, bad_teleporter_without_path);

  return s;
}

#endif
