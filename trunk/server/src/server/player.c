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
#ifndef WIN32 /* ---win32 remove headers */
#include <pwd.h>
#endif

static const char *CreateGravestone(object *op, mapstruct *m, int x, int y);

/* find a player name for a NORMAL string.
 * we use the hash table system.
 */
player * find_player(char *plname)
{
    char name[MAX_PLAYER_NAME+1];
    const char *name_hash;

    int name_len = strlen(plname); /* we assume a legal string */
    if (name_len <= 1 || name_len > MAX_PLAYER_NAME)
        return NULL;

    strcpy(name, plname); /* we need to copy it because we access the string */
    transform_player_name_string(name);
    if (!(name_hash = find_string(name)))
        return NULL;

    return find_player_hash(name_hash);
}

/* nearly the same as above except we
 * have the hash string when we call
 */
player * find_player_hash(const char *plname)
{
    player *pl;

    if(plname)
    {
        for (pl = first_player; pl != NULL; pl = pl->next)
        {
            if (pl->ob && !QUERY_FLAG(pl->ob, FLAG_REMOVED) && pl->ob->name == plname)
                return pl;
        }
    }
    return NULL;
}


void display_motd(object *op)
{
#ifdef MOTD
    char    buf[MEDIUM_BUF];
    FILE   *fp;

    sprintf(buf, "%s/%s", settings.localdir, MOTD_FILE);
    if ((fp = fopen(buf,"r")) == NULL)
    {
        sprintf(buf, "%s/%s", settings.localdir, MOTD_DEFAULT);
        if ((fp = fopen(buf,"r")) == NULL)
        {
            return;
        }
    }
    while (fgets(buf, MEDIUM_BUF, fp) != NULL)
    {
        char   *cp;
        if (*buf == '#')
            continue;
        cp = strchr(buf, '\n');
        if (cp != NULL)
            *cp = '\0';
        new_draw_info(NDI_UNIQUE, 0, op, "%s", buf);
    }
    fclose(fp);
    new_draw_info(NDI_UNIQUE, 0, op, " ");
#endif
}

void free_player(player *pl)
{
    /* first, we removing the player from map or whatever */
	LOG(llevDebug, "FREE_PLAYER(%s): state:%d g_status:%x\n", STRING_OBJ_NAME(pl->ob), pl->state, pl->group_status);
    if (pl->ob)
    {
        SET_FLAG(pl->ob, FLAG_NO_FIX_PLAYER);

        container_unlink(pl, NULL);

        /* remove the player from global gmaster lists */
        if(pl->gmaster_mode != GMASTER_MODE_NO)
            remove_gmaster_list(pl);

        /* remove player from party */
        if(pl->group_status & GROUP_STATUS_GROUP)
            party_remove_member(pl, TRUE);

        activelist_remove(pl->ob);
        if (!QUERY_FLAG(pl->ob, FLAG_REMOVED))
        {
            remove_ob(pl->ob);
            check_walk_off(pl->ob, NULL, MOVE_APPLY_VANISHED);
        }
    }
    /* Free pet links - moved that before remove from active list */
    while(pl->pets)
        objectlink_unlink(&pl->pets, NULL, pl->pets);

    /* this is needed so we don't destroy the player queue inside the
     * main loop. The socket loop can handle it nicely but the player
     * loop not - with this simple trick we avoid alot work.
     * Remember: at this point our player object is saved and removed from the game
     * only for the player loop and the socket its still present.
     */

#ifdef USE_CHANNELS
    /* remove player from ALL channels before player gets destroyed */
    leaveAllChannels(pl);
#endif

    if((pl->state & ST_ZOMBIE) && !(pl->state & ST_DEAD))
    {
        pl->state &= ~ST_PLAYING;
        pl->state |= ST_DEAD;
		LOG(llevDebug, "FREE_PLAYER(%s) --> ST_DEAD\n", STRING_OBJ_NAME(pl->ob));
        FREE_AND_COPY_HASH(pl->ob->name, "noname"); /* we neutralize the name - we don't want find this player anymore */
        insert_ob_in_ob(pl->ob, &void_container); /* Avoid gc of the player object */
        return;
    }
    /* Now remove from list of players */
    if (pl->prev)
        pl->prev->next = pl->next;
    else
        first_player = pl->next;

    if (pl->next)
        pl->next->prev = pl->prev;
    else
        last_player = pl->prev;
    player_active--;


    if(pl->socket.status != Ns_Disabled)
        free_newsocket(&pl->socket);

    if (pl->ob)
    {
        if (!QUERY_FLAG(pl->ob, FLAG_REMOVED))
            remove_ob(pl->ob);

        /* Force an out-of-loop gc to delete the player object NOW */
        object_gc();
    }
}

/* called from gc - we remove here the last allocated memory 
 * and release the hash strings
 */
void destroy_player_struct(player *pl)
{
    /* clear all hash strings */
    FREE_AND_CLEAR_HASH(pl->instance_name);
    FREE_AND_CLEAR_HASH(pl->group_invite_name);
    FREE_AND_CLEAR_HASH(pl->killer);
    FREE_AND_CLEAR_HASH(pl->savebed_map );
    FREE_AND_CLEAR_HASH(pl->orig_savebed_map);
    FREE_AND_CLEAR_HASH(pl->maplevel );
    FREE_AND_CLEAR_HASH(pl->orig_map);
    FREE_AND_CLEAR_HASH(pl->account_name);
}

void give_initial_items(object *pl, struct oblnk *items)
{
    object *op, *next = NULL;

    if (pl->randomitems != NULL)
        create_treasure_list(items, pl, GT_ONLY_GOOD | GT_NO_VALUE, 1, ART_CHANCE_UNSET, 0);

    for (op = pl->inv; op; op = next)
    {
        next = op->below;

        /* Forces get applied per default */
        if (op->type == FORCE)
        {
            SET_FLAG(op, FLAG_APPLIED);
        }
        /* we never give weapons/armour if these cannot be used
           by this player due to race restrictions */
        else if ((!QUERY_FLAG(pl, FLAG_USE_ARMOUR)
                    && (op->type == ARMOUR
                        || op->type == BOOTS
                        || op->type == CLOAK
                        || op->type == HELMET
                        || op->type == SHOULDER
                        || op->type == LEGS
                        || op->type == SHIELD
                        || op->type == GLOVES
                        || op->type == BRACERS
                        || op->type == GIRDLE))
                || (!QUERY_FLAG(pl, FLAG_USE_WEAPON) && op->type == WEAPON))
        {
            remove_ob(op); /* inventory action */
            continue;
        }
        else if (op->type == ABILITY)
        {
            CONTR(pl)->known_spells[CONTR(pl)->nrofknownspells++] = op->stats.sp;
            remove_ob(op);
            continue;
        }
        /* now we apply the stuff on default - *very* useful for real new players! */
        else  if(op->type == WEAPON || op->type == AMULET || op->type == RING ||
                op->type == BOOTS || op->type == HELMET || op->type == BRACERS || op->type == GIRDLE ||
                op->type == CLOAK || op->type == ARMOUR || op->type == SHIELD || op->type == GLOVES ||
                op->type == SHIELD || op->type == GLOVES || op->type == LEGS || op->type == SHOULDER)
        {
            if (need_identify(op))
            {
                SET_FLAG(op, FLAG_IDENTIFIED);
                CLEAR_FLAG(op, FLAG_CURSED);
                CLEAR_FLAG(op, FLAG_DAMNED);
            }

            /* WARNING: we force here the flag "applied" without calling manual_apply().
             * We do this to avoid apply messages & commands send to the client - item applying
             * is normally an action which works ONLY for active playing character.
             * We must ensure here, that our applyable startup items don't trigger deeper
             * game engine effects like items set function, scripts and such.
             * they will not trigger here and are so a source of possible bugs.
             */
            SET_FLAG(op, FLAG_APPLIED);
        }

        /* Give starting characters identified, uncursed, and undamned
         * items.  Just don't identify gold or silver, or it won't be
         * merged properly.
         */
        else if (need_identify(op))
        {
            SET_FLAG(op, FLAG_IDENTIFIED);
            CLEAR_FLAG(op, FLAG_CURSED);
            CLEAR_FLAG(op, FLAG_DAMNED);
        }
    } /* for loop of objects in player inv */
}

