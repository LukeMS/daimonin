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
#include <book.h>

/*
 * When stealing: dependent on the intelligence/wisdom of whom you're
 * stealing from (op in attempt_steal), offset by your dexterity and
 * skill at stealing. They may notice your attempt, whether successful
 * or not.
 */
int attempt_steal(object *op, object *who)
{
    object*success =    NULL, *tmp = NULL, *next;
    int                 roll = 0, chance = 0, stats_value = get_weighted_skill_stats(who) * 3;
    int victim_lvl = op->                           level*3, thief_lvl = SK_level(who) * 10;

    /* if the victim is aware of a thief in the area (FLAG_NO_STEAL set on them)
     * they will try to prevent stealing if they can. Only unseen theives will
     * have much chance of success.
     */
    if (op->type != PLAYER && QUERY_FLAG(op, FLAG_NO_STEAL))
    {
        if (1)
        {
            /* add here a distance/can see function!! */
            /* TODO: should probaly call set_npc_enemy() here instead */
            /* TODO: disabled while cleaning up monster.c
            npc_call_help(op); */
            CLEAR_FLAG(op, FLAG_UNAGGRESSIVE);
            new_draw_info(NDI_UNIQUE, 0, who, "Your attempt is prevented!");
            return 0;
        }
        else /* help npc to detect thief next time by raising its wisdom */
            op->stats.Wis += (op->stats.Int / 5) + 1;
    }

    /* Ok then, go thru their inventory, stealing */
    for (tmp = op->inv; tmp != NULL; tmp = next)
    {
        next = tmp->below;

        /* you can't steal worn items, starting items, wiz stuff,
         * innate abilities, or items w/o a type. Generally
         * speaking, the invisibility flag prevents experience or
         * abilities from being stolen since these types are currently
         * always invisible objects. I was implicit here so as to prevent
         * future possible problems. -b.t.
         * Flesh items generated w/ fix_flesh_item should have FLAG_NO_STEAL
         * already  -b.t.
         */

        if (QUERY_FLAG(tmp, FLAG_WIZ)
         || QUERY_FLAG(tmp, FLAG_APPLIED)
         || !(tmp->type)
         || tmp->type
         == EXPERIENCE
         || tmp->type
         == ABILITY
         || QUERY_FLAG(tmp,
                       FLAG_STARTEQUIP)
         || QUERY_FLAG(tmp,
                       FLAG_NO_STEAL)
         || IS_SYS_INVISIBLE(tmp))
            continue;

        /* Okay, try stealing this item. Dependent on dexterity of thief,
         * skill level, see the adj_stealroll fctn for more detail. */

        roll = random_roll(2, 100) / 2; /* weighted 1-100 */

        if ((chance = adj_stealchance(who, op, (stats_value + thief_lvl - victim_lvl))) == -1)
            return 0;
        else if (roll < chance)
        {
            if (op->type == PLAYER)
                esrv_del_item(CONTR(op), tmp->count, tmp->env);
            pick_up(who, tmp);
            if (can_pick(who, tmp))
            {
                /* for players, play_sound: steals item */
                success = tmp;
                CLEAR_FLAG(tmp, FLAG_INV_LOCKED);
            }
            break;
        }
    } /* for loop looking for an item */

    /* If you arent high enough level, you might get something BUT
     * the victim will notice your stealing attempt. Ditto if you
     * attempt to steal something heavy off them, they're bound to notice
     */

    if ((roll >= SK_level(who))
     || !chance
     || (tmp && tmp->weight > (250 * (random_roll(0, stats_value + thief_lvl - 1)))))
    {
        /* victim figures out where the thief is! */
        if (who->hide)
            make_visible(who);

        if (op->type != PLAYER)
        {
            /* The unaggressives look after themselves 8) */
            if (who->type == PLAYER)
            {
                /* TODO: should probaly call set_npc_enemy() here instead */
                /* TODO: disabled while cleaning up monster.c */
                /* npc_call_help(op); */
                new_draw_info(NDI_UNIQUE, 0, who, "%s notices your attempted pilfering!",
                              query_short_name(op, who));
            }
            CLEAR_FLAG(op, FLAG_UNAGGRESSIVE);
            /* all remaining npc items are guarded now. Set flag NO_STEAL
               * on the victim. */
            SET_FLAG(op, FLAG_NO_STEAL);
        }
        else
        {
            /* stealing from another player */
            /* Notify the other player */
            if (success && who->stats.Int > random_roll(0, 19))
            {
                new_draw_info(NDI_UNIQUE, 0, op, "Your %s is missing!",
                              query_name(success));
            }
            else
            {
                new_draw_info(NDI_UNIQUE, 0, op, "Your pack feels strangely lighter.");
            }

            if (!success)
            {
                if (QUERY_FLAG(who, FLAG_IS_INVISIBLE))
                {
                    new_draw_info(NDI_UNIQUE, 0, op, "you feel itchy fingers getting at your pack.");
                }
                else
                {
                    new_draw_info(NDI_UNIQUE, 0, op, "%s looks very shifty.",
                                  query_short_name(who, op));
                }
            }
        } /* else stealing from another player */
        /* play_sound("stop! thief!"); kindofthing */
    } /* if you weren't 100% successful */
    return success ? 1 : 0;
}

