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


/* i left find_arrow - find_arrow() and find_arrow()_ext should merge
   when the server sided range mode is removed at last from source */
static object  *find_arrow_ext(object *op, const char *type, int tag);

/* find a player name for a NORMAL string.
 * we use the hash table system.
 */
player * find_player(char *plname)
{
    player     *pl;
    char name[MAX_PLAYER_NAME > (16 - 1) ? MAX_PLAYER_NAME + 1 : 16];
    const char *name_hash;

    int         name_len                                                = strlen(plname); /* we assume a legal string */
    if (name_len <= 1 || name_len > MAX_PLAYER_NAME)
        return NULL;

    strcpy(name, plname); /* we need to copy it because we access the string */
    transform_name_string(name);
    if (!(name_hash = find_string(name)))
        return NULL;

    for (pl = first_player; pl != NULL; pl = pl->next)
    {
        if (pl->ob && !QUERY_FLAG(pl->ob, FLAG_REMOVED) && pl->ob->name == name_hash)
            return pl;
    }

    return NULL;
}

/* nearly the same as above except we
 * have the hash string when we call
 */
player * find_player_hash(const char *plname)
{
    player *pl;
    for (pl = first_player; pl != NULL; pl = pl->next)
    {
        if (pl->ob && !QUERY_FLAG(pl->ob, FLAG_REMOVED) && pl->ob->name == plname)
            return pl;
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
static player * get_player(player *p)
{
    object *op  = arch_to_object(get_player_archetype(NULL));
    int     i;

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

    /* thats our default start map + position from the arches */
    strcpy(p->savebed_map, EXIT_PATH(&map_archeytpe->clone));  /* Init. respawn position */
    p->bed_x = map_archeytpe->clone.stats.hp;
    p->bed_y = map_archeytpe->clone.stats.sp;

    p->gmaster_mode = GMASTER_MODE_NO;
    p->gmaster_node = NULL;
    p->mute_freq_shout=0;
    p->mute_freq_say=0;
    p->mute_counter=0;
    p->mute_msg_count=0;

    p->firemode_type = p->firemode_tag1 = p->firemode_tag2 = -1;
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
    p->gen_sp_armour = 0;
    p->last_speed = -1;
    p->shoottype = range_none;
    p->listening = 9;
    p->last_weapon_sp = -1;
    p->last_speed = 0;
    p->update_los = 1;

    /* the default skill groups for non guild players */
    p->base_skill_group[0]=SKILLGROUP_PHYSIQUE;
    p->base_skill_group[1]=SKILLGROUP_AGILITY;
    p->base_skill_group[2]=SKILLGROUP_WISDOM;

    strncpy(p->title, op->arch->clone.name, MAX_NAME);
    FREE_AND_COPY_HASH(op->race, op->arch->clone.race);

    /* Would be better of '0' was not a defined spell */
    for (i = 0; i < NROFREALSPELLS; i++)
        p->known_spells[i] = -1;

    p->chosen_spell = -1;

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
    if (pl->ob)
    {
        SET_FLAG(pl->ob, FLAG_NO_FIX_PLAYER);
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
    
    free_newsocket(&pl->socket);
    if (pl->ob)
        destroy_object(pl->ob);
}

/* Tries to add player on the connection passwd in ns.
 * All we can really get in this is some settings like host and display
 * mode.
 */

int add_player(NewSocket *ns)
{
    player *p;

    p = get_player(NULL);
    memcpy(&p->socket, ns, sizeof(NewSocket));
    /* Needed because the socket we just copied over needs to be cleared.
     * Note that this can result in a client reset if there is partial data
     * on the uncoming socket.
     */
    p->socket.pl = (struct player *) p;
    p->socket.status = Ns_Login; /* now, we start the login procedure! */
	p->socket.below_clear = 0;
    p->socket.update_tile = 0;
    p->socket.look_position = 0;

    start_info(p->ob);
    get_name(p->ob, 0); /* start a nice & clean login */

    insert_ob_in_ob(p->ob, &void_container); /* Avoid gc of the player */

    return 0;
}

/*
 * get_player_archetype() return next player archetype from archetype
 * list. Not very efficient routine, but used only creating new players.
 * Note: there MUST be at least one player archetype!
 */
archetype * get_player_archetype(archetype *at)
{
    archetype  *start   = at;
    for (; ;)
    {
        if (at == NULL || at->next == NULL)
            at = first_archetype;
        else
            at = at->next;
        if (at->clone.type == PLAYER)
            return at;
        if (at == start)
        {
            LOG(llevError, "ERROR: No Player achetypes\n");
            exit(-1);
        }
    }
}


#if 0
/* ARGH. this friendly list is a GLOBAL list. assuming 100 players on 75 map ... and every
 * mob on every map (perhaps some hundreds) will move through ALL of object of the
 * friendly list every time they try to target...
 * this must be changed. Look at friendly.c for more comments about it.
 * note, that only here and in pets.c this list is used. In pets.c it is only used to
 * remove or collect players pets,
 */
/* this is another example of the more or less broken friendly list use... this function seems not to trust
 * his own list - whats very bad because this is a core function - speed & elegance should used here
 * not crazy while loops to find invalid list entries.
 */
/* btw, this is not a "get nearest player" - its a "get nearest friendly object" */
/* i added now aggro range.
 * aggro range is the distance to target a mob will attack. Is the target out of this range,
 * the mob will not attack and/or not target it.
 * If a target moves out of aggro range for xx ticks, a mob will change
 * or leave target. Stealth will aggro range - 2. That sounds not much but is quite useful for normal
 * mobs.
 */
object *get_nearest_player(object *mon) {
    object *op = NULL;
    objectlink *ol;
    unsigned int lastdist, aggro_range, aggro_stealth;
    rv_vector   rv;

    /* lets set our aggro range. If mob is sleeping or blinded - half aggro range.
     * if target has stealth - sub. -2
     */
    aggro_range = mon->stats.Wis;
    if(mon->enemy || mon->attacked_by)
        aggro_range +=3;
    if(QUERY_FLAG(mon,FLAG_SLEEP) || QUERY_FLAG(mon,FLAG_BLIND))
    {
        aggro_range /=2;
        aggro_stealth = aggro_range-2;
    }
    else
    {
        aggro_stealth = aggro_range-2;
    }
    if(aggro_stealth<MIN_MON_RADIUS)
        aggro_stealth = MIN_MON_RADIUS;


    for(ol=first_friendly_object,lastdist=1000;ol!=NULL;ol=ol->next)
    {
        /* We should not find free objects on this friendly list, but it
        * does periodically happen.  Given that, lets deal with it.
        * While unlikely, it is possible the next object on the friendly
        * list is also free, so encapsulate this in a while loop.
        */
        while (!OBJECT_VALID(ol->ob, ol->id) || (!QUERY_FLAG(ol->ob, FLAG_FRIENDLY)&& ol->ob->type != PLAYER))
        {
            object *tmp=ol->ob;

            /* Can't do much more other than log the fact, because the object
            * itself will have been cleared.
            */
            LOG(llevDebug,"get_nearest_player: Found free/non friendly object on friendly list (%s)\n", STRING_OBJ_NAME(tmp));
            ol = ol->next;
            if (!ol) return op;
        }

        if (!can_detect_target(mon,ol->ob,aggro_range,aggro_stealth,&rv))
            continue;

        if(lastdist>rv.distance)
        {
            op=ol->ob;
            lastdist=rv.distance;
        }
    }

#if 0
    LOG(llevDebug,"get_nearest_player() mob %s (%x) finds friendly obj: %s (%x) aggro range: %d\n",query_name(mon),mon->count,query_name(op),op?op->count:-1, mon->stats.Wis);
#endif
    return op;
}
#endif

/* I believe this can safely go to 2, 3 is questionable, 4 will likely
 * result in a monster paths backtracking.  It basically determines how large a
 * detour a monster will take from the direction path when looking
 * for a path to the player.  The values are in the amount of direction
 * the deviation is
 */
#define DETOUR_AMOUNT   2

/* This is used to prevent infinite loops.  Consider a case where the
 * player is in a chamber (with gate closed), and monsters are outside.
 * with DETOUR_AMOUNT==2, the function will turn each corner, trying to
 * find a path into the chamber.  This is a good thing, but since there
 * is no real path, it will just keep circling the chamber for
 * ever (this could be a nice effect for monsters, but not for the function
 * to get stuck in.  I think for the monsters, if max is reached and
 * we return the first direction the creature could move would result in the
 * circling behaviour.  Unfortunately, this function is also used to determined
 * if the creature should cast a spell, so returning a direction in that case
 * is probably not a good thing.
 */
#define MAX_SPACES  50


/*
 * Returns the direction to the player, if valid.  Returns 0 otherwise.
 * modified to verify there is a path to the player.  Does this by stepping towards
 * player and if path is blocked then see if blockage is close enough to player that
 * direction to player is changed (ie zig or zag).  Continue zig zag until either
 * reach player or path is blocked.  Thus, will only return true if there is a free
 * path to player.  Though path may not be a straight line. Note that it will find
 * player hiding along a corridor at right angles to the corridor with the monster.
 *
 * Modified by MSW 2001-08-06 to handle tiled maps. Various notes:
 * 1) With DETOUR_AMOUNT being 2, it should still go and find players hiding
 * down corriders.
 * 2) I think the old code was broken if the first direction the monster
 * should move was blocked - the code would store the first direction without
 * verifying that the player can actually move in that direction.  The new
 * code does not store anything in firstdir until we have verified that the
 * monster can in fact move one space in that direction.
 * 3) I'm not sure how good this code will be for moving multipart monsters,
 * since only simple checks to blocked are being called, which could mean the monster
 * is blocking itself.
 */
/* TODO: this sould really use pathfinding instead. /Gecko */
int path_to_player(object *mon, object *pl, int mindiff)
{
    rv_vector   rv;
    int         x, y, lastx, lasty, dir, i, diff, firstdir = 0, lastdir, max = MAX_SPACES;
    mapstruct  *m, *lastmap;

    get_rangevector(mon, pl, &rv, 0);

    if ((int) rv.distance < mindiff)
        return 0;

    x = mon->x;
    y = mon->y;
    m = mon->map;
    dir = rv.direction;
    lastdir = firstdir = rv.direction; /* perhaps we stand next to pl, init firstdir too */
    diff = FABS(rv.distance_x) > FABS(rv.distance_y) ? FABS(rv.distance_x) : FABS(rv.distance_y);
    /* If we can't solve it within the search distance, return now. */
    if (diff > max)
        return 0;
    while (diff > 1 && max > 0)
    {
        lastx = x;
        lasty = y;
        lastmap = m;
        x = lastx + freearr_x[dir];
        y = lasty + freearr_y[dir];

        /* Space is blocked - try changing direction a little */
        if (arch_blocked(mon->arch, mon, m, x, y)) /* arch blocked controls multi arch with full map flags */
        {
            /* recalculate direction from last good location.  Possible
               * we were not traversing ideal location before.
               */
            get_rangevector_from_mapcoords(lastmap, lastx, lasty, pl->map, pl->x, pl->y, &rv, 0);
            if (rv.direction != dir)
            {
                /* OK - says direction should be different - lets reset the
                     * the values so it will try again.
                     */
                x = lastx;
                y = lasty;
                m = lastmap;
                dir = firstdir = rv.direction;
            }
            else
            {
                /* direct path is blocked - try taking a side step to
                     * either the left or right.
                     * Note increase the values in the loop below to be
                     * more than -1/1 respectively will mean the monster takes
                     * bigger detour.  Have to be careful about these values getting
                     * too big (3 or maybe 4 or higher) as the monster may just try
                     * stepping back and forth
                     */
                for (i = -DETOUR_AMOUNT; i <= DETOUR_AMOUNT; i++)
                {
                    if (i == 0)
                        continue;   /* already did this, so skip it */
                    /* Use lastdir here - otherwise,
                         * since the direction that the creature should move in
                         * may change, you could get infinite loops.
                         * ie, player is northwest, but monster can only
                         * move west, so it does that.  It goes some distance,
                         * gets blocked, finds that it should move north,
                         * can't do that, but now finds it can move east, and
                         * gets back to its original point.  lastdir contains
                         * the last direction the creature has successfully
                         * moved.
                         */

                    x = lastx + freearr_x[absdir(lastdir + i)];
                    y = lasty + freearr_y[absdir(lastdir + i)];
                    m = lastmap;

                    if (!arch_blocked(mon->arch, mon, m, x, y))
                        break;
                }
                /* go through entire loop without finding a valid
                     * sidestep to take - thus, no valid path.
                     */
                if (i == (DETOUR_AMOUNT + 1))
                    return 0;
                diff--;
                lastdir = dir;
                max--;
                if (!firstdir)
                    firstdir = dir + i;
            } /* else check alternate directions */
        } /* if blocked */
        else
        {
            /* we moved towards creature, so diff is less */
            diff--;
            max--;
            lastdir = dir;
            if (!firstdir)
                firstdir = dir;
        }
        if (diff <= 1)
        {
            /* Recalculate diff (distance) because we may not have actually
               * headed toward player for entire distance.
               */
            get_rangevector_from_mapcoords(m, x, y, pl->map, pl->x, pl->y, &rv, 0);
            diff = FABS(rv.distance_x) > FABS(rv.distance_y) ? FABS(rv.distance_x) : FABS(rv.distance_y);
        }
        if (diff > max)
            return 0;
    }
    /* If we reached the max, didn't find a direction in time */
    if (!max)
        return 0;

    return firstdir;
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
				op->type == CLOAK || op->type == ARMOUR || op->type == SHIELD || op->type == GLOVES)
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
    if (!(random_roll(0, 4, op, PREFER_LOW)) && random_roll(1, 20, op, PREFER_HIGH) >= savethrow[op->level])
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


/* check_pick sees if there is stuff to be picked up/picks up stuff.
 * IT returns 1 if the player should keep on moving, 0 if he should
 * stop.
 */
int check_pick(object *op)
{
    object *tmp, *next;
    tag_t   next_tag = 0, op_tag;
    int     stop    = 0;
    int     j, k, wvratio;
    char    putstring[128], tmpstr[16];


    /* if you're flying, you can't pick up anything */
    if (QUERY_FLAG(op, FLAG_FLYING))
        return 1;

    op_tag = op->count;

    next = op->below;
    if (next)
        next_tag = next->count;

    /* loop while there are items on the floor that are not marked as
     * destroyed */
    while (next && !was_destroyed(next, next_tag))
    {
        tmp = next;
        next = tmp->below;
        if (next)
            next_tag = next->count;

        if (was_destroyed(op, op_tag))
            return 0;

        if (!can_pick(op, tmp))
            continue;

        /* high bit set?  We're using the new autopickup model */
        if (!(CONTR(op)->mode & PU_NEWMODE))
        {
            switch (CONTR(op)->mode)
            {
                case 0:
                  return 1; /* don't pick up */
                case 1:
                  pick_up(op, tmp);
                  return 1;
                case 2:
                  pick_up(op, tmp);
                  return 0;
                case 3:
                  return 0; /* stop before pickup */
                case 4:
                  pick_up(op, tmp);
                  break;
                case 5:
                  pick_up(op, tmp);
                  stop = 1;
                  break;
                case 6:
                  if (QUERY_FLAG(tmp, FLAG_KNOWN_MAGICAL) && !QUERY_FLAG(tmp, FLAG_KNOWN_CURSED))
                      pick_up(op, tmp);
                  break;

                case 7:
                  if (tmp->type == MONEY || tmp->type == GEM || tmp->type == TYPE_PEARL || tmp->type == TYPE_JEWEL || tmp->type == TYPE_NUGGET)
                      pick_up(op, tmp);
                  break;

                default:
                  /* use value density */
                  if (!QUERY_FLAG(tmp, FLAG_UNPAID)
                   && ((double)query_cost(tmp, op, F_TRUE) * 100.0 / ((double) tmp->weight * (double) (MAX(tmp->nrof, 1))))
                   >= (double) CONTR(op)->mode)
                      pick_up(op, tmp);
            }
        } /* old model */
        else
        {
            /* NEW pickup handling */
            if (CONTR(op)->mode & PU_DEBUG)
            {
                /* some debugging code to figure out item information */
                if (tmp->name != NULL)
                    sprintf(putstring, "item name: %s    item type: %d    weight/value: %d", tmp->name, tmp->type,
                            (int) ((double)query_cost(tmp, op, F_TRUE) * 100 / (tmp->weight * MAX(tmp->nrof, 1))));
                else
                    sprintf(putstring, "item name: %s    item type: %d    weight/value: %d", tmp->arch->name, tmp->type,
                            (int) ((double)query_cost(tmp, op, F_TRUE) * 100 / (tmp->weight * MAX(tmp->nrof, 1))));
                new_draw_info(NDI_UNIQUE, 0, op, putstring);

                sprintf(putstring, "...flags: ");
                for (k = 0; k < 4; k++)
                {
                    for (j = 0; j < 32; j++)
                    {
                        if ((tmp->flags[k] >> j) & 0x01)
                        {
                            sprintf(tmpstr, "%d ", k * 32 + j);
                            strcat(putstring, tmpstr);
                        }
                    }
                }
                new_draw_info(NDI_UNIQUE, 0, op, putstring);

#if 0
    /* print the flags too */
    for(k=0;k<4;k++)
    {
      LOG(llevInfo ,"%d [%d] ", k, k*32+31);
      for(j=0;j<32;j++)
      {
        LOG(llevInfo ,"%d",tmp->flags[k]>>(31-j)&0x01);
        if(!((j+1)%4))LOG(llevInfo ," ");
      }
      LOG(llevInfo ," [%d]\n", k*32);
    }
#endif
            }
            /* philosophy:
            * It's easy to grab an item type from a pile, as long as it's
            * generic.  This takes no game-time.  For more detailed pickups
            * and selections, select-items shoul dbe used.  This is a
            * grab-as-you-run type mode that's really useful for arrows for
            * example.
            * The drawback: right now it has no frontend, so you need to
            * stick the bits you want into a calculator in hex mode and then
            * convert to decimal and then 'pickup <#>
            */

            /* the first two modes are exclusive: if NOTHING we return, if
             * STOP then we stop.  All the rest are applied sequentially,
             * meaning if any test passes, the item gets picked up. */

            /* if mode is set to pick nothing up, return */
            if (CONTR(op)->mode & PU_NOTHING)
                return 1;
            /* if mode is set to stop when encountering objects, return */
            /* take STOP before INHIBIT since it doesn't actually pick
             * anything up */
            if (CONTR(op)->mode & PU_STOP)
                return 0;
            /* useful for going into stores and not losing your settings... */
            /* and for battles wher you don't want to get loaded down while
             * fighting */
            if (CONTR(op)->mode & PU_INHIBIT)
                return 1;

            /* prevent us from turning into auto-thieves :) */
            if (QUERY_FLAG(tmp, FLAG_UNPAID))
                continue;

            /* all food and drink if desired */
            /* question: don't pick up known-poisonous stuff? */
            if (CONTR(op)->mode & PU_FOOD)
                if (tmp->type == FOOD)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"FOOD\n");*/ continue;
                }
            if (CONTR(op)->mode & PU_DRINK)
                if (tmp->type == DRINK)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"DRINK\n");*/ continue;
                }
            if (CONTR(op)->mode & PU_POTION)
                if (tmp->type == POTION)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"POTION\n");*/ continue;
                }

            /* pick up all magical items */
            if (CONTR(op)->mode & PU_MAGICAL)
                if (QUERY_FLAG(tmp, FLAG_KNOWN_MAGICAL) && !QUERY_FLAG(tmp, FLAG_KNOWN_CURSED))
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"MAGICAL\n");*/ continue;
                }

            if (CONTR(op)->mode & PU_VALUABLES)
            {
                if (tmp->type == MONEY || tmp->type == GEM || tmp->type == TYPE_PEARL || tmp->type == TYPE_JEWEL || tmp->type == TYPE_NUGGET)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"MONEY/GEM\n");*/ continue;
                }
            }

            /* bows and arrows. Bows are good for selling! */
            if (CONTR(op)->mode & PU_BOW)
                if (tmp->type == BOW)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"BOW\n");*/ continue;
                }
            if (CONTR(op)->mode & PU_ARROW)
                if (tmp->type == ARROW)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"ARROW\n");*/ continue;
                }

            /* all kinds of armor etc. */
            if (CONTR(op)->mode & PU_ARMOUR)
                if (tmp->type == ARMOUR)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"ARMOUR\n");*/ continue;
                }
            if (CONTR(op)->mode & PU_HELMET)
                if (tmp->type == HELMET)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"HELMET\n");*/ continue;
                }
            if (CONTR(op)->mode & PU_SHIELD)
                if (tmp->type == SHIELD)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"SHIELD\n");*/ continue;
                }
            if (CONTR(op)->mode & PU_BOOTS)
                if (tmp->type == BOOTS)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"BOOTS\n");*/ continue;
                }
            if (CONTR(op)->mode & PU_GLOVES)
                if (tmp->type == GLOVES)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"GLOVES\n");*/ continue;
                }
            if (CONTR(op)->mode & PU_CLOAK)
                if (tmp->type == CLOAK)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"CLOAK\n");*/ continue;
                }

            /* hoping to catch throwing daggers here */
            if (CONTR(op)->mode & PU_MISSILEWEAPON)
                if (tmp->type == WEAPON && QUERY_FLAG(tmp, FLAG_IS_THROWN))
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"MISSILEWEAPON\n");*/ continue;
                }

            /* misc stuff that's useful */
            if (CONTR(op)->mode & PU_KEY)
                if (tmp->type == KEY || tmp->type == SPECIAL_KEY)
                {
                    pick_up(op, tmp); /*LOG(llevInfo ,"KEY\n");*/ continue;
                }

            /* any of the last 4 bits set means we use the ratio for value
             * pickups */
            if (CONTR(op)->mode & PU_RATIO)
            {
                /* use value density to decide what else to grab */
                /* >=7 was >= CONTR(op)->mode */
                /* >=7 is the old standard setting.  Now we take the last 4 bits
                 * and multiply them by 5, giving 0..15*5== 5..75 */
                wvratio = (CONTR(op)->mode & PU_RATIO) * 5;
                if (((double)query_cost(tmp, op, F_TRUE) * 100 / (tmp->weight * MAX((signed long) tmp->nrof, 1))) >= wvratio)
                {
                    pick_up(op, tmp);

                    /*
                      LOG(llevInfo ,"HIGH WEIGHT/VALUE [");
                      if(tmp->name!=NULL) {
                        LOG(llevInfo ,"%s", tmp->name);
                      }
                      else
                      LOG(llevInfo ,"%s",tmp->arch->name);
                      LOG(llevInfo ,",%d] = ", tmp->type);
                      LOG(llevInfo ,"%d\n",(int)(query_cost(tmp,op,F_TRUE)*100 / (tmp->weight * MAX(tmp->nrof,1))));
                      */
                    continue;
                }
            }
        } /* the new pickup model */
    }
    return !stop;
}

