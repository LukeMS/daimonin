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
    const char *path = shstr_add_string("/dev/unit_tests/test_maploader");
    map_t *map;


    fail_unless(map_is_in_memory(path) == NULL, "Map already loaded");

    map = ready_map_name(NULL, path, MAP_STATUS_MULTI, NULL);
    fail_unless(map_is_in_memory(path) != NULL, "Map not loaded");
    fail_unless(map != NULL, "Couldn't load %s", path);
    fail_unless(strcmp(map->path, path) == 0, "Wierd path");
    fail_unless(map->path == path, "non-shared path");
    fail_if(strcmp(map->name, "Testmap") != 0, "Not the testmap");

    delete_map(map);
    fail_unless(map_is_in_memory(path) == NULL, "Map still loaded");
    FREE_AND_CLEAR_HASH(path);

    fail_if(memleak_detected(), "Memory leak detected");
}
END_TEST

#define TEST_NORMALIZE(src, dst, expect) \
    memset(buf, 0, sizeof(buf)); \
    res = normalize_path(src, dst, buf); \
    fail_if(strcmp(res, expect) != 0, \
            "normalize_path('%s','%s', buf) returned '%s' but '%s' was expected", \
            STRING_SAFE(src), STRING_SAFE(dst), STRING_SAFE(res), STRING_SAFE(expect)); \
    fail_if(res != buf, \
            "normalize_path('%s','%s', buf) didn't return pointer to buf", \
            STRING_SAFE(src), STRING_SAFE(dst));

START_TEST (path_normalizing)
{
    char buf[MAXPATHLEN], *res;

    /* Those are some normal cases */
    TEST_NORMALIZE("/some/path/map", "../other/path/map", "/some/other/path/map");
    TEST_NORMALIZE("/some/path/map", "../../other_path/map2", "/other_path/map2");
    TEST_NORMALIZE("/some/path/map", "/other_path/map2", "/other_path/map2");
    TEST_NORMALIZE("/some/path/map", "../other_path/../map2", "/some/map2");
    TEST_NORMALIZE("/some/path/map", "/other_path/../map2", "/map2");
    TEST_NORMALIZE("/some/path/map", "map2", "/some/path/map2");

    /* Test handling of "./" elements */
    TEST_NORMALIZE("/path/map", "./not_a_very_bad_path/../bar", "/path/bar");
    TEST_NORMALIZE("/path/to/map", "some/./path/bar", "/path/to/some/path/bar");
    TEST_NORMALIZE("/path/to/map", "./map2", "/path/to/map2");

    /* special paths are not touched */
    TEST_NORMALIZE("/some/path/map", "./data/players/very_bad_path/foo", "./data/players/very_bad_path/foo");
    TEST_NORMALIZE("/some/path/map", "./data/instance/very_bad_path/bar", "./data/instance/very_bad_path/bar");
    TEST_NORMALIZE("/some/path/map", "./data/players/very_bad_path/../foo", "./data/players/very_bad_path/../foo");

    /* Some error checking */
    TEST_NORMALIZE("/some/path/map", "../../../other_path/map2", "");
    TEST_NORMALIZE(NULL, "/other_path/map2", "");
}
END_TEST

Suite *map_suite(void)
{
  Suite *s = suite_create("Maps");
  TCase *tc_core = tcase_create("Core");

  tcase_add_checked_fixture(tc_core, setup, dummy_teardown);

  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, map_loading);
  tcase_add_test(tc_core, path_normalizing);

  return s;
}

#endif
