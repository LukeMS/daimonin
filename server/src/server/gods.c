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

#include "global.h"

static godlink_t * init_godslist()
{
    godlink_t    *gl  = (godlink_t *) malloc(sizeof(godlink_t));
    if (gl == NULL)
        LOG(llevError, "ERROR: init_godslist(): OOM.\n");
    gl->name = NULL;        /* how to describe the god to the player */
    gl->arch = NULL;    /* pointer to the archetype of this god */
    gl->id = 0;             /* id of the god */
    gl->pantheon = NULL;    /* the group to which the god belongs (not implemented) */
    gl->next = NULL;        /* next god in this linked list */

    return gl;
}

/* init_gods() - this takes a look at all of the archetypes to find
 * the objects which correspond to the GODS (type GOD) */

void init_gods(void)
{
    archetype_t  *at  = NULL;

    LOG(llevDebug, "Initializing gods...");
    for (at = first_archetype; at != NULL; at = at->next)
        if (at->clone.type == GOD)
            add_god_to_list(at);

    LOG(llevDebug, "done.\n");
}

/* add_god_to_list()- called only from init_gods */
void add_god_to_list(archetype_t *god_arch)
{
    godlink_t    *god;

    if (!god_arch)
    {
        LOG(llevBug, "BUG: Tried to add null god to list!\n");
        return;
    }

    god = init_godslist();

    god->arch = god_arch;
    SHSTR_FREE_AND_ADD_STRING(god->name, god_arch->clone.name);
    if (!first_god)
        god->id = 1;
    else
    {
        god->id = first_god->id + 1;
        god->next = first_god;
    }
    first_god = god;

#ifdef DEBUG_GODS
    LOG(llevDebug, "Adding god %s (%d) to list\n", god->name, god->id);
#endif
}

/* baptize_altar() - (cosmetically) change the name to that of the
 * god in question, then set the title for later use. -b.t.
 */

int baptize_altar(object_t *op)
{
    char    buf[MEDIUM_BUF];

    /* if the title field is pre-set, then that altar is
     * already dedicated. */
    if (!op->title)
    {
        godlink_t    *god = get_rand_god();
        if (!god || !god->name)
        {
            LOG(llevBug, "BUG: baptise_altar(): bizarre nameless god!\n");
            return 0;
        }
        /* if the object name hasnt' been changed, we tack on the gods name */
        if (op->name == op->arch->clone.name)
        {
            sprintf(buf, "%s of %s", op->name, god->name);
            SHSTR_FREE_AND_ADD_STRING(op->name, buf);
        }
        SHSTR_FREE_AND_ADD_STRING(op->title, god->name);
        return 1;
    }
    return 0;
}

godlink_t * get_rand_god(void)
{
    godlink_t    *god = first_god;
    int         i;

    if (god)
        for (i = RANDOM() % (god->id) + 1; god; god = god->next)
            if (god->id == i)
                break;

    if (!god)
        LOG(llevBug, "BUG: get_rand_god(): can't find a random god!\n");
    return god;
}

/* pntr_to_god_obj() - returns a pointer to the object
 * We need to be VERY carefull about using this, as we
 * are returning a pointer to the CLONE object. -b.t.
 */
object_t * pntr_to_god_obj(godlink_t *godlnk)
{
    object_t *god = NULL;

    if (godlnk && godlnk->arch)
        god = &godlnk->arch->clone;
    return god;
}

void free_all_god()
{
    godlink_t    *god, *godnext;

    LOG(llevDebug, "Freeing god information\n");
    for (god = first_god; god; god = godnext)
    {
        godnext = god->next;
        SHSTR_FREE(god->name);
        free(god);
    }
}


