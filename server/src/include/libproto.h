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

#ifndef __LIBPROTO_H
#define __LIBPROTO_H

/* anim.c */
extern void             free_all_anim(void);
extern void             init_anim(void);
extern int              find_animation(char *name);
extern void             animate_object(object *op, int count);
/* arch.c */
extern archetype       *find_archetype_by_object_name(const char *name);
extern object          *get_archetype_by_object_name(const char *name);
extern void             init_archetypes(void);
extern void             arch_info(object *op);
extern void             clear_archetable(void);
extern void             init_archetable(void);
extern void             dump_arch(archetype *at);
extern void             dump_all_archetypes(void);
extern void             free_all_archs(void);
extern archetype       *get_archetype_struct(void);
extern void             first_arch_pass(FILE *fp);
extern void             second_arch_pass(FILE *fp_start);
extern void             load_archetypes(void);
extern object          *arch_to_object(archetype *at);
extern object          *create_singularity(const char *name);
extern object          *get_archetype(const char *name);
extern archetype       *find_archetype(const char *name);
extern void             add_arch(archetype *at);
extern archetype       *type_to_archetype(int type);
extern object          *clone_arch(int type);
/* artifact.c */
extern void             load_artifacts(int mode);
extern inline artifactlist *find_artifactlist(int type);
extern artifact        *find_artifact(const char *name);
extern void             dump_artifacts(void);
extern void             add_artifact_archtype(void);
extern void             give_artifact_abilities(object *op, artifact *art);
extern int              generate_artifact(object *op, int difficulty, int t_style, int a_chance);
extern void             free_artifactlist(artifactlist *al);
extern int              legal_artifact_combination(object *op, artifact *art);
/* button.c */
extern void             signal_connection(object *op, object *activator, object *originator, mapstruct *m);
extern void             update_button(object *op, object *activator, object *originator);
extern void             update_buttons(mapstruct *m);
extern void             animate_turning(object *op);
extern int              check_altar_sacrifice(object *altar, object *sacrifice);
extern int              operate_altar(object *altar, object **sacrifice);
extern void             trigger_move(object *op, int state, object *trigger);
extern int              check_trigger(object *op, object *cause, object *originator);
extern void             add_button_links(object *button, mapstruct *map, char *connected);
extern void             add_button_link(object *button, mapstruct *map, int connected);
extern void             remove_button_link(object *op);
extern int              get_button_value(object *button);
extern void             do_mood_floor(object *op, object *op2);
extern object          *check_inv_recursive(object *op, object *trig);
extern void             check_inv(object *op, object *trig);
extern void             verify_button_links(mapstruct *map);
/* exp.c */
extern sint32           add_exp(object *op, int exp, int skill_nr, int cap);
extern void             apply_death_exp_penalty(object *op);
extern float            calc_level_difference(int who_lvl, int op_lvl);
extern int              calc_skill_exp(object *who, object *op, float mod, int level, int *real);
/* food.c */
extern void             remove_food_force(object *op);
extern void             food_force_reg(object *op);
extern void             create_food_buf_force(object *who, object *food, object *force);
extern int              dragon_eat_flesh(object *op, object *meal);
/* guild.c */
extern object          *guild_get(player *pl, char *name);
extern void             guild_remove_restricted_items(player *pl);
extern object          *guild_join(player *pl, char *name, int s1_group, int s1_value, int s2_group, int s2_value, int s3_group, int s3_value);
extern void             guild_leave(player *pl);
/* holy.c */
extern void             init_gods(void);
extern void             add_god_to_list(archetype *god_arch);
extern int              baptize_altar(object *op);
extern godlink         *get_rand_god(void);
extern object          *pntr_to_god_obj(godlink *godlnk);
extern void             free_all_god(void);
extern void             dump_gods(void);
/* image.c */
extern int              ReadBmapNames(void);
extern int              FindFace(const char *name, int error);
extern void             free_all_images(void);
/* item.c */
extern char            *describe_resistance(const object *const op, int newline);
extern char            *describe_attack(const object *const op, int newline);
extern char            *query_weight(object *op);
extern char            *get_levelnumber(int i);
extern char            *get_number(int i);
extern char            *query_short_name(const object *const op, const object *const caller);
extern char            *query_name_full(const object *op, const object *caller);
extern char            *query_base_name(object *op, object *caller);
extern char            *describe_item(const object *const op);
extern int              need_identify(const object *const op);
extern void             identify(object *op);
extern void             set_traped_flag(object *op);
extern int              check_magical_container(const object *op, const object *env);
/* links.c */
extern objectlink      *get_objectlink(int id);
extern oblinkpt        *get_objectlinkpt(void);
extern void             free_objectlink_recursive(objectlink *ol);
extern void             free_objectlinkpt(oblinkpt *obp);
extern void             free_objectlink(objectlink *ol);
extern objectlink      *objectlink_link(objectlink **startptr, objectlink **endptr,
                                        objectlink *afterptr, objectlink *beforeptr, objectlink *objptr);