void flee_player(object *op)
{
    int dir, diff;
    if (op->stats.hp <= 0)
    {
        LOG(llevDebug, "Fleeing player is dead.\n");
        CLEAR_FLAG(op, FLAG_SCARED);
        return;
    }
    if (op->enemy == NULL)
    {
        LOG(llevDebug, "Fleeing player had no enemy.\n");
        CLEAR_FLAG(op, FLAG_SCARED);
        return;
    }
    if (!random_roll(0, 4))
    {
        op->enemy = NULL;
        CLEAR_FLAG(op, FLAG_SCARED);
        return;
    }
    dir = absdir(4 + find_dir_2(op->x - op->enemy->x, op->y - op->enemy->y));
    for (diff = 0; diff < 3; diff++)
    {
        int m   = 1 - (RANDOM() & 2);
        if (move_ob(op, absdir(dir + diff * m), op) || (diff == 0 && move_ob(op, absdir(dir - diff * m), op)))
        {
            /*
               draw_client_map(op);
             */
            return;
        }
    }
    /* Cornered, get rid of scared */
    CLEAR_FLAG(op, FLAG_SCARED);
    op->enemy = NULL;
}

/* For B4 i redesigned move_player().
 * move_player() was always in sense of a "move" not a "walk".
 * The walking is only one part if this function. The main use
 * is to determinate a player can do a move (and moving to another
 * tile = walking is a move).
 * A glitch was, that move_player() can change the direction of the move
 * invoked for example by confusion.
 * In the past, firing was included in this function, i removed it now from
 * it and added some senseful return values.
 * if flag is TRUE, the function will do a step/walk and call move_ob(),
 * if FALSE (used from fire/range code), the function will return with status.
 * Return values:
 * -1 = doing the move failed (rotted, paralyzed...)
 * 0-x = move will go in that direction
 */
int move_player(object * const op, int dir, const int flag)
{
    player *pl = CONTR(op);

    pl->rest_sitting = pl->rest_mode = 0;

    if (op->map == NULL || op->map->in_memory != MAP_ACTIVE ||
        QUERY_FLAG(op,FLAG_PARALYZED) || QUERY_FLAG(op,FLAG_ROOTED))
        return -1;

    if ((dir = absdir(dir)))
        op->facing = dir;
    if (QUERY_FLAG(op, FLAG_CONFUSED) && dir)
        dir = absdir(dir + RANDOM() % 3 + RANDOM() % 3 - 2);

    op->anim_moving_dir = -1;
    op->anim_enemy_dir = -1;
    op->anim_last_facing = -1;

    /* puh, we should move hide to FLAG_ ASAP */
    if (op->hide)
    {
        op->anim_moving_dir = dir;
        do_hidden_move(op);
    }

    if (!flag)
		return dir;

	if (!move_ob(op, dir, op))
		op->anim_enemy_dir = dir;
	else
		op->anim_moving_dir = dir;

    /* But I need pushing for the Fluffy quest! -- Gecko :-( */
    /* Thats what the /push command will be for... ;-) */
    /* the old push & roll code - will be transfered to /push
        if (get_owner(tmp)==op ||
            ((QUERY_FLAG(tmp,FLAG_UNAGGRESSIVE) || tmp->type==PLAYER ||
            QUERY_FLAG(tmp, FLAG_FRIENDLY)) && !op_on_battleground(op, NULL, NULL)))
        {
            play_sound_map(op->map, op->x, op->y, SOUND_PUSH_PLAYER, SOUND_NORMAL);
            if(push_ob(tmp,dir,op))
                ret = 1;
            if(op->hide)
                make_visible(op);
            return ret;
        }
        else if(QUERY_FLAG(tmp,FLAG_CAN_ROLL))
        {
            recursive_roll(tmp,dir,op);
            if(action_makes_visible(op))
                make_visible(op);
        }
    */

    if (QUERY_FLAG(op, FLAG_ANIMATE)) /* hm, should be not needed - players always animated */
    {
        if (op->anim_enemy_dir == -1 && op->anim_moving_dir == -1)
            op->anim_last_facing = dir;
        animate_object(op, 0);
    }
    return dir;
}


/* main player command loop - read command from buffer
 * and execute it.
 * Return:
 *         0: turn speed used up or no commands left
 *        -1: player is invalid now
 */
int handle_newcs_player(player *pl)
{
    object *op  = pl->ob;

    if (!op || !OBJECT_ACTIVE(op))
        return -1;

    process_command_queue(&pl->socket, pl);

    if (!op || !OBJECT_ACTIVE(op) || pl->socket.status >= Ns_Zombie)
        return -1;

    /* player is fine, check for speed */
    if (op->speed_left < 0.0f || QUERY_FLAG(op,FLAG_PARALYZED) ||QUERY_FLAG(op,FLAG_ROOTED) )
        return 0;

    /* this movement action will dramatically change in the future.
     * basically we will go for a "steps per ticks"
     */

    if (op->direction && CONTR(op)->run_on)     /* automove */
    {
        /* All move commands take 1 tick, at least for now */
        move_player(op, op->direction, TRUE);
        op->speed_left--;
    }

    return 0;
}

int save_life(object *op)
{
    object *tmp;

    if (!QUERY_FLAG(op, FLAG_LIFESAVE))
        return 0;
    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
        if (QUERY_FLAG(tmp, FLAG_APPLIED) && QUERY_FLAG(tmp, FLAG_LIFESAVE))
        {
            play_sound_map(op->map, op->x, op->y, SOUND_OB_EVAPORATE, SOUND_NORMAL);
            new_draw_info(NDI_UNIQUE, 0, op, "Your %s vibrates violently, then evaporates.", query_name(tmp));
            remove_ob(tmp);
            CLEAR_FLAG(op, FLAG_LIFESAVE);
            if (op->stats.hp <= 0)
                op->stats.hp = op->stats.maxhp;
            return 1;
        }
    LOG(llevBug, "BUG: LIFESAVE set without applied object.\n");
    CLEAR_FLAG(op, FLAG_LIFESAVE);
    return 0;
}

