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

/* Bah, this shouldn't be in this file, perhaps */
START_TEST (buttons_check_blocked_tile)
{
    shstr_t *path = shstr_add_string("/dev/unit_tests/test_check_inv");
    map_t *map = ready_map_name(path, path, MAP_STATUS_MULTI, NULL);

    object_t *c1 = locate_beacon(shstr_find("c2_1"))->env;
    object_t *c2 = locate_beacon(shstr_find("c2_2"))->env;
    object_t *c3 = locate_beacon(shstr_find("c2_3"))->env;
    object_t *c4 = locate_beacon(shstr_find("c2_4"))->env;
    object_t *c5 = locate_beacon(shstr_find("c2_5"))->env;
    object_t *c6 = locate_beacon(shstr_find("c2_6"))->env;
    object_t *c7 = locate_beacon(shstr_find("c2_7"))->env;
    object_t *c8 = locate_beacon(shstr_find("c2_8"))->env;

    object_t *sword = locate_beacon(shstr_find("shortsword"))->env;

    object_t *cont1 = arch_to_object(find_archetype("chest")); /* "player" with sword */
    object_t *cont2 = arch_to_object(find_archetype("chest")); /* "player" without sword */

    cont1->type = cont2->type = PLAYER;

    remove_ob(sword);
    sword = insert_ob_in_ob(sword, cont1);

    fail_if( blocked_tile(cont1, NULL, map, c1->x, c1->y), "checker 1 blocked cont with sword");
    fail_if(!blocked_tile(cont1, NULL, map, c2->x, c2->y), "checker 2 didn't block cont with sword");
    fail_if( blocked_tile(cont1, NULL, map, c3->x, c3->y), "checker 3 blocked cont with sword");
    fail_if(!blocked_tile(cont1, NULL, map, c4->x, c4->y), "checker 4 didn't block cont with sword");
    fail_if( blocked_tile(cont1, NULL, map, c5->x, c5->y), "checker 5 blocked cont with sword");
    fail_if(!blocked_tile(cont1, NULL, map, c6->x, c6->y), "checker 6 didn't block cont with sword");
    fail_if( blocked_tile(cont1, NULL, map, c7->x, c7->y), "checker 7 blocked cont with sword");
    fail_if(!blocked_tile(cont1, NULL, map, c8->x, c8->y), "checker 8 didn't block cont with sword");
    
    fail_if(!blocked_tile(cont2, NULL, map, c1->x, c1->y), "checker 1 didn't block cont without sword");
    fail_if( blocked_tile(cont2, NULL, map, c2->x, c2->y), "checker 2 blocked cont without sword");
    fail_if(!blocked_tile(cont2, NULL, map, c3->x, c3->y), "checker 3 didn't block cont without sword");
    fail_if( blocked_tile(cont2, NULL, map, c4->x, c4->y), "checker 4 blocked cont without sword");
    fail_if(!blocked_tile(cont2, NULL, map, c5->x, c5->y), "checker 5 didn't block cont without sword");
    fail_if( blocked_tile(cont2, NULL, map, c6->x, c6->y), "checker 6 blocked cont without sword");
    fail_if(!blocked_tile(cont2, NULL, map, c7->x, c7->y), "checker 7 didn't block cont without sword");
    fail_if( blocked_tile(cont2, NULL, map, c8->x, c8->y), "checker 8 blocked cont without sword");
    
    cont1->type = cont2->type = CONTAINER;
}
END_TEST

