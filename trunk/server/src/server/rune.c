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

#include <global.h>

#ifdef NO_ERRNO_H
extern int      errno;
#else
#   include <errno.h>
#endif

#ifdef sequent
/* stoopid sequent includes don't do this like they should */
extern char    *sys_errlist[];
extern int      sys_nerr;
#endif

/*  peterm:
    write_rune:
    op:  rune writer
    dir:  orientation of rune, direction rune's contained spell will
        be cast in, if applicable
    inspell:  the spell index of the spell in the rune (if any)
    level:  level of casting of the rune
    runename:  name of the rune or message displayed by the rune for
        a rune of marking

*/
/* FIXME: This is probably broken. */

int write_rune(object_t *op, int dir, int inspell, int level, char *runename)
{
    archetype_t *at = NULL;
    map_t *m;
    sint16     x,
               y;
    msp_t  *msp;
    object_t    *this,
              *next;
    char       buf[MEDIUM_BUF];

    if (!dir)
    {
        dir = 1;
    }

    m = op->map;
    x = op->x + OVERLAY_X(dir);
    y = op->y + OVERLAY_Y(dir);

    if (msp_blocked(op, m, x, y))
    {
        ndi(NDI_UNIQUE, 0, op, "Can't make a rune there!");
        return 0;
    }

    msp = MSP_GET2(m, x, y);

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        if (this->type == RUNE)
        {
            ndi(NDI_UNIQUE, 0, op, "You can't write a rune there.");
            return 0;
        }
    }

    if (inspell)
    {
        /* can't have runes of small fireball!!!  */
        if (inspell == -1)
        {
            ndi(NDI_UNIQUE, 0, op, "You can't make a rune containing a spell you don't know.");
            return 0;
        }
        at = find_archetype(runename);

        /* What it compares to should probably be expanded.  But basically,
         * creating a rune of sword should not be allowed. */
        if (at && at->clone.type != RUNE)
        {
            ndi(NDI_UNIQUE, 0, op, "You can't make a rune of %s", runename);
            return 0;
        }
        /* next it attempts to look up a rune_archetype for this spell
        by doing some string manipulations */
        if (!at)
        {
            char    insp[MEDIUM_BUF];
            int     i;
            strcpy(insp, spells[inspell].name);
            for (i = 0; i < (int) strlen(insp); i++)
                if (insp[i] == ' ')
                    insp[i] = '_';
            sprintf(buf, "%s%s", "rune_", insp);
            at = find_archetype(buf);
        }

        if (!at)
            this = get_archetype("generic_rune");
        else
            this = arch_to_object(at);

        this->stats.sp = inspell; /* the spell it contains */

        sprintf(buf, "You set off a rune of %s", spells[inspell].name);
        FREE_AND_COPY_HASH(this->msg, buf);
        at = NULL;


        /*  the at=find_archetye(runename) is neccessary because
         * this=get_archetype returns a singulirity, not a null,
         * when it cannot find the archetype.
         * note: if some smartass
         * cast rune of marking, and gives the exact name
         * of a powerful rune, it won't do him any good,
         *  because a rune of marking will have level 0
         * and will thus never detonate.
         */
    }
    else if (level == -2 || (at = find_archetype(runename)) == NULL)
    {
        char    rune[HUGE_BUF];

        level = 0;
        this = get_archetype("rune_mark"); /* this is a rune of marking */
        at = NULL;
        if (runename)
        {
            if (strstr(runename, "endmsg"))
            {
                LOG(llevInfo, "write_rune: player %s tried to write bogus rune\n",
                    STRING_OBJ_NAME(op));
                return 0;
            }
            strncpy(rune, runename, HUGE_BUF - 2);
            rune[HUGE_BUF - 2] = 0;
            strcat(rune, "\n");
        }
        else
        {
            /* Not totally efficient, but keeps code simpler */
            strcpy(rune, "There is no message\n");
        }
        FREE_AND_COPY_HASH(this->msg, rune);
    }
    if (at)
        this = get_archetype(runename);
    this->stats.Cha = op->level / 2;  /* the invisibility parameter */
    this->x = x;
    this->y = y;
    this->map = op->map;
    this->direction = dir;  /* where any spell will go upon detonation */
    this->level = SK_level(op);  /* what level to cast the spell at */
    if (inspell || this->stats.dam)
        set_owner(this, op); /* runes without need no owner */
    insert_ob_in_map(this, op->map, op, 0);
    return 1;
}


