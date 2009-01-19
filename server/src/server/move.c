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

/* object op is trying to move in direction dir.
 * originator is typically the same as op, but
 * can be different if originator is causing op to
 * move (originator is pushing op)
 * returns 0 if the object is not able to move to the
 * desired space, 1 otherwise (in which case we also
 * move the object accordingly
 * Return -1 if the object is destroyed in the move process (most likely
 * when hit a deadly trap or something).
 */
int move_ob(object *op, int dir, object *originator)
{
    object     *tmp;
    mapstruct  *m;
    int         xt, yt, flags;

    if (op == NULL)
    {
        LOG(llevBug, "BUG: move_ob(): Trying to move NULL.\n");
        return 0;
    }

    if (QUERY_FLAG(op, FLAG_REMOVED))
    {
        LOG(llevBug, "BUG: move_ob: monster has been removed - will not process further\n");
        return 0;
    }

    /* this function should now only be used on the head - it won't call itself
     * recursively, and functions calling us should pass the right part.
     */
    if (op->head)
    {
        LOG(llevDebug, "move_ob() called with non head object: %s %s (%d,%d)\n", query_name(op->head),
            op->map->path ? op->map->path : "<no map>", op->x, op->y);
        op = op->head;
    }

    /* animation stuff */
    if (op->head)
        op->head->anim_moving_dir = dir;
    else
        op->anim_moving_dir = dir;
    op->direction = dir;

    xt = op->x + freearr_x[dir];
    yt = op->y + freearr_y[dir];


    /* we have here a out_of_map - we can skip all */
    if (!(m = out_of_map(op->map, &xt, &yt)))
        return 0;

    /* totally new logic here... blocked() handles now ALL map flags... blocked_two()
    * is called implicit from blocked() - really only called for nodes where a checker
    * is inside. blocked_link() is used for multi arch blocked_test().
    * Inside here we use a extended version of blocked_link(). Reason is, that even when
    * we can't move the multi arch on the new spot, we have perhaps a legal earthwall
    * in the step - we need to hit it here too. That a 3x3 multi arch monster can't
    * enter a corridor of 2 tiles is right - but when the entry is closed by a wall
    * then there is no reason why we can't hit the wall - even when we can't enter in.
    */

    /* multi arch objects... */
    if (op->more)
    {
        /* insert new blocked_link() here which can hit ALL earthwalls */
        /* but as long monster don't destroy walls and no mult arch player
         * are ingame - we can stay with this
         */
        /* look in single tile move to see how we handle doors.
         * This needs to be done before we allow multi tile mobs to do
         * more fancy things.
         */
        if (blocked_link(op, freearr_x[dir], freearr_y[dir]))
            return 0;

        remove_ob(op);
        if (check_walk_off(op, originator, MOVE_APPLY_MOVE) & (CHECK_WALK_DESTROYED | CHECK_WALK_MOVED))
            return 1;

        for (tmp = op; tmp != NULL; tmp = tmp->more)
            tmp->x += freearr_x[dir], tmp->y += freearr_y[dir];
        insert_ob_in_map(op, op->map, op, 0);

        return 1;
    }

    /* single arch */
    if (!QUERY_FLAG(op, FLAG_WIZPASS))
    {
        /* is the spot blocked from something? */
        if ((flags = blocked(op, m, xt, yt, op->terrain_flag)))
        {
            /* blocked!... BUT perhaps we have a door here to open.
             * If P_DOOR_CLOSED returned by blocked() then we have a door here.
             * If there is a door but not touchable from op, then blocked()
             * will hide the flag! So, if the flag is set, we can try our
             * luck - but only if op can open doors!
             */
            if ((flags & P_DOOR_CLOSED) && QUERY_FLAG(op, FLAG_CAN_OPEN_DOOR)) /* a (closed) door which we can open? */
            {
                if (open_door(op, m, xt, yt, 1)) /* yes, we can open this door */
                    return 1;
            }

            /* in any case we don't move - door or not. This will avoid we open the door
             * and do the move in one turn.
             */
            return 0;
        }
    }

    remove_ob(op);
    if (check_walk_off(op, originator, MOVE_APPLY_MOVE) & (CHECK_WALK_DESTROYED | CHECK_WALK_MOVED))
        return 1;

    op->x += freearr_x[dir];
    op->y += freearr_y[dir];

    insert_ob_in_map(op, op->map, originator, 0);

    return 1;
}


/*
 * Return value: 1 if object was destroyed, 0 otherwise.
 * Modified so that instead of passing the 'originator' that had no
 * real use, instead we pass the 'user' of the teleporter.  All the
 * callers know what they wanted to teleporter (move_teleporter or
 * shop map code)
 * tele_type is the type of teleporter we want to match against -
 * currently, this is either set to SHOP_MAT or TELEPORTER.
 * It is basically used so that shop_mats and normal teleporters can
 * be used close to each other and not have the player put to the
 * one of another type.
 */
