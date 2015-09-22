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
extern void             animate_object(object_t *op, int count);
/* arch.c */
/* artifact.c */
extern void             load_artifacts(int mode);
extern inline artifactlist *find_artifactlist(int type);
extern artifact        *find_artifact(const char *name);
extern void             dump_artifacts(void);
extern void             add_artifact_archetype_type(void);
extern void             give_artifact_abilities(object_t *op, artifact *art);
extern int              generate_artifact(object_t *op, int difficulty, int t_style, int a_chance);
extern void             free_artifactlist(artifactlist *al);
extern int              legal_artifact_combination(object_t *op, artifact *art);
/* button.c */
extern void             signal_connection(object_t *op, object_t *activator, object_t *originator, map_t *m);
extern void             update_button(object_t *op, object_t *activator, object_t *originator);
extern void             update_buttons(map_t *m);
extern void             animate_turning(object_t *op);
extern int              check_altar_sacrifice(object_t *altar, object_t *sacrifice);
extern int              operate_altar(object_t *altar, object_t **sacrifice);
extern void             trigger_move(object_t *op, int state, object_t *trigger);
extern int              check_trigger(object_t *op, object_t *cause, object_t *originator);
extern void             add_button_links(object_t *button, map_t *map, char *connected);
extern void             add_button_link(object_t *button, map_t *map, int connected);
extern void             remove_button_link(object_t *op);
extern int              get_button_value(object_t *button);
extern void             do_mood_floor(object_t *op, object_t *op2);
extern object_t          *check_inv_recursive(object_t *op, object_t *trig);
extern void             check_inv(object_t *op, object_t *trig);
extern void             verify_button_links(map_t *map);
/* exp.c */
extern sint32           add_exp(object_t *op, int exp, int skill_nr, int cap);
extern void             apply_death_exp_penalty(object_t *op);
extern float            calc_level_difference(int who_lvl, int op_lvl);
extern int              calc_skill_exp(object_t *who, object_t *op, float mod, int level, int *real);
/* food.c */
extern void             remove_food_force(object_t *op);
extern void             food_force_reg(object_t *op);
extern void             create_food_buf_force(object_t *who, object_t *food, object_t *force);
extern int              dragon_eat_flesh(object_t *op, object_t *meal);
/* guild.c */
extern object_t          *guild_get(player_t *pl, char *name);
extern void             guild_remove_restricted_items(player_t *pl);
extern object_t          *guild_join(player_t *pl, char *name, int s1_group, int s1_value, int s2_group, int s2_value, int s3_group, int s3_value);
extern void             guild_leave(player_t *pl);
/* holy.c */
extern void             init_gods(void);
extern void             add_god_to_list(archetype_t *god_arch);
extern int              baptize_altar(object_t *op);
extern godlink         *get_rand_god(void);
extern object_t          *pntr_to_god_obj(godlink *godlnk);
extern void             free_all_god(void);
extern void             dump_gods(void);
/* image.c */
extern int              ReadBmapNames(void);
extern int              FindFace(const char *name, int error);
extern void             free_all_images(void);
/* item.c */
extern char            *describe_resistance(const object_t *const op, int newline);
extern char            *describe_attack(const object_t *const op, int newline);
extern char            *describe_item(const object_t *const op);
extern int              need_identify(const object_t *const op);
extern void             identify(object_t *op);
extern void             set_traped_flag(object_t *op);
extern int              check_magical_container(const object_t *op, const object_t *env);
/* links.c */
/* living.c */
/* loader.c */
extern int              lex_load(object_t *op, int flags);
extern void             yyrestart(FILE *input_file);
extern void             yy_load_buffer_state(void);
extern int              yyerror(char *s);
extern void             delete_loader_buffer(void *buffer);
extern void            *create_loader_buffer(void *fp);
extern int              load_object(void *fp, object_t *op, void *mybuffer, int bufstate, int flags);
extern int              set_variable(object_t *op, char *buf);
extern void             save_double(char *buf, char *name, double v);
extern void             init_vars(void);
extern char            *get_ob_diff(const object_t *op, const object_t *op2);
extern void             save_object(FILE *fp, object_t *op, int flag);
/* logger.c */
/* los.c */
extern void             init_block(void);
extern void             set_block(int x, int y, int bx, int by);
extern void             update_los(player_t *pl);
extern void             print_los(object_t *op);
extern int              has_carried_lights(object_t *op);
extern void             make_sure_seen(object_t *op);
extern void             make_sure_not_seen(object_t *op);
extern int              obj_in_line_of_sight(object_t *op, object_t *obj, rv_t *rv);
extern void             los_find_target(player_t *pl, uint8 mode, sint16 start, sint16 stop, sint16 ox, sint16 oy);
extern void             adjust_light_source(msp_t *msp, int light);
extern void             check_light_source_list(map_t *map);
extern void             remove_light_source_list(map_t *map);
/* map.c */
/* map_tile.c */
/* material.c */
extern void             material_attack_damage(object_t *op, int num, int chance, int base);
extern sint64           material_repair_cost(object_t *item, object_t *owner);
extern void             material_repair_item(object_t *item, int skill_value);
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
extern void             dump_mempool_statistics(object_t *op, int *sum_used, int *sum_alloc);
/* missile.c */
extern float			do_throw(object_t *op, int dir);
extern float			fire_bow(object_t *op, int dir);
extern object_t          *find_arrow(object_t *op, const char *type);
extern object_t          *create_missile(object_t * const owner, const object_t * const bow, object_t *const missile, const int dir);
extern void             move_missile(object_t *op);
extern void             stop_missile(object_t *op);
/* object.c */
extern void             mark_object_removed(object_t *ob);
extern object_t          *merge_ob(object_t *op, object_t *tmp);
extern sint32           sum_weight(object_t *op);
extern object_t          *is_player_inv(object_t *op);
extern void             dump_object2(const object_t *op);
extern void             dump_object(const object_t *op);
extern void             dump_me(object_t *op, char *outstr, size_t bufsize);
extern void             dump_all_objects(void);
extern void             free_all_object_data(void);
extern object_t          *get_owner(object_t *op);
extern void             clear_owner(object_t *op);
extern void             set_owner(object_t * const op, object_t * const owner);
extern void             copy_owner(object_t *op, object_t *clone);
extern void             initialize_object(object_t *op);
extern void             copy_object(object_t *src, object_t *dest);
extern void             copy_object_data(object_t *op2, object_t *op);
extern object_t          *get_object(void);
extern void             update_turn_face(object_t *op);
extern void             activelist_insert(object_t *op);
extern void             activelist_remove(object_t *op);
extern void             update_ob_speed(object_t *op);
extern void             update_object(object_t *op, int action);
extern void             destroy_object(object_t *ob);
extern void             free_object_data(object_t *ob, int free_static_data);
extern int              count_free(void);
extern int              count_used(void);
extern void             remove_ob(object_t *op);
extern void             remove_ob_inv(object_t *op);
extern object_t        *kill_object(object_t *victim, object_t *killer, const char *headline, const char *detail);
extern object_t          *insert_ob_in_map(object_t *const op, map_t *m, object_t *const originator, const int flag);
extern void             replace_insert_ob_in_map(char *arch_string, object_t *op);
extern object_t          *get_split_ob(object_t *orig_ob, uint32 nr);
extern object_t          *decrease_ob_nr(object_t *op, uint32 i);
extern object_t          *insert_ob_in_ob(object_t *op, object_t *where);
extern int              check_walk_on(object_t *const op, object_t *const originator, int flags);
extern int              check_walk_off(object_t *op, object_t *originator, int flags);
extern object_t          *present_arch(archetype_t *at, map_t *m, sint16 x, sint16 y);
extern object_t          *present(unsigned char type, map_t *m, sint16 x, sint16 y);
extern object_t          *present_in_ob(unsigned char type, object_t *op);
extern object_t          *present_arch_in_ob(archetype_t *at, object_t *op);
extern object_t          *present_arch_in_ob_temp(archetype_t *at, object_t *op);
extern int              find_dir_2(int x, int y);
extern int              absdir(int d);
extern int              dirdiff(int dir1, int dir2);
extern object_t          *clone_object(object_t *original, uint8 mode);
extern object_t          *load_object_str(char *obstr);
extern void             object_gc();
extern int              auto_apply(object_t *op);
extern object_t          *locate_beacon(shstr_t *id);
extern void             init_object_initializers();
extern object_t          *find_next_object(object_t *op, uint8 type, uint8 mode, object_t *root);
extern void             revert_buff_stats(object_t *item);
extern void             fix_buff_stats(object_t *item);
extern object_t          *check_buff_exists(object_t *item, const char *name);
extern int              add_item_buff(object_t *item, object_t *buff, short just_checking);
extern int              remove_item_buff(object_t *item, char *name, uint32 nrof);
extern char            *query_name(object_t *what, object_t *who, uint32 nrof, uint8 mode);
extern map_t       *parent_map(object_t *what);
/* re-cmp.c */
extern char            *re_cmp(char *str, char *regexp);
/* readable.c */
extern int              nstrtok(const char *buf1, const char *buf2);
extern char            *strtoktolin(const char *buf1, const char *buf2);
extern int              book_overflow(const char *buf1, const char *buf2, int booksize);
extern void             init_readable(void);
extern void             change_book(object_t *book, int msgtype);
extern object_t          *get_random_mon(int level);
extern char            *mon_desc(object_t *mon);
extern object_t          *get_next_mon(object_t *tmp);
extern char            *mon_info_msg(int level, int booksize);
extern char            *artifact_msg(int level, int booksize);
extern char            *spellpath_msg(int level, int booksize);
extern void             make_formula_book(object_t *book, int level);
extern char            *msgfile_msg(int level, int booksize);
extern char            *god_info_msg(int level, int booksize);
extern void             tailor_readable_ob(object_t *book, int msg_type);
extern void             free_all_readable(void);
extern void             write_book_archive(void);
extern const char       *get_language(uint32 lang);
/* recipe.c */
extern recipelist      *get_formulalist(int i);
extern void             init_formulae(void);
extern void             check_formulae(void);
extern void             dump_alchemy(void);
extern archetype_t       *find_treasure_by_name(treasure *t, char *name, int depth);
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
extern objectlink_t      *link_treasurelists(char *liststring, uint32 flags);
extern void             unlink_treasurelists(objectlink_t *list, int flag);
extern object_t          *generate_treasure(struct objectlink_t *t, int difficulty);
extern void             create_treasure_list(struct objectlink_t *t, object_t *op, int flag, int difficulty, int a_chance, int tries);
extern int              create_treasure(treasurelist *t, object_t *op, int flag, int difficulty, int t_style,
                                        int a_chance, int magic, int magic_chance, int tries, struct _change_arch *change_arch);
