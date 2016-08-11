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

/* test_object.c
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

static void teardown()
{
}

/* Test the basic creation and deleting operations */
START_TEST (object_creation)
{
    object_t *obj;
    int nfree;

    obj = get_object();

    fail_if(obj == NULL, "No object");
    fail_if(obj->count == 0, "No tag");
    fail_if(OBJECT_FREE(obj), "Still free");
    fail_unless(QUERY_FLAG(obj, FLAG_REMOVED), "Not removed");
    nfree = pool_object->nrof_free[0];

    object_gc(); // Collect garbage

    fail_unless(OBJECT_FREE(obj), "Removed object not freed");
    fail_unless(obj->count == 0, "Tag not reset");
    fail_unless(pool_object->nrof_free[0] == nfree+1, "nrof_free not incremented");
}
END_TEST

/* test that object creation from archetypes work */
START_TEST (arch_creation)
{
    object_t *obj;
    obj = get_archetype("qwerty");
    fail_if(obj == NULL, "Didn't get any object");
    fail_unless(strcmp(obj->arch->name, "empty_archetype") == 0, "Not a singularity");

    obj = get_archetype("teddy"); // Just a random object
    fail_if(obj == NULL, "Didn't get any object");
    fail_unless(strcmp(obj->arch->name, "teddy") == 0, "Not a teddy");
}
END_TEST

/* Test that strings used in objects are correctly dereferenced */
START_TEST (object_strings)
{
    object_t *obj = get_object();

    int entries1, refs1, links1;
    int entries2, refs2, links2;
    int nrof_refs;

    shstr_get_totals(&entries1, &refs1, &links1);

    FREE_AND_COPY_HASH(obj->name, "qwerty1234");
    FREE_AND_COPY_HASH(obj->title, "qwerty1234");
    FREE_AND_COPY_HASH(obj->race, "qwerty1234");
    FREE_AND_COPY_HASH(obj->slaying, "qwerty1234");
    FREE_AND_COPY_HASH(obj->msg, "qwerty1234");
    nrof_refs = query_refcount(obj->name);
    shstr_get_totals(&entries2, &refs2, &links2);

    fail_unless(nrof_refs == 5, "Test setup failed");
    fail_unless(entries2 == entries1 + 1, "Test setup failed");
    fail_unless(refs2 == refs1 + 5, "Test setup failed");

    object_gc();

    shstr_get_totals(&entries2, &refs2, &links2);
    fail_unless(entries2 == entries1, "String not freed");
    fail_unless(refs2 == refs1, "String not dereferenced");
}
END_TEST

/*
 * Tests for specific object types and their related
 * support functions
 */

START_TEST (object_type_beacon)
{
    shstr_t *path = add_string("/dev/testmaps/testmap_plugin");
    shstr_t *b1_name = add_string("script_tester_beacon_1");
    shstr_t *b2_name = add_string("script_tester_beacon_2");

    map_t *map;
    object_t *beacon1, *beacon2, *lostsoul;

    fail_if(beacon_table == NULL, "Beacon hashtable not initialized\n");
    fail_if(locate_beacon(b1_name) != NULL, "Inv beacon available before test");
    fail_if(locate_beacon(b2_name) != NULL, "Map beacon available before test");

    map = ready_map_name(NULL, path, MAP_STATUS_MULTI, NULL);
    fail_unless(map != NULL, "Couldn't load %s", path);

    /* Locate the two beacons by hardcoded coordinates */
    lostsoul = present(MONSTER, map, 8, 3);
    fail_unless(lostsoul != NULL, "Couldn't find lost soul on %s", path);
    beacon1 = present_in_ob(TYPE_BEACON, lostsoul);
    fail_unless(beacon1 != NULL, "Couldn't find beacon in lost soul on %s", path);
    beacon2 = present(TYPE_BEACON, map, 8, 16);
    fail_unless(beacon2 != NULL, "Couldn't find beacon 2 on %s", path);

    fail_if(locate_beacon(b1_name) != beacon1, "Inventory beacon not available");
    fail_if(locate_beacon(b2_name) != beacon2, "Map beacon not available");

    delete_map(map);
    object_gc();

    fail_if(locate_beacon(b1_name) != NULL, "Inventory beacon available after garbage collection");
    fail_if(locate_beacon(b2_name) != NULL, "Map beacon available after garbage collection");
}
END_TEST