/*
 *  Find an arrow in the inventory and after that
 *  in the right type container (quiver). Pointer to the
 *  found object is returned.
 */
object * find_arrow(object *op, const char *type)
{
    object *tmp = NULL;

    for (op = op->inv; op; op = op->below)
        if (!tmp && op->type == CONTAINER && op->race == type && QUERY_FLAG(op, FLAG_APPLIED))
            tmp = find_arrow(op, type);
        else if (op->type == ARROW && op->race == type)
            return op;
    return tmp;
}

/*
 *  Player fires a bow.
 */
static void fire_bow(object *op, int dir)
{
    object *left_cont, *bow, *arrow = NULL, *left, *tmp_op;
    float dmg_tmp;
    tag_t   left_tag;

    if (!dir)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can't shoot yourself!");
        return;
    }

    bow = CONTR(op)->equipment[PLAYER_EQUIP_BOW];
    if (!bow)
        LOG(llevBug, "BUG: Range: bow without activated bow (%s - %d).\n", op->name, dir);

    if (!bow->race)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "Your %s is broken.", bow->name);
        return;
    }
    if ((arrow = find_arrow_ext(op, bow->race, CONTR(op)->firemode_tag2)) == NULL)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "You have no %s left.", bow->race);
        return;
    }
    if (wall(op->map, op->x + freearr_x[dir], op->y + freearr_y[dir]))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Something is in the way.");
        return;
    }
    /* this should not happen, but sometimes does */
    if (arrow->nrof == 0)
    {
        LOG(llevDebug, "BUG?: arrow->nrof == 0 in fire_bow() (%s)\n", query_name(arrow));
        remove_ob(arrow);
        return;
    }
    left = arrow; /* these are arrows left to the player */
    left_tag = left->count;
    left_cont = left->env;
    arrow = get_split_ob(arrow, 1);
    set_owner(arrow, op);
    arrow->direction = dir;
    arrow->x = op->x;
    arrow->y = op->y;
    arrow->speed = 1;

    /* now the trick: we transfer the shooting speed in the used
     * skill - that will allow us to use "set_skill_speed() as global
     * function.
     */
    op->chosen_skill->stats.maxsp = bow->stats.sp + arrow->last_grace;
    update_ob_speed(arrow);
    arrow->speed_left = 0;
    SET_ANIMATION(arrow, (NUM_ANIMATIONS(arrow) / NUM_FACINGS(arrow)) * dir);
    arrow->last_heal = arrow->stats.wc; /* save original wc and dam */
    arrow->stats.hp = arrow->stats.dam; /* will be put back in fix_arrow() */

    /* now we do this: arrow wc = wc base from skill + (wc arrow + magic) + (wc range weapon boni + magic) */
    if ((tmp_op = SK_skill(op)))
        arrow->stats.wc = tmp_op->last_heal; /* wc is in last heal */
    else
        arrow->stats.wc = 10;

    /* now we determinate how many tiles the arrow will fly.
     * again we use the skill base and add arrow + weapon values - but no magic add here.
     */
    arrow->last_sp = tmp_op->last_sp + bow->last_sp + arrow->last_sp;

    /* add in all our wc boni */
    arrow->stats.wc += (bow->magic + arrow->magic + SK_level(op) + thaco_bonus[op->stats.Dex] + bow->stats.wc);

    /* monster.c 970 holds the arrow code for monsters */
    /* moving dmg code to dmg/10 */
    dmg_tmp = ((float)arrow->stats.dam + (float)bow->stats.dam + ((float)dam_bonus[op->stats.Str]/2.0f)/10.0f)
              + (float)bow->magic + (float)arrow->magic;
    arrow->stats.dam = (int) FABS(arrow->stats.dam * lev_damage[SK_level(op)]);

    /* adjust with the lower of condition */
    if (bow->item_condition > arrow->item_condition)
        arrow->stats.dam = (sint16) (((float) arrow->stats.dam / 100.0f) * (float) arrow->item_condition);
    else
        arrow->stats.dam = (sint16) (((float) arrow->stats.dam / 100.0f) * (float) bow->item_condition);


    arrow->level = SK_level(op); /* this is used temporary when fired, arrow has
                                  * no level use elsewhere.
                                  */
    arrow->map = op->map;
    SET_MULTI_FLAG(arrow, FLAG_FLYING);
    SET_FLAG(arrow, FLAG_IS_MISSILE);
    SET_FLAG(arrow, FLAG_FLY_ON);
    SET_FLAG(arrow, FLAG_WALK_ON);
    arrow->stats.grace = arrow->last_sp; /* temp. buffer for "tiles to fly" */
    arrow->stats.maxgrace = 60 + (RANDOM() % 12); /* reflection timer */
    play_sound_map(op->map, op->x, op->y, SOUND_FIRE_ARROW, SOUND_NORMAL);
    if (insert_ob_in_map(arrow, op->map, op, 0))
        move_arrow(arrow);
    if (was_destroyed(left, left_tag))
        esrv_del_item(CONTR(op), left_tag, left_cont);
    else
        esrv_send_item(op, left);
}


