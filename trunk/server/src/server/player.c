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

/* find a player name for a NORMAL string.
 * we use the hash table system.
 */
player_t * find_player(char *plname)
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
player_t * find_player_hash(const char *plname)
{
    player_t *pl;

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


void display_motd(object_t *op)
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
        ndi(NDI_UNIQUE, 0, op, "%s", buf);
    }
    fclose(fp);
    ndi(NDI_UNIQUE, 0, op, " ");
#endif
}

void free_player(player_t *pl)
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
        insert_ob_in_ob(pl->ob, &void_container); /* Avoid gc of the player object_t */
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
void destroy_player_struct(player_t *pl)
{
    /* clear all hash strings */
    FREE_AND_CLEAR_HASH(pl->instance_name);
    FREE_AND_CLEAR_HASH(pl->group_invite_name);
    FREE_AND_CLEAR_HASH(pl->savebed_map );
    FREE_AND_CLEAR_HASH(pl->orig_savebed_map);
    FREE_AND_CLEAR_HASH(pl->maplevel );
    FREE_AND_CLEAR_HASH(pl->orig_map);
    FREE_AND_CLEAR_HASH(pl->account_name);
}

void give_initial_items(object_t *pl, struct objectlink_t *items)
{
    object_t *op,
           *next;

    if (pl->randomitems != NULL)
        create_treasure_list(items, pl, GT_ONLY_GOOD | GT_NO_VALUE, 1, ART_CHANCE_UNSET, 0);

    FOREACH_OBJECT_IN_OBJECT(op, pl, next)
    {
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

            /* WARNING: we force here the flag "applied" without calling apply_object().
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

void flee_player(object_t *op)
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
        int m = 1 - (RANDOM() & 2);

        if (move_ob(op, absdir(dir + diff * m), NULL) != MOVE_RESULT_INSERTION_FAILED ||
            (diff == 0 &&
             move_ob(op, absdir(dir - diff * m), NULL) != MOVE_RESULT_INSERTION_FAILED))
        {
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
int move_player(object_t * const op, int dir, const int flag)
{
    player_t *pl = CONTR(op);

    pl->rest_sitting = pl->rest_mode = 0;

    if (op->map == NULL || op->map->in_memory != MAP_MEMORY_ACTIVE ||
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

    if (flag)
    {
        if (move_ob(op, dir, NULL) != MOVE_RESULT_SUCCESS)
        {
            op->anim_enemy_dir = dir;
        }
        else
        {
            op->anim_moving_dir = dir;
        }

        if (op->anim_enemy_dir == -1 &&
            op->anim_moving_dir == -1)
        {
            op->anim_last_facing = dir;
        }

        animate_object(op, 0);
    }

    return dir;
}

/* regeneration helper functions - cleaner as a macro */
static inline void do_reg_hp(player_t *pl, object_t *op)
{
    if(op->stats.hp < op->stats.maxhp)
    {
        op->stats.hp += pl->reg_hp_num;
        if(op->stats.hp > op->stats.maxhp)
            op->stats.hp = op->stats.maxhp;
    }
}
static inline void do_reg_sp(player_t *pl, object_t *op)
{
    if(op->stats.sp < op->stats.maxsp)
    {
        op->stats.sp += pl->reg_sp_num;
        if(op->stats.sp > op->stats.maxsp)
            op->stats.sp = op->stats.maxsp;
    }
}
static inline void do_reg_grace(player_t *pl, object_t *op)
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
void do_some_living(object_t *op)
{
    player_t *pl = CONTR(op);

    if (!pl || !(pl->state & ST_PLAYING))
        return;

    /* sanity kill check */
    if (op->stats.hp <= 0)
    {
        ndi(NDI_UNIQUE, 0, op, "You died by low hitpoints!");
        kill_object(op, NULL, NULL, NULL);
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
                 /*ndi(NDI_UNIQUE, 0, op, "reg - combat sp/gr");*/
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
                    /*ndi(NDI_UNIQUE, 0, op, "reg - prepare %d", pl->resting_reg_timer);*/
                    --pl->resting_reg_timer; /* player is still in rest preparing phase */
                    pl->food_status = (1000/RESTING_DEFAULT_SEC_TIMER)*(pl->resting_reg_timer+1);
                }
                else /* all ok - we rest and regenerate with full speed */
                {
                    /*ndi(NDI_UNIQUE, 0, op, "reg - full rest");*/
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
                    /*ndi(NDI_UNIQUE, 0, op, "reg - normal tick");*/
                    pl->normal_reg_timer = REG_DEFAULT_SEC_TIMER;
                    do_reg_hp(pl, op);
                    do_reg_sp(pl, op);
                    do_reg_grace(pl, op);
                }
            }
        }
    }
}

/* cast_dust() - handles op throwing objects of type 'DUST' */
/* WARNING: FUNCTION NEED TO BE REWRITTEN. works for ae spells only now! */
void cast_dust(object_t *op, object_t *throw_ob, int dir)
{
    archetype_t *arch = NULL;

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
        ndi(NDI_UNIQUE, 0, op, "You cast %s.",
            QUERY_SHORT_NAME(throw_ob, op));

    if (!QUERY_FLAG(throw_ob, FLAG_REMOVED))
    {
        remove_ob(throw_ob);
        check_walk_off(throw_ob, NULL, MOVE_APPLY_DEFAULT);
    }
}

void make_visible(object_t *op)
{
    /*
       if(op->type==PLAYER)
       if(QUERY_FLAG(op, FLAG_UNDEAD)&&!is_true_undead(op))
         CLEAR_FLAG(op, FLAG_UNDEAD);
       update_object(op,UP_OBJ_FACE);
    */
}