/* adj_stealchance() - increased values indicate better attempts */
int adj_stealchance(object *op, object *victim, int roll)
{
    object *equip;
    int     used_hands  = 0;

    if (!op || !victim || !roll)
        return -1;

    /* ADJUSTMENTS */

    /* Its harder to steal from hostile beings! */
    if (!QUERY_FLAG(victim, FLAG_UNAGGRESSIVE))
        roll = roll / 2;

    /* Easier to steal from sleeping beings, or if the thief is
     * unseen */
    if (QUERY_FLAG(victim, FLAG_SLEEP))
        roll = roll * 3;
    else if (QUERY_FLAG(op, FLAG_IS_INVISIBLE))
        roll = roll * 2;

    /* check stealing 'encumberance'. Having this equipment applied makes
     * it quite a bit harder to steal. */
    for (equip = op->inv; equip; equip = equip->below)
    {
        if (equip->type == WEAPON && QUERY_FLAG(equip, FLAG_APPLIED))
        {
            roll -= equip->weight / 10000;
            used_hands++;
        }
        if (equip->type == BOW && QUERY_FLAG(equip, FLAG_APPLIED))
            roll -= equip->weight / 5000;
        if (equip->type == SHIELD && QUERY_FLAG(equip, FLAG_APPLIED))
        {
            roll -= equip->weight / 2000;
            used_hands++;
        }
        if (equip->type == ARMOUR && QUERY_FLAG(equip, FLAG_APPLIED))
            roll -= equip->weight / 5000;
        if (equip->type == GLOVES && QUERY_FLAG(equip, FLAG_APPLIED))
            roll -= equip->weight / 100;
    }

    if (roll < 0)
        roll = 0;
    if (op->type == PLAYER && used_hands >= 2)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "But you have no free hands to steal with!");
        roll = -1;
    }

    return roll;
}

int steal(object *op, int dir)
{
    object     *tmp, *next;
    mapstruct  *mt;

    int         x   = op->x + freearr_x[dir];
    int         y   = op->y + freearr_y[dir];

    if (dir == 0)
    {
        /* Can't steal from ourself! */
        return 0;
    }

    if (wall(op->map, x, y))
    {
        return 0;
    }

    if (!(mt = out_of_map(op->map, &x, &y)))
        return 0;
    /* Find the topmost object at this spot */
    for (tmp = GET_MAP_OB(mt, x, y); tmp != NULL && tmp->above != NULL; tmp = tmp->above)
        ;

    /* For all the stacked objects at this point, attempt a steal */
    for (; tmp != NULL; tmp = next)
    {
        next = tmp->below;
        /* Minor hack--for multi square beings - make sure we get
         * the 'head' coz 'tail' objects have no inventory! - b.t.
         */
        if (tmp->head)
            tmp = tmp->head;
        if (tmp->type != PLAYER && !QUERY_FLAG(tmp, FLAG_MONSTER))
            continue;
        if (attempt_steal(tmp, op))
        {
            if (tmp->type == PLAYER) /* no xp for stealing from another player */
                return 0;
            else
                return (calc_skill_exp(op, tmp, 1.0f,-1, NULL));
        }
    }
    return 0;
}

/* Implementation by bt. (thomas@astro.psu.edu)
 * monster implementation 7-7-95 by bt.
 */

int pick_lock(object *pl, int dir)
{
    object     *tmp;
    mapstruct  *m;
    int         x       = pl->x + freearr_x[dir];
    int         y       = pl->y + freearr_y[dir];
    int         success = 0;

    if (!dir)
        dir = pl->facing;

    /* For all the stacked objects at this point find a door*/
    if (!(m = out_of_map(pl->map, &x, &y)))
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "There is no lock there.");
        return 0;
    }

    for (tmp = GET_MAP_OB(m, x, y); tmp; tmp = tmp->above)
    {
        if (!tmp)
            continue;
        switch (tmp->type)
        {
            case DOOR:
              if (!QUERY_FLAG(tmp, FLAG_NO_PASS))
              {
                  new_draw_info(NDI_UNIQUE, 0, pl, "The door has no lock!");
              }
              else
              {
                  if (attempt_pick_lock(tmp, pl))
                  {
                      success = 1;
                      new_draw_info(NDI_UNIQUE, 0, pl, "You pick the lock.");
                  }
                  else
                  {
                      new_draw_info(NDI_UNIQUE, 0, pl, "You fail to pick the lock.");
                  }
              }
              break;
            case LOCKED_DOOR:
              new_draw_info(NDI_UNIQUE, 0, pl, "You can't pick that lock!");
              break;
            default:
              break;
        }
    }
    if (success)
        return calc_skill_exp(pl, NULL, 1.0f,-1, NULL);
    else
        return 0;
}

int attempt_pick_lock(object *door, object *pl)
{
    int bonus       = SK_level(pl);
    int difficulty  = pl->map->difficulty ? pl->map->difficulty : 0;
    int dex         = get_skill_stat1(pl) ? get_skill_stat1(pl) : 10;
    int success = 0, number;        /* did we get anything? */

    /* If has can_pass set, then its not locked! */
    if (!QUERY_FLAG(door, FLAG_NO_PASS))
        return 0;

    /* Try to pick the lock on this item (doors only for now).
     * Dependent on dexterity/skill SK_level of the player and
     * the map level difficulty.
     */

    number = (random_roll(2, 40) - 2) / 2;
    if (number < ((dex + bonus) - difficulty))
    {
        remove_door(door);
        success = 1;
    }
    else if (door->inv && door->inv->type == RUNE)
    {
        /* set off any traps? */
        spring_trap(door->inv, pl);
    }
    return success;
}

/* HIDE CODE. The user becomes undetectable (not just 'invisible') for
 * a short while (success and duration dependant on player SK_level,
 * dexterity, charisma, and map difficulty).
 * Players have a good chance of becoming 'unhidden' if they move
 * and like invisiblity will be come visible if they attack
 * Implemented by b.t. (thomas@astro.psu.edu)
 * July 7, 1995 - made hiding possible for monsters. -b.t.
 */

/* patched this to take terrain into consideration */