/* This goes throws the inventory and removes unpaid objects, and puts them
 * back in the map (location and map determined by values of env).  This
 * function will descend into containers.  op is the object to start the search
 * from.
 */
void remove_unpaid_objects(object *op, object *env)
{
    object *next;

    while (op)
    {
        next = op->below;   /* Make sure we have a good value, in case
                            * we remove object 'op'
                            */
        if (QUERY_FLAG(op, FLAG_UNPAID))
        {
            remove_ob(op);
            op->x = env->x;
            op->y = env->y;
            insert_ob_in_map(op, env->map, NULL, 0);
        }
        else if (op->inv)
            remove_unpaid_objects(op->inv, env);
        op = next;
    }
}

/* regeneration helper functions - cleaner as a macro */
static inline void do_reg_hp(player *pl, object *op)
{
    if(op->stats.hp < op->stats.maxhp)
    {
        op->stats.hp += pl->reg_hp_num;
        if(op->stats.hp > op->stats.maxhp)
            op->stats.hp = op->stats.maxhp;
    }
}
static inline void do_reg_sp(player *pl, object *op)
{
    if(op->stats.sp < op->stats.maxsp)
    {
        op->stats.sp += pl->reg_sp_num;
        if(op->stats.sp > op->stats.maxsp)
            op->stats.sp = op->stats.maxsp;
    }
}
static inline void do_reg_grace(player *pl, object *op)
{
    if(op->stats.grace < op->stats.maxgrace)
    {
        op->stats.grace += pl->reg_grace_num;
        if(op->stats.grace > op->stats.maxgrace)
            op->stats.grace = op->stats.maxgrace;
    }
}


/* Handles regenerations (hp, sp and grace) and "resting". Player only!
 * Reworked for beta 4 - food and digestion are removed.
 * When not in combat the player is automatically regenerating a small
 * amount of hp/sp/grace. When in combat, the player only recoveres sp and grace.
 * When resting, the server is counting down a "prepare resting" timer (usually 10-15 sec)
 * until rapid healing will start, so the player is fully healed and filled up with sp/grace
 * after 30-45 sec.
 * Alternative ways to regenerate are potions (will give INSTANT hp/sp/grace) or food, which
 * will give a set amount of hp/sp/grace in a given time (usually 8-x sec too).
 * So, the regeneration will be potion (instant), food (direct in 8-x sec) or resting (8-x sec + 30-45 sec).
 */
void do_some_living(object *op)
{
    player *pl = CONTR(op);

    if (!pl || !(pl->state & ST_PLAYING))
        return;

    /* sanity kill check */
    if (op->stats.hp <= 0)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You died by low hitpoints!");
        kill_player(op);
        return;
    }

    /* food_status < 0 marks an active food force - we don't want double regeneration! */
    if (pl->food_status>=0) /* be sure we are active and playing */
    {
        pl->food_status = 0;

        pl->reg_timer = 8; /* timer so we call this all 8 ticks = one per second */

        /* automatic rengeneration. If we are combat flagged, only regenerate sp/grace */
        if(pl->damage_timer > 0)
        {
            pl->damage_timer--;

            /* as long we are "in combat" there is no resting - reset the resting counter if we sit */
            if(pl->rest_sitting)
                pl->resting_reg_timer = RESTING_DEFAULT_SEC_TIMER;

             if(pl->normal_reg_timer > 0) /* only give back all x seconds some points */
                --pl->normal_reg_timer;
             else
             {
                 /*new_draw_info(NDI_UNIQUE, 0, op, "reg - combat sp/gr");*/
                 pl->normal_reg_timer = REG_DEFAULT_SEC_TIMER;
                 do_reg_sp(pl, op);
                 do_reg_grace(pl, op);
             }
        }
        else /* not in combat - regenerate or rest fully */
        {
            if(pl->rest_sitting) /* player is sitting && resting */
            {
                /* no rest mode? well, can happen... we got interrupted... reenter but with fresh set timer */
                if(!pl->rest_mode)
                {
                    pl->rest_mode = 1;
                    pl->resting_reg_timer = RESTING_DEFAULT_SEC_TIMER;
                }
                else if(pl->resting_reg_timer > 0)
                {
                    /*new_draw_info(NDI_UNIQUE, 0, op, "reg - prepare %d", pl->resting_reg_timer);*/
                    --pl->resting_reg_timer; /* player is still in rest preparing phase */
                    pl->food_status = (1000/RESTING_DEFAULT_SEC_TIMER)*(pl->resting_reg_timer+1);
                }
                else /* all ok - we rest and regenerate with full speed */
                {
                    /*new_draw_info(NDI_UNIQUE, 0, op, "reg - full rest");*/
                    pl->food_status = 999; /* "resting is active" marker */
                    do_reg_hp(pl, op);
                    do_reg_sp(pl, op);
                    do_reg_grace(pl, op);
                }
            }
            else /* normal regeneration, no combat... slowly bring the values up */
            {
                if(pl->normal_reg_timer > 0) /* only give back all x seconds some points */
                    --pl->normal_reg_timer;
                else
                {
                    /*new_draw_info(NDI_UNIQUE, 0, op, "reg - normal tick");*/
                    pl->normal_reg_timer = REG_DEFAULT_SEC_TIMER;
                    do_reg_hp(pl, op);
                    do_reg_sp(pl, op);
                    do_reg_grace(pl, op);
                }
            }
        }
    }
}

/* Creates a gravestone for the player (op) and inserts it on the spot where he
 * died. */
static const char *CreateGravestone(object *op, mapstruct *m, int x, int y)
{
    object        *gravestone;
    int            i;
    char           buf[MEDIUM_BUF];
    timeanddate_t  tad;

    if (!(gravestone = arch_to_object(archetype_global._gravestone)) ||
        (i = check_insertion_allowed(gravestone, m, x, y, 1,
                                     INS_NO_FORCE | INS_WITHIN_LOS)) == -1)
    {
        return NULL;
    }

    if (op->level >= 1)
    {
        gravestone->level = op->level;
    }

    sprintf(buf, "%s's gravestone", STRING_OBJ_NAME(op));
    FREE_AND_COPY_HASH(gravestone->name, buf);
    /* name */
    sprintf(buf, "R.I.P.\n\n%s", query_name(op));
    /* race, level */
    sprintf(strchr(buf, '\0'), " the %s\nwho was level %d\nwhen ",
            STRING_OBJ_RACE(op), (int)op->level);
    /* gender */
    sprintf(strchr(buf, '\0'), "%s",
            (QUERY_FLAG(op, FLAG_IS_MALE)) ?
            ((QUERY_FLAG(op, FLAG_IS_FEMALE)) ? "they were" : "he was") :
            ((QUERY_FLAG(op, FLAG_IS_FEMALE)) ? "she was" : "it was"));
    /* cause of death */
    sprintf(strchr(buf, '\0'), " killed by %s.\n\n",
            (CONTR(op)->killer) ? CONTR(op)->killer : "bad luck");
    /* date */
    get_tad(&tad);
    sprintf(strchr(buf, '\0'), "On %s\n",
            print_tad(&tad, TAD_SHOWDATE | TAD_LONGFORM));
    FREE_AND_COPY_HASH(gravestone->msg, buf);

    x += freearr_x[i];
    y += freearr_y[i];

    if (!(m = out_of_map(m, &x, &y)))
    {
        return NULL;
    }

    gravestone->x = x;
    gravestone->y = y;
    insert_ob_in_map(gravestone, m, NULL, INS_NO_WALK_ON);

    return gravestone->msg;
}