/*  peterm: rune_attack
 *
 * function handles those runes which detonate but do not cast spells.
 * Remember: at this point we KNOW the trap will hit the victim -
 * so no need for wce.
 * all we need is dam and a attack form
 */
void rune_attack(object_t *op, object_t *victim)
{
    int dam = op->stats.dam; /* save damage */

    /* lets first calc the damage - we use base dmg * level
     * For rune, the damage will *not* get additional
     * level range boni
     * we do here a more simple system like the normal monster damage.
     * with the hard set float we can control the damage a bit better.
     */
    op->stats.dam = (sint16) ((float) dam * (LEVEL_DAMAGE(op->level) * 0.925f));

    if (victim)
    {
        tag_t   tag = victim->count;
        if (!op->inv || op->inv->type != DISEASE)
        {
            damage_ob(victim, op->stats.dam, op, ENV_ATTACK_CHECK);
            if (!OBJECT_VALID(victim, tag))
            {
                op->stats.dam = dam;
                return;
            }
        }
        /*  if there's a disease in the needle, put it in the player */
        if (op->randomitems != NULL)
            create_treasure_list(op->randomitems, op, 0, op->level ? op->level : victim->map->difficulty, ART_CHANCE_UNSET, 0);
        if (op->inv && op->inv->type == DISEASE)
        {
            object_t *disease = op->inv;
            infect_object(victim, disease, 1);
            remove_ob(disease);
            check_walk_off(disease, NULL, MOVE_APPLY_VANISHED);
        }
    }
    else
    {
        tag_t tag = op->count;

        hit_map(op, MSP_KNOWN(op));

        if (!OBJECT_VALID(op, tag))
        {
            return;
        }
    }

    op->stats.dam = dam;
}

/*  This function generalizes attacks by runes/traps.  This ought to make
    it possible for runes to attack from the inventory,
    it'll spring the trap on the victim.  */

