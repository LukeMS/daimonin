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

/* This file deals with range related commands (casting, shooting,
 * throwing, etc.
 */

#include <global.h>

/* owner fires op (which is a rod, horn or wand)
*/
float fire_magic_tool(object *op, object *weap, int dir)
{
    float ticks = 0.0f;

    switch(weap->type)
    {
    case WAND:
        /* a wand can be used up but still applied... */
        if (weap->stats.food <= 0)
        {
            play_sound_player_only(CONTR(op), SOUND_WAND_POOF, SOUND_NORMAL, 0, 0);
            new_draw_info(NDI_UNIQUE, 0, op, "The wand says poof.");
        }
        else /* the wands fire the spell */
        {
            new_draw_info(NDI_UNIQUE, 0, op, "fire wand");
            if (cast_spell(op, weap, dir, weap->stats.sp, 0, spellWand, NULL))
            {
                SET_FLAG(op, FLAG_BEEN_APPLIED); /* You now know something about it */
                if (!(--weap->stats.food))
                {
                    if (weap->arch)
                    {
                        CLEAR_FLAG(weap, FLAG_ANIMATE);
                        weap->face = weap->arch->clone.face;
                        weap->speed = 0;
                        update_ob_speed(weap);
                    }
                    esrv_update_item(UPD_ANIM, weap);
                }
            }
        }
        break;

    case ROD:
    case HORN:
        if (weap->stats.hp < spells[weap->stats.sp].sp)
        {
            play_sound_player_only(CONTR(op), SOUND_WAND_POOF, SOUND_NORMAL, 0, 0);
            if (weap->type == ROD)
                new_draw_info(NDI_UNIQUE, 0, op, "The rod whines for a while, but nothing happens.");
            else
                new_draw_info(NDI_UNIQUE, 0, op, "No matter how hard you try you can't get another note out.");
        }
        else
        {
            /*new_draw_info(NDI_ALL|NDI_UNIQUE,5,NULL,"Use %s - cast spell %d\n",weap->name,weap->stats.sp);*/
            if (cast_spell(op, weap, dir, weap->stats.sp, 0, weap->type == ROD ? spellRod : spellHorn, NULL))
            {
                SET_FLAG(op, FLAG_BEEN_APPLIED); /* You now know something about it */
                drain_rod_charge(weap);
            }
        }
        break;
    }

    ticks = (float) (weap->last_grace) * RANGED_DELAY_TIME;

    return ticks;
}


/* object *op is the caster, params is the spell name.  We return the index
 * value of the spell in the spells array for a match, -1 if there is no
 * match, -2 if there are multiple matches.  Note that 0 is a valid entry, so
 * we can't use that as failure.
 *
 * Modified 03/24/98 - extra parameter 'options' specifies if the search is
 * done with the length of the input spell name, or the length of the stored
 * spell name.  This allows you to find out if the spell name entered had
 * extra optional parameters at the end (ie: marking rune <text>)
 *
 */
static int find_spell_byname(object *op, char *params, int options)
{
    int numknown; /* number of spells known by op */
    int spnum;  /* number of spell that is being cast */
    int match = -1, i;
    unsigned int paramlen = 0;

    numknown = (IS_GMASTER_WIZ(op)) ? NROFREALSPELLS : CONTR(op)->nrofknownspells;

    for (i = 0; i < numknown; i++)
    {
        spnum = (IS_GMASTER_WIZ(op)) ? i : CONTR(op)->known_spells[i];

        if (!options)
            paramlen = strlen(params);

        if (!strncmp(params, spells[spnum].name, options ? strlen(spells[spnum].name) : paramlen))
        {
            /* We already found a match previously - thus params is not
               * not unique, so return -2 stating this.
               */
            if (match >= 0)
                return -2;
            else
                match = spnum;
        }
    }
    return match;
}


/* Shows all spells that op knows.  If params is supplied, the must match
 * that.  If cleric is 1, show cleric spells, if not set, show mage
 * spells.
 */