/* If the player should die (lack of hp, food, etc), we call this.
 * op is the player in jeopardy.  If the player can not be saved (not
 * permadeath, no lifesave), this will take care of removing the player
 * file.
 *
 * Returns 0 if the player didn't really die, 1 if they did. This is used for PvP
 * stat tracking.
 */
int kill_player(object *op)
{
    player      *pl=CONTR(op);
    char        buf[MEDIUM_BUF];
    int         x, y;
    mapstruct  *map;  /*  this is for resurrection */
    object     *tmp,
               *dep;
    uint8       lost_a_stat = 0;

    if (save_life(op))
        return 0;

    /* If player dies on BATTLEGROUND, no stat/exp loss! For Combat-Arenas
     * in cities ONLY!!! It is very important that this doesn't get abused.
     * Look at op_on_battleground() for more info       --AndreasV
     */

    if (op_on_battleground(op, &x, &y))
    {
        new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, "You have been defeated in combat!");
        new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, "Local medics have saved your life...");

        /* restore player */
        cast_heal(op, 110, op, SP_CURE_POISON);
        /*cast_heal(op, op, SP_CURE_CONFUSION);*/
        cure_disease(op, NULL);  /* remove any disease */
        op->stats.hp = op->stats.maxhp;

        /* create a bodypart-trophy to make the winner happy */
        tmp = arch_to_object(find_archetype("finger"));
        if (tmp != NULL)
        {
            sprintf(buf, "%s's finger", STRING_OBJ_NAME(op));
            FREE_AND_COPY_HASH(tmp->name, buf);
            sprintf(buf, "  This finger has been cut off %s\n"
                         "  the %s, when he was defeated at\n  level %d by %s.\n",
                    STRING_OBJ_NAME(op), (op->title) ? STRING_OBJ_TITLE(op) :
                    STRING_OBJ_RACE(op), (int)op->level, (pl->killer) ?
                    pl->killer : "bad luck");
            FREE_AND_COPY_HASH(tmp->msg, buf);
            tmp->value = 0, tmp->material = 0, tmp->type = 0;
            tmp->x = op->x, tmp->y = op->y;
            insert_ob_in_map(tmp, op->map, op, 0);
        }

        /* teleport defeated player to new destination*/
        enter_map(op, NULL, op->map, x, y, 0, 0);
        return 1;
    }

    if(trigger_object_plugin_event(EVENT_DEATH,
                op, NULL, op, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
        return 0; /* Cheat death */

#if 0 /* Disabled global events */
    CFParm      CFP;
    int         evtid;
    /* GROS: Handle for the global death event */
    evtid = EVENT_GDEATH;
    CFP.Value[0] = (void *) (&evtid);
    CFP.Value[1] = NULL;
    CFP.Value[2] = (void *) (op);
    GlobalEvent(&CFP);
#endif

    /*  save the map location for corpse, gravestone*/
    x = op->x;
    y = op->y;
    map = op->map;
    play_sound_player_only(pl, SOUND_PLAYER_DIES, SOUND_NORMAL, 0, 0);

#ifdef NOT_PERMADEATH
    /* NOT_PERMADEATH code.  This basically brings the character back to life
     * if they are dead - it takes some exp and a random stat.  See the config.h
     * file for a little more in depth detail about this. */

    /* Clear out any food forces so if the player died while eating a bad
     * apple he won't keep redying in his apt. */
    remove_food_force(op);
    pl->food_status = 0;

    /* remove any poisoning and confusion the character may be suffering. */
    cast_heal(op, 110, op, SP_CURE_POISON);
    cure_disease(op, NULL);  /* remove any disease */
    /* FIXME: All the cure_what_ails_you()s are less than ideal as each one
     * potentially searches the player's entire inv. We should use a flag
     * system rather than st1 to do all the necessary forces in one pass,
     * and/or query flags here to be sure the player even has the ailment in
     * question. Not sure if all ailments have a flag though.
     * -- Smacky 20100608 */
    (void)cure_what_ails_you(op, ST1_FORCE_DEPLETE);
    (void)cure_what_ails_you(op, ST1_FORCE_DRAIN);
    (void)cure_what_ails_you(op, ST1_FORCE_SLOWED);
    (void)cure_what_ails_you(op, ST1_FORCE_FEAR);
    (void)cure_what_ails_you(op, ST1_FORCE_SNARE);
    (void)cure_what_ails_you(op, ST1_FORCE_PARALYZE);
    (void)cure_what_ails_you(op, ST1_FORCE_CONFUSED);
    (void)cure_what_ails_you(op, ST1_FORCE_BLIND);
    (void)cure_what_ails_you(op, ST1_FORCE_POISON);

    /* The rule is: only decrease stats when the player is at least level 3 or
     * higher!  */
    if (settings.stat_loss &&
        op->level >= MAX(3, ABS(settings.stat_loss) / 5))
    {
       uint8 i,
             num_stats_lose = 1,
             stats[NUM_STATS] = { 0, 0, 0, 0, 0, 0, 0 };

        /* Stats are lost on death through death sickness according to
         * settings.stat_loss -- see config.h/STAT_LOSS.
         *
         * The theory behind multiple stat loss is that lower level chars don't
         * lose as many stats because they suffer more if they do, while higher
         * level characters can afford things such as potions of restoration,
         * or better, stat potions. So we slug them that little bit harder. */
        /* FIXME: IIRC cure death sickness restores ALL stats and the cost is
         * based on player level, not the number or value of lost stats. Which
         * negates some of the financial reasoning just given.
         * --Smacky  20100603 */
        if (settings.stat_loss > 0)
        {
            num_stats_lose += op->level / settings.stat_loss;
        }

        for (i = 0; i < num_stats_lose; i++)
        {
            stats[RANDOM() % NUM_STATS]++;
        }

        if (!(dep = present_arch_in_ob(archetype_global._deathsick, op)))
        {
            dep = arch_to_object(archetype_global._deathsick);
            insert_ob_in_ob(dep, op);
        }

        for (i = 0; i < NUM_STATS; i++)
        {
            uint8 op_val = get_stat_value(&(op->stats), i),
                  dep_val = ABS(get_stat_value(&(dep->stats), i));

            if (dep_val + stats[i] >= op_val)
            {
                stats[i] = MAX(0, op_val - dep_val - 1);
            }

            if (stats[i])
            {
                change_stat_value(&(dep->stats), i, -stats[i]);
                new_draw_info(NDI_UNIQUE, 0, op, "%s (You lose ~%d~ %s).",
                                     lose_msg[i], stats[i], stat_name[i]);
                lost_a_stat = 1;
            }
        }
    }

    if (lost_a_stat)
    {
        SET_FLAG(dep, FLAG_APPLIED);
        /* apply_death_exp_penalty() below does a fix_player() so we don't need
         * this one. */
        /*FIX_PLAYER(op ,"kill player - change attr");*/
    }
    /* If no stat lost, tell the player. */
    /* FIXME: These messages need a rewrite -- one would assume
     * holy protection is from death itself, not just stat loss.
     * -- Smacky 20100103 */
    else
    {
        /* determine_god() seems to not work sometimes... why is this?
           Should I be using something else? GD */
        const char *god = determine_god(op);

        if (god != shstr_cons.none)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "For a brief moment you feel the holy presence of %s protecting you.",
                                 god);
        }
        else
        {
            new_draw_info(NDI_UNIQUE, 0, op, "For a brief moment you feel a holy presence protecting you.");
        }
    }