void dump_gods()
{
#ifdef DUMP_SWITCHES
    godlink_t    *glist;

    LOG(llevInfo, "\n");
    for (glist = first_god; glist; glist = glist->next)
    {
        object_t *god = pntr_to_god_obj(glist);
        char    tmpbuf[HUGE_BUF];
        int     tmpvar, gifts = 0;

        LOG(llevInfo, "GOD: %s\n", god->name);
        LOG(llevInfo, " avatar stats:\n");
        LOG(llevInfo, "  S:%d C:%d D:%d I:%d W:%d P:%d\n", god->stats.Str, god->stats.Con, god->stats.Dex,
            god->stats.Int, god->stats.Wis, god->stats.Pow);
        LOG(llevInfo, "  lvl:%d speed:%4.2f\n", god->level, god->speed);
        LOG(llevInfo, "  wc:%d ac:%d hp:%d dam:%d \n", god->stats.wc, god->stats.ac, god->stats.hp, god->stats.dam);
        LOG(llevInfo, " enemy: %s\n", STRING_OBJ_TITLE(god));
        if (god->other_arch)
        {
            object_t *serv    = &god->other_arch->clone;
            LOG(llevInfo, " servant stats: (%s)\n", STRING_ARCH_NAME(god->other_arch));
            LOG(llevInfo, "  S:%d C:%d D:%d I:%d W:%d P:%d\n", serv->stats.Str, serv->stats.Con, serv->stats.Dex,
                serv->stats.Int, serv->stats.Wis, serv->stats.Pow);
            LOG(llevInfo, "  lvl:%d speed:%4.2f\n", serv->level, serv->speed);
            LOG(llevInfo, "  wc:%d ac:%d hp:%d dam:%d \n", serv->stats.wc, serv->stats.ac, serv->stats.hp,
                serv->stats.dam);
        }
        else
            LOG(llevInfo, " servant: NONE\n");
        LOG(llevInfo, " aligned_race(s): %s\n", STRING_OBJ_RACE(god));
        LOG(llevInfo, " enemy_race(s): %s\n", STRING_OBJ_SLAYING(god));
        LOG(llevInfo, "%s", describe_resistance(god, 1));

        strcat(tmpbuf, "\n aura:");

        strcat(tmpbuf, "\n paths:");
        if ((tmpvar = (int) god->path_attuned))
        {
            strcat(tmpbuf, "\n  ");
            DESCRIBE_PATH(tmpbuf, tmpvar, "Attuned");
        }
        if ((tmpvar = (int) god->path_repelled))
        {
            strcat(tmpbuf, "\n  ");
            DESCRIBE_PATH(tmpbuf, tmpvar, "Repelled");
        }
        if ((tmpvar = (int) god->path_denied))
        {
            strcat(tmpbuf, "\n  ");
            DESCRIBE_PATH(tmpbuf, tmpvar, "Denied");
        }
        LOG(llevInfo, "%s\n", tmpbuf);
        LOG(llevInfo, " Desc: %s", STRING_OBJ_MSG(god));
        LOG(llevInfo, " Priest gifts/limitations: ");
        if (!QUERY_FLAG(god, FLAG_USE_WEAPON))
        {
            gifts = 1; LOG(llevInfo, "\n  weapon use is forbidden");
        }
        if (!QUERY_FLAG(god, FLAG_USE_ARMOUR))
        {
            gifts = 1; LOG(llevInfo, "\n  no armour may be worn");
        }
        if (QUERY_FLAG(god, FLAG_UNDEAD))
        {
            gifts = 1; LOG(llevInfo, "\n  is undead");
        }
        if (QUERY_FLAG(god, FLAG_SEE_IN_DARK))
        {
            gifts = 1; LOG(llevInfo, "\n  has infravision ");
        }
        if (QUERY_FLAG(god, FLAG_XRAYS))
        {
            gifts = 1; LOG(llevInfo, "\n  has X-ray vision");
        }
        if (QUERY_FLAG(god, FLAG_REFL_MISSILE))
        {
            gifts = 1; LOG(llevInfo, "\n  reflect missiles");
        }
        if (QUERY_FLAG(god, FLAG_REFL_CASTABLE))
        {
            gifts = 1; LOG(llevInfo, "\n  reflect spells");
        }
        if (QUERY_FLAG(god, FLAG_STEALTH))
        {
            gifts = 1; LOG(llevInfo, "\n  is stealthy");
        }
        if (QUERY_FLAG(god, FLAG_SEE_INVISIBLE))
        {
            gifts = 1; LOG(llevInfo, "\n  is (permanently) invisible");
        }
        if (QUERY_FLAG(god, FLAG_BLIND))
        {
            gifts = 1; LOG(llevInfo, "\n  is blind");
        }
        if (god->last_heal)
        {
            gifts = 1; LOG(llevInfo, "\n  hp regenerate at %d", god->last_heal);
        }
        if (god->last_sp)
        {
            gifts = 1; LOG(llevInfo, "\n  sp regenerate at %d", god->last_sp);
        }
        if (god->last_grace)
        {
            gifts = 1; LOG(llevInfo, "\n  grace regenerates at %d", god->last_grace);
        }
        if (!gifts)
            LOG(llevInfo, "NONE");
        LOG(llevInfo, "\n\n");
    }
#endif
}

