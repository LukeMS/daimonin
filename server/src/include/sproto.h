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
///* time.c */
//void                        move_firechest(object *op);
#endif
/* apply.c */
void                        move_apply(object_t *const trap, object_t *const victim, object_t *const originator, const uint16 flags);
int                         apply_object(object_t *op, object_t *tmp, int aflag);
int                         apply_equipment(object_t *who, object_t *op, int aflags);
void                        turn_on_light(object_t *op);
void                        turn_off_light(object_t *op);
void                        apply_light(object_t *who, object_t *op);
/* attack.c */
int                         attack_ob(object_t *op, object_t *hitter, object_t *hit_obj);
int                         damage_ob(object_t *op, int dam, object_t *hitter, attack_envmode_t env_attack);
sint32                      hit_map(object_t *hitter, msp_t *msp);
object_t                     *hit_with_arrow(object_t *op, object_t *victim);
void                        snare_player(object_t *op, object_t *hitter, int dam);
void                        poison_player(object_t *op, object_t *hitter, float dam);
void                        slow_player(object_t *op, object_t *hitter, int dam);
void                        fear_player(object_t *op, object_t *hitter, int dam);
void                        confuse_player(object_t *op, object_t *hitter, int ticks);
void                        blind_player(object_t *op, object_t *hitter, int dam);
void                        paralyze_player(object_t *op, object_t *hitter, int dam);
void                        remove_paralyze(object_t *op);
int                         is_aimed_missile(object_t *op);
int                         is_melee_range(object_t *hitter, object_t *enemy);
/* ban.c */
void                        load_ban_file(void);
void                        save_ban_file(void);
struct objectlink_t          *add_ban_entry(const char *account, const char *name, char *ip, int ticks, int ticks_left);
void                        remove_ban_entry(struct objectlink_t *entry);
int                         check_banned(NewSocket *ns, const char *account, const char *name, char *ip);
/* c_chat.c */
#ifndef USE_CHANNELS
int                         command_describe(object_t *op, char *params);
#endif
int                         command_say(object_t *op, char *params);
int                         command_gsay(object_t *op, char *params);
int                         command_shout(object_t *op, char *params);
int                         command_tell(object_t *op, char *params);
int                         command_nod(object_t *op, char *params);
int                         command_dance(object_t *op, char *params);
int                         command_kiss(object_t *op, char *params);
int                         command_bounce(object_t *op, char *params);
int                         command_smile(object_t *op, char *params);
int                         command_cackle(object_t *op, char *params);
int                         command_laugh(object_t *op, char *params);
int                         command_giggle(object_t *op, char *params);
int                         command_shake(object_t *op, char *params);
int                         command_puke(object_t *op, char *params);
int                         command_growl(object_t *op, char *params);
int                         command_scream(object_t *op, char *params);
int                         command_sigh(object_t *op, char *params);
int                         command_sulk(object_t *op, char *params);
int                         command_hug(object_t *op, char *params);
int                         command_cry(object_t *op, char *params);
int                         command_poke(object_t *op, char *params);
int                         command_accuse(object_t *op, char *params);
int                         command_grin(object_t *op, char *params);
int                         command_bow(object_t *op, char *params);
int                         command_clap(object_t *op, char *params);
int                         command_blush(object_t *op, char *params);
int                         command_burp(object_t *op, char *params);
int                         command_chuckle(object_t *op, char *params);
int                         command_cough(object_t *op, char *params);
int                         command_flip(object_t *op, char *params);
int                         command_frown(object_t *op, char *params);
int                         command_gasp(object_t *op, char *params);
int                         command_glare(object_t *op, char *params);
int                         command_groan(object_t *op, char *params);
int                         command_hiccup(object_t *op, char *params);
int                         command_lick(object_t *op, char *params);
int                         command_pout(object_t *op, char *params);
int                         command_shiver(object_t *op, char *params);
int                         command_shrug(object_t *op, char *params);
int                         command_slap(object_t *op, char *params);
int                         command_smirk(object_t *op, char *params);
int                         command_snap(object_t *op, char *params);
int                         command_sneeze(object_t *op, char *params);
int                         command_snicker(object_t *op, char *params);
int                         command_sniff(object_t *op, char *params);
int                         command_snore(object_t *op, char *params);
int                         command_spit(object_t *op, char *params);
int                         command_strut(object_t *op, char *params);
int                         command_thank(object_t *op, char *params);
int                         command_twiddle(object_t *op, char *params);
int                         command_wave(object_t *op, char *params);
int                         command_whistle(object_t *op, char *params);
int                         command_wink(object_t *op, char *params);
int                         command_yawn(object_t *op, char *params);
int                         command_beg(object_t *op, char *params);
int                         command_bleed(object_t *op, char *params);
int                         command_cringe(object_t *op, char *params);
int                         command_think(object_t *op, char *params);
int                         command_me(object_t *op, char *params);
/* c_misc.c */
int                         command_motd(object_t *op, char *params);
int                         command_bug(object_t *op, char *params);
void                        malloc_info(object_t *op);
int                         command_who(object_t *op, char *params);
int                         command_malloc(object_t *op, char *params);
int                         command_mapinfo(object_t *op, char *params);
int                         command_mspinfo(object_t *op, char *params);
int                         command_time(object_t *op, char *params);
int                         command_dumpbelowfull(object_t *op, char *params);
int                         command_dumpbelow(object_t *op, char *params);
int                         command_dumpallobjects(object_t *op, char *params);
int                         command_dumpfriendlyobjects(object_t *op, char *params);
int                         command_dumpallarchetypes(object_t *op, char *params);
int                         command_dumpactivelist(object_t *op, char *params);
int                         command_setmaplight(object_t *op, char *params);
#if 0
int                         command_dumpmap(object_t *op, char *params);
int                         command_dumpallmaps(object_t *op, char *params);
#endif
void                        bug_report(char *reportstring);
int                         command_resting(object_t *op, char *params);
int                         command_help(object_t *op, char *params);
int                         command_privacy(object_t *op, char *params);
char                        *get_subdir(const char *name);
int                         command_stuck(object_t *op, char *params);
/* c_move.c */
int                         command_push_object (object_t *op, char *params);
int                         command_turn_right (object_t *op, char *params);
int                         command_turn_left (object_t *op, char *params);
/* c_new.c */
int                         command_run(object_t *op, char *params);
int                         command_run_stop(object_t *op, char *params);
int                         command_combat(object_t *op, char *params);
int                         command_target(object_t *op, char *params);
void                        command_face_request(char *params, int len, NewSocket *ns);
void                        generate_ext_title(player_t *pl);
/* c_object.c */
int                         command_uskill(object_t *pl, char *params);
int                         command_rskill(object_t *pl, char *params);
int                         command_egobind ( object_t *pl, char *params);
object_t                     *find_marked_object(object_t *op);
char                       *examine_monster(object_t *op, object_t *tmp, char *buf, int flag);
char                       *examine(object_t *op, object_t *tmp, int flag);
/* c_party.c */
int                         command_party_invite ( object_t *pl, char *params);
int                         command_party_join ( object_t *pl, char *params);
int                         command_party_deny ( object_t *pl, char *params);
int                         command_party_leave ( object_t *pl, char *params);
int                         command_party_remove ( object_t *pl, char *params);
void                        party_add_member(player_t *leader, player_t *member);
void                        party_remove_member(player_t *member, int flag);
void                        party_message(int mode, int flags, int pri,object_t *leader, object_t *source, char *format, ...) DAI_GNUC_PRINTF(6, 7);
void                        party_client_group_status(object_t *member);
void                        party_client_group_kill(object_t *member);
void                        party_client_group_update(object_t *member, int flag);
/* c_range.c */
float                       fire_magic_tool(object_t *owner, object_t *op, int dir);
int                         command_cast_spell(object_t *op, char *params);
/* c_wiz.c */
int                         command_connections(object_t *op, char *params);
int                         command_kick(object_t *op, char *params);
int                         command_reboot(object_t *op, char *params);
int                         command_goto(object_t *op, char *params);
int                         command_create(object_t *op, char *params);
int                         command_generate(object_t *op, char *params);
int                         command_spawn(object_t *op, char *params);
int                         command_listarch(object_t *op, char *params);
int                         command_mutelevel(object_t *op, char *params);
int                         command_summon(object_t *op, char *params);
int                         command_teleport(object_t *op, char *params);
int                         command_inventory(object_t *op, char *params);
int                         command_dump(object_t *op, char *params);
int                         command_setskill(object_t *op, char *params);
int                         command_addexp(object_t *op, char *params);
int                         command_serverspeed(object_t *op, char *params);
int                         command_stats(object_t *op, char *params);
int                         command_setstat(object_t *op, char *params);
int                         command_resetmap(object_t *op, char *params);
int                         command_check_fd(object_t *op, char *params);
int                         command_mute(object_t *op, char *params);
int                         command_silence(object_t *op, char *params);
int                         command_ban(object_t *op, char *params);
int                         command_sa(object_t *op, char *params);
int                         command_mm(object_t *op, char *params);
int                         command_mw(object_t *op, char *params);
int                         command_gm(object_t *op, char *params);
int                         command_vol(object_t *op, char *params);
int                         command_gmasterlist(object_t *op, char *params);
int                         command_gmasterfile(object_t *op, char *params);
int                         command_listplugins(object_t *op, char *params);
int                         command_loadplugin(object_t *op, char *params);
int                         command_unloadplugin(object_t *op, char *params);
int                         command_ip(object_t *op, char *params);
int                         command_wizpass(object_t *op, char *params);
int                         command_matrix(object_t *op, char *params);
int                         command_stealth(object_t *op, char *params);
int                         command_invisibility(object_t *op, char *params);
int                         command_dm_dev(object_t *op, char *params);
int                         command_dm_light(object_t *op, char *params);
int                         command_password(object_t *op, char *params);
/* commands.c */
void                        init_commands(void);
/* container.c */
int                         container_link(player_t *const pl, object_t *const sack);
int                         container_unlink(player_t *const pl, object_t *sack);
int                         container_trap(object_t *const op, object_t *const container);
object_t                     *pick_up(object_t *who, object_t *what, object_t *where, uint32 nrof);
object_t                     *drop_to_floor(object_t *who, object_t *what, uint32 nrof);
/* disease.c */
int                         move_disease(object_t *disease);
int                         remove_symptoms(object_t *disease);
object_t                     *find_symptom(object_t *disease);
int                         check_infection(object_t *disease);
int                         infect_object(object_t *victim, object_t *disease, int force);
int                         do_symptoms(object_t *disease);
int                         grant_immunity(object_t *disease);
int                         move_symptom(object_t *symptom);
int                         check_physically_infect(object_t *victim, object_t *hitter);
object_t                     *find_disease(object_t *victim);
int                         cure_disease(object_t *sufferer, object_t *caster);
int                         reduce_symptoms(object_t *sufferer, int reduction);
/* egoitem.c */
int                         check_ego_item(object_t *pl, object_t *ob);
char                       *get_ego_item_name(object_t *ob);
void                        create_ego_item(object_t *ob, const char *name, int mode);
/* gmaster.c */
void                        remove_gmaster_list(player_t *pl);
int                         check_gmaster_file_entry(char *name, char *host, char *mode);
int                         load_gmaster_file(void);
void                        add_gmaster_file_entry(char *name, char *host, int mode_id);
void                        remove_gmaster_file_entry(objectlink_t *ol);
int                         check_gmaster_list(player_t *pl, int mode_id);
void                        set_gmaster_mode(player_t *pl, int mode_id);
void                        remove_gmaster_mode(player_t *pl);
int                         compare_gmaster_mode(int t, int p);
void                        write_gmaster_file(void);
void                        update_gmaster_file(void);
void                        free_gmaster_list(void);
/* gods.c */
/* init.c */
char                       *version_string(void);
void                        init(int argc, char **argv);
void                        compile_info(void);
void                        fatal_signal(int make_core, int close_sockets, uint8 status);
void                        init_library(void);
void                        free_strings(void);
void                        set_pticks_time(sint32 t);
void                        free_lists_and_tables(void);
/* ipcompare.c */
int                         parse_ip(const char * ip, unsigned char ip_terms[], int mask_pos[]);
int                         ip_compare(const char *ban_tmp, const char *ip_temp);
objectlink_t                 *find_players_on_ip(char *ipmask);
void                        free_iplist(objectlink_t *ip_list);
/* login.c */
int                         player_save(object_t *op);
addme_login_msg             player_load(NewSocket *ns, const char *name);
addme_login_msg             player_create(NewSocket *ns,player_t **pl_ret,char *name,int race,int gender,int skill_nr);
void                        player_addme_failed(NewSocket *ns, int error_msg);
/* main.c */
char                       *crypt_string(char *str);
int                         get_new_instance_num(void);
void                        process_players1(map_t *map);
void                        process_players2(map_t *map);
void                        clean_tmp_files(int flag);
void                        cleanup_without_exit(void);
void                        leave(player_t *pl, int draw_exit);
void                        dequeue_path_requests(void);
void                        do_specials(void);
void                        shutdown_agent(int timer, int ret, player_t *pl, char *reason);
int                         main(int argc, char **argv);
void                        process_events();
void                        iterate_main_loop();
/* monster.c */
object_t                     *get_active_waypoint(object_t *op);
object_t                     *get_aggro_waypoint(object_t *op);
object_t                     *get_return_waypoint(object_t *op);
object_t                     *find_waypoint(object_t *op, const char *name);
object_t                     *get_random_waypoint(object_t *op, object_t *ignore);
object_t                     *get_next_waypoint(object_t *op, object_t *wp);
int                         move_monster(object_t *op, int mode);
void                        object_accept_path(object_t *op);
void                        dump_abilities(void);
void                        print_monsters(void);
/* monster_memory.c */
void                        cleanup_mob_knowns(object_t *op, struct mob_known_obj **first, hashtable_t *ht);
void                        clear_mob_knowns(object_t *op, struct mob_known_obj **first, hashtable_t *ht);
struct mob_known_obj       *update_npc_knowledge(object_t *npc, object_t *other, int delta_friendship, int delta_attraction);
void                        update_npc_known_obj(struct mob_known_obj *known, int delta_friendship, int delta_attraction);
struct                      mob_known_obj *register_npc_known_obj(object_t *npc, object_t *other, int friendship, int attraction, int check_los);
rv_t                  *get_known_obj_rv(object_t *op, struct mob_known_obj *known_obj, int maxage);
/* monster_behaviourset.c */
struct mob_behaviourset    *parse_behaviourconfig(const char *conf_text, object_t *op);
void                        init_arch_default_behaviours();
void                        initialize_mob_data(struct mobdata *data);
void                        cleanup_mob_data(struct mobdata *data);
struct mob_behaviourset    *setup_behaviours(object_t *op);
void                        cleanup_behaviourset(struct mob_behaviourset *data);
void                        cleanup_mob_known_obj(struct mob_known_obj *data);
int                         can_hit(object_t *ob1, object_t *ob2, rv_t *rv);
void                        cleanup_all_behavioursets();
void                        reload_behaviours(object_t *op);
/* monster_behaviours.c */
int                         mob_can_see_obj(object_t *op, object_t *obj, struct mob_known_obj *known_obj);
int                         get_friendship(object_t *op, object_t *obj);
int                         get_attitude(object_t *op, object_t *obj);
object_t                     *monster_choose_random_spell(object_t *monster);
void                        monster_check_pickup(object_t *monster);
void                        monster_check_apply(object_t *mon, object_t *item);
void                        npc_call_help(object_t *op);
map_t                  *normalize_and_ready_map(map_t *defmap, const char **path);
/* mtrand.c */
void                        MTRand_init(const uint32 seed);
sint32                      MTRand_randComp(void);
/* npc_communicate.c */
void                        communicate(object_t *op, char *txt);
void                        talk_to_npc(player_t *pl, char *topic);
void                        gui_npc(object_t *who, uint8 mode, const char *text);
/* spawn_point.c */
void                        spawn_point(object_t *op);
void                        make_mob_homeless(object_t *mob);
void                        adjust_monster(object_t *monster);
objectlink_t                 *add_linked_spawn(object_t *spawn);
void                        remove_linked_spawn_list(map_t *map);
void                        send_link_spawn_signal(object_t *spawn, object_t *target, int signal);
/* move.c */
/* pets.c */
/* player.c */
player_t                     *find_player(char *plname);
player_t                     *find_player_hash(const char *plname);
void                        display_motd(object_t *op);
void                        free_player(player_t *pl);
void                        destroy_player_struct(player_t *pl);
object_t                     *get_nearest_player(object_t *mon);
int                         path_to_player(object_t *mon, object_t *pl, int mindiff);
void                        give_initial_items(object_t *pl, struct objectlink_t *items);
void                        flee_player(object_t *op);
int                         move_player(object_t *const op, int dir, const int flag);
void                        do_some_living(object_t *op);
void                        cast_dust(object_t *op, object_t *throw_ob, int dir);
void                        make_visible(object_t *op);
int                         is_true_undead(object_t *op);
int                         hideability(object_t *ob);
void                        do_hidden_move(object_t *op);
int                         stand_near_hostile(object_t *who);
int                         action_makes_visible(object_t *op);
void                        reset_instance_data(player_t *pl);
void                        kick_player(player_t *pl);
char                       *get_online_players_info(player_t *who, player_t *diff,
                                                    uint8 force);