#ifdef USE_GRAVESTONES
    /* Put a gravestone up where or near to where the character died if there
     * is a free spot and isn't already one there. */
    CreateGravestone(op, map, x, y);
#endif

    apply_death_exp_penalty(op);
    op->stats.hp = op->stats.maxhp;
    op->stats.sp = op->stats.maxsp;
    op->stats.grace = op->stats.maxgrace;

    /*
     * Check to see if the player is in a shop.  IF so, then check to see if
     * the player has any unpaid items.  If so, remove them and put them back
     * in the map.
     */
    GET_MAP_SPACE_SYS_OBJ(GET_MAP_SPACE_PTR(op->map, op->x, op->y), SHOP_FLOOR,
                          tmp);

    if (tmp)
    {
        remove_unpaid_objects(op->inv, op);
    }

    /****************************************/
    /*                                      */
    /* Move player to his current respawn-  */
    /* position (usually last savebed)      */
    /*                                      */
    /****************************************/
    /* JG (aka Grommit) 14-Mar-2007 - changed map_status to bed_status */

    enter_map_by_name(op, pl->savebed_map, pl->orig_savebed_map, pl->bed_x, pl->bed_y, pl->bed_status);

    /**************************************/
    /*                                    */
    /* Repaint the characters inv, and    */
    /* stats, and show a nasty message ;) */
    /*                                    */
    /**************************************/

    /* FIXME: The next line is only a demonstration of the
     * STATS_EVENT()-function. There should be added some more
     * data to a player death
     */
    /*STATS_EVENT(STATS_EVENT_PLAYER_DEATH, op->name);*/
    new_draw_info(NDI_UNIQUE, 0, op, "YOU HAVE DIED.");

    return 1;
#endif
}

/* cast_dust() - handles op throwing objects of type 'DUST' */
/* WARNING: FUNCTION NEED TO BE REWRITTEN. works for ae spells only now! */
void cast_dust(object *op, object *throw_ob, int dir)
{
    archetype  *arch    = NULL;

    if (!(spells[throw_ob->stats.sp].flags & SPELL_DESC_DIRECTION))
    {
        LOG(llevBug, "DEBUG: Warning, dust %s is not a ae spell!!\n", STRING_OBJ_NAME(throw_ob));
        return;
    }

    if (spells[throw_ob->stats.sp].archname)
        arch = find_archetype(spells[throw_ob->stats.sp].archname);

    /* casting POTION 'dusts' is really a use_magic_item skill */
    if (op->type == PLAYER && throw_ob->type == POTION && !change_skill(op, SK_MAGIC_DEVICES))
        return; /* no skill, no dust throwing */


    if (throw_ob->type == POTION && arch != NULL)
        cast_cone(op, throw_ob, dir, 10, throw_ob->stats.sp, arch, throw_ob->level, 1);
    else if ((arch = find_archetype("dust_effect")) != NULL)
    {
        /* dust_effect */
        cast_cone(op, throw_ob, dir, 1, 0, arch, throw_ob->level, 0);
    }
    else /* problem occured! */
        LOG(llevBug, "BUG: cast_dust() can't find an archetype to use!\n");

    if (op->type == PLAYER && arch)
        new_draw_info(NDI_UNIQUE, 0, op, "You cast %s.", query_name(throw_ob));
    if (!QUERY_FLAG(throw_ob, FLAG_REMOVED))
        destruct_ob(throw_ob);
}

void make_visible(object *op)
{
    /*
       if(op->type==PLAYER)
       if(QUERY_FLAG(op, FLAG_UNDEAD)&&!is_true_undead(op))
         CLEAR_FLAG(op, FLAG_UNDEAD);
       update_object(op,UP_OBJ_FACE);
    */
}

int is_true_undead(object *op)
{
    /*
      object *tmp=NULL;

      if(QUERY_FLAG(&op->arch->clone,FLAG_UNDEAD)) return 1;

      if(op->type==PLAYER)
        for(tmp=op->inv;tmp;tmp=tmp->below)
           if(tmp->type==TYPE_SKILLGROUP && tmp->stats.Wis)
          if(QUERY_FLAG(tmp,FLAG_UNDEAD)) return 1;
    */
    return 0;
}

/* look at the surrounding terrain to determine
 * the hideability of this object. Positive levels
 * indicate greater hideability.
 */

int hideability(object *ob)
{
#if 0
  int i,x,y,level=0;

  if(!ob||!ob->map) return 0;


  /* scan through all nearby squares for terrain to hide in */
  for(i=0,x=ob->x,y=ob->y;i<9;i++,x=ob->x+freearr_x[i],y=ob->y+freearr_y[i])
  {
    if(blocks_view(ob->map,x,y)) /* something to hide near! */
      level += 2;
    else /* open terrain! */
      level -= 1;
  }

  LOG(llevDebug,"hideability of %s is %d\n",ob->name,level);
  return level;
#endif
    return 0;
}

/* For Hidden creatures - a chance of becoming 'unhidden'
 * every time they move - as we subtract off 'invisibility'
 * AND, for players, if they move into a ridiculously unhideable
 * spot (surrounded by clear terrain in broad daylight). -b.t.
 */

void do_hidden_move(object *op)
{
    int hide = 0, num = random_roll(0, 19);

    if (!op || !op->map)
        return;

    /* its *extremely* hard to run and sneak/hide at the same time! */
    if (op->type == PLAYER && CONTR(op)->run_on)
    {
        if (num >= SK_level(op))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You ran too much! You are no longer hidden!");
            make_visible(op);
            return;
        }
        else
            num += 20;
    }
    num += op->map->difficulty;
    hide = hideability(op); /* modify by terrain hidden level */
    num -= hide;

    if (op->type == PLAYER && hide < -10)
    {
        make_visible(op);
        if (op->type == PLAYER)
            new_draw_info(NDI_UNIQUE, 0, op, "You moved out of hiding! You are visible!");
    }
}