int hide(object *op)
{
    /* int level= SK_level(op);*/

    /* the preliminaries -- Can we really hide now? */
    /* this keeps monsters from using invisibilty spells and hiding */

    if (QUERY_FLAG(op, FLAG_SEE_INVISIBLE))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You don't need to hide while invisible!");
        return 0;
    }
    else if (!op->hide && QUERY_FLAG(op, FLAG_IS_INVISIBLE) && op->type == PLAYER)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Your attempt to hide breaks the invisibility spell!");
        make_visible(op);
        return 0;
    }
    /*
     if(op->invisible>(50*level)) {
          new_draw_info(NDI_UNIQUE,0,op,"You are as hidden as you can get.");
          return 0;
     }*/

    if (attempt_hide(op))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You hide in the shadows.");
        update_object(op, UP_OBJ_FACE);
        return calc_skill_exp(op, NULL, 1.0f, -1, NULL);
    }
    new_draw_info(NDI_UNIQUE, 0, op, "You fail to conceal yourself.");
    return 0;
}

int attempt_hide(object *op)
{
    int level       = SK_level(op);
    int success = 0, number, difficulty = op->map->difficulty;
    int dexterity   = get_skill_stat1(op);
    int terrain     = hideability(op);

    level = level > 5 ? level / 5 : 1;

    /* first things... no hiding next to a hostile monster */
    dexterity = dexterity ? dexterity : 15;

    if (terrain < -10) /* not enough cover here */
        return 0;

    /*  Hiding success and duration dependant on SK_level,
     *  dexterity, map difficulty and terrain.
     */

    number = (random_roll(2, 25) - 2) / 2;
    if (!stand_near_hostile(op) && number && (number < (dexterity + level + terrain - difficulty)))
    {
        success = 1;
        op->hide = 1;
    }
    return success;
}

/* stop_jump() - End of jump. Clear flags, restore the map, and
 * freeze the jumper a while to simulate the exhaustion
 * of jumping.
 */

static int stop_jump(object *pl, int dist, int spaces)
{
    /* int load=dist/(pl->speed*spaces); */

    CLEAR_MULTI_FLAG(pl, FLAG_FLYING);
    insert_ob_in_map(pl, pl->map, pl, 0);

    /*if (pl->type==PLAYER) draw_client_map(pl); */

    /* pl->speed_left= (int) -FABS((load*8)+1); */
    return 0;
}


static int attempt_jump(object *pl, int dir, int spaces)
{
    object     *tmp;
    mapstruct  *m;
    int         i, xt, yt, exp = 0, dx = freearr_x[dir], dy = freearr_y[dir];

    /* Jump loop. Go through spaces opject wants to jump. Halt the
     * jump if a wall or creature is in the way. We set FLAG_FLYING
     * temporarily to allow player to aviod exits/archs that are not
     * fly_on, fly_off. This will also prevent pickup of objects
     * while jumping over them.
     */

    remove_ob(pl);
    if (check_walk_off(pl, NULL, MOVE_APPLY_VANISHED) != CHECK_WALK_OK)
        return 0;
    SET_MULTI_FLAG(pl, FLAG_FLYING);
    for (i = 0; i <= spaces; i++)
    {
        xt = pl->x + dx;
        yt = pl->y + dy;
        if (!(m = out_of_map(pl->map, &xt, &yt)))
        {
            (void) stop_jump(pl, i, spaces);
            return 0; /* no action, no exp */
        }
        for (tmp = GET_MAP_OB(m, xt, yt); tmp; tmp = tmp->above)
        {
            if (wall(tmp->map, xt, yt))
            {
                /* Jump into wall*/
                new_draw_info(NDI_UNIQUE, 0, pl, "Your jump is blocked.");
                (void) stop_jump(pl, i, spaces);
                return 0;
            }
            /* Jump into creature */
            if (QUERY_FLAG(tmp, FLAG_MONSTER) || tmp->type == PLAYER)
            {
                new_draw_info(NDI_UNIQUE, 0, pl, "You jump into%s%s.", tmp->type == PLAYER ? " " : " the ",
                                     tmp->name);
                if (tmp->type != PLAYER)
                    exp = skill_attack(tmp, pl, pl->facing, "kicked"); /* pl makes an attack */
                (void) stop_jump(pl, i, spaces);
                return exp;  /* note that calc_skill_exp() is already called by skill_attack() */
            }
            /* If the space has fly on set (no matter what the space is),
               * we should get the effects - after all, the player is
               * effectively flying.
               */
            if (QUERY_FLAG(tmp, FLAG_FLY_ON))
            {
                pl->x += dx,pl->y += dy;
                (void) stop_jump(pl, i, spaces);
                return 0;
            }
        }
        pl->x += dx;
        pl->y += dy;
    }
    (void) stop_jump(pl, i, spaces);
    return 0;
}

/* jump() - this is both a new type of movement for player/monsters and
 * an attack as well. -b.t.
 */

int jump(object *pl, int dir)
{
    int spaces = 0, stats;
    int str = get_skill_stat1(pl);
    int dex = get_skill_stat2(pl);

    dex = dex ? dex : 15;
    str = str ? str : 10;

    stats = str * str * str * dex;

    if (pl->carrying != 0)      /* don't want div by zero !! */
        spaces = (int) (stats / pl->carrying);
    else
        spaces = 2; /* pl has no objects - gets the far jump */

    if (spaces > 2)
        spaces = 2;
    else if (spaces == 0)
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "You are carrying too much weight to jump.");
        return 0;
    }
    return attempt_jump(pl, dir, spaces);
}


/* skill_ident() - this code is supposed to allow players to identify
 * classes of objects with the various "auto-ident" skills. Player must
 * have unidentified objects of the right type in order for the skill
 * to work. While multiple classes of objects may be identified,
 * this code is kind of yucky -- it would be nice to make it a bit
 * more generalized. Right now, skill indices are embedded in this routine.
 * Returns amount of experience gained (on successful ident).
 * - b.t. (thomas@astro.psu.edu)
 */