/* plugins.c */
object_t                     *get_event_object(object_t *op, int event_nr);
int                         trigger_object_plugin_event(int event_type,
                object_t *const me, object_t *const activator, object_t *const other,
                const char *msg, int *parm1, int *parm2, int *parm3, int flags);
int                         find_plugin_command(const char *cmd, object_t *op, CommArray_s *ret);
void                        displayPluginsList(object_t *op);
int                         findPlugin(const char *id);
void                        initPlugins(void);
void                        removeOnePlugin(const char *id);
void                        initOnePlugin(const char *pluginfile);
void                        removePlugins(void);
CFParm                     *CFWCmdRSkill(CFParm *PParm);
CFParm                     *CFWBecomeFollower(CFParm *PParm);
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
void                        send_plugin_custom_message(object_t *pl, char *buf);
/* rune.c */
int                         write_rune(object_t *op, int dir, int inspell, int level, char *runename);
void                        rune_attack(object_t *op, object_t *victim);
void                        spring_trap(object_t *trap, object_t *victim);
int                         dispel_rune(object_t *op, int dir, int risk);
int                         trap_see(object_t *op, object_t *trap, int level);
int                         trap_show(object_t *trap, object_t *where);
int                         trap_disarm(object_t *disarmer, object_t *trap, int risk);
void                        trap_adjust(object_t *trap, int difficulty);
/* shop.c */
sint64                      query_cost(object_t *tmp, object_t *who, int flag);
char                       *cost_string_from_value(sint64 cost, int mode);
char                       *query_cost_string(object_t *tmp, object_t *who, int flag, int mode);
sint64                      query_money(object_t *where, moneyblock_t *money);
uint8                       shop_pay_amount(sint64 amount, object_t *op);
uint8                       shop_checkout(object_t *op, object_t *this);
void                        shop_return_unpaid(object_t *who, msp_t *msp);
int                         get_money_from_string(char *text, struct moneyblock_t *money);
int                         query_money_type(object_t *op, int value);
int                         enumerate_coins(sint64 value, struct moneyblock_t *money);
object_t                     *create_financial_loot(moneyblock_t *money, object_t *who, uint8 mode);
/* skills.c */
int                         attack_melee_weapon(object_t *op, int dir, char *string);
int                         attack_hth(object_t *pl, int dir, char *string);
int                         skill_attack(object_t *tmp, object_t *pl, int dir, char *string);
int                         do_skill_attack(object_t *tmp, object_t *op, char *string);
int                         SK_level(object_t *op);
int                         find_traps(object_t *pl, int level);
int                         remove_trap(object_t *op, int dir, int level);
/* skill_util.c */
void                        init_skills(void);
void                        link_player_skills(player_t *pl);
void                        validate_skills(player_t *pl);
object_t                     *find_skill(object_t *op, int skillnr);
int                         do_skill(object_t *op, int dir, char *string);
int                         get_weighted_skill_stat_sum(object_t *who, int sk);
void                        dump_skills(void);
int                         lookup_skill_by_name(char *name);
int                         check_skill_to_apply(object_t *who, object_t *item);
int                         learn_skill(object_t *pl, int skillnr);
int                         use_skill(object_t *op, char *string);
sint8                       change_skill(object_t *who, sint16 nr);
void                        set_action_time(object_t *op, float ticks);
int                         check_skill_action_time(object_t *op, object_t *skill);
int                         get_skill_stat1(object_t *op);
int                         get_skill_stat2(object_t *op);
int                         get_skill_stat3(object_t *op);
int                         get_weighted_skill_stats(object_t *op);
/* spell_effect.c */
void                        prayer_failure(object_t *op, int failure, int power);
void                        cast_mana_storm(object_t *op, int lvl);
void                        cast_magic_storm(object_t *op, object_t *tmp, int lvl);
int                         recharge(object_t *op);
int                         probe(object_t *op);
int                         cast_invisible(object_t *op, object_t *caster, int spell_type);
int                         perceive_self(object_t *op);
int                         cast_heal(object_t *op, int level, object_t *target, int spell_type);
int                         cast_change_attr(object_t *op, object_t *caster, object_t *target, int dir, int spell_type);
int                         remove_curse(object_t *op, object_t *target, int type, SpellTypeFrom src);
int                         cast_identify(object_t *op, int level, object_t *single_ob, int mode);
int                         cast_detection(object_t *op, object_t *target, int type);
object_t                     *cure_what_ails_you(object_t *op, uint8 st1);
int                         fire_arch(object_t *op, object_t *caster, sint16 x, sint16 y, int dir, archetype_t *at, int type, int level, int magic);
void                        check_fired_arch(object_t *op);
void                        move_fired_arch(object_t *op);
/* spell_util.c */
void                        init_spells(void);
void                        dump_spells(void);
int                         insert_spell_effect(char *archname, map_t *m, int x, int y);
spell                      *find_spell(int spelltype);
int                         check_spell_known(object_t *op, int spell_type);
void                        do_learn_spell(object_t *op, int spell);
void                        do_forget_spell(object_t *op, int spell);
int                         cast_spell(object_t *op, object_t *caster, int dir, int type, int ability, SpellTypeFrom item,
                                       char *stringarg);