/* determine if who is standing near a hostile creature. */

int stand_near_hostile(object *who)
{
    object     *tmp = NULL;
    mapstruct  *m;
    int         i, xt, yt;

    if (!who)
        return 0;

    /* search adjacent squares */
    for (i = 1; i < 9; i++)
    {
        xt = who->x + freearr_x[i];
        yt = who->y + freearr_y[i];
        if (!(m = out_of_map(who->map, &xt, &yt)))
            continue;
        for (tmp = GET_MAP_OB(m, xt, yt); tmp; tmp = tmp->above)
        {
            if (!QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE) && get_friendship(who, tmp) <= FRIENDSHIP_ATTACK)
                return 1;
        }
    }
    return 0;
}

/* check the player los field for viewability of the
 * object op. This function works fine for monsters,
 * but we dont worry if the object isnt the top one in
 * a pile (say a coin under a table would return "viewable"
 * by this routine). Another question, should we be
 * concerned with the direction the player is looking
 * in? Realistically, most of use cant see stuff behind
 * our backs...on the other hand, does the "facing" direction
 * imply the way your head, or body is facing? Its possible
 * for them to differ. Sigh, this fctn could get a bit more complex.
 * -b.t.
 * This function is now map tiling safe.
 */

int player_can_view(object *pl, object *op)
{
    rv_vector   rv;
    int         dx, dy;

    if (pl->type != PLAYER)
    {
        LOG(llevBug, "BUG: player_can_view() called for non-player object\n");
        return -1;
    }
    if (!pl || !op)
        return 0;

    if (op->head)
    {
        op = op->head;
    }
    get_rangevector(pl, op, &rv, 0x1);

    /* starting with the 'head' part, lets loop
     * through the object and find if it has any
     * part that is in the los array but isnt on
     * a blocked los square.
     * we use the archetype to figure out offsets.
     */
    while (op)
    {
        dx = rv.distance_x + op->arch->clone.x;
        dy = rv.distance_y + op->arch->clone.y;

        /* only the viewable area the player sees is updated by LOS
         * code, so we need to restrict ourselves to that range of values
         * for any meaningful values.
         */
        if (FABS(dx)
         <= (CONTR(pl)->socket.mapx_2)
         && FABS(dy)
         <= (CONTR(pl)->socket.mapy_2)
         && CONTR(pl)->blocked_los[dx + (CONTR(pl)->socket.mapx_2)][dy + (CONTR(pl)->socket.mapy_2)]
         <= BLOCKED_LOS_BLOCKSVIEW)
            return 1;
        op = op->more;
    }
    return 0;
}

/* routine for both players and monsters. We call this when
 * there is a possibility for our action distrubing our hiding
 * place or invisiblity spell. Artefact invisiblity is not
 * effected by this. If we arent invisible to begin with, we
 * return 0.
 */
int action_makes_visible(object *op)
{
    /*
      if(QUERY_FLAG(op,FLAG_IS_INVISIBLE) && QUERY_FLAG(op,FLAG_ALIVE)) {
        if(!QUERY_FLAG(op,FLAG_SEE_INVISIBLE))
          return 0;
        else if(op->hide) {
          new_draw_info(NDI_UNIQUE, 0,op,"You become %!",op->hide?"unhidden":"visible");
          return 1;
        } else if(CONTR(op) && !CONTR(op)->shottype==range_magic) {
              new_draw_info(NDI_UNIQUE, 0,op,"Your invisibility spell is broken!");
              return 1;
        }
      }
    */
    return 0;
}

/* test for pvp area.
 * if only one opject is given, it test for it.
 * if 2 objects given, both player must be in pvp or
 * the function fails.
 * this function use map and x/y from the player object -
 * be sure player are valid and on map.
 * RETURN: FALSE = no pvp, TRUE= pvp possible
 */
int pvp_area(object *attacker, object *victim)
{
    if (attacker && attacker->map)
    {
/* Not positive what the intention behind both of these checks is, and the latter doesn't work anyway.
 * So I'll comment them out for now and put my own in. :) -- _people_
 */

/*
      if (!(attacker->map->map_flags & MAP_FLAG_PVP)) ||
            !(GET_MAP_FLAGS(attacker->map, attacker->x, attacker->y) & P_IS_PVP))
*/
        if (!MAP_PVP(attacker->map))
            return FALSE;
    }
    if (victim && victim->map)
    {
/*
        if (!(victim->map->map_flags & MAP_FLAG_PVP) || !(GET_MAP_FLAGS(victim->map, victim->x, victim->y) & P_IS_PVP))
*/
        if (!MAP_PVP(victim->map))
            return FALSE;
    }

    return TRUE;
}

/* op_on_battleground - checks if the given object op (usually
 * a player) is standing on a valid battleground-tile,
 * function returns TRUE/FALSE. If true x, y returns the battleground
 * -exit-coord. (and if x, y not NULL)
 */
/* TODO: sigh, this must be changed! we don't want loop tile objects in move/attack
 * or other action without any need.
*/
int op_on_battleground(object *op, int *x, int *y)
{
    /*object *tmp;*/

    /* A battleground-tile needs the following attributes to be valid:
     * is_floor 1 (has to be the FIRST floor beneath the player's feet),
     * name="battleground", no_pick 1, type=58 (type BATTLEGROUND)
     * and the exit-coordinates sp/hp must both be > 0.
     * => The intention here is to prevent abuse of the battleground-
     * feature (like pickable or hidden battleground tiles). */
/* floors are now in map node - this needs a new logic */
/*
    for (tmp = op->below; tmp != NULL; tmp = tmp->below)
    {
        if (tmp->type == FLOOR)
        {
            if (QUERY_FLAG(tmp, FLAG_NO_PICK)
             && tmp->name == shstr_cons.battleground
             && tmp->type == BATTLEGROUND
             && EXIT_X(tmp)
             && EXIT_Y(tmp))
            {
                if (x != NULL && y != NULL)
                    *x = EXIT_X(tmp), * y = EXIT_Y(tmp);
                return 1;
            }
        }
    }
*/
    /* If we got here, did not find a battleground */
    return 0;
}

/*
 * When a dragon-player gains a new stage of evolution,
 * he gets some treasure
 *
 * attributes:
 *      object *who        the dragon player
 *      int atnr           the attack-number of the ability focus
 *      int level          ability level
 */