/* Bah, this shouldn't be in this file, perhaps */
START_TEST (buttons_move_apply_check_inv)
{
    shstr_t *path = shstr_add_string("/dev/unit_tests/test_check_inv");
    map_t *map = ready_map_name(path, path, MAP_STATUS_MULTI, NULL);

    object_t *c7 = locate_beacon(shstr_find("c2_7"))->env;
    object_t *c8 = locate_beacon(shstr_find("c2_8"))->env;

    object_t *sword = locate_beacon(shstr_find("shortsword"))->env;

    /* Create a fake player (yes, this is dangerous...) */
    object_t *cont1 = arch_to_object(find_archetype("chest")); /* "player" with sword */
    player_t *fake_contr = calloc(1, sizeof(player));
    cont1->type = PLAYER;
    cont1->arch->clone.terrain_flag = 127;
    cont1->custom_attrset = fake_contr;
    CONTR(cont1)->skillgroup_ptr[SKILLGROUP_MAGIC] = cont1;
    CONTR(cont1)->skillgroup_ptr[SKILLGROUP_WISDOM] = cont1;
    insert_ob_in_ob(arch_to_object(find_archetype("skill_punching")), cont1);

    remove_ob(sword);    
    sword = insert_ob_in_ob(sword, cont1);
    
    cont1->x = sword->x;
    cont1->y = sword->y;
    insert_ob_in_map(cont1, map, NULL, 0);
    fail_if(cont1->x != c7->x+1 || cont1->y != c7->y, "insertion missed");
    fail_if(sword->nrof != 10, "sword nrof wrong before test");
    fail_if(move_ob(cont1, 7, NULL) != MOVE_RETURN_SUCCESS, "checker 7 blocked cont with sword");
    fail_if(sword->nrof > 9, "sword nrof not reduced"); 
    fail_if(sword->nrof < 9, "sword nrof reduced too much (got %d, expected 9)",sword->nrof); 
    remove_ob(cont1);
    move_check_off(cont1, cont1, MOVE_FLAG_VANISHED);
    fail_if(sword->nrof > 8, "sword nrof not reduced"); 
    fail_if(sword->nrof < 8, "sword nrof reduced too much (got %d, expected 8)",sword->nrof); 

    cont1->x = sword->x;
    cont1->y = sword->y;
    insert_ob_in_map(cont1, map, NULL, 0);
    fail_if(cont1->x != c8->x+1 || cont1->y != c8->y+1, "insertion missed");
    fail_if(sword->nrof != 8, "sword nrof changed (got %d)", sword->nrof); 
    fail_if(move_ob(cont1, 8, NULL) != MOVE_RETURN_INSERTION_FAILED, "checker 8 didn't block cont with sword");
    fail_if(sword->nrof != 8, "sword nrof changed (got %d)", sword->nrof); 
    remove_ob(cont1);
    move_check_off(cont1, cont1, MOVE_FLAG_VANISHED);
    fail_if(sword->nrof != 8, "sword nrof changed (got %d)", sword->nrof); 

    cont1->type = CONTAINER;
}
END_TEST