int skill_ident(object *pl)
{
    int     success = 0;

    if (!pl->chosen_skill)  /* should'nt happen... */
        return 0;

    if (pl->type != PLAYER)
        return 0;  /* only players will skill-identify */

    new_draw_info(NDI_UNIQUE, 0, pl, "You look at the objects nearby...");

    switch (pl->chosen_skill->stats.sp)
    {
        case SK_SMITH:
          success += do_skill_ident(pl, WEAPON)
                   + do_skill_ident(pl, ARMOUR)
                   + do_skill_ident(pl, BRACERS)
                   + do_skill_ident(pl,
                                    CLOAK)
                   + do_skill_ident(pl,
                                    BOOTS)
                   + do_skill_ident(pl,
                                    SHIELD)
                   + do_skill_ident(pl,
                                    GIRDLE)
                   + do_skill_ident(pl,
                                    HELMET)
                   + do_skill_ident(pl,
                                    SHOULDER)
                   + do_skill_ident(pl,
                                    LEGS)
                   + do_skill_ident(pl,
                                    GLOVES);
          break;
        case SK_BOWYER:
          success += do_skill_ident(pl, BOW) + do_skill_ident(pl, ARROW);
          break;
        case SK_ALCHEMY:
          success += do_skill_ident(pl, POTION)
                   + do_skill_ident(pl, POISON)
                   + do_skill_ident(pl, AMULET)
                   + do_skill_ident(pl,
                                    CONTAINER)
                   + do_skill_ident(pl,
                                    DRINK)
                   + do_skill_ident(pl,
                                    INORGANIC);
          break;
        case SK_WOODSMAN:
          success += do_skill_ident(pl, FOOD) + do_skill_ident(pl, DRINK) + do_skill_ident(pl, FLESH);
          break;
        case SK_JEWELER:
          success += do_skill_ident(pl, GEM)
              + do_skill_ident(pl, TYPE_PEARL)
              + do_skill_ident(pl, TYPE_JEWEL)
                   + do_skill_ident(pl, TYPE_NUGGET)
                   + do_skill_ident(pl,
                                    RING);
          break;
        case SK_LITERACY:
          success += do_skill_ident(pl, SPELLBOOK) + do_skill_ident(pl, SCROLL) + do_skill_ident(pl, BOOK);
          break;
        case SK_THAUMATURGY:
          success += do_skill_ident(pl, WAND) + do_skill_ident(pl, ROD) + do_skill_ident(pl, HORN);
          break;
        case SK_DET_CURSE:
          success = do_skill_detect_curse(pl);
          if (success)
              new_draw_info(NDI_UNIQUE, 0, pl, "...and discover cursed items!");
          break;
        case SK_DET_MAGIC:
          success = do_skill_detect_magic(pl);
          if (success)
              new_draw_info(NDI_UNIQUE, 0, pl, "...and discover items imbued with mystic forces!");
          break;
        default:
          LOG(llevBug, "BUG: bad call to skill_ident()");
          return 0;
          break;
    }
    if (!success)
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "...and learn nothing more.");
    }

    return success;
}

int do_skill_detect_curse(object *pl)
{
    object *tmp;
    int     success = 0;

    /* check the player inventory - stop after 1st success or
     * run out of unidented items
     */
    for (tmp = pl->inv; tmp; tmp = tmp->below)
        if (!QUERY_FLAG(tmp, FLAG_KNOWN_CURSED) && is_cursed_or_damned(tmp))
        {
            SET_FLAG(tmp, FLAG_KNOWN_CURSED);
            esrv_update_item(UPD_FLAGS, pl, tmp);
            success += calc_skill_exp(pl, tmp, 1.0f,-1, NULL);
        }
    return success;
}

int do_skill_detect_magic(object *pl)
{
    object *tmp;
    int     success = 0;

    /* check the player inventory - stop after 1st success or
     * run out of unidented items
     */
    for (tmp = pl->inv; tmp; tmp = tmp->below)
        if (!QUERY_FLAG(tmp, FLAG_KNOWN_MAGICAL) && (is_magical(tmp)))
        {
            SET_FLAG(tmp, FLAG_KNOWN_MAGICAL);
            esrv_update_item(UPD_FLAGS, pl, tmp);
            success += calc_skill_exp(pl, tmp, 1.0f, -1, NULL);
        }
    return success;
}

/* Helper function for do_skill_ident, so that we can loop
over inventory AND objects on the ground conveniently.  */
int do_skill_ident2(object *tmp, object *pl, int obj_class)
{
    int success = 0, chance;
    int skill_value = SK_level(pl) + get_weighted_skill_stats(pl);

    if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED)
     && !QUERY_FLAG(tmp, FLAG_NO_SKILL_IDENT)
     && need_identify(tmp)
     && !IS_SYS_INVISIBLE(tmp)
     && tmp->type == obj_class)
    {
        chance = random_roll(3, 10) - 3 + random_roll(0, (tmp->magic ? tmp->magic * 5 : 1) - 1);
        if (skill_value >= chance)
        {
            identify(tmp);
            if (pl->type == PLAYER)
            {
                new_draw_info(NDI_UNIQUE, 0, pl, "You identify %s.", long_desc(tmp, pl));
                if (tmp->msg)
                {
                    new_draw_info(NDI_UNIQUE, 0, pl, "The item has a story:");
                    new_draw_info(NDI_UNIQUE, 0, pl, "%s", tmp->msg);
                }
                /* identify will take care of updating the item if
                     * it is in the players inventory.  IF on map, do it
                     * here
                     */
                if (tmp->map)
                    esrv_send_item(pl, tmp);
            }
            success += calc_skill_exp(pl, tmp, 1.0f,-1, NULL);
        }
        else
            SET_FLAG(tmp, FLAG_NO_SKILL_IDENT);
    }
    return success;
}
/* do_skill_ident() - workhorse for skill_ident() -b.t. */
/*  Sept 95. I put in a probability for identification of artifacts.
 *  highly magical artifacts will be more difficult to ident -b.t.
 */
