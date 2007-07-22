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

    The author can be reached via e-mail to daimonin@nord-com.net
*/

#include <global.h>
#ifndef WIN32 /* ---win32 remove headers */
#include <pwd.h>
#endif

/* find a player name for a NORMAL string.
 * we use the hash table system.
 */
player * find_player(char *plname)
{
    char name[MAX_PLAYER_NAME > (16 - 1) ? MAX_PLAYER_NAME + 1 : 16];
    const char *name_hash;

    int         name_len                                                = strlen(plname); /* we assume a legal string */
    if (name_len <= 1 || name_len > MAX_PLAYER_NAME)
        return NULL;

    strcpy(name, plname); /* we need to copy it because we access the string */
    transform_name_string(name);
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
    char    buf[MAX_BUF];
    FILE   *fp;

    sprintf(buf, "%s/%s", settings.localdir, MOTD);
    if ((fp = fopen(buf,"r")) == NULL)
    {
        return;
    }
    while (fgets(buf, MAX_BUF, fp) != NULL)
    {
        char   *cp;
        if (*buf == '#')
            continue;
        cp = strchr(buf, '\n');
        if (cp != NULL)
            *cp = '\0';
        new_draw_info(NDI_UNIQUE, 0, op, buf);
    }
    fclose(fp);
    new_draw_info(NDI_UNIQUE, 0, op, " ");
#endif
}

int playername_ok(char *cp)
{
    char *tmp=cp;

    if(*cp==' ') /* we start with a whitespace? in this case we don't trim - kick*/
        return 0;

    for(;*cp!='\0';cp++)
    {
        if(!((*cp>='a'&&*cp<='z')||(*cp>='A'&&*cp<='Z'))&&*cp!='-'&&*cp!='_')
            return 0;
    }

    /* we don't want some special names & keywords here. */
    if(!strcasecmp(tmp,"on") || !strcasecmp(tmp,"off") || !strcasecmp(tmp,"allow"))
        return 0;
    return 1;
}

/* Redo this to do both get_player_ob and get_player.
 * Hopefully this will be less bugfree and simpler.
 * Returns the player structure.  If 'p' is null,
 * we create a new one.  Otherwise, we recycle
 * the one that is passed.
 */
player *get_player(player *p)
{
    static archetype *p_arch = NULL;
    object *op  = NULL;
    int     i;

    /* we need this to assign a "standard player object" - at this
     * moment a player is login in we have not loaded the player file
     * and no idea about the true object
     */
    if(!p_arch)
        p_arch = find_archetype("human_male");

    if (!p)
    {
        p = (player *) get_poolchunk(pool_player);
        memset(p, 0, sizeof(player));
        if (p == NULL)
            LOG(llevError, "ERROR: get_player(): out of memory\n");

        player_active++; /* increase player count */
        if(player_active_meta < player_active)
            player_active_meta = player_active;

        if (!last_player)
            first_player = last_player = p;
        else
        {
            last_player->next = p;
            p->prev = last_player;
            last_player = p;
        }
    }
    else
    {
        /* Clears basically the entire player structure except
         * for next and socket.
         */
        memset((void *) ((char *) p + offsetof(player, maplevel)), 0, sizeof(player) - offsetof(player, maplevel));
    }

    /* There are some elements we want initialized to non zero value -
     * we deal with that below this point.
     */
    p->group_id = GROUP_NO;

#ifdef AUTOSAVE
    p->last_save_tick = 9999999;
#endif

    p->gmaster_mode = GMASTER_MODE_NO;
    p->gmaster_node = NULL;
    p->mute_freq_shout=0;
    p->mute_freq_say=0;
    p->mute_counter=0;
    p->mute_msg_count=0;

    op  = arch_to_object(p_arch);
    op->custom_attrset = p; /* this is where we set up initial CONTR(op) */
    p->ob = op;
    op->speed_left = 0.5;
    op->speed = 1.0;
    op->direction = 5;     /* So player faces south */
    /* i let it in but there is no use atm for run_away and player */
    op->run_away = 0; /* Then we panick... */

    p->state = ST_CREATE_CHAR;

    p->target_hp = -1;
    p->target_hp_p = -1;
    p->listening = 9;
    p->last_weapon_sp = -1;
    p->last_speed_enc = 0;
    p->last_spell_fumble = 0;
    p->update_los = 1;

    FREE_AND_COPY_HASH(op->race, op->arch->clone.race);

    /* Would be better of '0' was not a defined spell */
    for (i = 0; i < NROFREALSPELLS; i++)
        p->known_spells[i] = -1;

    /* we need to clear these to -1 and not zero - otherwise,
     * if a player quits and starts a new character, we wont
     * send new values to the client, as things like exp start
     * at zero.
     */
    for (i = 0; i < NROFSKILLGROUPS; i++)
    {
        p->exp_obj_ptr[i] = NULL;
        p->last_exp_obj_exp[i] = -1;
        p->last_exp_obj_level[i] = -1;
    }

    p->set_skill_weapon = NO_SKILL_READY; /* quick skill reminder for select hand weapon */
    p->set_skill_archery = NO_SKILL_READY;
    p->last_stats.exp = -1;

    return p;
}

