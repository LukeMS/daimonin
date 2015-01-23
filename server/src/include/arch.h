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

#ifndef __ARCH_H
#define __ARCH_H

/*
 * The archetype structure is a set of rules on how to generate and manipulate
 * objects which point to archetypes.
 * This probably belongs in arch.h, but there really doesn't appear to
 * be much left in the archetype - all it really is is a holder for the
 * object and pointers.  This structure should get removed, and just replaced
 * by the object structure
 */
struct archetype_t
{
    const char                 *name;           /* More definite name, like "generate_kobold" */
    archetype_t               *next;           /* Next archetype in a linked list */
    archetype_t               *head;           /* The main part of a linked object_t */
    archetype_t               *more;           /* Next part of a linked object_t */
    object_t                   *base_clone;         /* used by artifacts list: if != NULL,
                                                     * this object is the base object and clone is
                                                     * the modified artifacts object.
                                                     * we use base_clone for unidentified objects
                                                     * (to get unified "non identified" values),
                                                     * or it is used to get a base object when we
                                                     * remove the artifacts changes (cancellation, dispel...)
                                                     */
    object_t                    clone;          /* An object from which to do copy_object() */
    struct mob_behaviourset    *ai; /* arch-default ai definition (optional)*/
};

EXTERN archetype_t           *first_archetype;

typedef struct _archetype_global
{
    archetype_t *_empty_archetype;
    archetype_t *_base_info;
    archetype_t *_waypoint;
    archetype_t *_level_up;
    archetype_t *_aggro_history;
    archetype_t *_dmg_info;
    archetype_t *_drain;
    archetype_t *_depletion;
    archetype_t *_ring_normal;
    archetype_t *_ring_generic;
    archetype_t *_amulet_generic;
    archetype_t *_mitcoin;
    archetype_t *_goldcoin;
    archetype_t *_silvercoin;
    archetype_t *_coppercoin;
    archetype_t *_quest_container;
    archetype_t *_quest_info;
    archetype_t *_quest_trigger;
    archetype_t *_quest_update;
    archetype_t *_player_info;
    archetype_t *_force;
    archetype_t *_guild_force;
    archetype_t *_alignment_force;
    archetype_t *_rank_force;
    archetype_t *_gravestone;
    archetype_t *_deathsick;
    archetype_t *_poisoning;
    archetype_t *_slowness;
    archetype_t *_fear;
    archetype_t *_snare;
    archetype_t *_confusion;
    archetype_t *_blindness;
    archetype_t *_paralyze;
    archetype_t *_corpse_default;
    archetype_t *_loot_container;
    archetype_t *_pvp_stat_force;
}
_archetype_global;

extern _archetype_global archetype_global;

extern archetype_t *find_archetype_by_object_name(const char *name);
extern object_t    *get_archetype_by_object_name(const char *name);
extern void         init_archetypes(void);
extern void         arch_info(object_t *op);
extern void         clear_archetable(void);
extern void         init_archetable(void);
extern void         dump_arch(archetype_t *at);
extern void         dump_all_archetypes(void);
extern void         free_all_archs(void);
extern archetype_t *get_archetype_struct(void);
extern void         first_arch_pass(FILE *fp);
extern void         second_arch_pass(FILE *fp_start);
extern void         load_archetypes(void);
extern object_t    *arch_to_object(archetype_t *at);
extern object_t    *create_singularity(const char *name);
extern object_t    *get_archetype(const char *name);
extern archetype_t *find_archetype(const char *name);
extern void         add_arch(archetype_t *at);
extern archetype_t *type_to_archetype(int type);
extern object_t    *clone_arch(int type);

#endif /* ifndef __ARCH_H */