void fire(object *op, int dir)
{
    object *weap        = NULL;
    int     spellcost   = 0;

    /* check for loss of invisiblity/hide */
    if (action_makes_visible(op))
        make_visible(op);

    /* a check for players, make sure things are groovy. This routine
     * will change the skill of the player as appropriate in order to
     * fire whatever is requested. In the case of spells (range_magic)
     * it handles whether cleric or mage spell is requested to be cast.
     * -b.t.
     */

    /* ext. fire mode - first step. We map the client side action to a server action. */
    /* forcing the shoottype var from player object to our needed range mode */
    if (op->type == PLAYER)
    {
        if (CONTR(op)->firemode_type == FIRE_MODE_NONE)
            return;

        if (CONTR(op)->firemode_type == FIRE_MODE_BOW)
            CONTR(op)->shoottype = range_bow;
        else if (CONTR(op)->firemode_type == FIRE_MODE_THROW)
        {
            object *tmp;

            /* insert here test for more throwing skills */
            if (!change_skill(op, SK_THROWING))
                return;
            /* special case - we must redirect the fire cmd to throwing something */
            tmp = find_throw_tag(op, (tag_t) CONTR(op)->firemode_tag1);
            if (tmp)
            {
                if (!check_skill_action_time(op, op->chosen_skill))
                    return;
                do_throw(op, tmp, dir);
                get_skill_time(op, op->chosen_skill->stats.sp);
            }
            return;
        }
        else if (CONTR(op)->firemode_type == FIRE_MODE_SPELL)
            CONTR(op)->shoottype = range_magic;
        else if (CONTR(op)->firemode_type == FIRE_MODE_WAND)
        {
            CONTR(op)->shoottype = range_wand; /* we do a jump in fire wand if we haven one */
        }
        else if (CONTR(op)->firemode_type == FIRE_MODE_SKILL)
        {
            command_rskill(op, CONTR(op)->firemode_name);
            CONTR(op)->shoottype = range_skill;
        }
        else if (CONTR(op)->firemode_type == FIRE_MODE_SUMMON)
            CONTR(op)->shoottype = range_scroll;
        else
            CONTR(op)->shoottype = range_none;

        if (!check_skill_to_fire(op))
            return;
    }

    switch (CONTR(op)->shoottype)
    {
        case range_none:
          return;

        case range_bow:
          if (CONTR(op)->firemode_tag2 != -1)
          {
              /* we still recover from range action? */
              if (!check_skill_action_time(op, op->chosen_skill))
                  return;

              fire_bow(op, dir);
              get_skill_time(op, op->chosen_skill->stats.sp);
              /* op->speed_left -= get_skill_time(op,op->chosen_skill->stats.sp); */
          }
          return;

        case range_magic:
          /* Casting spells */

          if (!check_skill_action_time(op, op->chosen_skill))
              return;
          spellcost = cast_spell(op, op, dir, CONTR(op)->chosen_spell, 0, spellNormal, NULL);

          if (spells[CONTR(op)->chosen_spell].flags & SPELL_DESC_WIS)
              op->stats.grace -= spellcost;
          else
              op->stats.sp -= spellcost;

          get_skill_time(op, op->chosen_skill->stats.sp);
          return;

        case range_wand:
          for (weap = op->inv; weap != NULL; weap = weap->below)
              if (weap->type == WAND && QUERY_FLAG(weap, FLAG_APPLIED))
                  break;
          if (weap == NULL)
          {
              CONTR(op)->shoottype = range_rod;
              goto trick_jump;
          }

          if (!check_skill_action_time(op, op->chosen_skill))
              return;
          if (weap->stats.food <= 0)
          {
              play_sound_player_only(CONTR(op), SOUND_WAND_POOF, SOUND_NORMAL, 0, 0);
              new_draw_info(NDI_UNIQUE, 0, op, "The wand says poof.");
              /*op->speed_left -= get_skill_time(op,op->chosen_skill->stats.sp);*/
              return;
          }

          new_draw_info(NDI_UNIQUE, 0, op, "fire wand");
          if (cast_spell(op, weap, dir, weap->stats.sp, 0, spellWand, NULL))
          {
              SET_FLAG(op, FLAG_BEEN_APPLIED); /* You now know something about it */
              if (!(--weap->stats.food))
              {
                  object   *tmp;
                  if (weap->arch)
                  {
                      CLEAR_FLAG(weap, FLAG_ANIMATE);
                      weap->face = weap->arch->clone.face;
                      weap->speed = 0;
                      update_ob_speed(weap);
                  }
                  if ((tmp = is_player_inv(weap)))
                      esrv_update_item(UPD_ANIM, tmp, weap);
              }
          }
          get_skill_time(op, op->chosen_skill->stats.sp);
          /*op->speed_left -= get_skill_time(op,op->chosen_skill->stats.sp);*/
          return;
        case range_rod:
        case range_horn:
          trick_jump:
          for (weap = op->inv; weap != NULL; weap = weap->below)
              if (QUERY_FLAG(weap, FLAG_APPLIED) && weap->type == (CONTR(op)->shoottype == range_rod ? ROD : HORN))
                  break;
          if (weap == NULL)
          {
              if (CONTR(op)->shoottype == range_rod)
              {
                  CONTR(op)->shoottype = range_horn;
                  goto trick_jump;
              }
              else
              {
                  char      buf[MAX_BUF];
                  sprintf(buf, "You have no tool readied.");
                  new_draw_info(NDI_UNIQUE, 0, op, buf);
                  return;
              }
          }
          if (!check_skill_action_time(op, op->chosen_skill))
              return;
          if (weap->stats.hp < spells[weap->stats.sp].sp)
          {
              play_sound_player_only(CONTR(op), SOUND_WAND_POOF, SOUND_NORMAL, 0, 0);
              if (CONTR(op)->shoottype == range_rod)
                  new_draw_info(NDI_UNIQUE, 0, op, "The rod whines for a while, but nothing happens.");
              else
                  new_draw_info(NDI_UNIQUE, 0, op, "No matter how hard you try you can't get another note out.");
              return;
          }
          /*new_draw_info_format(NDI_ALL|NDI_UNIQUE,5,NULL,"Use %s - cast spell %d\n",weap->name,weap->stats.sp);*/
          if (cast_spell(op, weap, dir, weap->stats.sp, 0, CONTR(op)->shoottype == range_rod ? spellRod : spellHorn,
                         NULL))
          {
              SET_FLAG(op, FLAG_BEEN_APPLIED); /* You now know something about it */
              drain_rod_charge(weap);
          }
          get_skill_time(op, op->chosen_skill->stats.sp);
          return;
        case range_scroll:
          /* Control summoned monsters from scrolls */
          if (CONTR(op)->golem == NULL)
          {
              CONTR(op)->shoottype = range_none;
              CONTR(op)->chosen_spell = -1;
          }
          else
              control_golem(CONTR(op)->golem, dir);
          return;

        case range_skill:
          if (!op->chosen_skill)
          {
              if (op->type == PLAYER)
                  new_draw_info(NDI_UNIQUE, 0, op, "You have no applicable skill to use.");
              return;
          }
          if (op->chosen_skill->sub_type1 != ST1_SKILL_USE)
              new_draw_info(NDI_UNIQUE, 0, op, "You can't use this skill in this way.");
          else
              (void) do_skill(op, dir, NULL);
          return;
        default:
          new_draw_info(NDI_UNIQUE, 0, op, "Illegal shoot type.");
          return;
    }
}

