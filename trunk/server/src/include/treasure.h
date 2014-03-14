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
/*
 * defines and variables used by the artifact generation routines
 */

#ifndef __TREASURE_H
#define __TREASURE_H

#define CHANCE_FOR_ARTIFACT 20
#define T_MAGIC_UNSET (-999)
#define T_MAGIC_CHANCE_UNSET (-999)

#define NUM_COINS 4 /* number of coin types */

/*
 * Flags to generate_treasures():
 */

/* i really hate all this value not documented. I wasted some times debugging by
 * see that i have included/copy a wrong flag of this kind somehwere.
 */
enum
{
    GT_ENVIRONMENT                                                                          = 0x0001,
    GT_INVISIBLE                                                                            = 0x0002,
    GT_NO_DROP                                                                           = 0x0004,
    GT_APPLY                                                                                = 0x0008,
    /* treasure gets applied when inserted in mob! (food eaten, skill applied...) */
    GT_ONLY_GOOD                                                                            = 0x0010,
    GT_UPDATE_INV                                                                           = 0x0020,
    GT_NO_VALUE                                                                             = 0x0040,      /* set value of all created treasures to 0 */
    GT_IDENTIFIED                                                                           = 0x0080       /* treasure is identified */
};


/* when a treasure got cloned from archlist, we want perhaps change some default
 * values. All values in this structure will override the default arch.
 * TODO: It is a bad way to implement this with a special structure.
 * Because the real arch list is a at runtime not changed, we can grap for example
 * here a clone of the arch, store it in the treasure list and then run the original
 * arch parser over this clone, using the treasure list as script until an END comes.
 * This will allow ANY changes which is possible and we use ony one parser.
 */

typedef struct _change_arch
{
    const char *name;              /* is != NULL, copy this over the original arch name */
    const char *race;
    const char *title;
    const char *slaying;
    int         face_id;
    New_Face   *face;
    int         anim_id;           /* An index into the animation array */
    int         animate;           /* if == -1: not set. 0/1 turn IS_ANIMATED */
    int         item_race;
    int         material;                   /* the real, fixed material value */
    int         material_quality;           /* find a material matching this quality */
    int         material_range;             /* using material_quality, find quality inside this range */
    int         quality;                    /* quality value. It overwrites the material default value */
    int         quality_range;              /* used for random range */
} _change_arch;


/* used for personlized treasure lists ('&' tail command in randomitems) */
typedef struct _tlist_tweak
{
    char const *name;
    int magic;        /* related to the same commands as in treasure file */
    int magic_chance;
    int drop_chance; /* chance 1/x */
    int drop100;     /* chance in % */
    int style;
    int difficulty;
    int artifact_chance;
    int identified; /* mark the tlist items identified when generated */
    int break_list; /* if TRUE, stop treasure generation IF this list has generated something */
    _change_arch c_arch;
} tlist_tweak;

/*
 * treasure is one element in a linked list, which together consist of a
 * complete treasure-list.  Any arch can point to a treasure-list
 * to get generated standard treasure when an archetype of that type
 * is generated (from a generator)
*/

typedef struct treasurestruct
{
    struct archt               *item;           /* Which item this link can be */
    const char                 *name;               /* If non null, name of list to use instead */
    struct treasureliststruct  *tlist; /* this list */
    struct treasurestruct      *next;       /* Next treasure-item in a linked list */
    struct treasurestruct      *next_yes;  /* If this item was generated, use */
    /* this link instead of ->next */
    struct treasurestruct      *next_no;   /* If this item was not generated, */
    /* then continue here */
    int                         t_style;        /* local t_style (will overrule global one) - used from artifacts */
    int                         magic_chance;   /* value from 0-1000. chance of item is magic. */
    int                         magic_fix;  /* if this value is != 0, use this as fixed magic value.
                                             * if it 0, look at magic to generate perhaps a random magic value
                                             */
    int                         artifact_chance;    /* default = -1 = ignore this value. 0=NEVER make a artifact for this treasure.
                                                     * 1-100 = % chance of make a artifact from this treasure.
                                                     */

    int                         magic;      /* Max magic bonus to item */
    int                         difficulty; /* If the entry is a list transition,
                                              * it contains the difficulty
                                              * required to go to the new list
                                              */
    uint16                      nrof;               /* random 1 to nrof items are generated */
    sint16                      chance_fix;     /* will overrule chance: if set (!=-1) it will create 1/chance_single */
    uint8                       chance;             /* Percent chance for this item */
    struct _change_arch         change_arch;  /* override default arch values if set in treasure list */
} treasure;


typedef struct treasureliststruct
{
    const char                 *listname;               /* Usually monster-name/combination */
    int                         t_style;                /* global style (used from artifacts file) */
    int                         artifact_chance;
    sint16                      chance_fix; /* if set it will overrule total_chance: */
    sint16                      total_chance;           /* If non-zero, only 1 item on this
                                                        * list should be generated.  The
                                                        * total_chance contains the sum of
                                                        * the chance for this list.
                                                        */
    struct treasureliststruct  *next;   /* Next treasure-item in linked list */
    struct treasurestruct      *items;      /* Items in this list, linked */
} treasurelist;

#endif /* ifndef __TREASURE_H */