int do_skill_ident(object *pl, int obj_class)
{
    object *tmp;
    int     success = 0;
    /* check the player inventory */
    for (tmp = pl->inv; tmp; tmp = tmp->below)
        success += do_skill_ident2(tmp, pl, obj_class);
    /*  check the ground */
    for (tmp = GET_MAP_OB(pl->map, pl->x, pl->y); tmp; tmp = tmp->above)
        success += do_skill_ident2(tmp, pl, obj_class);

    return success;
}

/* players using this skill can 'charm' a monster --
 * into working for them. It can only be used on
 * non-special (see below) 'neutral' creatures.
 * -b.t. (thomas@astro.psu.edu)
 */

int use_oratory(object *pl, int dir)
{
    LOG(llevBug, "BUG: unimplemented oratory skill used\n");
    /* TODO: update for the new AI and pet systems. Gecko 2006-04-30 */
#if 0
    int x = pl->x + freearr_x[dir], y = pl->y + freearr_y[dir], chance;
    int             stat1   = get_skill_stat1(pl);
    object         *tmp;
    mapstruct      *m;

    if (pl->type != PLAYER)
        return 0;   /* only players use this skill */
    if (!(m = out_of_map(pl->map, &x, &y)))
        return 0;

    for (tmp = GET_MAP_OB(m, x, y); tmp; tmp = tmp->above)
    {
        if (!tmp)
            return 0;
        if (!QUERY_FLAG(tmp, FLAG_MONSTER))
            continue;
        /* can't persude players - return because there is nothing else
        * on that space to charm.  Same for multi space monsters and
        * special monsters - we don't allow them to be charmed, and there
        * is no reason to do further processing since they should be the
        * only monster on the space.
        */
        if (tmp->type == PLAYER)
            return 0;
        if (tmp->more || tmp->head)
            return 0;
        if (tmp->msg)
            return 0;


        new_draw_info(NDI_UNIQUE, 0, pl, "You orate to the %s.", query_name(tmp));

        /* the following conditions limit who may be 'charmed' */

        /* it's hostile! */
        if (!QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE) && !QUERY_FLAG(tmp, FLAG_FRIENDLY))
        {
            new_draw_info(NDI_UNIQUE, 0, pl, "Too bad the %s isn't listening!\n", query_name(tmp));
            return 0;
        }

        /* it's already allied! */
        if (QUERY_FLAG(tmp, FLAG_FRIENDLY) && (tmp->move_type == PETMOVE))
        {
            if (get_owner(tmp) == pl)
            {
                new_draw_info(NDI_UNIQUE, 0, pl, "Your follower loves your speach.\n");
                return 0;
            }
            else if (SK_level(pl) > tmp->level)
            {
                /* you steal the follower! */
                set_owner(tmp, pl);
                new_draw_info(NDI_UNIQUE, 0, pl, "You convince the %s to follow you instead!\n", query_name(tmp));
                /* Abuse fix - don't give exp since this can otherwise
                     * be used by a couple players to gets lots of exp.
                     */
                return 0;
            }
        } /* Creature was already a pet of someone */

        chance = SK_level(pl) * 2 + (stat1 - 2 * tmp->stats.Int) / 2;

        /* Ok, got a 'sucker' lets try to make them a follower */
        if (chance > 0 && tmp->level < (random_roll(0, chance - 1) - 1))
        {
            new_draw_info(NDI_UNIQUE, 0, pl, "You convince the %s to become your follower.\n", query_name(tmp));

            set_owner(tmp, pl);
            SET_FLAG(tmp, FLAG_MONSTER);
            tmp->stats.exp = 0;
            SET_FLAG(tmp, FLAG_FRIENDLY);
            tmp->move_type = PETMOVE;
            return calc_skill_exp(pl, tmp, 1.0f, -1, NULL);
        }
        /* Charm failed.  Creature may be angry now */
        else if ((SK_level(pl) + ((stat1 - 10) / 2)) < random_roll(1, 2 * tmp->level))
        {
            new_draw_info(NDI_UNIQUE, 0, pl, "Your speech angers the %s!\n", query_name(tmp));
            /* TODO: should probaly call set_npc_enemy() here instead/also */
            if (QUERY_FLAG(tmp, FLAG_FRIENDLY))
            {
                CLEAR_FLAG(tmp, FLAG_FRIENDLY);
                tmp->move_type = 0;     /* needed? */
            }
            CLEAR_FLAG(tmp, FLAG_UNAGGRESSIVE);
        }
    } /* For loop cyclign through the objects on this space */
#endif
    return 0;   /* Fall through - if we get here, we didn't charm anything */
}

/* Singing() -this skill allows the player to pacify nearby creatures.
 * There are few limitations on who/what kind of
 * non-player creatures that may be pacified. Right now, a player
 * may pacify creatures which have Int == 0. In this routine, once
 * successfully pacified the creature gets Int=1. Thus, a player
 * may only pacify a creature once.
 * BTW, I appologize for the naming of the skill, I couldnt think
 * of anything better! -b.t.
 */