extern objectlink      *objectlink_unlink(objectlink **startptr, objectlink **endptr, objectlink *objptr);
/* living.c */
extern void             set_stat_value(living *stats, int stat, signed char value);
extern void             change_stat_value(living *stats, int stat, signed char value);
extern signed char      get_stat_value(const living *const stats, const int stat);
extern void             check_stat_bounds(living *stats);
extern int              change_abil(object *op, object *tmp);
extern object          *check_obj_stat_buffs(object *ob, object *pl);
extern void             drain_stat(object *op);
extern void             drain_specific_stat(object *op, int deplete_stats);
extern void             drain_level(object *op, int level, int mode, int ticks);
extern float            get_player_stat_bonus(int value);
void                    fix_player_weight(object *op);
#ifdef DEBUG_FIX_PLAYER
extern void             fix_player(object *op, char *msg);
#else
extern void             fix_player(object *op);
#endif
extern void             set_dragon_name(object *pl, object *abil, object *skin);
extern void             dragon_level_gain(object *who);
extern void             fix_monster(object *op);
extern object          *insert_base_info_object(object *op);
extern object          *find_base_info_object(object *op);
extern void             set_mobile_speed(object *op, int factor);
extern void             leech_hind(object *leecher, object *leechee,
                                   uint8 attack, sint16 plose, sint16 pmod,
                                   uint8 chance);
