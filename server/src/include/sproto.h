/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001 Michael Toennies

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

#ifndef __SPROTO_H
#define __SPROTO_H

/* TODO: Reference only. Will be removed. */
#if 0
///* alchemy.c */
//char                       *cauldron_sound(void);
//void                        attempt_do_alchemy(object *caster, object *cauldron);
//int                         content_recipe_value(object *op);
//int                         numb_ob_inside(object *op);
//object                     *attempt_recipe(object *caster, object *cauldron, int ability, recipe *rp, int nbatches);
//void                        adjust_product(object *item, int lvl, int yield);
//object                     *make_item_from_recipe(object *cauldron, recipe *rp);
//object                     *find_transmution_ob(object *first_ingred, recipe *rp);
//void                        alchemy_failure_effect(object *op, object *cauldron, recipe *rp, int danger);
//void                        remove_contents(object *first_ob, object *save_item);
//int                         calc_alch_danger(object *caster, object *cauldron);
///* gods.c */
//int                         worship_forbids_use(object *op, object *skillgroup, uint32 flag, char *string);
//void                        stop_using_item(object *op, int type, int number);
//void                        update_priest_flag(object *god, object *skillgroup, uint32 flag);
//archetype                  *determine_holy_arch(object *god, const char *type);
//void                        god_intervention(object *op, object *god);
//int                         god_examines_priest(object *op, object *god);
//int                         god_examines_item(object *god, object *item);
//int                         get_god(object *priest);
//int                         tailor_god_spell(object *spellop, object *caster);
//void                        lose_priest_exp(object *pl, int loss);
///* resurrection.c */
//void                        dead_player(object *op);
//int                         cast_raise_dead_spell(object *op, int dir, int spell_type, object *corpseobj);
//int                         resurrection_fails(int levelcaster, int leveldead);
//int                         resurrect_player(object *op, char *playername, int rspell);
//void                        dead_character(char *name);
//int                         dead_player_exists(char *name);
///* spell_effect.c */
//int                         cast_regenerate_spellpoints(object *op);
//int                         summon_hostile_monsters(object *op, int n, const char *monstername);
//int                         alchemy(object *op);
//int                         spell_find_dir(mapstruct *m, int x, int y, object *exclude);
//void                        move_ball_lightning(object *op);
//int                         cast_destruction(object *op, object *caster, int dam, int attacktype);
//int                         magic_wall(object *op, object *caster, int dir, int spell_type);
//int                         cast_light(object *op, object *caster, int dir);
//void                        fire_a_ball(object *op, int dir, int strength);
//void                        move_swarm_spell(object *op);
//void                        fire_swarm(object *op, object *caster, int dir, archetype *swarm_type, int spell_type,
//                                       int n, int magic);
//int                         create_aura(object *op, object *caster, archetype *aura_arch, int spell_type, int magic);
//object                     *get_pointed_target(object *op, int dir);
//int                         cast_smite_spell(object *op, object *caster, int dir, int type);
//void                        aggravate_monsters(object *op);
//int                         cast_speedball(object *op, int dir, int type);
//int                         dimension_door(object *op, int dir);
//int                         cast_wor(object *op, object *caster);
//void                        execute_wor(object *op);
//int                         cast_wow(object *op, int dir, int ability, SpellTypeFrom item);
//int                         cast_earth2dust(object *op, object *caster);
//int                         cast_create_obj(object *op, object *caster, object *new_op, int dir);
//int                         cast_create_food(object *op, object *caster, int dir, char *stringarg);
//int                         cast_create_town_portal(object *op, object *caster, int dir);
//int                         cast_create_missile(object *op, object *caster, int dir, char *stringarg);
//int                         summon_monster(object *op, object *caster, int dir, archetype *at, int spellnum);
//int                         summon_pet(object *op, int dir, SpellTypeFrom item);
//int                         create_bomb(object *op, object *caster, int dir, int spell_type, char *name);
//void                        animate_bomb(object *op);
//int                         fire_cancellation(object *op, int dir, archetype *at, int magic);
//void                        move_cancellation(object *op);
//void                        cancellation(object *op);
//int                         cast_pacify(object *op, object *weap, archetype *arch, int spellnum);
//int                         summon_fog(object *op, object *caster, int dir, int spellnum);
//int                         create_the_feature(object *op, object *caster, int dir, int spell_effect);
//int                         cast_transfer(object *op, int dir);
//int                         drain_magic(object *op, int dir);
//void                        counterspell(object *op, int dir);
//int                         cast_charm(object *op, object *caster, archetype *arch, int spellnum);
//int                         cast_charm_undead(object *op, object *caster, archetype *arch, int spellnum);
//object                     *choose_cult_monster(object *pl, object *god, int summon_level);
//int                         summon_cult_monsters(object *op, int old_dir);
//int                         summon_avatar(object *op, object *caster, int dir, archetype *at, int spellnum);
//object                     *fix_summon_pet(archetype *at, object *op, int dir, int type);
//int                         cast_consecrate(object *op);
//int                         finger_of_death(object *op, object *caster, int dir);
//int                         animate_weapon(object *op, object *caster, int dir, archetype *at, int spellnum);
//int                         cast_daylight(object *op);
//int                         cast_nightfall(object *op);
//int                         cast_faery_fire(object *op, object *caster);
//int                         make_object_glow(object *op, int radius, int time);
//int                         cast_cause_disease(object *op, object *caster, int dir, archetype *disease_arch, int type);
//void                        move_aura(object *aura);
//void                        move_peacemaker(object *op);
///* swamp.c */
//void                        walk_on_deep_swamp(object *op, object *victim);
//void                        move_deep_swamp(object *op);
///* time.c */
//void                        move_firechest(object *op);
#endif
/* apply.c */
int                         check_item(object *op, const char *item);
void                        eat_item(object *op, const char *item);
int                         check_weapon_power(object *who, int improvs);
int                         improve_weapon_stat(object *op, object *improver, object *weapon, signed char *stat,
                                                int sacrifice_count, char *statname);
