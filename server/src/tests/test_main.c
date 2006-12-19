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

/* test_main.c 
 * Copyright (C) 2005 Björn Axelsson
 */

#include <global.h>

#if defined HAVE_CHECK && defined BUILD_UNIT_TESTS
#include <check.h>
/* See http://check.sourceforge.net/doc/ for a check tutorial */

/* Just a stupid test to make sure everything is set up correctly */
START_TEST (first)
{
    fail_unless(1, "1 is false?");
}
END_TEST

Suite *basic_suite(void)
{
  Suite *s = suite_create("Basics");
  TCase *tc_core = tcase_create("Core");

  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, first);

  return s;
}

/* Suites from other files */
extern Suite *shstr_suite(void);
extern Suite *object_suite(void);
extern Suite *map_suite(void);
extern Suite *hashtable_suite(void);
extern Suite *treasurelists_suite(void);
extern Suite *readable_suite(void);
extern Suite *holy_suite(void);
extern Suite *buttons_suite(void);
extern Suite *lua_suite(void);
    
void run_unit_tests(void)
{
    int failed = 0;
    SRunner *sr = srunner_create(basic_suite());
    srunner_add_suite(sr, shstr_suite());
    srunner_add_suite(sr, object_suite());
    srunner_add_suite(sr, map_suite());
    srunner_add_suite(sr, hashtable_suite());
    srunner_add_suite(sr, treasurelists_suite());
    srunner_add_suite(sr, readable_suite());
    srunner_add_suite(sr, holy_suite());
    srunner_add_suite(sr, buttons_suite());
    srunner_add_suite(sr, lua_suite());
    
    fprintf(stderr, "Running Daimonin Test Suites\n");
    
    srunner_run_all(sr, CK_NORMAL);
    failed += srunner_ntests_failed(sr);
    srunner_free(sr);

    exit(failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

#else
void run_unit_tests(void)
{
    fprintf(stderr, "Unit tests not available. Please reconfigure and recompile\n");
    exit(EXIT_FAILURE);
}
#endif