/* loader.c */
extern int              lex_load(object *op, int map_flags);
extern void             yyrestart(FILE *input_file);
extern void             yy_load_buffer_state(void);
extern int              yyerror(char *s);
extern void             delete_loader_buffer(void *buffer);
extern void            *create_loader_buffer(void *fp);
extern int              load_object(void *fp, object *op, void *mybuffer, int bufstate, int map_flags);
extern int              set_variable(object *op, char *buf);
extern void             save_double(char *buf, char *name, double v);
extern void             init_vars(void);
extern char            *get_ob_diff(const object *op, const object *op2);
extern void             save_object(FILE *fp, object *op, int flag);
/* logger.c */
extern void             LOG(LogLevel logLevel, char *format, ...) DAI_GNUC_PRINTF(2, 3);
extern void             CHATLOG(char *format, ...) DAI_GNUC_PRINTF(1, 2);
/* los.c */
extern void             init_block(void);
extern void             set_block(int x, int y, int bx, int by);
extern void             update_los(object *op);
extern void             expand_sight(object *op);
extern int              has_carried_lights(object *op);
extern inline void      clear_los(object *op);
extern void             print_los(object *op);
extern void             make_sure_seen(object *op);
extern void             make_sure_not_seen(object *op);
extern void             adjust_light_source(mapstruct *map, int x, int y, int light);
extern void             check_light_source_list(mapstruct *map);
extern void             remove_light_source_list(mapstruct *map);
extern int              obj_in_line_of_sight(object *op, object *obj, rv_vector *rv);
/* map.c */
extern mapstruct       *map_is_in_memory(shstr *name);
extern mapstruct       *map_is_ready(shstr *path_sh);
extern char            *create_mapdir_pathname(const char *name);
extern sint8            check_path(const char *name, uint8 prepend_dir);
extern char            *normalize_path(const char *src, const char *dst, char *path);
extern char            *normalize_path_direct(const char *src, const char *dst, char *path);
extern mapstruct       *ready_inherited_map(mapstruct *orig_map, shstr *new_map_path);
extern void             dump_map(mapstruct *m, player *pl, int list, char *ref);
extern void             dump_msp(mapstruct *m, int x, int y, player *pl);
extern void             set_map_darkness(mapstruct *m, int value);
extern mapstruct       *map_save(mapstruct *m);
extern void             delete_map(mapstruct *m);
extern shstr           *create_safe_path_sh(const char *path);
extern shstr           *create_unique_path_sh(object *op, shstr *orig_path_sh);
extern shstr           *create_instance_path_sh(player *pl, shstr *orig_path_sh, uint32 flags);
extern mapstruct       *ready_map_name(shstr *path_sh, shstr *orig_path_sh, uint32 flags, shstr *reference);
extern void             clean_tmp_map(mapstruct *m);
extern void             free_all_maps(void);
extern void             set_bindpath_by_name(player *pl, const char *dst, const char *src, int status, int x, int y);
extern void             set_bindpath_by_default(player *pl);
extern void             set_mappath_by_name(player *pl, const char *dst, const char *src, int status, int x, int y);
extern void             set_mappath_by_map(object* op);
extern void             set_mappath_by_default(player *pl);
extern uint16           map_player_unlink(mapstruct *m, shstr *path_sh);
extern void             map_player_link(mapstruct *m, sint16 x, sint16 y, uint8 flag);
extern void             read_map_log(void);
extern void             swap_map(mapstruct *map, int force_flag);
extern void             map_check_in_memory(mapstruct *m);
extern int              players_on_map(mapstruct *m);
/* map_tile.c */
extern void             map_set_slayers(MapSpace *msp, object *op, uint8 insert);
extern void             update_position(mapstruct *m, MapSpace *mspace,int x, int y);
extern int              map_brightness(mapstruct *m, int x, int y);
extern int              wall(mapstruct *m, int x, int y);
extern int              blocks_view(mapstruct *m, int x, int y);
extern int              blocks_magic(mapstruct *m, int x, int y);
extern int              blocks_cleric(mapstruct *m, int x, int y);
extern int              blocked(object *op, mapstruct *m, int x, int y, int terrain);
extern int              blocked_link(object *op, mapstruct *map, int x, int y);
extern int              blocked_tile(object *op, mapstruct *m, int x, int y);
extern int              arch_blocked(archetype *at, object *op, mapstruct *m, int x, int y);
extern int              arch_out_of_map(archetype *at, mapstruct *m, int x, int y);
extern mapstruct       *out_of_map(mapstruct *m, int *x, int *y);
extern mapstruct       *out_of_map2(mapstruct *m, int *x, int *y);
extern int              get_rangevector(object *op1, object *op2, rv_vector *retval, int flags);
extern int              get_rangevector_from_mapcoords(mapstruct *map1, int x1, int y1, mapstruct *map2, int x2, int y2,
                                                       rv_vector *retval, int flags);
extern int              get_rangevector_full(object *op1, mapstruct *map1, int x1, int y1, object *op2, mapstruct *map2, int x2, int y2,
                                             rv_vector *retval, int flags);
extern int              on_same_map(object *op1, object *op2);
extern int              on_same_tileset(object *op1, object *op2);
extern int              in_same_instance(mapstruct *m1, mapstruct *m2);
extern void             map_transfer_apartment_items(mapstruct *map_old, mapstruct * map_new, int x, int y);
/* material.c */
extern void             material_attack_damage(object *op, int num, int chance, int base);
extern sint64           material_repair_cost(object *item, object *owner);
extern void             material_repair_item(object *item, int skill_value);
/* mempool.c */
extern uint32           nearest_pow_two_exp(uint32 n);
extern void             init_mempools();
extern void             cleanup_mempools();
extern void             free_mempool(struct mempool *pool);
extern struct mempool * (create_mempool)(const char *description, uint32 expand, uint32 size,
                uint32 flags, chunk_initialisator initialisator, chunk_deinitialisator deinitialisator,
                chunk_constructor constructor, chunk_destructor destructor);
