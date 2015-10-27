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
float fire_magic_tool(object_t *op, object_t *weap, int dir)
{
    float ticks = 0.0f;

    switch(weap->type)
    {
    case WAND:
        /* a wand can be used up but still applied... */
        if (weap->stats.food <= 0)
        {
            play_sound_player_only(CONTR(op), SOUND_WAND_POOF, SOUND_NORMAL, 0, 0);
            ndi(NDI_UNIQUE, 0, op, "The wand says poof.");
        }
        else /* the wands fire the spell */
        {
            ndi(NDI_UNIQUE, 0, op, "fire wand");
            if (cast_spell(op, weap, dir, weap->stats.sp, 0, spellWand, NULL))
            {
                SET_FLAG(op, FLAG_BEEN_APPLIED); /* You now know something about it */
                if (!(--weap->stats.food))
                {
                    if (weap->arch)
                    {
                        CLEAR_FLAG(weap, FLAG_ANIMATE);
                        weap->face = weap->arch->clone.face;
                        weap->speed = 0.0f;
                        update_ob_speed(weap);
                    }

#ifndef USE_OLD_UPDATE
                    OBJECT_UPDATE_UPD(weap, UPD_ANIM);
#else
                    esrv_update_item(UPD_ANIM, weap);
#endif
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
                ndi(NDI_UNIQUE, 0, op, "The rod whines for a while, but nothing happens.");
            else
                ndi(NDI_UNIQUE, 0, op, "No matter how hard you try you can't get another note out.");
        }
        else
        {
            /*ndi(NDI_ALL|NDI_UNIQUE,5,NULL,"Use %s - cast spell %d\n",weap->name,weap->stats.sp);*/
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

/* sets up to cast a spell.  op is the caster, params is the spell name,
 * This function use the name of a spell following the /cast command
 * to invoke a spell (cast_spell() does the rest).
 * the next function fire_cast_spell prepares for fire command the same
 * but without invoking the spell.
 */

int command_cast_spell(object_t *op, char *params)
{
    player_t *pl;
    int     spnum;
    int     value;

    if (!op ||
        !(pl = CONTR(op)))
    {
        return 0;
    }

    if (!params)
        return 1;

    /* When we control a golem we can't cast again - if we do, it breaks control */
    if (pl->golem)
    {
        send_golem_control(pl->golem, GOLEM_CTR_RELEASE);
        (void)kill_object(pl->golem, NULL, "was released through lack of concentration", NULL);
        pl->golem = NULL;
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
    /* we don't know this spell name */
    if ((spnum = look_up_spell_by_name(op, params)) == -1)
    {
        ndi(NDI_UNIQUE, 0, op, "You don't know the spell %s.", params);
        return 0;
    }

    if (!change_skill(op, (spells[spnum].type == SPELL_TYPE_PRIEST ? SK_DIVINE_PRAYERS : SK_WIZARDRY_SPELLS)) &&
        !(pl->gmaster_mode & GMASTER_MODE_SA))
    {
        return 0;
    }

    /* we still recover from a casted spell before */
    if (!check_skill_action_time(op, op->chosen_skill))
        return 0;

    value = cast_spell(op, op, op->facing, spnum, 0, spellNormal, NULL);

    if (value)
    {
        float   ticks = (float)(spells[spnum].time) * RANGED_DELAY_TIME;
        sint16 *stat = ((spells[spnum].flags & SPELL_DESC_WIS)) ? &op->stats.grace : &op->stats.sp;

        LOG(llevDebug, "AC-spells(%d): %2.2f\n", spnum, ticks);
        set_action_time(op, ticks);
        *stat = MAX(0, *stat - value);
    }

    return 0;
}