extern int              create_all_treasures(treasure *t, object_t *op, int flag, int difficulty, int t_style,
                                             int a_chance,  int magic, int magic_chance, int tries, struct _change_arch *change_arch);
extern int              create_one_treasure(treasurelist *tl, object_t *op, int flag, int difficulty, int t_style,
                                            int a_chance,  int magic, int magic_chance, int tries, struct _change_arch *change_arch);
extern void             set_abs_magic(object_t *op, int magic);
extern int              set_ring_bonus(object_t *op, int bonus, int level);
extern int              get_magic(int diff);
extern int              fix_generated_item(object_t **op, object_t *creator, int difficulty, int a_chance, int t_style,
                                           int max_magic, int chance_magic, int flags);
extern void             dump_monster_treasure_rec(const char *name, treasure *t, int depth);
extern void             fix_flesh_item(object_t *item, object_t *donor);
extern void             free_treasurestruct(treasure *t);
extern void             free_all_treasures(void);
extern void             dump_monster_treasure(const char *name);
extern int              get_enviroment_level(object_t *op);
/* utils.c */
extern int              random_roll(int min, int max);
extern int              look_up_spell_name(const char *spname);
extern racelink        *find_racelink(const char *name);
extern char            *cleanup_string(char *ustring);
extern char            *get_token(char *string, char *token, uint8 qflag);
extern int              buf_overflow(const char *buf1, const char *buf2, int bufsize);
extern void             bitstostring(long bits, int num, char *str);
extern int              clipped_percent(int a, int b);
extern void             NDI_LOG(log_t logLevel, int flags, int pri, object_t *ob, char *format, ...) DAI_GNUC_PRINTF(5, 6);
/* view_map.c */
void            draw_client_map(player_t *pl);
void            draw_client_map2(player_t *pl);
void            set_personal_light(player_t *pl, int value);

#endif /* ifndef __LIBPROTO_H */