int singing(object *pl, int dir)
{
    LOG(llevBug, "BUG: unimplemented singing skill used\n");
    return 0;
    /* TODO: update for the new AI and pet systems. Gecko 2006-05-01 */
#if 0
    int         xt, yt, i, exp = 0, stat1 = get_skill_stat1(pl), chance;
    object     *tmp;
    mapstruct  *m;

    if (pl->type != PLAYER)
        return 0;    /* only players use this skill */

    new_draw_info(NDI_UNIQUE, 0, pl, "You sing");
    for (i = dir; i < (dir + MIN(SK_level(pl), SIZEOFFREE)); i++)
    {
        xt = pl->x + freearr_x[i];
        yt = pl->y + freearr_y[i];
        if (!(m = out_of_map(pl->map, &xt, &yt)))
            continue;
        for (tmp = GET_MAP_OB(m, xt, yt); tmp; tmp = tmp->above)
        {
            if (!tmp)
                return 0;
            if (!QUERY_FLAG(tmp, FLAG_MONSTER))
                continue;

            /* can't affect players */
            if (tmp->type == PLAYER)
                break;

            /* Only the head listens to music - not other parts.  head
               * is only set if an object has extra parts.  This is also
               * necessary since the other parts may not have appropriate
               * skills/flags set.
               */
            if (tmp->head)
                break;

            /* the following monsters can't be calmed */

            if (QUERY_FLAG(tmp, FLAG_SPLITTING) /* have no ears! */ || QUERY_FLAG(tmp, FLAG_HITBACK))
                break;

            /*if(tmp->stats.Int>0) break;   */ /* is too smart */
            if (tmp->level > SK_level(pl))
                break;  /* too powerfull */
            if (QUERY_FLAG(tmp, FLAG_UNDEAD))
                break; /* undead dont listen! */

            if (QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE) /* already calm */ || QUERY_FLAG(tmp, FLAG_FRIENDLY))
                break;

            /* stealing isn't really related (although, maybe it should
               * be).  This is mainly to prevent singing to the same monster
               * over and over again and getting exp for it.
               */
            chance = SK_level(pl) * 2 + (stat1 - 5 - tmp->stats.Int) / 2;
            if (chance && tmp->level * 2 < random_roll(0, chance - 1))
            {
                SET_FLAG(tmp, FLAG_UNAGGRESSIVE);
                new_draw_info(NDI_UNIQUE, 0, pl, "You calm down the %s\n", query_name(tmp));
                tmp->stats.Int = 1; /* this prevents re-pacification */
                /* Give exp only if they are not aware */
                if (!QUERY_FLAG(tmp, FLAG_NO_STEAL))
                    exp += calc_skill_exp(pl, tmp, 1.0f, -1, NULL);
                SET_FLAG(tmp, FLAG_NO_STEAL);
            }
            else
            {
                new_draw_info(NDI_UNIQUE, 0, pl, "Too bad the %s isn't listening!\n", query_name(tmp));
            }
        }
    }
    return exp;
#endif
}

/* This skill allows the player to regain a few sp or hp for a
 * brief period of concentration. No armour or weapons may be
 * wielded/applied for this to work. The amount of time needed
 * to concentrate and the # of points regained is dependant on
 * the level of the user. - b.t. thomas@astro.psu.edu
 */

/* Sept 95. Now meditation is level dependant (score). User may
 * meditate w/ more armour on as they get higher level
 * Probably a better way to do this is based on overall encumberance
 * -b.t.
 */

void meditate(object *pl)
{
    object *tmp;
    int     lvl = pl->level;
    /* int factor = 10/(1+(pl->level/10)+(pl->stats.Int/15)+(pl->stats.Wis/15)); */

    if (pl->type != PLAYER)
        return; /* players only */

    /* check if pl has removed encumbering armour and weapons */

    if (QUERY_FLAG(pl, FLAG_READY_WEAPON) && (lvl < 6))
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "You can't concentrate while wielding a weapon!\n");
        return;
    }
    else
    {
        for (tmp = pl->inv; tmp; tmp = tmp->below)
            if (((tmp->type == ARMOUR && lvl < 12)
              || (tmp->type == HELMET && lvl < 10)
              || (tmp->type == SHIELD && lvl < 6)
              || (tmp->type == SHOULDER && lvl < 6)
              || (tmp->type == LEGS && lvl < 6)
              || (tmp->type == BOOTS && lvl < 4)
              || (tmp->type == GLOVES && lvl < 2))
             && QUERY_FLAG(tmp,
                           FLAG_APPLIED))
            {
                new_draw_info(NDI_UNIQUE, 0, pl, "You can't concentrate while wearing so much armour!\n");
                return;
            }
    }

    /* ok let's meditate!  Spell points are regained first, then once
     * they are maxed we get back hp. Actual incrementing of values
     * is handled by the do_some_living() (in player.c). This way magical
     * bonuses for healing/sp regeneration are included properly
     * No matter what, we will eat up some playing time trying to
     * meditate. (see 'factor' variable for what sets the amount of time)
     */

    new_draw_info(NDI_UNIQUE, 0, pl, "You meditate.");
    /*   pl->speed_left -= (int) FABS(factor); */

    if (pl->stats.sp < pl->stats.maxsp)
    {
        pl->stats.sp++;
        pl->last_sp = -1;
    }
    else if (pl->stats.hp < pl->stats.maxhp)
    {
        pl->stats.hp++;
        pl->last_heal = -1;
    }
    else
        return;

}

/* write_on_item() - wrapper for write_note and write_scroll */

