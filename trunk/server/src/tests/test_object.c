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

/* test_object.c 
 * Copyright (C) 2005 Bj�rn Axelsson
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

/* Test the basic creation and deleting operations */
START_TEST (object_creation)
{
    object *obj;
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
    object *obj;
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
    object *obj = get_object();

    int entries1, refs1, links1;
    int entries2, refs2, links2;
    int nrof_refs;

    ss_get_totals(&entries1, &refs1, &links1);
    
    FREE_AND_COPY_HASH(obj->name, "qwerty1234");
    FREE_AND_COPY_HASH(obj->title, "qwerty1234");
    FREE_AND_COPY_HASH(obj->race, "qwerty1234");
    FREE_AND_COPY_HASH(obj->slaying, "qwerty1234");
    FREE_AND_COPY_HASH(obj->msg, "qwerty1234");
    nrof_refs = query_refcount(obj->name);
    ss_get_totals(&entries2, &refs2, &links2);

    fail_unless(nrof_refs == 5, "Test setup failed");
    fail_unless(entries2 == entries1 + 1, "Test setup failed");
    fail_unless(refs2 == refs1 + 5, "Test setup failed");
    
    object_gc();

    ss_get_totals(&entries2, &refs2, &links2);
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
    shstr *path = add_string("/dev/testmaps/testmap_plugin");
    shstr *b1_name = add_string("script_tester_beacon_1");
    shstr *b2_name = add_string("script_tester_beacon_2");
    
    mapstruct *map;
    object *beacon1, *beacon2, *lostsoul;

    fail_if(beacon_table == NULL, "Beacon hashtable not initialized\n");
    fail_if(locate_beacon(b1_name) != NULL, "Inv beacon available before test");
    fail_if(locate_beacon(b2_name) != NULL, "Map beacon available before test");
    
    map = ready_map_name(path, 0);
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


Suite *object_suite(void)
{
  Suite *s = suite_create("Objects");
  TCase *tc_core = tcase_create("Core");

  tcase_add_checked_fixture(tc_core, setup, teardown);
  
  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, object_creation);
  tcase_add_test(tc_core, arch_creation);
  tcase_add_test(tc_core, object_strings);
  tcase_add_test(tc_core, object_type_beacon);

  return s;
}

#endif