int move_player(object *op, int dir)
{

    CONTR(op)->praying = 0;

    if (op->map == NULL || op->map->in_memory != MAP_IN_MEMORY ||
        QUERY_FLAG(op,FLAG_PARALYZED) || QUERY_FLAG(op,FLAG_ROOTED))
        return 0;

    if (dir)
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

    /* firemode is set from client command fire xx xx xx */
    if (CONTR(op)->firemode_type != -1)
    {
        fire(op, dir);
        if (dir)
            op->anim_enemy_dir = dir;
        else
            op->anim_enemy_dir = op->facing;
        CONTR(op)->fire_on = 0;
    }
    else
    {
        if (!move_ob(op, dir, op))
            op->anim_enemy_dir = dir;
        else
            op->anim_moving_dir = dir;
    }

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
    /* i disabled automatically pickup - This is a needed option
     * in a rogue like game but its a exploit in a modern MMORPG.
     * we really DON'T want that people run around picking automatically
     * all up - because we don't want junk items they will then always pickup
     * valuable things.
     */
    /*pick = check_pick(op);*/

    /* running/firing is now handled different.
       if (CONTR(op)->fire_on || (CONTR(op)->run_on && pick!=0)) {
    op->direction = dir;
       } else {
    op->direction=0;
       }
       */
    if (QUERY_FLAG(op, FLAG_ANIMATE)) /* hm, should be not needed - players always animated */
    {
        if (op->anim_enemy_dir == -1 && op->anim_moving_dir == -1)
            op->anim_last_facing = dir;
        animate_object(op, 0);
    }
    return 0;
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

    if (op->direction && (CONTR(op)->run_on || CONTR(op)->fire_on))     /* automove or fire */
    {
        /* All move commands take 1 tick, at least for now */
        move_player(op, op->direction);
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
            /*enter_player_savebed(op);*/ /* bring him home. */
            return 1;
        }
    LOG(llevBug, "BUG: LIFESAVE set without applied object.\n");
    CLEAR_FLAG(op, FLAG_LIFESAVE);
    enter_player_savebed(op); /* bring him home. */
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


void do_some_living(object *op)
{
    if (CONTR(op)->state == ST_PLAYING)
    {
        /* hp reg */
		if(CONTR(op)->damage_timer)
			CONTR(op)->damage_timer--;
        else if (CONTR(op)->gen_hp)
        {
			if (--op->last_heal < 0)
			{
				op->last_heal = CONTR(op)->base_hp_reg;

				if (op->stats.hp < op->stats.maxhp)
				{
					int last_food   = op->stats.food;

					op->stats.hp += CONTR(op)->reg_hp_num;
					if (op->stats.hp > op->stats.maxhp)
					    op->stats.hp = op->stats.maxhp;

					/* faster hp reg - faster digestion... evil */
					op->stats.food--;
					if (CONTR(op)->digestion < 0)
						op->stats.food += CONTR(op)->digestion;
					else if (CONTR(op)->digestion > 0 && random_roll(0, CONTR(op)->digestion, op, PREFER_HIGH))
					    op->stats.food = last_food;
				}
			}
        }

        /* sp reg */
        if (CONTR(op)->gen_sp)
        {
            if (--op->last_sp < 0)
            {
                op->last_sp = CONTR(op)->base_sp_reg;
                if (op->stats.sp < op->stats.maxsp)
                {
                    op->stats.sp += CONTR(op)->reg_sp_num;
                    if (op->stats.sp > op->stats.maxsp)
                        op->stats.sp = op->stats.maxsp;
                }
            }
        }

        /* "stay and pray" mechanism */
        if (CONTR(op)->praying && !CONTR(op)->was_praying)
        {
            if (op->stats.grace < op->stats.maxgrace)
            {
                object *god = find_god(determine_god(op));
                if (god)
                {
                    if (CONTR(op)->combat_mode)
                    {
                        new_draw_info_format(NDI_UNIQUE, 0, op, "You stop combat and start praying to %s...", god->name);
                        CONTR(op)->combat_mode = 0;
                        update_pets_combat_mode(op);
                        send_target_command(CONTR(op));
                    }
                    else
                        new_draw_info_format(NDI_UNIQUE, 0, op, "You start praying to %s...", god->name);
                    CONTR(op)->was_praying = 1;
                }
                else
                {
                    new_draw_info(NDI_UNIQUE, 0, op, "You worship no deity to pray to!");
                    CONTR(op)->praying = 0;
                }
                op->last_grace = CONTR(op)->base_grace_reg;
            }
            else
            {
                CONTR(op)->praying = 0;
                CONTR(op)->was_praying = 0;
            }
        }
        else if (!CONTR(op)->praying && CONTR(op)->was_praying)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You stop praying.");
            CONTR(op)->was_praying = 0;
            op->last_grace = CONTR(op)->base_grace_reg;
        }

        /* grace reg */
        if (CONTR(op)->praying && CONTR(op)->gen_grace)
        {
            if (--op->last_grace < 0)
            {
                if (op->stats.grace < op->stats.maxgrace)
                    op->stats.grace += CONTR(op)->reg_grace_num;
                if (op->stats.grace >= op->stats.maxgrace)
                {
                    op->stats.grace = op->stats.maxgrace;
                    new_draw_info(NDI_UNIQUE, 0, op, "Your are full of grace and stop praying.");
                    CONTR(op)->was_praying = 0;
                }
                op->last_grace = CONTR(op)->base_grace_reg;
            }
        }

        /* Digestion */
        if (--op->last_eat < 0)
        {
            int bonus   = CONTR(op)->digestion > 0 ? CONTR(op)->digestion : 0,
                        penalty = CONTR(op)->digestion < 0 ? -CONTR(op)->digestion : 0;
            if (CONTR(op)->gen_hp > 0)
                op->last_eat = 25 * (1 + bonus) / (CONTR(op)->gen_hp + penalty + 1);
            else
                op->last_eat = 25 * (1 + bonus) / (penalty + 1);
            op->stats.food--;
        }

        if (op->stats.food < 0 && op->stats.hp >= 0)
        {
            object *tmp, *flesh = NULL;

            for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
            {
                if (!QUERY_FLAG(tmp, FLAG_UNPAID))
                {
                    if (tmp->type == FOOD || tmp->type == DRINK || tmp->type == POISON)
                    {
                        new_draw_info(NDI_UNIQUE, 0, op, "You blindly grab for a bite of food.");
                        manual_apply(op, tmp, 0);
                        if (op->stats.food >= 0 || op->stats.hp < 0)
                            break;
                    }
                    else if (tmp->type == FLESH)
                        flesh = tmp;
                } /* End if paid for object */
            } /* end of for loop */

            /* If player is still starving, it means they don't have any food, so
                    * eat flesh instead.
                    */
            if (op->stats.food < 0 && op->stats.hp >= 0 && flesh)
            {
                new_draw_info(NDI_UNIQUE, 0, op, "You blindly grab for a bite of food.");
                manual_apply(op, flesh, 0);
            }
        } /* end if player is starving */

        while (op->stats.food<0 && op->stats.hp>0)
        {
            op->stats.food++;
            /* new: no dying from food. hp will fall to 1 but not under it.
                     * we must check here for negative because we don't want ADD here
                     */
            if (op->stats.hp)
            {
                op->stats.hp--;
                if (!op->stats.hp)
                    op->stats.hp = 1;
            }
        };

        /* we can't die by no food but perhaps by poisoned food? */
        if ((op->stats.hp <= 0 || op->stats.food < 0) && !QUERY_FLAG(op, FLAG_WIZ))
            kill_player(op);
    }
}