int write_on_item(object *pl, char *params)
{
    object *item;
    char   *string  = params;
    int     msgtype;

    if (pl->type != PLAYER)
        return 0;
    if (!params)
    {
        params = "";
        string = params;
    }

    /* Need to be able to read before we can write! */

    if (!find_skill(pl, SK_LITERACY))
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "You must learn to read before you can write!");
        return 0;
    }

    /* if skill name occurs at begining of the string
    * we have to reset pointer to miss it and trailing space(s)
    */
    /*
    * GROS: Bugfix here. if you type
    * use_skill inscription bla
    * params will contain "bla" only, so looking for the skill name
    * shouldn't be done anymore.
    */
    /*  if(lookup_skill_by_name(params)>=0){
        for(i=strcspn(string," ");i>0;i--) string++;
        for(i=strspn(string," ");i>0;i--) string++;
        }
    */
    /* if there is a message then it goes in a book and no message means
     * write active spell into the scroll
     */
    msgtype = (string[0] != '\0') ? BOOK : SCROLL;

    /* find an item of correct type to write on */
    if (!(item = find_marked_object(pl)))
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "You don't have anything marked.");
        return 0;
    }

    if (item)
    {
        if (QUERY_FLAG(item, FLAG_UNPAID))
        {
            new_draw_info(NDI_UNIQUE, 0, pl, "You had better pay for that before you write on it.");
            return 0;
        }
        switch (item->type)
        {
            case SCROLL:
              return write_scroll(pl, item);
              break;
            case BOOK:
              {
                  return write_note(pl, item, string);
                  break;
              }
            default:
              break;
        }
    }
    new_draw_info(NDI_UNIQUE, 0, pl, "You have no %s to write on", msgtype == BOOK ? "book" : "scroll");
    return 0;
}

/* write_note() - this routine allows players to inscribe messages in
 * ordinary 'books' (anything that is type BOOK). b.t.
 */

int write_note(object *pl, object *item, char *msg)
{
    char    buf[BOOK_BUF];
    object *newBook = NULL;

    /* a pair of sanity checks */
    if (!item || item->type != BOOK)
        return 0;

    if (!msg)
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "No message to write!");
        new_draw_info(NDI_UNIQUE, 0, pl, "Usage: use_skill %s <message>", skills[SK_INSCRIPTION].name);
        return 0;
    }
    if (strstr(msg, "endmsg"))
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "Trying to cheat now are we?");
        return 0;
    }

    if(trigger_object_plugin_event(EVENT_TRIGGER, item, pl, NULL,
                msg, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
        return 0;

    if (!book_overflow(item->msg, msg, BOOK_BUF))
    {
        /* add msg string to book */
        if (item->msg)
        {
            strcpy(buf, item->msg);
            FREE_AND_CLEAR_HASH2(item->msg);
        }
        strcat(buf, msg);
        strcat(buf, "\n"); /* new msg needs a LF */
        if (item->nrof > 1)
        {
            newBook = get_object();
            copy_object(item, newBook);
            decrease_ob(item);
            esrv_send_item(pl, item);
            newBook->nrof = 1;
            FREE_AND_COPY_HASH(newBook->msg, buf);
            newBook = insert_ob_in_ob(newBook, pl);
            esrv_send_item(pl, newBook);
        }
        else
        {
            FREE_AND_COPY_HASH(item->msg, buf);
            esrv_send_item(pl, item);
        }
        new_draw_info(NDI_UNIQUE, 0, pl, "You write in the %s.", query_short_name(item, pl));
        return strlen(msg);
    }
    else
        new_draw_info(NDI_UNIQUE, 0, pl, "Your message won't fit in the %s!", query_short_name(item, pl));
    return 0;
}

/* write_scroll() - this routine allows players to inscribe spell scrolls
 * of spells which they know. Backfire effects are possible with the
 * severity of the backlash correlated with the difficulty of the scroll
 * that is attempted. -b.t. thomas@astro.psu.edu
 */

int write_scroll(object *pl, object *scroll)
{
    int     success = 0, confused = 0, chosen_spell = -1, stat1 = get_skill_stat1(pl);
    object *newScroll;

    /* this is a sanity check */
    if (scroll->type != SCROLL)
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "A spell can only be inscribed into a scroll!");
        return 0;
    }

    /* Check if we are ready to attempt inscription */
    chosen_spell = scroll->stats.sp;
    if (chosen_spell < 0)
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "You need a spell readied in order to inscribe!");
        return 0;
    }
    if (!(spells[chosen_spell].spell_use & SPELL_USE_SCROLL))
    {
        /* Tried to write non-scroll spell */
        new_draw_info(NDI_UNIQUE, 0, pl, "The spell %s cannot be inscribed.", spells[chosen_spell].name);
        return 0;
    }
    if (spells[chosen_spell].flags & SPELL_DESC_WIS && spells[chosen_spell].sp > pl->stats.grace)
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "You don't have enough grace to write a scroll of %s.",
                             spells[chosen_spell].name);
        return 0;
    }
    else if (spells[chosen_spell].sp > pl->stats.sp)
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "You don't have enough mana to write a scroll of %s.",
                             spells[chosen_spell].name);
        return 0;
    }

    /* if there is a spell already on the scroll then player could easily
     * accidently read it while trying to write the new one.  give player
     * a 50% chance to overwrite spell at their own level
     */
    if (scroll->stats.sp && random_roll(0, scroll->level * 2) > SK_level(pl))
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "Oops! You accidently read it while trying to write on it.");
        manual_apply(pl, scroll, 0);
        change_skill(pl, SK_INSCRIPTION);
        return 0;
    }

    /* ok, we are ready to try inscription */

    if (QUERY_FLAG(pl, FLAG_CONFUSED))
        confused = 1;

    /* Lost mana/grace no matter what */
    if (spells[chosen_spell].flags & SPELL_DESC_WIS)
        pl->stats.grace -= spells[chosen_spell].sp;
    else
        pl->stats.sp -= spells[chosen_spell].sp;

    if (random_roll(0, spells[chosen_spell].level * 4 - 1) < SK_level(pl))
    {
        newScroll = get_object();
        copy_object(scroll, newScroll);
        decrease_ob(scroll);
        newScroll->nrof = 1;

        if (!confused)
        {
            newScroll->level = (SK_level(pl) > spells[chosen_spell].level ? SK_level(pl) : spells[chosen_spell].level);
        }
        else
        {
            /* a  confused scribe gets a random spell */
            do
            {
                chosen_spell = random_roll(0, NROFREALSPELLS - 1);
                /* skip all non active spells */
                while (!spells[chosen_spell].is_active)
                {
                    chosen_spell++;
                    if (chosen_spell >= NROFREALSPELLS)
                        chosen_spell = 0;
                }
            }
            while (!(spells[chosen_spell].spell_use & SPELL_USE_SCROLL));

            newScroll->level = SK_level(pl)
                             > spells[chosen_spell].level
                             ? spells[chosen_spell].level
                             : (random_roll(1, SK_level(pl)));
        }

        if (newScroll->stats.sp == chosen_spell)
            new_draw_info(NDI_UNIQUE, 0, pl, "You overwrite the scroll.");
        else
        {
            new_draw_info(NDI_UNIQUE, 0, pl, "You succeed in writing a new scroll.");
            newScroll->stats.sp = chosen_spell;
        }

        /* wait until finished manipulating the scroll before inserting it */
        newScroll = insert_ob_in_ob(newScroll, pl);
        esrv_send_item(pl, newScroll);
        success = calc_skill_exp(pl, newScroll, 1.0f, -1, NULL);
        if (!confused)
            success *= 2;
        return success;
    }
    else
    {
        /* Inscription has failed */

        if (spells[chosen_spell].level > SK_level(pl) || confused)
        {
            /*backfire!*/
            new_draw_info(NDI_UNIQUE, 0, pl, "Ouch! Your attempt to write a new scroll strains your mind!");
            if (random_roll(0, 1) == 1)
                drain_specific_stat(pl, 4);
            else
            {
                confuse_player(pl, pl, 160);
                /*      return (-3*calc_skill_exp(pl,newScroll));*/
                return (-30 * spells[chosen_spell].level);
            }
        }
        else if (random_roll(0, stat1 - 1) < 15)
        {
            new_draw_info(NDI_UNIQUE, 0, pl, "Your attempt to write a new scroll rattles your mind!");
            confuse_player(pl, pl, 160);
        }
        else
            new_draw_info(NDI_UNIQUE, 0, pl, "You fail to write a new scroll.");
    }
    /*    return (-1*calc_skill_exp(pl,newScroll));*/
    return (-10 * spells[chosen_spell].level);
}