int                         prepare_weapon(object *op, object *improver, object *weapon);
int                         improve_weapon(object *op, object *improver, object *weapon);
int                         check_improve_weapon(object *op, object *tmp);
int                         improve_armour(object *op, object *improver, object *armour);
int                         convert_item(object *item, object *converter, object *originator);
void                        move_apply(object *const trap, object *const victim, object *const originator, const int flags);
void                        do_learn_spell(object *op, int spell, int special_prayer);
void                        do_forget_spell(object *op, int spell);
int                         manual_apply(object *op, object *tmp, int aflag);
int                         player_apply(object *pl, object *op, int aflag);
void                        player_apply_below(object *pl);
int                         apply_special(object *who, object *op, int aflags);
int                         monster_apply_special(object *who, object *op, int aflags);
void                        turn_on_light(object *op);
void                        turn_off_light(object *op);
void                        apply_light(object *who, object *op);
void                        scroll_failure(object *op, int failure, int power);
/* attack.c */
int                         attack_ob(object *op, object *hitter, object *hit_obj);
int                         damage_ob(object *op, int dam, object *hitter, int env_attack);
int                         hit_map(object *op, int dir);
int                         kill_object(object *op, int dam, object *hitter, int type);
object                     *hit_with_arrow(object *op, object *victim);
void                        tear_down_wall(object *op);
void                        snare_player(object *op, object *hitter, int dam);
void                        poison_player(object *op, object *hitter, float dam);
void                        slow_player(object *op, object *hitter, int dam);
void                        fear_player(object *op, object *hitter, int dam);
void                        confuse_player(object *op, object *hitter, int ticks);
void                        blind_player(object *op, object *hitter, int dam);
void                        paralyze_player(object *op, object *hitter, int dam);
void                        remove_paralyze(object *op);
int                         is_aimed_missile(object *op);
int                         is_melee_range(object *hitter, object *enemy);
/* ban.c */
void                        load_ban_file(void);
void                        save_ban_file(void);
struct objectlink          *add_ban_entry(const char *account, const char *name, char *ip, int ticks, int ticks_left);
void                        remove_ban_entry(struct oblnk *entry);
int                         check_banned(NewSocket *ns, const char *account, const char *name, char *ip);
/* c_chat.c */
#ifndef USE_CHANNELS
int                         command_describe(object *op, char *params);
#endif
int                         command_say(object *op, char *params);
int                         command_gsay(object *op, char *params);
int                         command_shout(object *op, char *params);
int                         command_tell(object *op, char *params);
int                         command_nod(object *op, char *params);
int                         command_dance(object *op, char *params);
int                         command_kiss(object *op, char *params);
int                         command_bounce(object *op, char *params);
int                         command_smile(object *op, char *params);
int                         command_cackle(object *op, char *params);
int                         command_laugh(object *op, char *params);
int                         command_giggle(object *op, char *params);
int                         command_shake(object *op, char *params);
int                         command_puke(object *op, char *params);
int                         command_growl(object *op, char *params);
int                         command_scream(object *op, char *params);
int                         command_sigh(object *op, char *params);
int                         command_sulk(object *op, char *params);
int                         command_hug(object *op, char *params);
int                         command_cry(object *op, char *params);
int                         command_poke(object *op, char *params);
int                         command_accuse(object *op, char *params);
int                         command_grin(object *op, char *params);
int                         command_bow(object *op, char *params);
int                         command_clap(object *op, char *params);
int                         command_blush(object *op, char *params);
int                         command_burp(object *op, char *params);
int                         command_chuckle(object *op, char *params);
int                         command_cough(object *op, char *params);
int                         command_flip(object *op, char *params);
int                         command_frown(object *op, char *params);
int                         command_gasp(object *op, char *params);
int                         command_glare(object *op, char *params);
int                         command_groan(object *op, char *params);
int                         command_hiccup(object *op, char *params);
int                         command_lick(object *op, char *params);
int                         command_pout(object *op, char *params);
int                         command_shiver(object *op, char *params);
int                         command_shrug(object *op, char *params);
int                         command_slap(object *op, char *params);
int                         command_smirk(object *op, char *params);
int                         command_snap(object *op, char *params);
int                         command_sneeze(object *op, char *params);
int                         command_snicker(object *op, char *params);
int                         command_sniff(object *op, char *params);
int                         command_snore(object *op, char *params);
int                         command_spit(object *op, char *params);
int                         command_strut(object *op, char *params);
int                         command_thank(object *op, char *params);
int                         command_twiddle(object *op, char *params);
int                         command_wave(object *op, char *params);
int                         command_whistle(object *op, char *params);
int                         command_wink(object *op, char *params);
int                         command_yawn(object *op, char *params);
int                         command_beg(object *op, char *params);
int                         command_bleed(object *op, char *params);
int                         command_cringe(object *op, char *params);
int                         command_think(object *op, char *params);
int                         command_me(object *op, char *params);
/* c_misc.c */
int                         command_spell_reset(object *op, char *params);
int                         command_motd(object *op, char *params);
int                         command_bug(object *op, char *params);
void                        malloc_info(object *op);
int                         command_who(object *op, char *params);
int                         command_malloc(object *op, char *params);
int                         command_mapinfo(object *op, char *params);
int                         command_mspinfo(object *op, char *params);
int                         command_sstable(object *op, char *params);
int                         command_time(object *op, char *params);
int                         command_archs(object *op, char *params);
int                         command_debug(object *op, char *params);
int                         command_dumpbelowfull(object *op, char *params);
int                         command_dumpbelow(object *op, char *params);
int                         command_dumpallobjects(object *op, char *params);
int                         command_dumpfriendlyobjects(object *op, char *params);
int                         command_dumpallarchetypes(object *op, char *params);
int                         command_dumpactivelist(object *op, char *params);
int                         command_setmaplight(object *op, char *params);
#if 0
int                         command_dumpmap(object *op, char *params);
int                         command_dumpallmaps(object *op, char *params);
#endif
int                         command_printlos(object *op, char *params);
void                        bug_report(char *reportstring);
int                         command_listen(object *op, char *params);
int                         command_statistics(object *pl, char *params);
int                         command_fix_me(object *op, char *params);
int                         command_logs(object *op, char *params);
int                         command_resistances(object *op, char *params);
int                         command_resting(object *op, char *params);
int                         command_help(object *op, char *params);
int                         command_privacy(object *op, char *params);
char                        *get_subdir(const char *name);
int                         command_stuck(object *op, char *params);
/* c_move.c */
int                         command_push_object (object *op, char *params);
int                         command_turn_right (object *op, char *params);
int                         command_turn_left (object *op, char *params);
/* c_new.c */
int                         command_run(object *op, char *params);
int                         command_run_stop(object *op, char *params);
int                         command_combat(object *op, char *params);
int                         command_target(object *op, char *params);
void                        command_face_request(char *params, int len, NewSocket *ns);
void                        generate_ext_title(player *pl);
/* c_object.c */
int                         command_uskill(object *pl, char *params);
int                         command_rskill(object *pl, char *params);
int                         command_egobind ( object *pl, char *params);
object                     *find_marked_object(object *op);
char                       *examine_monster(object *op, object *tmp, char *buf, int flag);
char                       *examine(object *op, object *tmp, int flag);
/* c_party.c */
int                         command_party_invite ( object *pl, char *params);
int                         command_party_join ( object *pl, char *params);
int                         command_party_deny ( object *pl, char *params);
int                         command_party_leave ( object *pl, char *params);
int                         command_party_remove ( object *pl, char *params);
void                        party_add_member(player *leader, player *member);
void                        party_remove_member(player *member, int flag);
void                        party_message(int mode, int flags, int pri,object *leader, object *source, char *format, ...) DAI_GNUC_PRINTF(6, 7);
void                        party_client_group_status(object *member);
void                        party_client_group_kill(object *member);
void                        party_client_group_update(object *member, int flag);
/* c_range.c */
float                       fire_magic_tool(object *owner, object *op, int dir);
int                         command_cast_spell(object *op, char *params);
/* c_wiz.c */
int                         command_connections(object *op, char *params);
int                         command_setgod(object *op, char *params);
int                         command_kick(object *op, char *params);
int                         command_reboot(object *op, char *params);
int                         command_goto(object *op, char *params);
int                         command_create(object *op, char *params);
int                         command_generate(object *op, char *params);
int                         command_spawn(object *op, char *params);
int                         command_listarch(object *op, char *params);
int                         command_mutelevel(object *op, char *params);
int                         command_summon(object *op, char *params);
int                         command_teleport(object *op, char *params);
int                         command_inventory(object *op, char *params);
int                         command_dump(object *op, char *params);
int                         command_patch(object *op, char *params);
int                         command_remove(object *op, char *params);
int                         command_free(object *op, char *params);
int                         command_setskill(object *op, char *params);
int                         command_addexp(object *op, char *params);
int                         command_serverspeed(object *op, char *params);
int                         command_stats(object *op, char *params);
int                         command_setstat(object *op, char *params);
int                         command_resetmap(object *op, char *params);
int                         command_check_fd(object *op, char *params);
int                         command_mute(object *op, char *params);
int                         command_silence(object *op, char *params);
int                         command_ban(object *op, char *params);
int                         command_sa(object *op, char *params);
int                         command_mm(object *op, char *params);
int                         command_mw(object *op, char *params);
int                         command_gm(object *op, char *params);
int                         command_vol(object *op, char *params);
int                         command_gmasterlist(object *op, char *params);
int                         command_gmasterfile(object *op, char *params);
int                         command_learn_spell(object *op, char *params);
int                         command_learn_special_prayer(object *op, char *params);
int                         command_forget_spell(object *op, char *params);
int                         command_listplugins(object *op, char *params);
int                         command_loadplugin(object *op, char *params);
int                         command_unloadplugin(object *op, char *params);
int                         command_ip(object *op, char *params);
int                         command_wizpass(object *op, char *params);
int                         command_matrix(object *op, char *params);
int                         command_stealth(object *op, char *params);
int                         command_invisibility(object *op, char *params);
int                         command_dm_dev(object *op, char *params);
int                         command_dm_light(object *op, char *params);
int                         command_password(object *op, char *params);
/* commands.c */
void                        init_commands(void);
/* container.c */
int                         container_link(player *const pl, object *const sack);
int                         container_unlink(player *const pl, object *sack);
int                         container_trap(object *const op, object *const container);
object                     *pick_up(object *who, object *what, object *where, uint32 nrof);
object                     *drop_to_floor(object *who, object *what, uint32 nrof);
/* disease.c */
int                         move_disease(object *disease);
int                         remove_symptoms(object *disease);
object                     *find_symptom(object *disease);
int                         check_infection(object *disease);
int                         infect_object(object *victim, object *disease, int force);
int                         do_symptoms(object *disease);
int                         grant_immunity(object *disease);
int                         move_symptom(object *symptom);
int                         check_physically_infect(object *victim, object *hitter);
object                     *find_disease(object *victim);
int                         cure_disease(object *sufferer, object *caster);
int                         reduce_symptoms(object *sufferer, int reduction);
/* egoitem.c */
int                         check_ego_item(object *pl, object *ob);
char                       *get_ego_item_name(object *ob);
void                        create_ego_item(object *ob, const char *name, int mode);
/* gmaster.c */
void                        remove_gmaster_list(player *pl);
int                         check_gmaster_file_entry(char *name, char *host, char *mode);
int                         load_gmaster_file(void);
void                        add_gmaster_file_entry(char *name, char *host, int mode_id);
void                        remove_gmaster_file_entry(objectlink *ol);
int                         check_gmaster_list(player *pl, int mode_id);
void                        set_gmaster_mode(player *pl, int mode_id);
void                        remove_gmaster_mode(player *pl);
int                         compare_gmaster_mode(int t, int p);
void                        write_gmaster_file(void);
void                        update_gmaster_file(void);
void                        free_gmaster_list(void);
/* gods.c */
int                         lookup_god_by_name(const char *name);
const char                 *determine_god(object *op);
object                     *find_god(const char *name);
void                        pray_at_altar(object *pl, object *altar);
void                        become_follower(object *op, object *new_god);
/* init.c */
char                       *version_string(void);
void                        init(int argc, char **argv);
void                        compile_info(void);
void                        fatal_signal(int make_core, int close_sockets, uint8 status);
void                        init_library(void);
void                        free_strings(void);
void                        set_pticks_time(long t);
void                        free_lists_and_tables(void);
/* ipcompare.c */
int                         parse_ip(const char * ip, unsigned char ip_terms[], int mask_pos[]);
int                         ip_compare(const char *ban_tmp, const char *ip_temp);
objectlink                 *find_players_on_ip(char *ipmask);
void                        free_iplist(objectlink *ip_list);
/* login.c */
int                         player_save(object *op);
addme_login_msg             player_load(NewSocket *ns, const char *name);
addme_login_msg             player_create(NewSocket *ns,player **pl_ret,char *name,int race,int gender,int skill_nr);
void                        player_addme_failed(NewSocket *ns, int error_msg);
void                        show_stream_info(NewSocket *ns);
/* main.c */
char                       *crypt_string(char *str);
int                         get_new_instance_num(void);
void                        process_players1(mapstruct *map);
void                        process_players2(mapstruct *map);
void                        clean_tmp_files(int flag);
void                        cleanup_without_exit(void);
void                        leave(player *pl, int draw_exit);
void                        dequeue_path_requests(void);
void                        do_specials(void);
void                        shutdown_agent(int timer, int ret, player *pl, char *reason);
int                         main(int argc, char **argv);
void                        process_events();
void                        iterate_main_loop();
/* monster.c */
object                     *get_active_waypoint(object *op);
object                     *get_aggro_waypoint(object *op);
object                     *get_return_waypoint(object *op);
object                     *find_waypoint(object *op, const char *name);
object                     *get_random_waypoint(object *op, object *ignore);
object                     *get_next_waypoint(object *op, object *wp);
int                         move_monster(object *op, int mode);
void                        object_accept_path(object *op);
void                        dump_abilities(void);
void                        print_monsters(void);
/* monster_memory.c */
void                        cleanup_mob_knowns(object *op, struct mob_known_obj **first, hashtable *ht);
void                        clear_mob_knowns(object *op, struct mob_known_obj **first, hashtable *ht);
struct mob_known_obj       *update_npc_knowledge(object *npc, object *other, int delta_friendship, int delta_attraction);
void                        update_npc_known_obj(struct mob_known_obj *known, int delta_friendship, int delta_attraction);
struct                      mob_known_obj *register_npc_known_obj(object *npc, object *other, int friendship, int attraction, int check_los);
rv_vector                  *get_known_obj_rv(object *op, struct mob_known_obj *known_obj, int maxage);
/* monster_behaviourset.c */
struct mob_behaviourset    *parse_behaviourconfig(const char *conf_text, object *op);
void                        init_arch_default_behaviours();
void                        initialize_mob_data(struct mobdata *data);
void                        cleanup_mob_data(struct mobdata *data);
struct mob_behaviourset    *setup_behaviours(object *op);
void                        cleanup_behaviourset(struct mob_behaviourset *data);
void                        cleanup_mob_known_obj(struct mob_known_obj *data);
int                         can_hit(object *ob1, object *ob2, rv_vector *rv);
void                        cleanup_all_behavioursets();
void                        reload_behaviours(object *op);
/* monster_behaviours.c */
int                         mob_can_see_obj(object *op, object *obj, struct mob_known_obj *known_obj);
int                         get_friendship(object *op, object *obj);
int                         get_attitude(object *op, object *obj);
object                     *monster_choose_random_spell(object *monster);
void                        monster_check_pickup(object *monster);
void                        monster_check_apply(object *mon, object *item);
void                        npc_call_help(object *op);
mapstruct                  *normalize_and_ready_map(mapstruct *defmap, const char **path);
/* mtrand.c */
void                        MTRand_init(const uint32 seed);
sint32                      MTRand_randComp(void);
/* npc_communicate.c */
void                        communicate(object *op, char *txt);
void                        talk_to_npc(player *pl, char *topic);
void                        gui_npc(object *who, uint8 mode, const char *text);
/* spawn_point.c */
void                        spawn_point(object *op);
void                        make_mob_homeless(object *mob);
void                        adjust_monster(object *monster);
objectlink                 *add_linked_spawn(object *spawn);
void                        remove_linked_spawn_list(mapstruct *map);
void                        send_link_spawn_signal(object *spawn, object *target, int signal);
/* move.c */
int                         move_ob(object *op, int dir, object *originator);
int                         teleport(object *teleporter, uint8 tele_type, object *user);
int                         push_roll_object(object * const op, int dir, const int flag);
void                        recursive_roll(object *op, int dir, object *pusher);
int                         roll_ob(object *op, int dir, object *pusher);
int                         push_ob(object *who, int dir, object *pusher);
int                         missile_reflection_adjust(object *op, int flag);
uint8                       leave_map(player *pl, mapstruct *newmap);
mapstruct                  *enter_map_by_name(object *op, const char *path, const char *src_path, int x, int y, int flags);
int                         enter_map_by_exit(object *op, object *exit_ob);
int                         check_insertion_allowed(object *op, mapstruct *map, int x, int y, int mode, int ins_flags);
int                         enter_map(object *op, object *originator, mapstruct *newmap, int x, int y, int flags, int ins_flags);
/* pets.c */
void                        update_pets_combat_mode(object *owner);
int                         add_pet(object *owner, object *pet, int mode);
void                        terminate_all_pets(object *owner);
void                        save_all_pets(FILE *fp, object *owner, int flag);
void                        save_pet(object *pet);
void                        remove_all_pets(mapstruct *map);
void                        pets_follow_owner(object *owner);
void                        pet_follow_owner(object *pet);
/* player.c */
player                     *find_player(char *plname);
player                     *find_player_hash(const char *plname);
void                        display_motd(object *op);
void                        free_player(player *pl);
void                        destroy_player_struct(player *pl);
object                     *get_nearest_player(object *mon);
int                         path_to_player(object *mon, object *pl, int mindiff);
void                        give_initial_items(object *pl, struct oblnk *items);
void                        flee_player(object *op);
int                         move_player(object *const op, int dir, const int flag);
int                         handle_newcs_player(player *pl);
int                         save_life(object *op);
void                        remove_unpaid_objects(object *op, object *env);
void                        do_some_living(object *op);
int                         kill_player(object *op);
void                        cast_dust(object *op, object *throw_ob, int dir);
void                        make_visible(object *op);
int                         is_true_undead(object *op);
int                         hideability(object *ob);
void                        do_hidden_move(object *op);
int                         stand_near_hostile(object *who);
int                         player_can_view(object *pl, object *op);
int                         action_makes_visible(object *op);
int                         pvp_area(object *attacker, object *victim);
int                         op_on_battleground(object *op, int *x, int *y);
void                        dragon_ability_gain(object *who, int atnr, int level);
int                         atnr_is_dragon_enabled(int attacknr);
int                         is_dragon_pl(object *op);
void                        reset_instance_data(player *pl);
void                        kick_player(player *pl);
char                       *get_online_players_info(player *who, player *diff,
                                                    uint8 force);