/* If the player should die (lack of hp, food, etc), we call this.
 * op is the player in jeopardy.  If the player can not be saved (not
 * permadeath, no lifesave), this will take care of removing the player
 * file.
 */
void kill_player(object *op)
{
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
                    op->name, CONTR(op)->title, (int) (op->level), CONTR(op)->killer);
            FREE_AND_COPY_HASH(tmp->msg, buf);
            tmp->value = 0, tmp->material = 0, tmp->type = 0;
            tmp->x = op->x, tmp->y = op->y;
            insert_ob_in_map(tmp, op->map, op, 0);
        }

        /* teleport defeated player to new destination*/
        transfer_ob(op, x, y, 0, NULL, NULL);
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
        strcpy(CONTR(op)->killer, "starvation");
    }
    else
        sprintf(buf, "%s died.", op->name);

    play_sound_player_only(CONTR(op), SOUND_PLAYER_DIES, SOUND_NORMAL, 0, 0);

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
            change_attr_value(&(CONTR(op)->orig_stats), i, -1);
            check_stat_bounds(&(CONTR(op)->orig_stats));
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
                        if (random_roll(0, loss_chance + keep_chance - 1, op, PREFER_LOW) < keep_chance)
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
                    fix_player(op);
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
                 "by %s.\n", op->name, CONTR(op)->title,
            CONTR(op)->killer);
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

    enter_player_savebed(op);

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
    check_score(op);
    if (CONTR(op)->golem != NULL)
    {
        send_golem_control(CONTR(op)->golem, GOLEM_CTR_RELEASE);
        destruct_ob(CONTR(op)->golem);
        CONTR(op)->golem = NULL;
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
        /* maybe resurrection code should fix map also */
        strcpy(CONTR(op)->maplevel, EMERGENCY_MAPPATH);
        if (op->map != NULL)
            op->map = NULL;
        op->x = EMERGENCY_X;
        op->y = EMERGENCY_Y;
        container_unlink(CONTR(op), NULL);
        save_player(op, 1);
        op->map = map;
        /* please see resurrection.c: peterm */
        dead_player(op);
#endif
#endif
    }
    /*play_again(op);*/
    CONTR(op)->socket.status = Ns_Dead;