extern void             free_empty_puddles(struct mempool *pool);
extern void             return_poolchunk_array_real(void *data, uint32 arraysize_exp, struct mempool *pool);
extern void            *get_poolchunk_array_real(struct mempool *pool, uint32 arraysize_exp);
extern void             dump_mempool_statistics(object *op, int *sum_used, int *sum_alloc);
/* missile.c */
extern float			do_throw(object *op, int dir);
extern float			fire_bow(object *op, int dir);
extern object          *find_arrow(object *op, const char *type);
extern object		   *create_missile(object * const owner, const object * const bow,
											  object *const missile, const int dir);
extern void             move_missile(object *op);
extern void             stop_missile(object *op);
/* object.c */
extern void             mark_object_removed(object *ob);
extern object          *merge_ob(object *op, object *tmp);
extern sint32           sum_weight(object *op);
extern object          *is_player_inv(object *op);
extern void             dump_object2(const object *op);
extern void             dump_object(const object *op);
extern void             dump_me(object *op, char *outstr, size_t bufsize);
extern void             dump_all_objects(void);
extern void             free_all_object_data(void);
extern object          *get_owner(object *op);
extern void             clear_owner(object *op);
extern void             set_owner(object * const op, object * const owner);
extern void             copy_owner(object *op, object *clone);
extern void             initialize_object(object *op);
extern void             copy_object(object *src, object *dest);
extern void             copy_object_data(object *op2, object *op);
extern object          *get_object(void);
extern void             update_turn_face(object *op);
extern void             activelist_insert(object *op);
extern void             activelist_remove(object *op);
extern void             update_ob_speed(object *op);
extern void             update_object(object *op, int action);
extern void             destroy_object(object *ob);
extern void             free_object_data(object *ob, int free_static_data);
extern int              count_free(void);
extern int              count_used(void);
extern void             remove_ob(object *op);
extern void             destruct_ob(object *op);
extern void             drop_ob_inv(object *op);
extern void             remove_ob_inv(object *op);
extern object          *insert_ob_in_map(object *const op, mapstruct *m, object *const originator, const int flag);
extern void             replace_insert_ob_in_map(char *arch_string, object *op);
extern object          *get_split_ob(object *orig_ob, uint32 nr);
extern object          *decrease_ob_nr(object *op, uint32 i);
extern object          *insert_ob_in_ob(object *op, object *where);
extern int              check_walk_on(object *const op, object *const originator, int flags);
extern int              check_walk_off(object *op, object *originator, int flags);
extern object          *present_arch(archetype *at, mapstruct *m, int x, int y);
extern object          *present(unsigned char type, mapstruct *m, int x, int y);
extern object          *present_in_ob(unsigned char type, object *op);
extern object          *present_arch_in_ob(archetype *at, object *op);
extern object          *present_arch_in_ob_temp(archetype *at, object *op);
extern int              find_free_spot(archetype *at, object *op, mapstruct *m, int x, int y, int ins_flags, int start, int stop);
extern int              find_first_free_spot(archetype *at, object *op, mapstruct *m, int x, int y);
extern int              find_dir(mapstruct *m, int x, int y, object *exclude);
extern int              find_dir_2(int x, int y);
extern int              absdir(int d);
extern int              dirdiff(int dir1, int dir2);
extern object          *clone_object(object *original, uint8 mode);
extern int              was_destroyed(const object *const op, const tag_t old_tag);
extern object          *load_object_str(char *obstr);
extern void             object_gc();
extern int              auto_apply(object *op);
extern object          *locate_beacon(shstr *id);
extern void             init_object_initializers();
extern object          *find_next_object(object *op, uint8 type, uint8 mode, object *root);
extern void             revert_buff_stats(object *item);
extern void             fix_buff_stats(object *item);
extern object          *check_buff_exists(object *item, const char *name);
extern int              add_item_buff(object *item, object *buff, short just_checking);
extern int              remove_item_buff(object *item, char *name, uint32 nrof);