int                         fire_bolt(object_t *op, object_t *caster, int dir, int type, int magic);
int                         cast_cone(object_t *op, object_t *caster, int dir, int strength, int spell_type,
                                      archetype_t *spell_arch, int level, int magic);
void                        check_cone_push(object_t *op);
void                        cone_drop(object_t *op);
void                        move_cone(object_t *op);
void                        explosion(object_t *op);
void                        forklightning(object_t *op, object_t *tmp);
int                         reflwall(msp_t *msp, object_t *sp_op);
void                        move_bolt(object_t *op);
void                        move_golem(object_t *op);
void                        control_golem(object_t *op, int dir);
void                        move_magic_missile(object_t *op);
void                        explode_object(object_t *op);
void                        drain_rod_charge(object_t *rod);
void                        fix_rod_speed(object_t *rod);
int                         find_target_for_spell(object_t *op, object_t *item, object_t **target, int dir, uint32 flags);
int                         can_see_monsterP(map_t *m, int x, int y, int dir);
int                         SP_level_dam_adjust(object_t *op, object_t *caster, int spell_type);
int                         SP_level_strength_adjust(object_t *op, object_t *caster, int spell_type);
int                         SP_level_spellpoint_cost(object_t *op, object_t *caster, int spell_type);
int                         look_up_spell_by_name(object_t *op, const char *spname);
void                        shuffle_attack(object_t *op, int change_face);
int                         SP_lvl_dam_adjust(int level, int spell_type, int base_dam);
/* stats.c */
void                        stats_event(stats_event_type type, ...);
/* time.c */
void                        regenerate_rod(object_t *rod);
void                        remove_force(object_t *op);
void                        poison_more(object_t *op);
void                        move_gate(object_t *op);
void                        move_timed_gate(object_t *op);
void                        move_detector(object_t *op);
void                        move_conn_sensor(object_t *op);
void                        move_environment_sensor(object_t *op);
void                        animate_trigger(object_t *op);
void                        move_pit(object_t *op);
void                        change_object(object_t *op);
void                        move_teleporter(object_t *op);
void                        move_firewall(object_t *op);
void                        move_player_mover(object_t *op);
void                        move_creator(object_t *op);
void                        move_marker(object_t *op);
int                         process_object(object_t *op);
/* timers.c */
void                        cftimer_process_timers(void);
int                         cftimer_create(int id, long delay, object_t *ob, int mode);
int                         cftimer_destroy(int id);
int                         cftimer_find_free_id(void);
/* pathfinder.c */
/* swamp.c */
extern object_t *sparkly_create(archetype_t *at, object_t *who, sint16 t, sint16 nr, uint8 stype);
extern void      sparkly_move(object_t *effect);

int command_level(object_t *op, char *params);

#endif /* ifndef __SPROTO_H */