void dragon_ability_gain(object *who, int atnr, int level)
{
    treasurelist   *trlist  = NULL;   /* treasurelist */
    treasure       *tr;                  /* treasure */
    object         *tmp;                   /* tmp. object */
    object         *item;                  /* treasure object */
    char            buf[MEDIUM_BUF];             /* tmp. string buffer */
    int             i = 0, j = 0;

    /* get the appropriate treasurelist */
    if (atnr == ATNR_FIRE)
        trlist = find_treasurelist("dragon_ability_fire");
    else if (atnr == ATNR_COLD)
        trlist = find_treasurelist("dragon_ability_cold");
    else if (atnr == ATNR_ELECTRICITY)
        trlist = find_treasurelist("dragon_ability_elec");
    else if (atnr == ATNR_POISON)
        trlist = find_treasurelist("dragon_ability_poison");

    if (trlist == NULL || who->type != PLAYER)
        return;

    for (i = 0, tr = trlist->items; tr != NULL && i < level - 1; tr = tr->next, i++)
        ;

    if (tr == NULL || tr->item == NULL)
    {
        /* printf("-> no more treasure for %s\n", change_resist_msg[atnr]); */
        return;
    }

    /* everything seems okay - now bring on the gift: */
    item = &(tr->item->clone);

    /* grant direct spell */
    if (item->type == SPELLBOOK)
    {
        int spell = look_up_spell_name(item->slaying);
        if (spell >= 0 &&
            !check_spell_known(who, spell))
        {
            new_draw_info(NDI_UNIQUE | NDI_BLUE, 0, who, "You gained the ability of %s", spells[spell].name);
            do_learn_spell(who, spell, 0);

            return;
        }
    }
    else if (item->type == FORCE)
    {
        /* forces in the treasurelist can alter the player's stats */
        object *skin;
        /* first get the dragon skin force */
        for (skin = who->inv; skin != NULL && skin->arch->name != shstr_cons.dragon_skin_force; skin = skin->below)
            ;
        if (skin == NULL)
            return;

        /* adding new spellpath attunements */
        if (item->path_attuned > 0 && !(skin->path_attuned & item->path_attuned))
        {
            skin->path_attuned |= item->path_attuned; /* add attunement to skin */

            /* print message */
            sprintf(buf, "You feel attuned to ");
            for (i = 0, j = 0; i < NRSPELLPATHS; i++)
            {
                if (item->path_attuned & (1 << i))
                {
                    if (j)
                        strcat(buf, " and ");
                    else
                        j = 1;
                    strcat(buf, spellpathnames[i]);
                }
            }
            strcat(buf, ".");
            new_draw_info(NDI_UNIQUE | NDI_BLUE, 0, who, "%s", buf);
        }

        /* evtl. adding flags: */
        if (QUERY_FLAG(item, FLAG_XRAYS))
            SET_FLAG(skin, FLAG_XRAYS);
        if (QUERY_FLAG(item, FLAG_STEALTH))
            SET_FLAG(skin, FLAG_STEALTH);
        if (QUERY_FLAG(item, FLAG_SEE_IN_DARK))
            SET_FLAG(skin, FLAG_SEE_IN_DARK);

        /* print message if there is one */
        if (item->msg != NULL)
            new_draw_info(NDI_UNIQUE | NDI_BLUE, 0, who, "%s", item->msg);
    }
    else
    {
        /* generate misc. treasure */
        tmp = arch_to_object(tr->item);
        new_draw_info(NDI_UNIQUE | NDI_BLUE, 0, who, "You gained %s", query_name(tmp));
        tmp = insert_ob_in_ob(tmp, who);
    }
}



/* Determine if the attacktype represented by the
 * specified attack-number is enabled for dragon players.
 * A dragon player (quetzal) can gain resistances for
 * all enabled attacktypes.
 */
int atnr_is_dragon_enabled(int attacknr)
{
    if (attacknr == ATNR_CHANNELLING
     || attacknr == ATNR_FIRE
     || attacknr == ATNR_ELECTRICITY
     || attacknr == ATNR_COLD
     || attacknr == ATNR_ACID
     || attacknr == ATNR_POISON)
        return 1;
    return 0;
}

/*
 * returns true if the adressed object 'ob' is a player
 * of the dragon race.
 */
int is_dragon_pl(object *op)
{
    if (op != NULL
     && op->type == PLAYER
     && op->arch != NULL
     && op->arch->clone.race != NULL
     && op->arch->clone.race == shstr_cons.dragon)
        return 1;
    return 0;
}

/* we reset the instance information of the player */
void reset_instance_data(player *pl)
{
    if(pl)
    {
        pl->instance_flags = 0;
        pl->instance_num = MAP_INSTANCE_NUM_INVALID;
        FREE_AND_CLEAR_HASH(pl->instance_name);
    }
}

/* kick_player(NULL) global kicks *all* players.
 * kick_player(player) kicks the specific player.
 */
void kick_player(player *pl)
{
    player *tmp;

    for (tmp = first_player; tmp; tmp = tmp->next)
    {
        if (!pl ||
            tmp == pl)
        {
            /* Save the player. */
            PLAYER_SAVE(tmp);

            /* Kick the player. */
            activelist_remove(tmp->ob);
            /* remove_ob(tmp->ob); */
            check_walk_off(tmp->ob, NULL, MOVE_APPLY_VANISHED);
            tmp->ob->direction = 0;
            LOG(llevInfo, "%s is kicked out of the game.\n",
                STRING_OBJ_NAME(tmp->ob));
            container_unlink(tmp, NULL);
            tmp->socket.status = Ns_Dead;

            /* Just one player to kick? Leave now that it's done. */
            if (pl)
                return;
        }
    }
}

/* This is called in five circumstances: when a client pings the server; when a
 * player types /who; when a player enters the game; when a player leaves the
 * game; when a player toggles privacy mode.
 *
 * The function maintains 3 static buffers: one for pings; one for gmasters
 * (SA/GM/VOL); one for other players. A call may clear all buffers or rewrite
 * one, and may return one buffer or NULL. 
 *
 * If the force flag is non-zero, all buffers are reset before any further
 * action.
 *
 * If diff is non-NULL (usually means a player has entered or left the game),
 * all buffers are reset and NULL is returned.
 *
 * If both who and diff are NULL, the ping buffer is used. If who is non-NULL
 * and is a gmaster, the gmaster buffer is used. Otherwise, the normal buffer
 * is used.
 *
 * If the buffer is empty, it is rewritten and then returned. Otherwise, the
 * existing buffer is returned.
 *
 * As the info is rewritten less frequently than before (when the entire string
 * was recreated every time anyone typed /who), data such as players' levels
 * and what map they are on is not included as this changes frequently. */