#ifdef NOT_PERMADEATH
    tmp = arch_to_object(find_archetype("gravestone"));
    sprintf(buf, "%s's gravestone", op->name);
    FREE_AND_COPY_HASH(tmp->name, buf);
    sprintf(buf, "RIP\nHere rests the hero %s the %s,\nwho was killed by %s.\n", op->name, CONTR(op)->title,
            CONTR(op)->killer);
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
        cast_cone(op, throw_ob, dir, 10, throw_ob->stats.sp, arch, 1);
    else if ((arch = find_archetype("dust_effect")) != NULL)
    {
        /* dust_effect */
        cast_cone(op, throw_ob, dir, 1, 0, arch, 0);
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
    int hide = 0, num = random_roll(0, 19, op, PREFER_LOW);

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
    int         i, xt, yt, friendly = 0, player = 0;

    if (!who)
        return 0;

    if (who->type == PLAYER)
        player = 1;
    else
        friendly = QUERY_FLAG(who, FLAG_FRIENDLY);

    /* search adjacent squares */
    for (i = 1; i < 9; i++)
    {
        xt = who->x + freearr_x[i];
        yt = who->y + freearr_y[i];
        if (!(m = out_of_map(who->map, &xt, &yt)))
            continue;
        for (tmp = get_map_ob(m, xt, yt); tmp; tmp = tmp->above)
        {
            if ((player || friendly) && QUERY_FLAG(tmp, FLAG_MONSTER) && !QUERY_FLAG(tmp, FLAG_UNAGGRESSIVE))
                return 1;
            else if (tmp->type == PLAYER)
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
        } else if(CONTR(op) && !CONTR(op)->shoottype==range_magic) {
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


/*  extended find arrow version, using tag and containers.
 *  Find an arrow in the inventory and after that
 *  in the right type container (quiver). Pointer to the
 *  found object is returned.
 */
static object * find_arrow_ext(object *op, const char *type, int tag)
{
    object *tmp = NULL;

    if (tag == -2)
    {
        for (op = op->inv; op; op = op->below)
            if (!tmp && op->type == CONTAINER && op->race == type && QUERY_FLAG(op, FLAG_APPLIED))
                tmp = find_arrow_ext(op, type, -2);
            else if (op->type == ARROW && op->race == type)
                return op;
        return tmp;
    }
    else
    {
        if (tag == -1)
            return tmp;
        for (op = op->inv; op; op = op->below)
        {
            if (op->count == (tag_t) tag)
            {
                /* the simple task: we have a arrow marked */
                if (op->race == type && op->type == ARROW)
                    return op;
                /* we have container marked as missile source. Skip search when there is
                nothing in. Use the standard search now */
                /* because we don't want container in container, we don't care abvout applied */
                if (op->race == type && op->type == CONTAINER)
                {
                    tmp = find_arrow_ext(op, type, -2);
                    return tmp;
                }
            }
        }
        return tmp;
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