void spring_trap(object_t *trap, object_t *victim)
{
    int     spell_in_rune;
    object_t *env;
    tag_t   trap_tag    = trap->count;

    if (!trap->stats.hp ||      // prevents recursion
        (GET_GMASTER_MODE(victim) & GMASTER_MODE_SA))
    {
        return;
    }

    /* get the spell number from the name in the slaying field, and set
     * that as the spell to be cast.
    */
    if (trap->slaying && (spell_in_rune = look_up_spell_by_name(NULL, trap->slaying)) != -1)
        trap->stats.sp = spell_in_rune;

    /* Only living objects can trigger runes that don't cast spells, as
    * doing direct damage to a non-living object doesn't work anyway.
    * Typical example is an arrow attacking a door.
    */
    if (!IS_LIVE(victim) && !trap->stats.sp)
        return;

    /* Event trigger and quick exit */
    if(trigger_object_plugin_event(EVENT_TRIGGER,
                trap, victim, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
        return;

    if (victim && victim->type == PLAYER && trap->msg)
        ndi(NDI_UNIQUE, 0, victim, "%s", trap->msg);

    /* Flash an image of the trap on the map so the poor sod
    *   knows what hit him.
    */
    for (env = trap; env->env != NULL; env = env->env)
        ;

    if(trap->sub_type1 != 128)
        trap_show(trap, env);
#if 0
    trap->type = MISC_OBJECT;  /* make the trap impotent */
    CLEAR_FLAG(trap, FLAG_FLY_ON);
    CLEAR_FLAG(trap, FLAG_WALK_ON);
    FREE_AND_CLEAR_HASH2(trap->msg);
    trap->stats.food = 20;  /* make it stick around until its spells are gone */
    SET_FLAG(trap, FLAG_IS_USED_UP);    /* ok, let the trap wear off */
    trap->speed = trap->speed_left = 1.0f;
    update_ob_speed(trap);
#endif

    if (!trap->stats.sp)
    {
        rune_attack(trap, victim);
        set_traped_flag(env);
        if (!OBJECT_VALID(trap, trap_tag))
            return;
    }
    else
    {
        /* This is necessary if the trap is inside something else */
        remove_ob(trap);
        check_walk_off(trap, NULL, MOVE_APPLY_VANISHED);
        set_traped_flag(env);
        trap->x = victim->x;trap->y = victim->y;
        if (!insert_ob_in_map(trap, victim->map, trap, 0))
            return;
        cast_spell(trap, trap, trap->direction, trap->stats.sp - 1, 1, spellNormal, NULL);
    }

    if (trap->stats.hp > 0)
        trap->stats.hp--;

    if (trap->stats.hp == 0)
    {
        trap->type = MISC_OBJECT;  /* make the trap impotent */
        CLEAR_FLAG(trap, FLAG_FLY_ON);
        CLEAR_FLAG(trap, FLAG_WALK_ON);
        FREE_AND_CLEAR_HASH2(trap->msg);
        trap->stats.food = 20;  /* make it stick around until its spells are gone */
        SET_FLAG(trap, FLAG_IS_USED_UP);
        trap->speed = trap->speed_left = 1.0f;
        update_ob_speed(trap);
    }
}

/*  dispel_rune:  by peterm
    dispels the target rune, depending on the level of the actor
and the level of the rune  risk flag, if true, means that there is
a chance that the trap/rune will detonate */

int dispel_rune(object_t *op, int dir, int risk)
{
    map_t *m = op->map;
    sint16     x = op->x + OVERLAY_X(dir),
               y = op->y + OVERLAY_Y(dir);
    msp_t  *msp = MSP_GET2(m, x, y);
    object_t    *this,
              *next;

    if (!msp)
    {
        return 0;
    }

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        if (this->type != RUNE)
        {
            object_t *that,
                   *next2;

            FOREACH_OBJECT_IN_OBJECT(that, this, next2)
            {
                if (that->type == RUNE)
                {
                    this = that;
                    break;
                }
            }
        }

        if (this->type == RUNE)
        {
            break;
        }
    }

    if (this)
    {
        trap_disarm(op, this, risk);
        return 1;
    }

    ndi(NDI_UNIQUE, 0, op, "There's no trap there!");
    return 0;
}

int trap_see(object_t *op, object_t *trap, int level)
{
    int     chance;

    chance = random_roll(0, 99);

    /*  decide if we see the rune or not */
    if ((trap->level <= level && RANDOM() % 10)
     || trap->stats.Cha
     == 1
     || (chance
       > MIN(95,
             MAX(5,
                 ((int) ((float) (op->map->difficulty + trap->level + trap->stats.Cha - op->level) / 10.0 * 50.0))))))
    {
        // ndi(NDI_UNIQUE, 0, op, "You spot a %s (lvl %d)!", trap->name, trap->level);
        return 1;
    }
    return 0;
}

/* I changed the use of this function... Now, this function deos what the
 * name says: He get a trap and makes them visible and reinsert it.
 * This can be used from trigger but
 * also from detection sources - just be sure you set FLAG_IS_USED_UP in
 * your trigger functions as speed when you want start the auto destroy. MT-2003
 */
int trap_show(object_t *trap, object_t *where)
{
    object_t *env;

    if (where == NULL)
        return 0;

    /* Because we don't want unhide traps by using "see invisible" (because
     * traps are not invisibile - they are hidden) and the normal "see hidden"
     * we put them in as sys_object ... truely invisible for the client.
     * There are only 2 ways to make then visible - fire them by moving/apply them
     * or use a "see/unhide traps" skill/spell.
     * For that, we set sys_object 0 and layer 4. Don't forget to set the player
     * because level 0 is really a special thing.
     */
    env = trap->env;
    /* FIXME: Not entirely sure we really need do a full remove/insert any
     * more, but certainly careful msp_set_slices()/update_ob() OR
     * esrv_send_item(), depending on if trap->map or trap->env, is needed.
     *
     *
     * -- Smacky 20130228 */
    remove_ob(trap); /* we must remove and reinsert it.. */
    CLEAR_FLAG(trap, FLAG_SYS_OBJECT);
    CLEAR_MULTI_FLAG(trap, FLAG_IS_INVISIBLE);
    trap->layer = MSP_SLAYER_ITEMB;
    trap->stats.Cha = 1; // make it visible

    if (env && env->type != PLAYER && env->type != MONSTER && env->type != LOCKED_DOOR && !QUERY_FLAG(env, FLAG_NO_PASS))
    {
        SET_FLAG(env, FLAG_IS_TRAPED);
#ifndef USE_OLD_UPDATE
        OBJECT_UPDATE_UPD(env, UPD_FLAGS);
#else
        if (!env->env) /* env object is on map */
            update_object(env, UP_OBJ_FACE);
        else /* somewhere else - if visible, update */
        {
            esrv_update_item(UPD_FLAGS, env);
        }
#endif

        insert_ob_in_ob(trap, env);
//        esrv_update_item(UPD_LOCATION, env, trap);
    }
    else
        insert_ob_in_map(trap, where->map, NULL, 0);
    return 1;
}

int trap_disarm(object_t *disarmer, object_t *trap, int risk)
{
    object_t *env             = trap->env;
    int     trapworth;  /* need to compute the experience worth of the trap
                           before we kill it */
    int     disarmer_level  = SK_level(disarmer);

    /* this formula awards a more reasonable amount of exp */
    trapworth = MAX(1, trap->level) * disarmer->map->difficulty * SQR(MAX(trap->stats.dam, spells[trap->stats.sp].sp))
              / disarmer_level;

    if ((trap->level <= disarmer_level && (RANDOM() % 10))
     || !(random_roll(0, (MAX(2, MIN(20, trap->level - disarmer_level + 5 - disarmer->stats.Dex / 2)) - 1))))
    {
        ndi(NDI_UNIQUE, 0, disarmer, "You successfuly remove the %s (lvl %d)!", trap->name, trap->level);
        remove_ob(trap);
        check_walk_off(trap, NULL, MOVE_APPLY_VANISHED);
        set_traped_flag(env);
        /* If it is your own trap, (or any players trap), don't you don't
           * get exp for it.
           */
        if (trap->owner && trap->owner->type != PLAYER && risk)
            return trapworth;
        else
            return 1; /* give minimal exp and say success */
    }
    else
    {
        ndi(NDI_UNIQUE, 0, disarmer, "You fail to remove the %s (lvl %d).", trap->name, trap->level);
        if ((trap->level > disarmer_level * 1.4f || (RANDOM() % 3)))
        {
            if (!(random_roll(0, (MAX(2, disarmer_level - trap->level + disarmer->stats.Dex / 2 - 6)) - 1))
             && risk)
            {
                ndi(NDI_UNIQUE, 0, disarmer, "In fact, you set it off!");
                spring_trap(trap, disarmer);
            }
        }
        return 0;
    }
}

/*  traps need to be adjusted for the difficulty of the map.  The
default traps are too strong for wimpy level 1 players, and
unthreatening to anyone of high level */

void trap_adjust(object_t *trap, int difficulty)
{
    int off;

    if (difficulty < 1)
        difficulty = 1;

    off = (int) ((float) difficulty * 0.2f);

    trap->level = random_roll(difficulty - off, difficulty + off);
    if (trap->level < 1)
        trap->level = 1;

    /* set the hiddenness of the trap, similar formula to above */
    trap->stats.Cha = random_roll(0, 19) + random_roll(difficulty - off, difficulty + off);
    if (trap->stats.Cha < 1)
        trap->stats.Cha = 1;
}