/* plugins.c */
object                     *get_event_object(object *op, int event_nr);
int                         trigger_object_plugin_event(int event_type,
                object *const me, object *const activator, object *const other,
                const char *msg, int *parm1, int *parm2, int *parm3, int flags);
int                         find_plugin_command(const char *cmd, object *op, CommArray_s *ret);
void                        displayPluginsList(object *op);
int                         findPlugin(const char *id);
void                        initPlugins(void);
void                        removeOnePlugin(const char *id);
void                        initOnePlugin(const char *pluginfile);
void                        removePlugins(void);
CFParm                     *CFWCmdRSkill(CFParm *PParm);
CFParm                     *CFWBecomeFollower(CFParm *PParm);
CFParm                     *CFWGetMapObject(CFParm *PParm);
CFParm                     *CFWOutOfMap(CFParm *PParm);
CFParm                     *CFWFindPlayer(CFParm *PParm);
CFParm                     *CFWManualApply(CFParm *PParm);
CFParm                     *CFWCheckSpellKnown(CFParm *PParm);
CFParm                     *CFWDoLearnSpell(CFParm *PParm);
CFParm                     *CFWDoLearnSkill(CFParm *PParm);
CFParm                     *CFWUpdateSpeed(CFParm *PParm);
CFParm                     *CFWAddExp(CFParm *PParm);
CFParm                     *CFWDetermineGod(CFParm *PParm);
CFParm                     *CFWFindGod(CFParm *PParm);
CFParm                     *CFWDumpObject(CFParm *PParm);
CFParm                     *CFWLoadObject(CFParm *PParm);
CFParm                     *CFWSendCustomCommand(CFParm *PParm);
CFParm                     *CFWCommunicate(CFParm *PParm);
CFParm                     *CFWFindMarkedObject(CFParm *PParm);
CFParm                     *CFWIdentifyObject(CFParm *PParm);
CFParm                     *CFWTeleportObject(CFParm *PParm);
CFParm                     *RegisterGlobalEvent(CFParm *PParm);
CFParm                     *UnregisterGlobalEvent(CFParm *PParm);
void                        GlobalEvent(CFParm *PParm);
CFParm                     *CFWCreateObject(CFParm *PParm);
CFParm                     *CFMapSave(CFParm *PParm);
CFParm                     *CFMapDelete(CFParm *PParm);
CFParm                     *CFInterface(CFParm *PParm);
void                        send_plugin_custom_message(object *pl, char *buf);
/* rune.c */
int                         write_rune(object *op, int dir, int inspell, int level, char *runename);
void                        rune_attack(object *op, object *victim);
void                        spring_trap(object *trap, object *victim);
int                         dispel_rune(object *op, int dir, int risk);
int                         trap_see(object *op, object *trap, int level);
int                         trap_show(object *trap, object *where);
int                         trap_disarm(object *disarmer, object *trap, int risk);
void                        trap_adjust(object *trap, int difficulty);
/* shop.c */
sint64                      query_cost(object *tmp, object *who, int flag);
char                       *cost_string_from_value(sint64 cost, int mode);
char                       *query_cost_string(object *tmp, object *who, int flag, int mode);
sint64                      query_money(object *where, _money_block *money);
uint8                       shop_pay_amount(sint64 amount, object *op);
uint8                       shop_checkout(object *op, object *this);
int                         get_money_from_string(char *text, struct _money_block *money);
int                         query_money_type(object *op, int value);
int                         enumerate_coins(sint64 value, struct _money_block *money);
object                     *create_financial_loot(_money_block *money, object *who, uint8 mode);
/* skills.c */
int                         attack_melee_weapon(object *op, int dir, char *string);
int                         attack_hth(object *pl, int dir, char *string);
int                         skill_attack(object *tmp, object *pl, int dir, char *string);
int                         do_skill_attack(object *tmp, object *op, char *string);
int                         SK_level(object *op);
int                         find_traps(object *pl, int level);
int                         remove_trap(object *op, int dir, int level);
/* skill_util.c */
void                        init_skills(void);
void                        link_player_skills(player *pl);
void                        validate_skills(player *pl);
object                     *find_skill(object *op, int skillnr);
int                         do_skill(object *op, int dir, char *string);
int                         get_weighted_skill_stat_sum(object *who, int sk);
void                        dump_skills(void);
int                         lookup_skill_by_name(char *name);
int                         check_skill_to_apply(object *who, object *item);
int                         learn_skill(object *pl, int skillnr);
int                         use_skill(object *op, char *string);
int                         change_skill(object *who, int sk_index);
int                         change_skill_to_skill(object *who, object *skl);
void                        set_action_time(object *op, float ticks);
int                         check_skill_action_time(object *op, object *skill);
int                         get_skill_stat1(object *op);
int                         get_skill_stat2(object *op);
int                         get_skill_stat3(object *op);
int                         get_weighted_skill_stats(object *op);
/* spell_effect.c */
void                        prayer_failure(object *op, int failure, int power);
void                        cast_mana_storm(object *op, int lvl);
void                        cast_magic_storm(object *op, object *tmp, int lvl);
int                         recharge(object *op);
int                         probe(object *op);
int                         cast_invisible(object *op, object *caster, int spell_type);
int                         perceive_self(object *op);
int                         cast_heal(object *op, int level, object *target, int spell_type);
int                         cast_change_attr(object *op, object *caster, object *target, int dir, int spell_type);
int                         remove_curse(object *op, object *target, int type, SpellTypeFrom src);
int                         cast_identify(object *op, int level, object *single_ob, int mode);
int                         cast_detection(object *op, object *target, int type);
object                     *cure_what_ails_you(object *op, uint8 st1);
int                         fire_arch(object *op, object *caster, sint16 x, sint16 y, int dir, archetype *at, int type, int level, int magic);
void                        check_fired_arch(object *op);
void                        move_fired_arch(object *op);
/* spell_util.c */
void                        init_spells(void);
void                        dump_spells(void);
int                         insert_spell_effect(char *archname, mapstruct *m, int x, int y);
spell                      *find_spell(int spelltype);
int                         check_spell_known(object *op, int spell_type);
int                         cast_spell(object *op, object *caster, int dir, int type, int ability, SpellTypeFrom item,
                                       char *stringarg);