int teleport(object *teleporter, uint8 tele_type, object *user)
{
    object     *altern[120]; /* Better use c/malloc here in the future */
    int         i, j, k, nrofalt = 0, xt, yt;
    object     *other_teleporter, *tmp;
    mapstruct  *m;

    if (user == NULL)
        return 0;
    if (user->head != NULL)
        user = user->head;

    /* Find all other teleporters within range.  This range
     * should really be setable by some object attribute instead of
     * using hard coded values.
     */
    for (i = -5; i < 6; i++)
        for (j = -5; j < 6; j++)
        {
            if (i == 0 && j == 0)
                continue;
            xt = teleporter->x + i;
            yt = teleporter->y + j;
            if (!(m = out_of_map(teleporter->map, &xt, &yt)))
                continue;
            other_teleporter = get_map_ob(m, xt, yt);

            while (other_teleporter)
            {
                if (other_teleporter->type == tele_type)
                    break;
                other_teleporter = other_teleporter->above;
            }
            if (other_teleporter)
                altern[nrofalt++] = other_teleporter;
        }

    if (!nrofalt)
    {
        LOG(llevBug, "BUG: No alternative teleporters around!\n");
        return 0;
    }

    other_teleporter = altern[RANDOM() % nrofalt];
    k = find_free_spot(user->arch, user, other_teleporter->map, other_teleporter->x, other_teleporter->y, 0, 1, SIZEOFFREE1);
    if (k == -1)
        return 0;

    remove_ob(user);
    if (check_walk_off(user, NULL, MOVE_APPLY_VANISHED) != CHECK_WALK_OK)
        return 1;

    /* Update location for the object */
    for (tmp = user; tmp != NULL; tmp = tmp->more)
    {
        tmp->x = other_teleporter->x + freearr_x[k] + (tmp->arch == NULL ? 0 : tmp->arch->clone.x);
        tmp->y = other_teleporter->y + freearr_y[k] + (tmp->arch == NULL ? 0 : tmp->arch->clone.y);
    }

    return (insert_ob_in_map(user, other_teleporter->map, NULL, 0) == NULL);
}

void recursive_roll(object *op, int dir, object *pusher)
{
    if (!roll_ob(op, dir, pusher))
    {
        new_draw_info_format(NDI_UNIQUE, 0, pusher, "You fail to push the %s.", query_name(op));
        return;
    }
    (void) move_ob(pusher, dir, pusher);
    new_draw_info_format(NDI_WHITE, 0, pusher, "You roll the %s.", query_name(op));
    return;
}


/*
 * this is not perfect yet.
 * it does not roll objects behind multipart objects properly.
 */

int roll_ob(object *op, int dir, object *pusher)
{
    object     *tmp;
    mapstruct  *m;
    int         x, y;

    if (op->head)
        op = op->head;

    if (!QUERY_FLAG(op, FLAG_CAN_ROLL)
     || (op->weight && random_roll(0, op->weight / 50000 - 1) > pusher->stats.Str))
        return 0;

    x = op->x + freearr_x[dir];
    y = op->y + freearr_y[dir];
    if (!(m = out_of_map(op->map, &x, &y)))
        return 0;

    for (tmp = get_map_ob(m, x, y); tmp != NULL; tmp = tmp->above)
    {
        if (tmp->head == op)
            continue;
        tmp->direction = dir;
        if (IS_LIVE(tmp) || (QUERY_FLAG(tmp, FLAG_NO_PASS) && !roll_ob(tmp, dir, pusher)))
            return 0;
    }

    if(blocked_link(op, freearr_x[dir], freearr_y[dir]))
        return 0;
    if (QUERY_FLAG(op, FLAG_ANIMATE))
	{
	    op->anim_moving_dir = dir;
        op->direction = dir;
	}
    remove_ob(op);
    if (check_walk_off(op, NULL, MOVE_APPLY_VANISHED) != CHECK_WALK_OK)
        return 0;

    for (tmp = op; tmp != NULL; tmp = tmp->more)
        tmp->x += freearr_x[dir],tmp->y += freearr_y[dir];
    insert_ob_in_map(op, op->map, pusher, 0);
    return 1;
}