/* disabled - we have now spell list in client
static void show_matching_spells(object *op, char *params, int cleric)
{
    int i,spnum,first_match=0;
    char lev[80], cost[80];

    for (i = 0; i < (IS_GMASTER_WIZ(op)) ? NROFREALSPELLS : CONTR(op)->nrofknownspells; i++) {
    spnum = (IS_GMASTER_WIZ(op)) ? i : CONTR(op)->known_spells[i];
    if (spells[spnum].type != (unsigned int) cleric) continue;
    if (params && strncmp(spells[spnum].name,params, strlen(params)))
        continue;
    if (!first_match) {
        first_match=1;
        if (!cleric)
        new_draw_info(NDI_UNIQUE, 0, op, "Mage spells");
        else
        new_draw_info(NDI_UNIQUE, 0, op, "Priest spells");
        new_draw_info(NDI_UNIQUE, 0,op,"[ sp] [lev] spell name");
    }
    if (spells[spnum].path & op->path_denied) {
        strcpy(lev,"den");
            strcpy(cost,"den");
    } else {
        sprintf(lev,"%3d",spells[spnum].level);
            sprintf(cost,"%3d",SP_level_spellpoint_cost(op,op,spnum));
        }

    new_draw_info(NDI_UNIQUE,0,op,"[%s] [%s] %s",
        cost, lev, spells[spnum].name);
    }
}

*/

/* sets up to cast a spell.  op is the caster, params is the spell name,
 * This function use the name of a spell following the /cast command
 * to invoke a spell (cast_spell() does the rest).
 * the next function fire_cast_spell prepares for fire command the same
 * but without invoking the spell.
 */

int command_cast_spell(object *op, char *params)
{
    char       *cp              = NULL;
    int         spnum = -1, spnum2 = -1;  /* number of spell that is being cast */
    int         value;
    float       ticks;

    if (!IS_GMASTER_WIZ(op) &&
        !CONTR(op)->nrofknownspells)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You don't know any spells.");

        return 0;
    }

    if (!params)
        return 1;

    /* When we control a golem we can't cast again - if we do, it breaks control */
    if (CONTR(op)->golem != NULL)
    {
        send_golem_control(CONTR(op)->golem, GOLEM_CTR_RELEASE);
        destruct_ob(CONTR(op)->golem);
        CONTR(op)->golem = NULL;
    }

    /* This assumes simply that if the name of
     * the spell being cast as input by the player is shorter than or
     * equal to the length of the spell name, then there is no options
     * but if it is longer, then everything after the spell name is
     * an option.  It determines if the spell name is shorter or
     * longer by first iterating through the actual spell names, checking
     * to the length of the typed in name.  If that fails, then it checks
     * to the length of each spell name.  If that passes, it assumes that
     * anything after the length of the actual spell name is extra options
     * typed in by the player (ie: marking rune Hello there)
     */
    if (((spnum2 = spnum = find_spell_byname(op, params, 0)) < 0) && ((spnum = find_spell_byname(op, params, 1)) >= 0))
    {
        params[strlen(spells[spnum].name)] = '\0';
        cp = &params[strlen(spells[spnum].name) + 1];
        if (strncmp(cp, "of ", 3) == 0)
            cp += 3;
    }

    /* we don't know this spell name */
    if (spnum == -1)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You don't know the spell %s.", params);

        return 0;
    }


    if (!change_skill(op, (spells[spnum].type == SPELL_TYPE_PRIEST ? SK_PRAYING : SK_SPELL_CASTING)))
    {
        if (!IS_GMASTER_WIZ(op))
        {
            return 0;
        }
    }

    /* we still recover from a casted spell before */
    if (!check_skill_action_time(op, op->chosen_skill))
        return 0;

    value = cast_spell(op, op, op->facing, spnum, 0, spellNormal, cp);

    if (value)
    {
        ticks = (float) (spells[spnum].time) * RANGED_DELAY_TIME;
        LOG(llevDebug, "AC-spells(%d): %2.2f\n", spnum, ticks);
        set_action_time(op, ticks);

        if (spells[spnum].flags & SPELL_DESC_WIS)
            op->stats.grace -= value;
        else
            op->stats.sp -= value;
    }

    return 0;
}