START_TEST (buttons_check_inv_recursive)
{
    shstr_t *path = shstr_add_string("/dev/unit_tests/test_check_inv");
    map_t *map = ready_map_name(path, path, MAP_STATUS_MULTI, NULL);

    object_t *check1 = locate_beacon(shstr_find("check1"))->env;
    object_t *check2 = locate_beacon(shstr_find("check2"))->env;
    object_t *check3 = locate_beacon(shstr_find("check3"))->env;
    object_t *check4 = locate_beacon(shstr_find("check4"))->env;
    object_t *check5 = locate_beacon(shstr_find("check5"))->env;
    object_t *check8 = locate_beacon(shstr_find("check8"))->env;
    object_t *check9 = locate_beacon(shstr_find("check9"))->env;

    object_t *key1 = locate_beacon(shstr_find("key1"))->env;
    object_t *key2 = locate_beacon(shstr_find("key2"))->env;
    object_t *key3 = locate_beacon(shstr_find("key3"))->env;
    object_t *key4 = locate_beacon(shstr_find("key4"))->env;

    object_t *cont1 = arch_to_object(find_archetype("chest"));
    object_t *cont2 = arch_to_object(find_archetype("chest"));
    object_t *cont3 = arch_to_object(find_archetype("chest"));
    object_t *cont4 = arch_to_object(find_archetype("chest"));

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

/* Test mapload initialization */
START_TEST (buttons_check_mapload)
{
    shstr_t *path = shstr_add_string("/dev/unit_tests/test_connections");
    map_t *map = ready_map_name(path, path, MAP_STATUS_MULTI, NULL);

    object_t *lever = locate_beacon(shstr_find("lever"))->env;

    object_t *creator1 = locate_beacon(shstr_find("creator1"))->env;
    object_t *creator2 = locate_beacon(shstr_find("creator2"))->env;
    object_t *creator3 = locate_beacon(shstr_find("creator3"))->env;
    object_t *creator4 = locate_beacon(shstr_find("creator4"))->env;
    object_t *creator5 = locate_beacon(shstr_find("creator5"))->env;

    object_t *fire1a = locate_beacon(shstr_find("fire1a"))->env;
    object_t *fire1b = locate_beacon(shstr_find("fire1b"))->env;
    object_t *fire2a = locate_beacon(shstr_find("fire2a"))->env;
    object_t *fire2b = locate_beacon(shstr_find("fire2b"))->env;
    object_t *fire3a = locate_beacon(shstr_find("fire3a"))->env;
    object_t *fire3b = locate_beacon(shstr_find("fire3b"))->env;
    object_t *fire4a = locate_beacon(shstr_find("fire4a"))->env;
    object_t *fire4b = locate_beacon(shstr_find("fire4b"))->env;
    object_t *fire5a = locate_beacon(shstr_find("fire5a"))->env;
    object_t *fire5b = locate_beacon(shstr_find("fire5b"))->env;

    /* 1. Make sure nothing is created at mapload */
    fail_if(creator1->above || creator1->below, "creator1 created something");
    fail_if(creator2->above || creator2->below, "creator2 created something");
    fail_if(creator3->above || creator3->below, "creator3 created something");
    fail_if(creator4->above || creator4->below, "creator4 created something");
    fail_if(creator5->above || creator5->below, "creator5 created something");

    /* 2. Ensure that the correct lights are burning */
    fail_if(fire1a->glow_radius, "light1a glows");
    fail_if(fire1b->glow_radius, "light1b glows");
    fail_if(fire2a->glow_radius, "light2a glows");
    fail_if(fire2b->glow_radius, "light2b glows");
    fail_if(!fire3a->glow_radius, "light3a doesn't glow");
    fail_if(!fire3b->glow_radius, "light3b doesn't glow");
    fail_if(fire4a->glow_radius, "light4a glows");
    fail_if(fire4b->glow_radius, "light4b glows");
    fail_if(fire5a->glow_radius, "light5a glows");
    fail_if(fire5b->glow_radius, "light5b glows");

    /* Switch the lever and make sure the inverse is true */
    apply_object(lever, lever, 0);

    fail_unless(creator1->above || creator1->below, "creator1 created nothing");
    fail_unless(creator2->above || creator2->below, "creator2 created nothing");
    fail_if(creator3->above || creator3->below, "creator3 created something");
    fail_unless(creator4->above || creator4->below, "creator4 created nothing");
    fail_unless(creator5->above || creator5->below, "creator5 created nothing");

    fail_unless(fire1a->glow_radius, "light1a doesn't glow");
    fail_unless(fire1b->glow_radius, "light1b doesn't glow");
    fail_unless(fire2a->glow_radius, "light2a doesn't glow");
    fail_unless(fire2b->glow_radius, "light2b doesn't glow");
    fail_unless(!fire3a->glow_radius, "light3a glows");
    fail_unless(!fire3b->glow_radius, "light3b glows");
    fail_unless(fire4a->glow_radius, "light4a doesn't glow");
    fail_unless(fire4b->glow_radius, "light4b doesn't glow");
    fail_unless(fire5a->glow_radius, "light5a doesn't glow");
    fail_unless(fire5b->glow_radius, "light5b doesn't glow");
}
END_TEST

/* Test environment sensors */
START_TEST (buttons_check_env_sensor)
{
    shstr_t *path = shstr_add_string("/dev/unit_tests/test_env_sensor");
    map_t *map = ready_map_name(path, path, MAP_STATUS_MULTI, NULL);

    object_t *lever = locate_beacon(shstr_find("lever_beacon"))->env; // Lever controlling light3
    object_t *sensor = locate_beacon(shstr_find("sensor_beacon"))->env; // Sensor activated by light3
    object_t *light1 = locate_beacon(shstr_find("gravelight_beacon"))->env; // applyable light connected to sensor
    object_t *light2 = locate_beacon(shstr_find("light_beacon"))->env; // light source connected to sensor
    object_t *light3 = locate_beacon(shstr_find("light_beacon_2"))->env;

    /* Give the env sensor a chance to sense */
    process_events();
    process_events();

    /* At map load time, no lights should shine */
    fail_if(light1->glow_radius, "applyable light glows");
    fail_if(light2->glow_radius, "light source 1 glows");
    fail_if(light3->glow_radius, "light source 2 glows");
    fail_if(sensor->weight_limit, "env sensor senses");

    /* Switch the lever and make sure the inverse is true */
    apply_object(lever, lever, 0);
    fail_if(!light3->glow_radius, "light source 2 doesn't glow");

    /* The new light should activate the env sensor, let it sense... */
    process_events();
    process_events();

    fail_if(!light1->glow_radius, "applyable light doesn't glow");
    fail_if(!light2->glow_radius, "light source 1 doesn't glow");
    fail_if(!sensor->weight_limit, "env sensor doesn't sense");

    /* Switch the lever and make sure the inverse is true */
    apply_object(lever, lever, 0);
    process_events();
    process_events();
    fail_if(sensor->weight_limit, "env sensor senses though light being off");
    fail_if(light1->glow_radius, "applyable light glows though being turned off");
    fail_if(light2->glow_radius, "light source 1 glows though being turned off");
    fail_if(light3->glow_radius, "light source 2 glows though being turned off");
}
END_TEST

/* Test pedestals
 * Related bugs:
 *   - 0000480: Invisible pedestal does not fire connection
 */
START_TEST (buttons_check_pedestal)
{
    shstr_t *path = shstr_add_string("/dev/unit_tests/test_pedestal");
    map_t *map = ready_map_name(path, path, MAP_STATUS_MULTI, NULL);

    object_t *lever = locate_beacon(shstr_find("lever"))->env;
    object_t *pedestal1 = locate_beacon(shstr_find("visible_pedestal"))->env;
    object_t *pedestal2 = locate_beacon(shstr_find("invisible_pedestal"))->env;
    object_t *pedestal3 = locate_beacon(shstr_find("sys_invisible_pedestal"))->env;
    object_t *pedestal4 = locate_beacon(shstr_find("visible_pedestal_2"))->env;
    object_t *pedestal5 = locate_beacon(shstr_find("invisible_pedestal_2"))->env;
    object_t *pedestal6 = locate_beacon(shstr_find("sys_invisible_pedestal_2"))->env;

    fail_if(pedestal1->weight_limit, "visible pedestal 1 is triggered");
    fail_if(pedestal2->weight_limit, "invisible pedestal 1 is triggered");
    fail_if(pedestal3->weight_limit, "sys. invisible pedestal 1 is triggered");
    fail_if(pedestal4->weight_limit, "visible pedestal 2 is triggered");
    fail_if(pedestal5->weight_limit, "invisible pedestal 2 is triggered");
    fail_if(pedestal6->weight_limit, "sys. visible pedestal 2 is triggered");

    /* Switch the lever and make sure the inverse is true */
    apply_object(lever, lever, 0);

    fail_unless(pedestal1->weight_limit, "visible pedestal 1 is not triggered");
    fail_unless(pedestal2->weight_limit, "invisible pedestal 1 is not triggered"); /* Bug 0000480 fails here */
    fail_unless(pedestal3->weight_limit, "sys. invisible pedestal 1 is not triggered");
    fail_unless(pedestal4->weight_limit, "visible pedestal 2 is not triggered");
    fail_unless(pedestal5->weight_limit, "invisible pedestal 2 is not triggered");
    fail_unless(pedestal6->weight_limit, "sys. visible pedestal 2 is not triggered");
}
END_TEST

/* Test creator with mover inside
 * Related bugs:
 * 0000482: Creator with mover in inventory crashes server
 */
START_TEST (buttons_creator_with_mover)
{
    shstr_t     *path = shstr_add_string("/dev/unit_tests/test_creator");
    map_t *map = ready_map_name(path, path, MAP_STATUS_MULTI, NULL);
    object_t    *lever = locate_beacon(shstr_find("lever"))->env,
              *beacon = locate_beacon(shstr_find("beacon_square")),
              *mover;
    MapStruct *msp = MSP_KNOWN(beacon);

    process_events();
    process_events(); /* Bug 000482 crashes here */
    apply_object(lever, lever, 0);

    for(mover = MSP_GET_FIRST(msp); mover; mover = mover->above)
    {
        if (mover->type == PLAYERMOVER)
        {
            break;
        }
    }

    fail_if(mover == NULL, "No mover created");
}
END_TEST

Suite *buttons_suite(void)
{
  Suite *s = suite_create("Buttons");
  TCase *tc_core = tcase_create("Core");

  tcase_add_checked_fixture(tc_core, setup, teardown);

  suite_add_tcase (s, tc_core);
  tcase_add_test(tc_core, buttons_check_blocked_tile);
  tcase_add_test(tc_core, buttons_check_inv_recursive);
  tcase_add_test(tc_core, buttons_move_apply_check_inv);
  tcase_add_test(tc_core, buttons_check_mapload);
  tcase_add_test(tc_core, buttons_check_env_sensor);
  tcase_add_test(tc_core, buttons_check_pedestal);
  tcase_add_test(tc_core, buttons_creator_with_mover);
  return s;
}

#endif