/* returns 1 if pushing invokes a attack, 0 when not */
/* new combat command do the attack now - i disabled push attacks */
int push_ob(object *who, int dir, object *pusher)
{
    int     str1, str2;
    object *owner;

    if (who->head != NULL)
        who = who->head;
    owner = get_owner(who);

    /* Wake up sleeping monsters that may be pushed */
    CLEAR_FLAG(who, FLAG_SLEEP);

    /* player change place with his pets or summoned creature */
    /* TODO: allow multi arch pushing. Can't be very difficult */
    if (who->more == NULL && owner == pusher)
    {
        int temp;
        temp = pusher->x;
        temp = pusher->y;
        remove_ob(who);
        if (check_walk_off(who, NULL, MOVE_APPLY_DEFAULT) != CHECK_WALK_OK)
            return 0;

        remove_ob(pusher);
        if (check_walk_off(pusher, NULL, MOVE_APPLY_DEFAULT) != CHECK_WALK_OK)
        {
            /* something is wrong, put who back */
            insert_ob_in_map(who, who->map, NULL, 0);
            return 0;
        }
        pusher->x = who->x;
        who->x = temp;
        pusher->y = who->y;
        who->y = temp;

        insert_ob_in_map(who, who->map, pusher, 0);
        insert_ob_in_map(pusher, pusher->map, pusher, 0);

        return 0;
    }
/* TODO: allow multi arch pushing. Can't be very difficult */
    if (who->more != NULL && owner == pusher)
    {
        remove_ob(who);
        if (check_walk_off(who, NULL, MOVE_APPLY_DEFAULT) != CHECK_WALK_OK)
            return 0;
        move_ob(pusher, dir, pusher);
        pet_follow_owner(who);


        return 1;
    }

    /* We want ONLY become enemy of evil, unaggressive monster. We must RUN in them */
    /* In original we have here a unaggressive check only - that was the reason why */
    /* we so often become an enemy of friendly monsters... */
    /* funny: was they set to unaggressive 0 (= not so nice) they don't attack */

    /* i disabled run/push attacks
       if(owner != pusher &&  pusher->type == PLAYER && who->type != PLAYER &&
                                                    !QUERY_FLAG(who,FLAG_FRIENDLY)) {
    if(CONTR(pusher)->run_on) {
        new_draw_info_format(NDI_UNIQUE, 0, pusher,
                 "You start to attack %s !!",who->name);
           update_npc_knowledge(who, pusher, FRIENDSHIP_PUSH, 0);
           return 1;
    }
    else
    {
        new_draw_info_format(NDI_UNIQUE, 0, pusher,
                 "You avoid to attack %s .",who->name);
    }
       }*/

    /* now, lets test stand still we NEVER can push stand_still monsters. */
    if (QUERY_FLAG(who, FLAG_STAND_STILL))
    {
        new_draw_info_format(NDI_UNIQUE, 0, pusher, "You can't push %s.", who->name);
        return 0;
    }

    /* This block is basically if you are pushing
     * non pet creatures. With move-push attacks removed, we can now
	 * try to push even aggressive mobs, maybe into a fire pit.
     * It basically does a random strength comparision to
     * determine if you can push someone around.  Note that
     * this pushes the other person away - its not a swap.
	 * As of B4 mobs have no str by default. Mapmakers can add
	 * str to a mob, usually for scripted pick up. If a mob has
	 * no str we use 9 + level/10 to give a pseudo str. Also a weight factor
	 * is added for the object being pushed, a beholder should be harder to push
	 * than an ant even if they are the same level.
     */

    str1 = (who->stats.Str > 0 ? who->stats.Str : 9 + who->level / 10);
	str1 = str1 + who->weight / 50000 - 1;
    str2 = (pusher->stats.Str > 0 ? pusher->stats.Str : 9 + pusher->level / 10);
    if (QUERY_FLAG(who, FLAG_WIZ)
     || random_roll(str1, str1 / 2 + str1 * 2)
     >= random_roll(str2, str2/ 2 + str2 * 2)
     || !move_ob(who, dir, pusher))
    {
        new_draw_info_format(NDI_UNIQUE, 0, who, "%s tried to push you.", pusher->name);
        return 0;
    }

    /* If we get here, the push succeeded.  Let each now the
     * status.  I'm not sure if the second statement really needs
     * to be in an else block - the message is going to a different
     * player
     */
    (void) move_ob(pusher, dir, pusher);
    new_draw_info_format(NDI_UNIQUE, 0, who, "%s pushed you.", pusher->name);
    new_draw_info_format(NDI_UNIQUE, 0, pusher, "You pushed %s back.", who->name);

    return 1;
}
int push_roll_object(object * const op, int dir, const int flag)
{

    object     *tmp;
    mapstruct  *m;
    int         xt, yt, ret;
    ret = 0;
	/* we check for all conditions where op can't push anything */
    if (dir <= 0 || CONTR(op)->rest_mode || QUERY_FLAG(op,FLAG_PARALYZED) ||
		QUERY_FLAG(op,FLAG_ROOTED) || IS_AIRBORNE(op))
        return 0;
    xt = op->x + freearr_x[dir];
    yt = op->y + freearr_y[dir];
    if (!(m = out_of_map(op->map, &xt, &yt)))
        return 0;
	for (tmp = get_map_ob(m, xt, yt); tmp != NULL; tmp = tmp->above)
	{
        if (IS_LIVE(tmp)|| QUERY_FLAG(tmp,FLAG_CAN_ROLL))
		{
			break;
		}
	}
	if (tmp == NULL)
	{
        new_draw_info_format(NDI_UNIQUE, 0, op, "You fail to push anything.");
		return 0;
	}
	/* here we try to push pets, players and mobs */
	if (get_owner(tmp)==op || IS_LIVE(tmp))
    {
        play_sound_map(op->map, op->x, op->y, SOUND_PUSH_PLAYER, SOUND_NORMAL);
        if(push_ob(tmp,dir,op))
            ret = 1;
        if(op->hide)
            make_visible(op);
        return ret;
    }
	/* here we try to push moveable objects */
    else if(QUERY_FLAG(tmp,FLAG_CAN_ROLL))
    {
        tmp->direction = dir;
        recursive_roll(tmp,dir,op);
        if(action_makes_visible(op))
            make_visible(op);
    }
    return ret;
}
int missile_reflection_adjust(object *op, int flag)
{
    if (!op->stats.maxgrace) /* no more direction/reflection! */
        return FALSE;

    op->stats.maxgrace--;
    /* restore the "how long we can fly" counter */
    if (!flag)
        op->last_sp = op->stats.grace;

    return TRUE; /* go on with reflection/direction */
}