char *get_online_players_info(player *who, player *diff, uint8 force)
{
    player      *pl;
    uint16       pri = 0;
    char        *buf;
    static char  buf_ping[HUGE_BUF] = "",
                 buf_gmaster[HUGE_BUF] = "",
                 buf_normal[HUGE_BUF] = "";

    LOG(llevInfo, "INFO:: get_online_players_info was called and ");

    /* When force is non-zero or a player enters or leaves the game, reset both
     * buffers before doing anything else. */
    if (force ||
        diff)
    {
        LOG(llevInfo, "all buffers were reset");
        buf_ping[0] = '\0';
        buf_gmaster[0] = '\0';
        buf_normal[0] = '\0';

        /* When a player enters or leaves the game, return NULL. */
        if (diff)
        {
            LOG(llevInfo, ".\n");

            return NULL;
        }
        else
        {
            LOG(llevInfo, " then ");
        }
    }

    /* We decide which buffer to potentially write to. */
    if (!who &&
        !diff)
    {
        LOG(llevInfo, "the existing ping buffer was ");
        buf = buf_ping;
    }
    else if (who &&
             (who->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM | GMASTER_MODE_VOL)))
    {
        LOG(llevInfo, "the existing gmaster buffer was ");
        buf = buf_gmaster;
    }
    else
    {
        LOG(llevInfo, "the existing normal buffer was ");
        buf = buf_normal;
    }

    /* We decide if writing is necessary at all -- if there is something in the
     * buffer, just return it. */
    if (*buf)
    {
        LOG(llevInfo, "returned.\n");

        return (char *)buf;
    }

    /* Lets rewrite the buffer. */
    LOG(llevInfo, "rewritten.\n");

    /* Begin string with time of rewrite and player numbers in hex. */
    sprintf(buf, "%lx %x ", ROUND_TAG, player_active);

    for (pl = first_player; pl; pl = pl->next)
    {
        /* Player is in login? Skip to the next. */
        if (!pl->ob->map)
        {
            continue;
        }

        /* Player has requested privacy? Increment pri and if the normal buffer
         * is being written or the player is a SA (who can hiide even from
         * other gmasters), skip to the next. Prior to 0.10.6 other SAs could
         * see privacy-seeking SAs. Currently this is not possible. */
        if (pl->privacy)
        {
            pri++;

            if (buf == buf_normal ||
                (pl->gmaster_mode & GMASTER_MODE_SA))
            {
                continue;
            }
        }

        if ((pl->state & ST_PLAYING))
        {
            /* To conserve bandwidth, just send essentials. */
            /* TODO: In 0.11.0 when FILE_CLIENT_SETTINGS is sorted out we will
             * send a number for the race too. */
            if (buf == buf_ping)
            {
                sprintf(strchr(buf, '\0'), "%s %u %s %d %d\n",
                        pl->quick_name,
                        (QUERY_FLAG(pl->ob, FLAG_IS_MALE))
                        ? ((QUERY_FLAG(pl->ob, FLAG_IS_FEMALE)) ? 2 : 0)
                        : ((QUERY_FLAG(pl->ob, FLAG_IS_FEMALE)) ? 1 : 3),
                        pl->ob->race, pl->socket.lx, pl->socket.ly);
            }
            /* Here we make things a bit prettier. */ 
            else
            {
                sprintf(strchr(buf, '\0'), "~%s~ the %s %s",
                        pl->quick_name,
                        (QUERY_FLAG(pl->ob, FLAG_IS_MALE))
                        ? ((QUERY_FLAG(pl->ob, FLAG_IS_FEMALE)) ? "hermaphrodite"
                                                                : "male")
                        : ((QUERY_FLAG(pl->ob, FLAG_IS_FEMALE)) ? "female"
                                                                : "neuter"),
                        pl->ob->race);

                if (buf == buf_gmaster)
                {
                    sprintf(strchr(buf, '\0'), "%s\n  ~IP~: %s\n  ~Account~: %s",
                            (pl->privacy) ? " ~Privacy mode~" : "",
                            pl->socket.ip_host, pl->account_name);
                }

                strcat(buf, "\n");
            }
        }
    }

    /* Add a short string if anyone has requested privacy and this is the
     * normal buffer (the gmaster buffer already has the info). */
    if (pri > 0 &&
        buf == buf_normal)
    {
        sprintf(strchr(buf, '\0'), "%u player%s seek%s privacy.",
                pri, (pri == 1) ? "" : "s", (pri == 1) ? "s" : "");
    }

    return (char *)buf;
}

/* 
 * Keep track of certain stats for a player's PvP career. Stats are stored in a pvp_stat_force (see archetype_global).
 * The stats and their respective attributes are as follows:
 * Total kills - maxhp
 * Total deaths - hp
 * Kills in this round - maxsp
 * Deaths in this round - sp
 * The logic behind these is that the total kills should end up higher than round kills (even round kills can get over 65k).
 */
void increment_pvp_counter(object *op, int counter)
{
    object *pvp_force;

    // Probably not needed, but usually won't hurt.
    if (!op)
        return;

    if (!(pvp_force = present_arch_in_ob(archetype_global._pvp_stat_force, op)))
        // The PvP stat-tracking force doesn't exist in op's inv, so create it.
        pvp_force = insert_ob_in_ob(arch_to_object(archetype_global._pvp_stat_force), op);

    if (!pvp_force)
        return;

    if (counter & PVP_STATFLAG_KILLS_TOTAL)
        pvp_force->stats.maxhp++;

    if (counter & PVP_STATFLAG_KILLS_ROUND)
        pvp_force->stats.maxsp++;

    /* In theory, we can't have DEATH and KILL flags in the same instance of this function. But I'll give room for
     * clever MW's and MM's to thwart my theory. */
    if (counter & PVP_STATFLAG_DEATH_TOTAL)
        pvp_force->stats.hp++;

    if (counter & PVP_STATFLAG_DEATH_ROUND)
        pvp_force->stats.sp++;
}

int command_pvp_stats(object *op, char *params)
{
    char       *name = params;
    const char *name_hash;
    player     *pl;

    // Query op since no-one else was specified.
    if (!name)
    {
        object *pvp_force = present_arch_in_ob(archetype_global._pvp_stat_force, op);

        if (pvp_force)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You  have killed ~%u~ player%s in PvP. You have been killed by a player in PvP ~%u~ time%s.",
                                         pvp_force->stats.maxhp, pvp_force->stats.maxhp != 1 ? "s":"",
                                         pvp_force->stats.hp, pvp_force->stats.hp != 1 ? "s":"");
        }
        else
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You have not participated in any PvP.");
        }

        return 0;
    }

    // Make sure the specified character is logged in.
    transform_player_name_string(name);

    if (!(name_hash = find_string(name)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");
        return 0;
    }

    for (pl = first_player; pl != NULL; pl = pl->next)
    {
        if (pl->ob->name == name_hash)
        {
            object *pvp_force = present_arch_in_ob(archetype_global._pvp_stat_force, pl->ob);

            if (pvp_force)
            {
                new_draw_info(NDI_UNIQUE, 0, op, "Player: ~%s~\nTotal PvP kills: ~%u~\nTotal PvP deaths: ~%u~",
                                                  query_name(pl->ob), pvp_force->stats.maxhp, pvp_force->stats.hp);
            }
            else
            {
                new_draw_info(NDI_UNIQUE, 0, op, "That player has not participated in PvP.");
            }

            return 0;
        }
    }

    new_draw_info(NDI_UNIQUE, 0, op, "No such player.");

    return 0;
}