// TODO: look over usage of those two functions and see if we can get
// away without the strncmp/strcmp calls
int lookup_god_by_name(const char *name)
{
    int godnr = -1, nmlen = strlen(name);

    if (name && name != shstr_cons.none)
    {
        godlink_t    *gl;
        for (gl = first_god; gl; gl = gl->next)
            if (!strncmp(name, gl->name, MIN((int) strlen(gl->name), nmlen)))
                break;
        if (gl)
            godnr = gl->id;
    }
    return godnr;
}

/* Determines if op worships a god. Returns the godname if they do. In the case
 * of an NPC, if they have no god, we give them a random one. */
const char *determine_god(object_t *op)
{
    /* Spells. */
    if (IS_GOD_SPELL(op) &&
        op->title)
    {
        if (lookup_god_by_name(op->title) >= 0)
        {
            return op->title;
        }
    }

    /* If we are player, lets search a bit harder for the god.  This
     * is a fix for perceive self (before, we just looked at the active
     * skill). */
    if (op->type == PLAYER)
    {
        object_t *this,
               *next;

        FOREACH_OBJECT_IN_OBJECT(this, op, next)
        {
            /* Gecko: we should actually only need to check either
             * this->stats.Wis or this->sub_type1, but to avoid future
             * mistakes we check both here. */
            if (this->type == TYPE_SKILLGROUP &&
                this->stats.Wis &&
                this->sub_type1 == 5) // TODO: meaningful constant?
            {
                if (this->title)
                {
                    return this->title;
                }
                else
                {
                    return shstr_cons.none;
                }
            }
        }
    }
    else if (QUERY_FLAG(op, FLAG_ALIVE))
    {
        if (!op->title)
        {
            godlink_t *gl;
            int      godnr;

            for (gl = first_god; gl; gl = gl->next)
            {
                if (gl == first_god)
                {
                    godnr = random_roll(1, gl->id);
                }
                else if (gl->id == godnr)
                {
                    SHSTR_FREE_AND_ADD_STRING(op->title, gl->name);

                    break;
                }
            }
        }

        return op->title;
    }

    return shstr_cons.none;
}

object_t * find_god(const char *name)
{
    object_t *god = NULL;

    if (name && name != shstr_cons.none)
    {
        godlink_t    *gl;
        for (gl = first_god; gl; gl = gl->next)
            if (!strcmp(name, gl->name))
                break;
        if (gl)
            god = pntr_to_god_obj(gl);
    }
    return god;
}