/* All this really is is a glorified remove_object that also updates
* the counts on the map if needed.
*/
void leave_map(object *op)
{
    mapstruct  *oldmap  = op->map;

    activelist_remove(op);
    remove_ob(op); /* TODO: hmm... never drop inv here? */
    check_walk_off(op, NULL, MOVE_APPLY_VANISHED);

    if (oldmap && !oldmap->player_first && !oldmap->perm_load)
        set_map_timeout(oldmap);
}


/* same as enter_map_by_exit() but without exit_ob().
* Function is used from player loader, scripts and other "direct access" situations.
* By calling without op and/or using MAP_STATUS_NO_FALLBACK and MAP_STATUS_LOAD_ONLY,
* enter_map_by_name() will only load and return single maps.
* There are 2 special cases:
* When called with src_path == NULL, ready_map_name() will only check the loaded maps for
* that path and, when found, we set src_path to newmap->orig_path.
* When called with path == NULL, we create it using flags
* RETURN: loaded map ptr or NULL
*/
mapstruct *enter_map_by_name(object *op, const char *path, const char *src_path, int x, int y, int flags)
{
    mapstruct  *newmap;
    const char *dyn_path = NULL;
    shstr      *reference = (op && op->type == PLAYER) ? op->name : NULL;

    /* new unique maps & instance pathes must be generated by the caller (scripts,...).
    * Easy enough with the 2 helper functions create_unique_path_sh() and create_instance_path_sh()
    */
    if(!path)
    {
        if(!src_path)
            return NULL;

        if(flags & (MAP_STATUS_UNIQUE|MAP_STATUS_INSTANCE))
        {
            if(!op || op->type != PLAYER || !CONTR(op)) /* just some sanity checks */
                return NULL;

            if(flags & MAP_STATUS_UNIQUE)
                path = dyn_path = create_unique_path_sh(op, src_path);
            else /* ATM we always get here a new instance... can't see the sense to use an old one in this case */
            {
                /* a forced instance... this is for example done by a script forcing a player in an instance! */
                CONTR(op)->instance_num = MAP_INSTANCE_NUM_INVALID;
                /* the '0' as flags are right: enter_map_by_exit() will automatically use
                 * the exit_ob settings for this flags.
                 * enter_map_by_name() is called at this point only
                 * from scripts or other controllers which will do the setup after the call.
                 */
                path = dyn_path = create_instance_path_sh(CONTR(op), src_path, 0);
            }
        }
        else /* we can just copy src_path */
            path = src_path; /* not a bug: add_refcount() is not needed because we KNOW that the hash string is valid */
    }
    else
    {
        /* this is our special case for instance maps
         * the caller has requested the direct access to an instance
         * ATM we only check the map is there. If the map exists, the
         * instance exists too because the "unique directory setup".
         * We can add here more tricky stuff like instances with a time limit
         * or instances which don't allow a relogin.
         */
        if(src_path && flags & MAP_STATUS_INSTANCE)
        {
            if( check_path(path, FALSE) == -1) /* file don't exits */
            {
                /* for non player just return with NULL...
                 * for player override with his bind point now
                 */
                if(!op || op->type != PLAYER || !CONTR(op) || flags & (MAP_STATUS_NO_FALLBACK|MAP_STATUS_LOAD_ONLY))
                    return NULL;

                path = CONTR(op)->savebed_map;
                src_path = CONTR(op)->orig_savebed_map;
                flags = CONTR(op)->bed_status;
                x = CONTR(op)->bed_x;
                y = CONTR(op)->bed_y;
            }
        }
    }

    newmap = ready_map_name(path, src_path, MAP_STATUS_TYPE(flags), reference);
    FREE_ONLY_HASH(dyn_path);

    if (!newmap) /* map don't exists, fallback to savebed and/or emergency when possible */
    {
        if(!op || flags & MAP_STATUS_NO_FALLBACK || !src_path) /* we only try path !  - no fallback to a different map */
            return NULL;

        /* for player we first try to the bind point (aka savebed) */
        if(op->type == PLAYER && CONTR(op))
        {
            LOG( llevBug, "BUG: enter_map_by_name(): pathname to map does not exist! player: %s (%s)\n", query_name(op), STRING_SAFE(src_path));
            newmap = ready_map_name(CONTR(op)->savebed_map, CONTR(op)->orig_savebed_map, CONTR(op)->bed_status, reference);
            x = CONTR(op)->bed_x;
            y = CONTR(op)->bed_y;

            if(!newmap)
            {
                /* something is wrong with our bind point... reset */
                set_bindpath_by_default(CONTR(op));
                newmap = ready_map_name(shstr_cons.emergency_mappath, shstr_cons.emergency_mappath, MAP_STATUS_MULTI, reference);

                /* If we can't load the emergency map, something is probably really screwed up, so bail out now. */
                if (!newmap)
                    LOG(llevError, "ERROR: enter_map_by_name(): could not load emergency map? Fatal error! (player: %s)\n", query_name(op));
            }
        }
        else /* we NEVER use the emergency map for mobs */
            return NULL;

    }

    if(!op || flags & MAP_STATUS_LOAD_ONLY )
        return NULL;

    /* This is EXTREMLY useful for quest maps with treasure rooms. */
    if (MAP_FIXEDLOGIN(newmap) || flags & MAP_STATUS_FIXED_LOGIN || x == -1 || y == -1)
    {
        x = MAP_ENTER_X(newmap);
        y = MAP_ENTER_Y(newmap);
    }

    if(!src_path) /* special case - we have identified the map by his dest path from loaded map list */
    {
        src_path = add_refcount(newmap->orig_path);
        /* because we can't be sure in this case about the map status, we overrule it with the loaded map! */
        flags = newmap->map_status;
    }

    /* have in mind that op can be destroyed by enter_map() */
    enter_map(op, NULL, newmap, x, y, flags);

    return newmap;
}