/* The FIND_TRAPS skill. This routine is taken mostly from the
 * command_search loop. It seemed easier to have a separate command,
 * rather than overhaul the existing code - this makes sure things
 * still work for those people who don't want to have skill code
 * implemented. */
int find_traps(object *op, int level)
{
    uint8 i,
          found = 0,
          aware = 0;

    /* Search the squares in the 8 directions. */
    for (i = 0; i < 9; i++)
    {
        int         xt = op->x + freearr_x[i],
                    yt = op->y + freearr_y[i];
        mapstruct  *m;
        object     *next,
                   *this;

        /* Ensure the square isn't out of bounds. */
        if (!(m = out_of_map(op->map, &xt, &yt)))
            continue;
 
        next = GET_MAP_OB(m, xt, yt);
        while ((this = next))
        {
            /* this is the object on the map, that is the current object under
             * consideration. */
            object *that = this;

            next = this->above;

            /* op, players, and monsters are opaque to find traps. */
            if (that == op || that->type == PLAYER || that->type == MONSTER)
                continue;

            /* Otherwise, check that and (if necessary) inventory of that. */
            while (that)
            {
                if (that->type == RUNE && that->stats.Cha > 1)
                {
                    if (trap_see(op, that, level))
                    {
                        trap_show(that, this);
                        found++;
                    }
                    else
                        if (that->level <= (level * 1.8f))
                            aware = 1;
                }
                that = find_next_object(that, RUNE, FNO_MODE_CONTAINERS, that);
            }
        }
    }

   /* Only players get messages. */
    if (op->type == PLAYER && CONTR(op))
    {
        if (!found)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You find no new traps this time...");
            if (aware)
                new_draw_info(NDI_UNIQUE, 0, op, "But you find signs of traps hidden beyond your skill...");
        }
        else
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You find %d new traps!",
                                                      found);
            if (aware)
                new_draw_info(NDI_UNIQUE, 0, op, "You also find signs of more traps hidden beyond your skill...");
        }
    }

    return 0;
}

/* remove_trap() - This skill will disarm any previously discovered trap
 * the algorithm is based (almost totally) on the old command_disarm() - b.t.
 */

int remove_trap(object *op, int dir, int level)
{
    object     *tmp, *tmp2;
    mapstruct  *m;
    int         i, x, y;

    for (i = 0; i < 9; i++)
    {
        x = op->x + freearr_x[i];
        y = op->y + freearr_y[i];
        if (!(m = out_of_map(op->map, &x, &y)))
            continue;

        /*  Check everything in the square for trapness */
        for (tmp = GET_MAP_OB(m, x, y); tmp != NULL; tmp = tmp->above)
        {
            /* And now we'd better do an inventory traversal of each
                       * of these objects' inventory */

            for (tmp2 = tmp->inv; tmp2 != NULL; tmp2 = tmp2->below)
            {
                if (tmp2->type == RUNE && tmp2->stats.Cha <= 1)
                {
                    if (QUERY_FLAG(tmp2, FLAG_SYS_OBJECT) || QUERY_FLAG(tmp2, FLAG_IS_INVISIBLE))
                        trap_show(tmp2, tmp);
                    trap_disarm(op, tmp2, 1);
                    return 0;
                }
            }
            if (tmp->type == RUNE && tmp->stats.Cha <= 1)
            {
                if (QUERY_FLAG(tmp, FLAG_SYS_OBJECT) || QUERY_FLAG(tmp, FLAG_IS_INVISIBLE))
                    trap_show(tmp, tmp);
                trap_disarm(op, tmp, 1);
                return 0;
            }
        }
    }
    new_draw_info(NDI_UNIQUE, 0, op, "You have found no nearby traps to remove yet!");
    return 0;
}