int                         fire_bolt(object *op, object *caster, int dir, int type, int magic);
int                         cast_cone(object *op, object *caster, int dir, int strength, int spell_type,
                                      archetype *spell_arch, int level, int magic);
void                        check_cone_push(object *op);
void                        cone_drop(object *op);
void                        move_cone(object *op);
void                        explosion(object *op);
void                        forklightning(object *op, object *tmp);
int                         reflwall(mapstruct *m, int x, int y, object *sp_op);
void                        move_bolt(object *op);
void                        move_golem(object *op);
void                        control_golem(object *op, int dir);
void                        move_magic_missile(object *op);
void                        explode_object(object *op);
void                        drain_rod_charge(object *rod);
void                        fix_rod_speed(object *rod);
int                         find_target_for_spell(object *op, object *item, object **target, int dir, uint32 flags);
int                         can_see_monsterP(mapstruct *m, int x, int y, int dir);
int                         SP_level_dam_adjust(object *op, object *caster, int spell_type);
int                         SP_level_strength_adjust(object *op, object *caster, int spell_type);
int                         SP_level_spellpoint_cost(object *op, object *caster, int spell_type);
int                         look_up_spell_by_name(object *op, const char *spname);
void                        shuffle_attack(object *op, int change_face);
int                         SP_lvl_dam_adjust(int level, int spell_type, int base_dam, int stats_bonus);
/* stats.c */
void                        stats_event(stats_event_type type, ...);
/* time.c */
object                     *find_key(object *op, object *door);
int                         open_door(object *op, mapstruct *m, int x, int y, int mode);
void                        remove_door(object *op);
void                        remove_door2(object *op, object *opener);
void                        remove_door3(object *op);
void                        regenerate_rod(object *rod);
void                        remove_force(object *op);
void                        poison_more(object *op);
void                        move_gate(object *op);
void                        move_timed_gate(object *op);
void                        move_detector(object *op);
void                        move_conn_sensor(object *op);
void                        move_environment_sensor(object *op);
void                        animate_trigger(object *op);
void                        move_pit(object *op);
void                        change_object(object *op);
void                        move_teleporter(object *op);
void                        move_firewall(object *op);
void                        move_player_mover(object *op);
void                        move_creator(object *op);
void                        move_marker(object *op);
int                         process_object(object *op);
/* timers.c */
void                        cftimer_process_timers(void);
int                         cftimer_create(int id, long delay, object *ob, int mode);
int                         cftimer_destroy(int id);
int                         cftimer_find_free_id(void);
/* pathfinder.c */
int                         pathfinder_queue_enqueue(object *waypoint);
object                     *pathfinder_queue_dequeue(tag_t *count);
void                        request_new_path(object *waypoint);
object                     *get_next_requested_path(void);
struct path_segment        *encode_path(path_node *path, struct path_segment **last_segment);
int                         get_path_next(const char *buf, sint16 *off, const char **mappath, mapstruct **map, int *x,
                                          int *y);
path_node                  *compress_path(path_node *path);
float                       distance_heuristic(path_node *start, path_node *current, path_node *goal, object *op1, object *op2);
int                         find_neighbours(path_node *node, path_node **open_list, path_node **closed_list,
                                            path_node *start, path_node *goal, object *op, uint32 id);
path_node                  *find_path(object *op, mapstruct *map1, int x1, int y1, mapstruct *map2, int x2, int y2);
/* calendar.c */
void                        get_tad(timeanddate_t *tad, sint32 offset);
sint32                      get_tad_offset_from_string(const char *string);
char                       *print_tad(timeanddate_t *tad, int flags);
void                        tick_tadclock(void);
void                        write_tadclock(void);

int command_level(object *op, char *params);

#endif /* ifndef __SPROTO_H */