/* Tries to move 'op' to exit_ob.  op is the character or monster that is
* using the exit, where exit_ob is the exit object (boat, door, teleporter,
* etc.)
* This is now the one and only "use an exit" function. Every object from type EXIT
* or TELEPORTER will handled here. Beside the instance maps is the biggest change in
* map handling that we get the type of maps now with a "inheritance" concept from the
* root (caller) map. If the exit don't change explicit the map type (normal, unique, instance...)
* the type is the same as the map where the exit object is part off (or always "normal" when
* there is no root map.
* RETURN: TRUE: we have loaded and entered a map. FALSE: we failed to enter
* MT-2006
*/
int enter_map_by_exit(object *op, object *exit_ob)
{
    int flags, mstatus;
    mapstruct  *newmap;
    const char *file_path = NULL, *dyn_path = NULL;
    shstr      *reference = NULL;

    if (op->head)
        op = op->head;

    if (!exit_ob)
    {
        LOG(llevBug, "BUG: enter_map_by_exit(): called with object %s but without exit ob!\n", query_name(op));
        return FALSE;
    }

    /* Event trigger and quick exit */
    if(trigger_object_plugin_event(EVENT_TRIGGER,
                exit_ob, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
        return FALSE;

    if(! EXIT_PATH(exit_ob))
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "The %s is closed.", query_name(exit_ob));
        return FALSE;
    }

    /* our target map is in ->slaying in non normalized format, in last_eat the type.
    * Non normalized means, it can be something like "../test/../test/....."
    * To know path/map A is the same like B we need to normalize the path & name.
    * in ->race *can* be the normalized src path, *if* the exit was used before.
    * if not, we have to process it now first.
    */
    if(!exit_ob->race)
    {
        char tmp_path[MAXPATHLEN];

        /* note the use of ->orig_path as normalized src path inside /maps */
        exit_ob->race = add_string(normalize_path(exit_ob->map->orig_path, EXIT_PATH(exit_ob), tmp_path));
    }

    /* now we have some choices:
    * If the new map type is inheritanced from exit_ob->map , we can generate
    * a static path which will not change for this map/exit anymore.
    * For an explicit _INSTANCE and _UNIQUE we have always to use a dynamic path.
    * For MAP_STATUS_MULTI we can simply copy the ob->race ptr.
    */
    if(MAP_STATUS_TYPE(exit_ob->last_eat))
    {
        reference = op->name;

        if(exit_ob->last_eat & (MAP_STATUS_MULTI|MAP_STATUS_STYLE))
            file_path = exit_ob->race; /* multi == original map path */
        else /* dynamic path */
        {
            if(op->type != PLAYER || !CONTR(op))
                return FALSE;

            if(exit_ob->last_eat & MAP_STATUS_UNIQUE)
                file_path = dyn_path = create_unique_path_sh(op, exit_ob->race);
            else if(exit_ob->last_eat & MAP_STATUS_INSTANCE)
            {
                player *pl = CONTR(op);

                /* we give here a player a "temporary" instance directory inside /instance
                * which is identified by global_instance_num (directory name  = itoa(num)).
                * we use the normalized original map path of the exit_ob as identifier of the
                * instanced map or map set itself.
                */
                if( pl->instance_name == exit_ob->race && pl->instance_id == global_instance_id)
                {
                    /* add here the instance validation checks for this player */
                }
                else
                    pl->instance_num = MAP_INSTANCE_NUM_INVALID; /* if set, we generate a NEW instance! */

                /* create_instance..() will try to load an old instance, will fallback to
                * a new one if needed and setup all what need be done to start the instance.
                */
                file_path = dyn_path = create_instance_path_sh(pl, exit_ob->race,
                                        QUERY_FLAG(exit_ob, FLAG_IS_MALE)?INSTANCE_FLAG_NO_REENTER:0);
            }
            else
                LOG(llevError, "FATAL ERROR: enter_map_by_exit(): Map %s without valid map status (dynamic) (%d)!\n", STRING_MAP_PATH(exit_ob->map));
        }
    }
    else /* dst path is static and stored in exit_ob->title */
    {
        reference = exit_ob->map->reference;

        if(!exit_ob->title) /* first call, generate the static path first */
        {
            if(exit_ob->map->map_status & (MAP_STATUS_MULTI|MAP_STATUS_STYLE))
                exit_ob->title = add_refcount(exit_ob->race); /* multi = original map path */
            else if(exit_ob->map->map_status & (MAP_STATUS_INSTANCE|MAP_STATUS_UNIQUE) )
            {
                char tmp_path[MAXPATHLEN];

                /* we have now this:
                * - in map->path a normalized path to /players or /instance
                * - in exit_ob->race the normalized path + name to the original map
                * we create now a new path out of it by using the root from ->path
                * and use path_to_name() to exchange all '/' through '$' to have a unique map name.
                * NOTE: the path to /maps is always part of the unique/instance map name.
                */
                exit_ob->title = add_string(normalize_path_direct( exit_ob->map->path,
                    path_to_name(exit_ob->race), tmp_path));
            }
            else /* this should never happen and will break the inheritance system - so kill the server */
                LOG(llevError, "FATAL ERROR: enter_map_by_exit(): Map %s loaded without valid map status (%d)!\n",
                    STRING_MAP_PATH(exit_ob->map),exit_ob->map->map_status);
        }
        file_path = exit_ob->title;
    }

    /* lets fetch the right map status */
    mstatus = exit_ob->last_eat ? (int)MAP_STATUS_TYPE(exit_ob->last_eat) : (int)MAP_STATUS_TYPE(exit_ob->map->map_status);

    /* get the map ptr - load the map if needed */
    newmap = ready_map_name( file_path, exit_ob->race, mstatus, reference);

    FREE_ONLY_HASH(dyn_path);

    if (!newmap)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "The %s is closed.", query_name(exit_ob));
        return FALSE;
    }

    /* Now we got for sure transported away.
     * IF this exit has the "neutralize instance" flag set AND the old map type
     * was instance and the new one NOT - then neutralize the instance now.
     */
    if(exit_ob->map->map_status & MAP_STATUS_INSTANCE && QUERY_FLAG(exit_ob, FLAG_IS_FEMALE)
        && !(mstatus & MAP_STATUS_INSTANCE))
        reset_instance_data(CONTR(op));

    /* lets play a sound where we have left the map */
    if (exit_ob->sub_type1 == ST1_EXIT_SOUND && exit_ob->map)
        play_sound_map(exit_ob->map, exit_ob->x, exit_ob->y, SOUND_TELEPORT, SOUND_NORMAL);

    /* Send any exit message to exiter */
    if (exit_ob->msg)
        new_draw_info(NDI_NAVY, 0, op, exit_ob->msg);

    /* collect the flag settings from our exit_ob */
    flags = EXIT_STATUS(exit_ob);
    if(EXIT_POS_FIX(exit_ob))
        flags |=  MAP_STATUS_FIXED_POS;
    if(EXIT_POS_FREE(exit_ob))
        flags |=  MAP_STATUS_FREE_POS_ONLY;
    if(EXIT_POS_RANDOM(exit_ob))
        flags |=  MAP_STATUS_RANDOM_POS;

    /* if 1 was returned from enter_map() object was destroyed in the meantime! */
    if(enter_map( op, NULL, newmap, EXIT_X(exit_ob), EXIT_Y(exit_ob), flags))
        return FALSE;

    /* some "exits" like a pit will move you for sure to a new map - but for a price ... */
    if (exit_ob && exit_ob->stats.dam)
        damage_ob(op, exit_ob->stats.dam, exit_ob, ENV_ATTACK_CHECK);

    return TRUE;
}