void pray_at_altar(object_t *pl, object_t *altar)
{
    object_t *pl_god  = find_god(determine_god(pl));

    /* If non consecrate altar, don't do anything */
    if (!altar->other_arch)
        return;

    /* hmm. what happend depends on pl's current god, level, etc */
    if (!pl_god)
    {
        /*new convert */
        become_follower(pl, &altar->other_arch->clone);
        return;
    }
    else if (pl_god->name == altar->other_arch->clone.name)
    {
        /* pray at your gods altar */
        int bonus   = ((pl->stats.Wis / 10) + (SK_level(pl) / 10));

        ndi(NDI_UNIQUE, 0, pl, "You feel the powers of your deity %s.", pl_god->name);

        /* we can get neg grace up faster */
        if (pl->stats.grace < 0)
            pl->stats.grace += (bonus > -1 * (pl->stats.grace / 10) ? bonus : -1 * (pl->stats.grace / 10));
        /* we can super-charge grace to 2x max */
        if (pl->stats.grace < (2 * pl->stats.maxgrace))
        {
            pl->stats.grace += bonus / 2;
        }
        /* I think there was a bug here in that this was nested
         * in the if immediately above
         */
        if (pl->stats.grace > pl->stats.maxgrace)
        {
            pl->stats.grace = pl->stats.maxgrace;
        }
//
//        /* Every once in a while, the god decides to checkup on their
//         * follower, and may intervene to help them out.
//         */
//        bonus = MAX(1, bonus); /* -- DAMN -- */
//
//        if ((random_roll(0, 399) - bonus) < 0)
//            god_intervention(pl, pl_god);
    }
    else
    {
        /* praying to another god! */
        int loss = 0, angry = 1;

        /* I believe the logic for detecting opposing gods was completely
         * broken - I think it should work now.  altar->other_arch
         * points to the god of this altar (which we have
         * already verified is non null).  pl_god->other_arch
         * is the opposing god - we need to verify that exists before
         * using its values.
         */
        if (pl_god->other_arch && (altar->other_arch->name == pl_god->other_arch->name))
        {
            angry = 2;
            if (random_roll(0, SK_level(pl) + 2) - 5 > 0)
            {
                /* you really screwed up */
                angry = 3;
                ndi(NDI_UNIQUE | NDI_NAVY, 0, pl, "Foul Priest! %s punishes you!", pl_god->name);
                cast_mana_storm(pl, pl_god->level + 20);
            }
            ndi(NDI_UNIQUE | NDI_NAVY, 0, pl, "Foolish heretic! %s is livid!", pl_god->name);
        }
        else
            ndi(NDI_UNIQUE | NDI_NAVY, 0, pl, "Heretic! %s is angered!", pl_god->name);

        /* May switch Gods, but its random chance based on our current level
         * note it gets harder to swap gods the higher we get */
        if ((angry == 1) && !(random_roll(0, pl->chosen_skill->skillgroup->level)))
        {
            become_follower(pl, &altar->other_arch->clone);
        } /* If angry... switching gods */
        else
        {
            /* toss this player off the altar.  He can try again. */
            ndi(NDI_UNIQUE | NDI_NAVY, 0, pl, "A divine force pushes you off the altar.");
            move_player(pl, absdir(pl->facing + 4), 1); /* back him off the way he came. */
        } /* didn't successfully change, so forced off altar. */
    } /* If prayed at altar to other god */
}

/*
 * become_follower - This function is called whenever a player has
 * switched to a new god. It handles basically all the stat changes
 * that happen to the player, including the removal of godgiven
 * items (from the former cult).
 */
void become_follower(object_t *op, object_t *new_god)
{
    object_t     *skillgroup = op->chosen_skill->skillgroup; /* obj. containing god data */
    object_t     *old_god = NULL;                      /* old god */
    int         i;

    if (!op || !new_god)
        return;

    CONTR(op)->socket.ext_title_flag = 1;
    /* get old god */
    if (skillgroup->title)
        old_god = find_god(skillgroup->title);

    if (op->race && new_god->slaying && strstr(op->race, new_god->slaying))
    {
        ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "Fool! %s detests your kind!", new_god->name);
        if (random_roll(0, op->level - 1) - 5 > 0)
            cast_mana_storm(op, new_god->level + 10);
        return;
    }

    ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "You become a follower of %s!", new_god->name);

    if (skillgroup->title)
    {
        /* get rid of old god */
        ndi(NDI_UNIQUE, 0, op, "%s's blessing is withdrawn from you.", skillgroup->title);
        CLEAR_FLAG(skillgroup, FLAG_APPLIED);
        change_abil(op, skillgroup);
        SHSTR_FREE(skillgroup->title);
    }

    /* now change to the new gods attributes to skillgroup */
    SHSTR_FREE_AND_ADD_STRING(skillgroup->title, new_god->name);
    skillgroup->path_attuned = new_god->path_attuned;
    skillgroup->path_repelled = new_god->path_repelled;
    skillgroup->path_denied = new_god->path_denied;
    /* copy god's resistances */
    memcpy(skillgroup->resist, new_god->resist, sizeof(new_god->resist));

    /* make sure that certain immunities do NOT get passed
     * to the follower! */
    for (i = 0; i < NROFATTACKS; i++)
        if (skillgroup->resist[i] > 30 && (i == ATNR_FIRE || i == ATNR_COLD || i == ATNR_ELECTRICITY || i == ATNR_POISON))
            skillgroup->resist[i] = 30;

    ndi(NDI_UNIQUE, 0, op, "You are bathed in %s's aura.", new_god->name);
    SET_FLAG(skillgroup, FLAG_APPLIED);
    change_abil(op, skillgroup);
}