START_TEST (object_type_check_inv)
{
    map_t *map = ready_map_name(NULL, add_string("/dev/unit_tests/test_check_inv"), MAP_STATUS_MULTI, NULL);

    object_t *check1 = locate_beacon(find_string("check1"))->env;
    object_t *check2 = locate_beacon(find_string("check2"))->env;
    object_t *key1 = locate_beacon(find_string("key1"))->env;
    object_t *cont1 = arch_to_object(find_archetype("chest"));
    object_t *cont2 = arch_to_object(find_archetype("chest"));

    remove_ob(key1);
    key1=insert_ob_in_ob(key1, cont1);

    fail_if(blocked_tile(cont1, NULL, map, check1->x, check1->y), "inv_check doesn't allow pass through");
    fail_unless(blocked_tile(cont1, NULL, map, check2->x, check2->y), "inv_check doesn't block pass through");
    fail_unless(blocked_tile(cont2, NULL, map, check1->x, check1->y), "inv_check doesn't block pass through");

    /* Test inversed match */
    check1->last_sp = 0;
    check2->last_sp = 0;

    fail_unless(blocked_tile(cont1, NULL, map, check1->x, check1->y), "inv_check doesn't block pass through");
    fail_if(blocked_tile(cont1, NULL, map, check2->x, check2->y), "inv_check doesn't allow pass through");
    fail_if(blocked_tile(cont2, NULL, map, check1->x, check1->y), "inv_check doesn't allow pass through");

    /* Turn off blockage (need to update map flags) */
    check1->last_grace = 0;
    update_object(check1, UP_OBJ_ALL);
    fail_if(blocked_tile(cont1, NULL, map, check1->x, check1->y), "inv_check doesn't allow pass through");

    /* TODO: test script triggering */
}
END_TEST

/*
 * Tests for memleak in merge_ob/insert_ob_in_ob()
 */

START_TEST (object_merge_memleak)
{
    int nrof_objs = pool_object->nrof_allocated[0] - pool_object->nrof_free[0];
    object_t *container = arch_to_object(find_archetype("chest"));
    object_t *coin1 = arch_to_object(find_archetype("goldcoin"));
    object_t *coin2 = arch_to_object(find_archetype("goldcoin"));
    object_t *coin3;

    fail_unless(container != NULL, "No container object");
    fail_if(container->inv, "Container not empty before test");
    fail_unless(coin1 || coin2, "No coin objects");

    coin1->nrof = 10;
    coin2->nrof = 20;

    insert_ob_in_ob(coin1, container);
    fail_unless(coin1->env == container, "Container not containing coin");
    fail_unless(container->inv == coin1, "Coin1 not in container");

    coin3 = insert_ob_in_ob(coin2, container);
    fail_unless(container->inv != NULL, "Container emptied?");
    fail_if(container->inv->above || container->inv->below, "Coins didn't merge");

    fail_unless(coin3 == container->inv, "insert_ob_in_ob didn't return correct object");
    fail_unless(coin3->nrof == 30, "Coins doesn't add up correctly");

    fail_unless(QUERY_FLAG(coin1, FLAG_REMOVED) || QUERY_FLAG(coin2, FLAG_REMOVED), "Both coins non-removed");

    object_gc();
    fail_unless(nrof_objs == pool_object->nrof_allocated[0] - pool_object->nrof_free[0], "Some objects not returned after gc (diff = %d)", (pool_object->nrof_allocated[0] - pool_object->nrof_free[0]) - nrof_objs);
}
END_TEST

/*
 * More tests for object insertion. Make sure the inserted object
 * is removed from its original place in case of a merge.
 */
START_TEST (object_insert_ob_in_ob)
{
    object_t *container1 = arch_to_object(find_archetype("chest"));
    object_t *container2 = arch_to_object(find_archetype("chest"));
    object_t *coin1 = arch_to_object(find_archetype("goldcoin"));
    object_t *coin2 = arch_to_object(find_archetype("goldcoin"));
    object_t *coin3;

    coin1->nrof = 10;
    coin2->nrof = 20;

    insert_ob_in_ob(coin1, container1);
    insert_ob_in_ob(coin2, container2);
    remove_ob(coin2);
    coin3 = insert_ob_in_ob(coin2, container1);

    fail_unless(coin3 == container1->inv, "insert_ob_in_ob didn't return correct object");
    fail_unless(coin3->nrof == 30, "Coins doesn't add up correctly");

    fail_unless(QUERY_FLAG(coin1, FLAG_REMOVED) || QUERY_FLAG(coin2, FLAG_REMOVED), "Both coins non-removed");
    fail_unless(container2->inv == NULL, "Coin2 not removed from original container");
}
END_TEST;

Suite *object_suite(void)
{
  Suite *s = suite_create("Objects");
  TCase *tc_core = tcase_create("Core");

  tcase_add_checked_fixture(tc_core, setup, teardown);

  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, object_creation);
  tcase_add_test(tc_core, arch_creation);
  tcase_add_test(tc_core, object_strings);
  tcase_add_test(tc_core, object_merge_memleak);
  tcase_add_test(tc_core, object_insert_ob_in_ob);
  tcase_add_test(tc_core, object_type_beacon);
  tcase_add_test(tc_core, object_type_check_inv);

  return s;
}

#endif