/* re-cmp.c */
extern char            *re_cmp(char *str, char *regexp);
/* readable.c */
extern int              nstrtok(const char *buf1, const char *buf2);
extern char            *strtoktolin(const char *buf1, const char *buf2);
extern int              book_overflow(const char *buf1, const char *buf2, int booksize);
extern void             init_readable(void);
extern void             change_book(object *book, int msgtype);
extern object          *get_random_mon(int level);
extern char            *mon_desc(object *mon);
extern object          *get_next_mon(object *tmp);
extern char            *mon_info_msg(int level, int booksize);
extern char            *artifact_msg(int level, int booksize);
extern char            *spellpath_msg(int level, int booksize);
extern void             make_formula_book(object *book, int level);
extern char            *msgfile_msg(int level, int booksize);
extern char            *god_info_msg(int level, int booksize);
extern void             tailor_readable_ob(object *book, int msg_type);
extern void             free_all_readable(void);
extern void             write_book_archive(void);
extern const char       *get_language(uint32 lang);
/* recipe.c */
extern recipelist      *get_formulalist(int i);
extern void             init_formulae(void);
extern void             check_formulae(void);
extern void             dump_alchemy(void);
extern archetype       *find_treasure_by_name(treasure *t, char *name, int depth);
extern sint64           find_ingred_cost(const char *name);
extern void             dump_alchemy_costs(void);
extern const char      *ingred_name(const char *name);
extern int              strtoint(const char *buf);
extern artifact        *locate_recipe_artifact(recipe *rp);
extern int              numb_ingred(const char *buf);
extern recipelist      *get_random_recipelist(void);
extern recipe          *get_random_recipe(recipelist *rpl);
extern void             free_all_recipes(void);
/* shstr.c */
extern void             init_hash_table(void);
extern const char      *add_string(const char *str);
extern const char      *add_lstring(const char *str, int n);
extern int              query_refcount(const char *str);
extern const char      *find_string(const char *str);
extern const char      *add_refcount(const char *str);
extern void             free_string_shared(const char *str);
extern char            *ss_dump_statistics(char *msg);
extern char            *ss_dump_table(int what);
extern void             ss_get_totals(int *entries, int *refs, int *links);
/* treasure.c */
extern void             load_treasures(void);
extern treasurelist    *find_treasurelist(const char *name);
extern objectlink      *link_treasurelists(char *liststring, uint32 flags);
extern void             unlink_treasurelists(objectlink *list, int flag);
extern object          *generate_treasure(struct oblnk *t, int difficulty);
extern void             create_treasure_list(struct oblnk *t, object *op, int flag, int difficulty, int a_chance, int tries);
extern int              create_treasure(treasurelist *t, object *op, int flag, int difficulty, int t_style,
                                        int a_chance, int magic, int magic_chance, int tries, struct _change_arch *change_arch);
extern int              create_all_treasures(treasure *t, object *op, int flag, int difficulty, int t_style,
                                             int a_chance,  int magic, int magic_chance, int tries, struct _change_arch *change_arch);
extern int              create_one_treasure(treasurelist *tl, object *op, int flag, int difficulty, int t_style,
                                            int a_chance,  int magic, int magic_chance, int tries, struct _change_arch *change_arch);
extern void             set_abs_magic(object *op, int magic);
extern int              set_ring_bonus(object *op, int bonus, int level);
extern int              get_magic(int diff);
extern int              fix_generated_item(object **op, object *creator, int difficulty, int a_chance, int t_style,
                                           int max_magic, int chance_magic, int flags);
extern void             dump_monster_treasure_rec(const char *name, treasure *t, int depth);
extern void             fix_flesh_item(object *item, object *donor);
extern void             free_treasurestruct(treasure *t);
extern void             free_all_treasures(void);
extern void             dump_monster_treasure(const char *name);
extern int              get_enviroment_level(object *op);
/* utils.c */
extern int              random_roll(int min, int max);
extern int              look_up_spell_name(const char *spname);
extern racelink        *find_racelink(const char *name);
extern char            *cleanup_string(char *ustring);
extern char            *get_token(char *string, char *token, uint8 qflag);
extern int              buf_overflow(const char *buf1, const char *buf2, int bufsize);
extern void             bitstostring(long bits, int num, char *str);
extern int              clipped_percent(int a, int b);
extern void             NDI_LOG(LogLevel logLevel, int flags, int pri, object *ob, char *format, ...) DAI_GNUC_PRINTF(5, 6);
/* view_map.c */
void            draw_client_map(object *pl);
void            draw_client_map2(object *pl);
void            set_personal_light(player *pl, int value);

#endif /* ifndef __LIBPROTO_H */