#if 0
/* Random maps are disabled ATM.
* TODO: reactivate the module, implement it as a threaded instance and add it to the
* map instance patch
* This function should then be merged into enter_map().
* I leaved it here as example.
*/
/* The player is trying to enter a randomly generated map.  In this case, generate the
* random map as needed.
*/
static void enter_random_map(object *pl, object *exit_ob)
{
    mapstruct  *new_map;
    char        newmap_name[HUGE_BUF];
    static int  reference_number    = 0;
    RMParms     rp;

    memset(&rp, 0, sizeof(RMParms));
    rp.Xsize = -1;
    rp.Ysize = -1;
    if (exit_ob->msg)
        set_random_map_variable(&rp, exit_ob->msg);
    rp.origin_x = exit_ob->x;
    rp.origin_y = exit_ob->y;
    rp.generate_treasure_now = 1;
    strcpy(rp.origin_map, pl->map->path);

    /* pick a new pathname for the new map.  Currently, we just
    * use a static variable and increment the counter one each time.
    */
    sprintf(newmap_name, "/random/%016d", reference_number++);

    /* now to generate the actual map. */
    new_map = (mapstruct *) generate_random_map(newmap_name, &rp);

    /* Update the exit_ob so it now points directly at the newly created
    * random maps.  Not that it is likely to happen, but it does mean that a
    * exit in a unique map leading to a random map will not work properly.
    * It also means that if the created random map gets reset before
    * the exit leading to it, that the exit will no longer work.
    */
    if (new_map)
    {
        int x, y;
        x = EXIT_X(exit_ob) = MAP_ENTER_X(new_map);
        y = EXIT_Y(exit_ob) = MAP_ENTER_Y(new_map);
        FREE_AND_COPY_HASH(EXIT_PATH(exit_ob), newmap_name);
        FREE_AND_COPY_HASH(new_map->path, newmap_name);
        enter_map(pl, new_map, x, y, 0);
    }
}
#endif