int is_true_undead(object_t *op)
{
    /*
      object_t *tmp=NULL;

      if(QUERY_FLAG(&op->arch->clone,FLAG_UNDEAD)) return 1;

      if(op->type==PLAYER)
        FOREACH_OBJECT_IN_OBJECT(tmp, op)
           if(tmp->type==TYPE_SKILLGROUP && tmp->stats.Wis)
          if(QUERY_FLAG(tmp,FLAG_UNDEAD)) return 1;
    */
    return 0;
}

/* look at the surrounding terrain to determine
 * the hideability of this object. Positive levels
 * indicate greater hideability.
 */

int hideability(object_t *ob)
{
#if 0
  int i,x,y,level=0;

  if(!ob||!ob->map) return 0;


  /* scan through all nearby squares for terrain to hide in */
  for(i=0,x=ob->x,y=ob->y;i<9;i++,x=ob->x+OVERLAY_X(i),y=ob->y+OVERLAY_Y(i))
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

void do_hidden_move(object_t *op)
{
    int hide = 0, num = random_roll(0, 19);

    if (!op || !op->map)
        return;

    /* its *extremely* hard to run and sneak/hide at the same time! */
    if (op->type == PLAYER && CONTR(op)->run_on)
    {
        if (num >= SK_level(op))
        {
            ndi(NDI_UNIQUE, 0, op, "You ran too much! You are no longer hidden!");
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
            ndi(NDI_UNIQUE, 0, op, "You moved out of hiding! You are visible!");
    }
}

/* determine if who is standing near a hostile creature. */
int stand_near_hostile(object_t *who)
{
    uint8 i;

    if (!who)
    {
        return 0;
    }

    /* search adjacent squares */
    for (i = 1; i < 9; i++)
    {
        map_t *m = who->map;
        sint16     x = who->x + OVERLAY_X(i),
                   y = who->y + OVERLAY_Y(i);
        msp_t  *msp = MSP_GET2(m, x, y);
        object_t    *this,
                  *next;

        if (!msp)
        {
            continue;
        }

        FOREACH_OBJECT_IN_MSP(this, msp, next)
        {
            if (!QUERY_FLAG(this, FLAG_UNAGGRESSIVE) &&
                get_friendship(who, this) <= FRIENDSHIP_ATTACK)
            {
                return 1;
            }
        }
    }

    return 0;
}

/* routine for both players and monsters. We call this when
 * there is a possibility for our action distrubing our hiding
 * place or invisiblity spell. Artefact invisiblity is not
 * effected by this. If we arent invisible to begin with, we
 * return 0.
 */
int action_makes_visible(object_t *op)
{
    /*
      if(QUERY_FLAG(op,FLAG_IS_INVISIBLE) && QUERY_FLAG(op,FLAG_ALIVE)) {
        if(!QUERY_FLAG(op,FLAG_SEE_INVISIBLE))
          return 0;
        else if(op->hide) {
          ndi(NDI_UNIQUE, 0,op,"You become %!",op->hide?"unhidden":"visible");
          return 1;
        } else if(CONTR(op) && !CONTR(op)->shottype==range_magic) {
              ndi(NDI_UNIQUE, 0,op,"Your invisibility spell is broken!");
              return 1;
        }
      }
    */
    return 0;
}

/* we reset the instance information of the player */
void reset_instance_data(player_t *pl)
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
void kick_player(player_t *pl)
{
    player_t *tmp;

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
char *get_online_players_info(player_t *who, player_t *diff, uint8 force)
{
    player_t      *pl;
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
    sprintf(buf, "%x %x ", ROUND_TAG, player_active);

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
                sprintf(strchr(buf, '\0'), "|%s| the %s %s",
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
void increment_pvp_counter(object_t *op, int counter)
{
    object_t *pvp_force;

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

int command_pvp_stats(object_t *op, char *params)
{
    char       *name = params;
    const char *name_hash;
    player_t     *pl;

    // Query op since no-one else was specified.
    if (!name)
    {
        object_t *pvp_force = present_arch_in_ob(archetype_global._pvp_stat_force, op);

        if (pvp_force)
        {
            ndi(NDI_UNIQUE, 0, op, "You  have killed ~%u~ player%s in PvP. You have been killed by a player in PvP ~%u~ time%s.",
                                         pvp_force->stats.maxhp, pvp_force->stats.maxhp != 1 ? "s":"",
                                         pvp_force->stats.hp, pvp_force->stats.hp != 1 ? "s":"");
        }
        else
        {
            ndi(NDI_UNIQUE, 0, op, "You have not participated in any PvP.");
        }

        return 0;
    }

    // Make sure the specified character is logged in.
    transform_player_name_string(name);

    if (!(name_hash = find_string(name)))
    {
        ndi(NDI_UNIQUE, 0, op, "No such player.");
        return 0;
    }

    for (pl = first_player; pl != NULL; pl = pl->next)
    {
        if (pl->ob->name == name_hash)
        {
            object_t *pvp_force = present_arch_in_ob(archetype_global._pvp_stat_force, pl->ob);

            if (pvp_force)
            {
                ndi(NDI_UNIQUE, 0, op, "Player: %s\nTotal PvP kills: ~%u~\nTotal PvP deaths: ~%u~",
                    QUERY_SHORT_NAME(pl->ob, NULL), pvp_force->stats.maxhp, pvp_force->stats.hp);
            }
            else
            {
                ndi(NDI_UNIQUE, 0, op, "That player has not participated in PvP.");
            }

            return 0;
        }
    }

    ndi(NDI_UNIQUE, 0, op, "No such player.");

    return 0;
}