void free_player(player *pl)
{
    /* first, we removing the player from map or whatever */
	LOG(llevDebug, "FREE_PLAYER(%s): state:%d g_status:%x\n", query_name(pl->ob), pl->state, pl->group_status);
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

    if(pl->state == ST_ZOMBIE)
    {
        pl->state = ST_DEAD;
		LOG(llevDebug, "FREE_PLAYER(%s) --> ST_DEAD\n", query_name(pl->ob));
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

    /* clear all hash strings */
    FREE_AND_CLEAR_HASH(pl->instance_name);
    FREE_AND_CLEAR_HASH(pl->group_invite_name);
    FREE_AND_CLEAR_HASH(pl->killer);
    FREE_AND_CLEAR_HASH(pl->savebed_map );
    FREE_AND_CLEAR_HASH(pl->orig_savebed_map);
    FREE_AND_CLEAR_HASH(pl->maplevel );
    FREE_AND_CLEAR_HASH(pl->orig_map);

    if(pl->socket.status != ST_SOCKET_NO)
        free_newsocket(&pl->socket);

    if (pl->ob)
    {
        if (!QUERY_FLAG(pl->ob, FLAG_REMOVED))
            remove_ob(pl->ob);

        /* Force an out-of-loop gc to delete the player object NOW */
        object_gc();
    }
}

/* Tries to add player on the connection passwd in ns.
 * All we can really get in this is some settings like host and display
 * mode.
 */

player *add_player(NewSocket *ns)
{
    player *p;

    p = get_player(NULL);
    if(!p)
        return NULL;

    memcpy(&p->socket, ns, sizeof(NewSocket));
    /* Needed because the socket we just copied over needs to be cleared.
     * Note that this can result in a client reset if there is partial data
     * on the uncoming socket.
     */
    p->socket.pl = p;
    p->socket.status = Ns_Login; /* now, we start the login procedure! */
    p->socket.below_clear = 0;
    p->socket.update_tile = 0;
    p->socket.look_position = 0;

    start_info(p->ob);
    get_name(p->ob, 0); /* start a nice & clean login */

    insert_ob_in_ob(p->ob, &void_container); /* Avoid gc of the player */

    return p;
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
            manual_apply(pl, op,0);
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


/*
 * value can be:
 * 0= name is not taken, no player with that name in the system
 * 1= name is blocked and login to this name is not allowed (is in creating or system use)
 * 2= name is taken and its logged in right now
 * 3= name is taken and banned
 * 4= name is taken and not logged in
 * 5= illegal password
 * 6= verify password don't match
 */
void get_name(object *op, int value)
{
    char buf[8];

    sprintf(buf, "QN%d", value);
    CONTR(op)->state = ST_GET_NAME;
    send_query(&CONTR(op)->socket, CS_QUERY_HIDEINPUT, buf);
}

/* if we are here, the login name is valid */
void get_password(object *op, int value)
{
    char buf[8];

    sprintf(buf, "QP%d", value);
    CONTR(op)->state = ST_GET_PASSWORD;
    send_query(&CONTR(op)->socket, CS_QUERY_HIDEINPUT, buf);
}

void confirm_password(object *op, int value)
{
    char buf[8];

    sprintf(buf, "QV%d", value);
    CONTR(op)->state = ST_CONFIRM_PASSWORD;
    send_query(&CONTR(op)->socket, CS_QUERY_HIDEINPUT, buf);
}


void flee_player(object *op)
{
    int dir, diff;
    if (op->stats.hp < 0)
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

    if (op->map == NULL || op->map->in_memory != MAP_IN_MEMORY ||
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

    HandleClient(&pl->socket, pl);

    if (!op || !OBJECT_ACTIVE(op) || pl->socket.status == Ns_Dead)
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
    char    buf[MAX_BUF];
    if (!QUERY_FLAG(op, FLAG_LIFESAVE))
        return 0;
    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
        if (QUERY_FLAG(tmp, FLAG_APPLIED) && QUERY_FLAG(tmp, FLAG_LIFESAVE))
        {
            play_sound_map(op->map, op->x, op->y, SOUND_OB_EVAPORATE, SOUND_NORMAL);
            sprintf(buf, "Your %s vibrates violently, then evaporates.", query_name(tmp));
            new_draw_info(NDI_UNIQUE, 0, op, buf);
            if (CONTR(op))
                esrv_del_item(CONTR(op), tmp->count, tmp->env);
            remove_ob(tmp);
            CLEAR_FLAG(op, FLAG_LIFESAVE);
            if (op->stats.hp < 0)
                op->stats.hp = op->stats.maxhp;
            if (op->stats.food < 0)
                op->stats.food = 999;
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

    /* food_status < 0 marks an active food force - we don't want double regeneration! */
    if (pl && pl->state == ST_PLAYING && pl->food_status>=0) /* be sure we are active and playing */
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
                    /*new_draw_info_format(NDI_UNIQUE, 0, op, "reg - prepare %d", pl->resting_reg_timer);*/
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

/* If the player should die (lack of hp, food, etc), we call this.
 * op is the player in jeopardy.  If the player can not be saved (not
 * permadeath, no lifesave), this will take care of removing the player
 * file.
 */
void kill_player(object *op)
{
    player      *pl=CONTR(op);
    char        buf[MAX_BUF];
    int         x, y, i;
    mapstruct  *map;  /*  this is for resurrection */
    object     *tmp;
    int         z;
    int         num_stats_lose;
    int         lost_a_stat;
    int         lose_this_stat;
    int         this_stat;

    if (save_life(op))
        return;

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
        if (op->stats.food <= 0)
            op->stats.food = 999;

        /* create a bodypart-trophy to make the winner happy */
        tmp = arch_to_object(find_archetype("finger"));
        if (tmp != NULL)
        {
            sprintf(buf, "%s's finger", op->name);
            FREE_AND_COPY_HASH(tmp->name, buf);
            sprintf(buf, "  This finger has been cut off %s\n"
                         "  the %s, when he was defeated at\n  level %d by %s.\n",
                    op->name, op->title?op->title:op->race, (int) (op->level), pl->killer?pl->killer:"bad luck");
            FREE_AND_COPY_HASH(tmp->msg, buf);
            tmp->value = 0, tmp->material = 0, tmp->type = 0;
            tmp->x = op->x, tmp->y = op->y;
            insert_ob_in_map(tmp, op->map, op, 0);
        }

        /* teleport defeated player to new destination*/
        enter_map(op, NULL, op->map, x, y, 0);
        return;
    }

    if(trigger_object_plugin_event(EVENT_DEATH,
                op, NULL, op, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
        return; /* Cheat death */

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

    if (op->stats.food < 0)
    {
        sprintf(buf, "%s starved to death.", op->name);
        FREE_AND_ADD_REF_HASH(pl->killer, shstr_cons.starvation);
    }
    else
        sprintf(buf, "%s died.", op->name);

    play_sound_player_only(pl, SOUND_PLAYER_DIES, SOUND_NORMAL, 0, 0);

    /*  save the map location for corpse, gravestone*/
    x = op->x;y = op->y;map = op->map;


#ifdef NOT_PERMADEATH
    /* NOT_PERMADEATH code.  This basically brings the character back to life
     * if they are dead - it takes some exp and a random stat.  See the config.h
     * file for a little more in depth detail about this.
     */

    /* Basically two ways to go - remove a stat permanently, or just
     * make it depletion.  This bunch of code deals with that aspect
     * of death.
     */

    if (settings.balanced_stat_loss)
    {
        /* If stat loss is permanent, lose one stat only. */
        /* Lower level chars don't lose as many stats because they suffer more
           if they do. */
        /* Higher level characters can afford things such as potions of
           restoration, or better, stat potions. So we slug them that little
           bit harder. */
        /* GD */
        if (settings.stat_loss_on_death)
            num_stats_lose = 1;
        else
            num_stats_lose = 1 + op->level / BALSL_NUMBER_LOSSES_RATIO;
    }
    else
    {
        num_stats_lose = 1;
    }
    lost_a_stat = 0;

    /* the rule is:
     * only decrease stats when you are level 3 or higher!
     */
    for (z = 0; z < num_stats_lose; z++)
    {
        if (settings.stat_loss_on_death && op->level > 3)
        {
            /* Pick a random stat and take a point off it.  Tell the player
              * what he lost.
              */
            i = RANDOM() % 7;
            change_attr_value(&(op->stats), i, -1);
            check_stat_bounds(&(op->stats));
            change_attr_value(&(pl->orig_stats), i, -1);
            check_stat_bounds(&(pl->orig_stats));
            new_draw_info(NDI_UNIQUE, 0, op, lose_msg[i]);
            lost_a_stat = 1;
        }
        else if (op->level > 3)
        {
            /* deplete a stat */
            archetype  *deparch = find_archetype("deathsick");
            object     *dep;

            i = RANDOM() % 7;
            dep = present_arch_in_ob(deparch, op);
            if (!dep)
            {
                dep = arch_to_object(deparch);
                insert_ob_in_ob(dep, op);
            }
            lose_this_stat = 1;
            if (settings.balanced_stat_loss)
            {
                /* GD */
                /* Get the stat that we're about to deplete. */
                this_stat = get_attr_value(&(dep->stats), i);
                if (this_stat < 0)
                {
                    int loss_chance = 1 + op->level / BALSL_LOSS_CHANCE_RATIO;
                    int keep_chance = this_stat *this_stat;
                    /* Yes, I am paranoid. Sue me. */
                    if (keep_chance < 1)
                        keep_chance = 1;

                    /* There is a maximum depletion total per level. */
                    if (this_stat < -1 - op->level / BALSL_MAX_LOSS_RATIO)
                    {
                        lose_this_stat = 0;
                        /* Take loss chance vs keep chance to see if we retain the stat. */
                    }
                    else
                    {
                        if (random_roll(0, loss_chance + keep_chance - 1) < keep_chance)
                            lose_this_stat = 0;
                        /* LOG(llevDebug, "Determining stat loss. Stat: %d Keep: %d Lose: %d Result: %s.\n",
                             this_stat, keep_chance, loss_chance,
                             lose_this_stat?"LOSE":"KEEP"); */
                    }
                }
            }

            if (lose_this_stat)
            {
                this_stat = get_attr_value(&(dep->stats), i);
                /* We could try to do something clever like find another
                     * stat to reduce if this fails.  But chances are, if
                     * stats have been depleted to -50, all are pretty low
                     * and should be roughly the same, so it shouldn't make a
                     * difference.
                     */
                if (this_stat >= -50)
                {
                    change_attr_value(&(dep->stats), i, -1);
                    SET_FLAG(dep, FLAG_APPLIED);
                    new_draw_info(NDI_UNIQUE, 0, op, lose_msg[i]);
                    FIX_PLAYER(op ,"kill player - change attr");
                    lost_a_stat = 1;
                }
            }
        }
    }
    /* If no stat lost, tell the player. */
    if (!lost_a_stat)
    {
        /* determine_god() seems to not work sometimes... why is this?
           Should I be using something else? GD */
        const char *god = determine_god(op);
        if (god != shstr_cons.none)
            new_draw_info_format(NDI_UNIQUE, 0, op,
                                 "For a brief moment you feel the holy presence of\n%s protecting you.", god);
        else
            new_draw_info(NDI_UNIQUE, 0, op, "For a brief moment you feel a holy presence\nprotecting you.");
    }

    /* Put a gravestone up where the character 'almost' died.  List the
     * exp loss on the stone.
     */
    tmp = arch_to_object(find_archetype("gravestone"));
    sprintf(buf, "%s's gravestone", op->name);
    FREE_AND_COPY_HASH(tmp->name, buf);
    sprintf(buf, "RIP\nHere rests the hero %s the %s,\n"
                 "who was killed\n"
                 "by %s.\n", op->name, op->title?op->title:op->race,
            pl->killer?pl->killer:"bad luck");
    FREE_AND_COPY_HASH(tmp->msg, buf);
    tmp->x = op->x,tmp->y = op->y;
    insert_ob_in_map(tmp, op->map, NULL, 0);

    /**************************************/
    /*                                    */
    /* Subtract the experience points,    */
    /* if we died cause of food, give us  */
    /* food, and reset HP's...            */
    /*                                    */
    /**************************************/

    /* remove any poisoning and confusion the character may be suffering. */
    cast_heal(op, 110, op, SP_CURE_POISON);
    /*cast_heal(op, op, SP_CURE_CONFUSION);*/
    cure_disease(op, NULL);  /* remove any disease */
    restoration(NULL, op);

    apply_death_exp_penalty(op);
    if (op->stats.food < 0)
        op->stats.food = 900;
    op->stats.hp = op->stats.maxhp;
    op->stats.sp = op->stats.maxsp;
    op->stats.grace = op->stats.maxgrace;

    /*
     * Check to see if the player is in a shop.  IF so, then check to see if
     * the player has any unpaid items.  If so, remove them and put them back
     * in the map.
     */

    tmp = get_map_ob(op->map, op->x, op->y);
    if (tmp && tmp->type == SHOP_FLOOR)
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
    STATS_EVENT(STATS_EVENT_PLAYER_DEATH, op->name);
    new_draw_info(NDI_UNIQUE, 0, op, "YOU HAVE DIED.");
    save_player(op, 1);
    return;
#endif

    /* If NOT_PERMADETH is set, then the rest of this is not reachable.  This
     * should probably be embedded in an else statement.
     */

    new_draw_info(NDI_UNIQUE | NDI_ALL, 0, NULL, buf);
    if (pl->golem != NULL)
    {
        send_golem_control(pl->golem, GOLEM_CTR_RELEASE);
        destruct_ob(pl->golem);
        pl->golem = NULL;
    }
    loot_object(op); /* Remove some of the items for good */
    remove_ob(op);
    check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
    op->direction = 0;
    if (!QUERY_FLAG(op, FLAG_WIZ) && op->stats.exp)
    {
        delete_character(op->name);
#ifndef NOT_PERMADEATH
#ifdef RESURRECTION
        /* save playerfile sans equipment when player dies
        ** then save it as player.pl.dead so that future resurrection
        ** type spells will work on them nicely
        */
        op->stats.hp = op->stats.maxhp_adj;
        op->stats.food = 999;

        /*  set the location of where the person will reappear when  */
        container_unlink(pl, NULL);
        save_player(op, 1);
        op->map = map;
        /* please see resurrection.c: peterm */
        dead_player(op);
#endif
#endif
    }
    /*play_again(op);*/
    pl->socket.status = Ns_Dead;
#ifdef NOT_PERMADEATH
    tmp = arch_to_object(find_archetype("gravestone"));
    sprintf(buf, "%s's gravestone", op->name);
    FREE_AND_COPY_HASH(tmp->name, buf);
    sprintf(buf, "RIP\nHere rests the hero %s the %s,\nwho was killed by %s.\n", op->name, op->title?op->title:op->race,
            pl->killer?pl->killer:"bad luck");
    FREE_AND_COPY_HASH(tmp->msg, buf);
    tmp->x = x,tmp->y = y;
    insert_ob_in_map(tmp, map, NULL, 0);
#else
    /*  peterm:  added to create a corpse at deathsite.  */
    /*
       tmp=arch_to_object(find_archetype("corpse_pl"));
       sprintf(buf,"%s", op->name);
       FREE_AND_COPY_HASH(tmp->name,buf);
       tmp->level=op->level;
       tmp->x=x;tmp->y=y;
       FREE_AND_COPY_HASH(tmp->msg, gravestone_text(op));
       insert_ob_in_map (tmp, map, NULL,0);
    */
#endif
}


void loot_object(object *op)
{
    /* Grab and destroy some treasure */
    object *tmp, *tmp2, *next;

    if (op->type == PLAYER && CONTR(op)->container)
    {
        /* close open sack first */
        esrv_apply_container(op, CONTR(op)->container);
    }

    for (tmp = op->inv; tmp != NULL; tmp = next)
    {
        next = tmp->below;
        if (tmp->type == EXPERIENCE || IS_SYS_INVISIBLE(tmp))
            continue;
        remove_ob(tmp);
        tmp->x = op->x,tmp->y = op->y;
        if (tmp->type == CONTAINER)
        {
            /* empty container to ground */
            loot_object(tmp);
        }
        if (QUERY_FLAG(tmp, FLAG_STARTEQUIP) || QUERY_FLAG(tmp, FLAG_NO_DROP) || !(RANDOM() % 3))
        {
            if (tmp->nrof > 1)
            {
                tmp2 = get_split_ob(tmp, 1 + RANDOM() % (tmp->nrof - 1));
                insert_ob_in_map(tmp, op->map, NULL, 0);
            }
        }
        else
            insert_ob_in_map(tmp, op->map, NULL, 0);
    }
}

/* cast_dust() - handles op throwing objects of type 'DUST' */
/* WARNING: FUNCTION NEED TO BE REWRITTEN. works for ae spells only now! */
void cast_dust(object *op, object *throw_ob, int dir)
{
    archetype  *arch    = NULL;

    if (!(spells[throw_ob->stats.sp].flags & SPELL_DESC_DIRECTION))
    {
        LOG(llevBug, "DEBUG: Warning, dust %s is not a ae spell!!\n", query_name(throw_ob));
        return;
    }

    if (spells[throw_ob->stats.sp].archname)
        arch = find_archetype(spells[throw_ob->stats.sp].archname);

    /* casting POTION 'dusts' is really a use_magic_item skill */
    if (op->type == PLAYER && throw_ob->type == POTION && !change_skill(op, SK_USE_MAGIC_ITEM))
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
        new_draw_info_format(NDI_UNIQUE, 0, op, "You cast %s.", query_name(throw_ob));
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
           if(tmp->type==EXPERIENCE && tmp->stats.Wis)
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
        for (tmp = get_map_ob(m, xt, yt); tmp; tmp = tmp->above)
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
          new_draw_info_format(NDI_UNIQUE, 0,op,"You become %!",op->hide?"unhidden":"visible");
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
    if (attacker)
    {
        if (!(attacker->map->map_flags & MAP_FLAG_PVP)
         || !(GET_MAP_FLAGS(attacker->map, attacker->x, attacker->y) & P_IS_PVP))
            return FALSE;
    }

    if (victim)
    {
        if (!(victim->map->map_flags & MAP_FLAG_PVP) || !(GET_MAP_FLAGS(victim->map, victim->x, victim->y) & P_IS_PVP))
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
    char            buf[MAX_BUF];             /* tmp. string buffer */
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
        int spell   = look_up_spell_name(item->slaying);
        if (spell < 0 || check_spell_known(who, spell))
            return;
        if (IS_SYS_INVISIBLE(item))
        {
            sprintf(buf, "You gained the ability of %s", spells[spell].name);
            new_draw_info(NDI_UNIQUE | NDI_BLUE, 0, who, buf);
            do_learn_spell(who, spell, 0);
            return;
        }
    }
    else if (item->type == SKILL)
    {
        if (item->title == shstr_cons.clawing && change_skill(who, SK_CLAWING))
        {
            /* adding new attacktypes to the clawing skill */
            tmp = who->chosen_skill; /* clawing skill object */

            if (tmp->type == SKILL && tmp->name == shstr_cons.clawing /*&& !(tmp->attacktype & item->attacktype)*/)
            {
                /* always add physical if there's none */
                /*
                if (tmp->attacktype == 0)
                    tmp->attacktype = AT_PHYSICAL;
                */

                /* we add the new attacktype to the clawing ability */
                /* tmp->attacktype += item->attacktype; */

                if (item->msg != NULL)
                    new_draw_info(NDI_UNIQUE | NDI_BLUE, 0, who, item->msg);
            }
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
            new_draw_info(NDI_UNIQUE | NDI_BLUE, 0, who, buf);
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
            new_draw_info(NDI_UNIQUE | NDI_BLUE, 0, who, item->msg);
    }
    else
    {
        /* generate misc. treasure */
        tmp = arch_to_object(tr->item);
        sprintf(buf, "You gained %s", query_short_name(tmp, NULL));
        new_draw_info(NDI_UNIQUE | NDI_BLUE, 0, who, buf);
        tmp = insert_ob_in_ob(tmp, who);
        if (who->type == PLAYER)
            esrv_send_item(who, tmp);
    }
}



/* Determine if the attacktype represented by the
 * specified attack-number is enabled for dragon players.
 * A dragon player (quetzal) can gain resistances for
 * all enabled attacktypes.
 */
int atnr_is_dragon_enabled(int attacknr)
{
    if (attacknr == ATNR_MAGIC
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