/* Try to find a spot for op on map on or near x, y depending on mode and
 * freeonly.
 * If a spot is found, the return is >= 0. This is the index into
 * freearr_x/freearr_y. Otherwise, the return is -1. */
int check_insertion_allowed(object *op, mapstruct *map, int x, int y, int mode, int freeonly, int los)
{
    int i;

    switch (mode)
    {
        /* first available location */
        case 1:
            i = find_first_free_spot(op->arch, op, map, x, y);

            break;

        /* Fixed location. */
        case 2:
            i = (arch_blocked(op->arch, op, map, x, y)) ? -1 : 0;

            break;

        /* Random location, 1 squares radius. */
        case 3:
            i = find_free_spot(op->arch, op, map, x, y, 0, 0, SIZEOFFREE1);

            break;

        /* Random location, 2 squares radius. */
        case 4:
            i = find_free_spot(op->arch, op, map, x, y, los, 0, SIZEOFFREE2);

            break;

        /* Random location, 3 squares radius. */
        case 5:
            i = find_free_spot(op->arch, op, map, x, y, los, 0, SIZEOFFREE - 1);

            break;

        /* Random location, progressive radius. */
        case 6:
            i = find_free_spot(op->arch, op, map, x, y, 0, 0, SIZEOFFREE1);

            if (i == -1)
            {
                i = find_free_spot(op->arch, op, map, x, y, los, SIZEOFFREE1 + 1, SIZEOFFREE2);

                if (i == -1)
                    i = find_free_spot(op->arch, op, map, x, y, los, SIZEOFFREE2 + 1, SIZEOFFREE - 1);
            }

            break;

        default:
            LOG(llevDebug, "DEBUG:: %s/check_insertion_allowed(): Illegal mode (defaulting to first available location)!\n",
                __FILE__);
            i = find_first_free_spot(op->arch, op, map, x, y);
    }

#if 0
    LOG(llevDebug, "DEBUG: %s/check_insertion_allowed(): mode=%d, freeonly=%d, i=%d\n",
        __FILE__, mode, freeonly, i);
#endif

    /* Couldn't find a free spot? If we have specified free spot only, this
     * means failure. Otherwise, force to the default spot. */
    if (i == -1)
    {
        if (freeonly)
            return -1;
        else
            return 0;
    }

    return i;
}

