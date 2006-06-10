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

/* test_buttons.c 
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

START_TEST (buttons_check_inv_recursive)
{
    mapstruct *map = ready_map_name(add_string("/dev/unit_tests/test_check_inv"), 0);

    object *check1 = locate_beacon(find_string("check1"))->env;
    object *check2 = locate_beacon(find_string("check2"))->env;
    object *check3 = locate_beacon(find_string("check3"))->env;
    object *check4 = locate_beacon(find_string("check4"))->env;
    object *check5 = locate_beacon(find_string("check5"))->env;
    object *check8 = locate_beacon(find_string("check8"))->env;
    object *check9 = locate_beacon(find_string("check9"))->env;
    
    object *key1 = locate_beacon(find_string("key1"))->env;
    object *key2 = locate_beacon(find_string("key2"))->env;
    object *key3 = locate_beacon(find_string("key3"))->env;
    object *key4 = locate_beacon(find_string("key4"))->env;

    object *cont1 = arch_to_object(find_archetype("chest"));
    object *cont2 = arch_to_object(find_archetype("chest"));
    object *cont3 = arch_to_object(find_archetype("chest"));
    object *cont4 = arch_to_object(find_archetype("chest"));
   
    remove_ob(key1);
    key1 = insert_ob_in_ob(key1, cont1);
    remove_ob(key2);
    key2 = insert_ob_in_ob(key2, cont2);
    remove_ob(key3);
    key3 = insert_ob_in_ob(key3, cont3);
    remove_ob(key4);
    key4 = insert_ob_in_ob(key4, cont4);

    fail_if(check_inv_recursive(cont1, check1) == NULL, "key1 check1 failed");
    fail_if(check_inv_recursive(cont1, check2) != NULL, "key1 check2 failed");
    fail_if(check_inv_recursive(cont1, check3) != NULL, "key1 check3 failed");
    fail_if(check_inv_recursive(cont1, check4) != NULL, "key1 check4 failed");
    fail_if(check_inv_recursive(cont1, check5) != NULL, "key1 check5 failed");
    fail_if(check_inv_recursive(cont1, check8) == NULL, "key1 check8 failed");
    fail_if(check_inv_recursive(cont1, check9) != NULL, "key1 check9 failed");
    
    fail_if(check_inv_recursive(cont2, check1) != NULL, "key2 check1 failed");
    fail_if(check_inv_recursive(cont2, check2) == NULL, "key2 check2 failed");
    fail_if(check_inv_recursive(cont2, check3) != NULL, "key2 check3 failed");
    fail_if(check_inv_recursive(cont2, check4) != NULL, "key2 check4 failed");
    fail_if(check_inv_recursive(cont2, check5) != NULL, "key2 check5 failed");
    fail_if(check_inv_recursive(cont2, check8) != NULL, "key2 check8 failed");
    fail_if(check_inv_recursive(cont2, check9) == NULL, "key2 check9 failed");
    
    fail_if(check_inv_recursive(cont3, check1) == NULL, "key3 check1 failed");
    fail_if(check_inv_recursive(cont3, check2) != NULL, "key3 check2 failed");
    fail_if(check_inv_recursive(cont3, check3) == NULL, "key3 check3 failed");
    fail_if(check_inv_recursive(cont3, check4) != NULL, "key3 check4 failed");
    fail_if(check_inv_recursive(cont3, check5) == NULL, "key3 check5 failed");
    fail_if(check_inv_recursive(cont3, check8) == NULL, "key3 check8 failed");
    fail_if(check_inv_recursive(cont3, check9) == NULL, "key3 check9 failed");
    
    fail_if(check_inv_recursive(cont4, check1) != NULL, "key4 check1 failed");
    fail_if(check_inv_recursive(cont4, check2) == NULL, "key4 check2 failed");
    fail_if(check_inv_recursive(cont4, check3) != NULL, "key4 check3 failed");
    fail_if(check_inv_recursive(cont4, check4) == NULL, "key4 check4 failed");
    fail_if(check_inv_recursive(cont4, check5) == NULL, "key4 check5 failed");
    fail_if(check_inv_recursive(cont4, check8) == NULL, "key4 check8 failed");
    fail_if(check_inv_recursive(cont4, check9) == NULL, "key4 check9 failed");
}
END_TEST

Suite *buttons_suite(void)
{
  Suite *s = suite_create("Buttons");
  TCase *tc_core = tcase_create("Core");

  tcase_add_checked_fixture(tc_core, setup, teardown);
  
  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, buttons_check_inv_recursive);

  return s;
}

#endif