/*
* enter_map():  Moves the player and pets from current map (if any) to
* new map.  map, x, y must be set.  map is the map we are moving the
* player to - it could be the map he just came from.
* The place the object is put on is x,y. Depending on setting MAP_STATUS_FIXED_POS
* the function tries to find a free spot around it or use it as fixed position.
* @return 1 if object was destroyed, 0 otherwise.
*/
int enter_map(object *op, object *originator, mapstruct *newmap, int x, int y, int flags)
{
    int        mode,
               freeonly,
               i;
    object    *tmp;
    mapstruct *oldmap = op->map;
    player    *pl;

    if (op->head)
    {
        op = op->head;
        LOG(llevBug, "BUG: enter_map(): called from tail of object! (obj:%s map: %s (%d,%d))\n", op->name, newmap->path,
            x, y);
    }

    /* this is a last secure check. In fact, newmap MUST legal and we only
    * check x and y. No out_of_map() - we want check that x,y is part of this newmap.
    * if not, we have somewhere missed some checks - give a note to the log.
    */
    if (OUT_OF_REAL_MAP(newmap, x, y))
    {
        LOG(llevBug, "BUG: enter_map(): supplied coordinates are not within the map! (obj:%s map: %s (%d,%d))\n",
            op->name, newmap->path, x, y);
        x = MAP_ENTER_X(newmap);
        y = MAP_ENTER_Y(newmap);
    }

    if (flags & MAP_STATUS_RANDOM_POS)
        mode = 6;
    else if (flags & MAP_STATUS_RANDOM_POS_3)
        mode = 5;
    else if (flags & MAP_STATUS_RANDOM_POS_2)
        mode = 4;
    else if (flags & MAP_STATUS_RANDOM_POS_1)
        mode = 3;
    else if (flags & MAP_STATUS_FIXED_POS)
        mode = 2;
    else
        mode = 1;

    freeonly = (flags & MAP_STATUS_FREE_POS_ONLY) ? 1 : 0;

    if ((i = check_insertion_allowed(op, newmap, x, y, mode, freeonly, 1)) == -1)
        return 0;

    /* If it is a player login, he has yet to be inserted anyplace.
    * otherwise, we need to deal with removing the playe here.
    */
    if (!QUERY_FLAG(op, FLAG_REMOVED))
    {
        remove_ob(op);
        if (check_walk_off(op, originator, MOVE_APPLY_DEFAULT) != CHECK_WALK_OK)
            return 1;
    }

#if 0
    if (op->map != NULL && op->type == PLAYER && !op->head)
    {
        int         evtid;
        CFParm      CFP;

        /* GROS : Here we handle the MAPLEAVE global event */
        evtid = EVENT_MAPLEAVE;
        CFP.Value[0] = (void *) (&evtid);
        CFP.Value[1] = (void *) (op);
        GlobalEvent(&CFP);
    };
#endif

    /* set single or all part of a multi arch */
    for (tmp = op; tmp != NULL; tmp = tmp->more)
    {
        tmp->x = tmp->arch->clone.x + x + freearr_x[i];
        tmp->y = tmp->arch->clone.y + y + freearr_y[i];
    }
    if (!insert_ob_in_map(op, newmap, originator, 0))
        return 1;

    newmap->timeout = 0;

    /* do some action special for players after we have inserted them */
    if (op->type == PLAYER && (pl = CONTR(op)))
    {
        pl->count = 0;

        /* Update any golems */
        if (pl->golem != NULL)
        {
            int i   = find_free_spot(pl->golem->arch, op, newmap, x, y, 1, 1, SIZEOFFREE - 1);

            remove_ob(pl->golem);
            if (check_walk_off(pl->golem, NULL, MOVE_APPLY_VANISHED) != CHECK_WALK_OK)
                i = -1;

            /* why not using the same spot as the we insert the player for the golem, when
            * there is nothing free? It does no harm and its more senseful as this.
            */
            if (i == -1)
            {
                send_golem_control(pl->golem, GOLEM_CTR_RELEASE);
                pl->golem = NULL;
            }
            else
            {
                object *tmp;
                for (tmp = pl->golem; tmp != NULL; tmp = tmp->more)
                {
                    tmp->x = x + freearr_x[i] + (tmp->arch == NULL ? 0 : tmp->arch->clone.x);
                    tmp->y = y + freearr_y[i] + (tmp->arch == NULL ? 0 : tmp->arch->clone.y);
                    tmp->map = newmap;
                }
                /* we assume insert_ob_in_map() will release the golem when its destroyed */
                if (insert_ob_in_map(pl->golem, newmap, NULL, 0))
                    pl->golem->direction = find_dir_2(op->x - pl->golem->x, op->y - pl->golem->y);
            }
        }

        op->direction = 0;

        /* If the player is changing maps, we need to do some special things
        * Do this after the player is on the new map - otherwise the force swap of the
        * old map does not work.
        */
        if (oldmap != newmap && oldmap && !oldmap->player_first && !oldmap->perm_load)
            set_map_timeout(oldmap);

#ifdef MAX_OBJECTS_LWM
        swap_below_max(newmap->path);
#endif
    }
    return 0;
}
