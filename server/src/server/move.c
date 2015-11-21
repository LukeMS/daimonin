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

static int PushLiving(object_t *who, sint8 dir, object_t *pusher);

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
sint8 move_ob(object_t *who, sint8 dir, object_t *originator)
{
    uint32  block;
    object_t *part,
           *next;
    sint16  wx,
            wy;
    sint8   ox,
            oy;

    /* Sanity checks. */
    if (!who)
    {
        LOG(llevBug, "BUG: move_ob(): Trying to move NULL.\n");
        return MOVE_RESULT_INSERTION_FAILED;
    }
    else if (QUERY_FLAG(who, FLAG_REMOVED))
    {
        LOG(llevBug, "BUG: move_ob: monster has been removed - will not process further\n");
        return MOVE_RESULT_INSERTION_FAILED;
    }
    else if (who->head)
    {
        LOG(llevBug, "BUG:: move_ob() called with non head object: %s %s (%d,%d)\n",
            STRING_OBJ_NAME(who->head), STRING_MAP_PATH(who->map), who->x, who->y);
        who = who->head;
    }

    who->anim_moving_dir = who->direction = dir;
    wx = who->x;
    wy = who->y;
    ox = OVERLAY_X(dir);
    oy = OVERLAY_Y(dir);

    /* Nothing can move out of map, obviously. */
    if ((block = msp_blocked(who, NULL, ox, oy)) == (uint32)MSP_FLAG_OUT_OF_MAP)
    {
        return MOVE_RESULT_INSERTION_FAILED;
    }
    /* Otherwise only consider blockages if not a gmaster with wizpass. */
    else if (!IS_GMASTER_WIZPASS(who))
    {
        /* If the block is a door (ONLY) this means msp_blocked() has already
         * done the necessary checks; we know already that those msps are valid
         * and unblocked (apart from the door(s)) and that who can open any
         * door(s) on those msps so all we need to do is issue the
         * instructions. */
        /* Much of the following code block is adapted from msp_blocked(), so
         * see that function for futher explanation. */
        if (block == MSP_FLAG_DOOR_CLOSED)
        {
            /* For multiparts we need to loop through each part again to find
             * the msps of the non-overlapping parts after the move. */
            if (who->more)
            {
                FOREACH_PART_OF_OBJECT(part, who, next)
                {
                    sint16     x2 = part->arch->clone.x + ox,
                               y2 = part->arch->clone.y + oy;
                    object_t    *part2,
                              *next2;

                    FOREACH_PART_OF_OBJECT(part2, who, next2)
                    {
                        if (x2 == part2->arch->clone.x &&
                            y2 == part2->arch->clone.y)
                        {
                            break;
                        }
                    }

                    if (!part2)
                    {
                        map_t *m = who->map;
                        sint16     x = part->x + ox,
                                   y = part->y + oy;
                        msp_t  *msp = MSP_GET2(m, x, y);

                        (void)open_door(who, msp, 1);
                    }
                }
            }
            /* Singleparts are easy. */
            else
            {
                map_t *m = who->map;
                sint16     x = wx + ox,
                           y = wy + oy;
                msp_t  *msp = MSP_GET2(m, x, y);

                (void)open_door(who, msp, 1);
            }

            /* When who opens a door, that's his movement over for this turn. */
            return MOVE_RESULT_SUCCESS;
        }
        /* Any other block halts movement. */
        else if (block)
        {
            return MOVE_RESULT_INSERTION_FAILED;
        }
    }

    remove_ob(who);

    /* Remember that the actual process of moving from A to B can itself result
     * in the destruction of who (for example, who steps off a button which
     * triggers an explosion) so we're not home free yet. */
    if (check_walk_off(who, originator, MOVE_APPLY_MOVE) != CHECK_WALK_OK)
    {
        return MOVE_RESULT_WHO_DESTROYED;
    }

    /* If we've got this far, who is actually able to be moved. */
    /* This works for both multiparts and singleparts. */
    FOREACH_PART_OF_OBJECT(part, who, next)
    {
        part->x += ox;
        part->y += oy;
    }

    insert_ob_in_map(who, who->map, originator, 0);
    return MOVE_RESULT_SUCCESS;
}

/*
 * Return value: 1 if object was destroyed, 0 otherwise.
 * Modified so that instead of passing the 'originator' that had no
 * real use, instead we pass the 'user' of the teleporter.  All the
 * callers know what they wanted to teleporter (move_teleporter or
 * shop map code)
 */
int teleport(object_t *teleporter, object_t *user)
{
    object_t *altern[TINY_BUF];
    int     i, j, nrofalt = 0;
    object_t *other_teleporter;
    msp_t  *msp;
    object_t *this;

    if (user == NULL)
        return 0;
    if (user->head != NULL)
        user = user->head;

    /* Find all other teleporters within range.  This range
     * should really be setable by some object attribute instead of
     * using hard coded values.
     */
    for (i = -5; i < 6; i++)
    {
        for (j = -5; j < 6; j++)
        {
            map_t *mt;
            sint16     xt,
                       yt;
            object_t    *next;

            if (i == 0 &&
                j == 0)
            {
                continue;
            }

            mt = teleporter->map;
            xt = teleporter->x + i;
            yt = teleporter->y + j;
            msp = MSP_GET(mt, xt, yt);

            if (!msp)
            {
                continue;
            }

            FOREACH_OBJECT_IN_MSP(this, msp, next)
            {
                if (this->type == teleporter->type)
                {
                    altern[nrofalt++] = this;
                    break;
                }
            }
        }
    }

    if (!nrofalt)
    {
        LOG(llevMapbug, "MAPBUG:: %s[%s %d %d]: No destination %s around!\n",
            STRING_OBJ_NAME(teleporter), STRING_MAP_PATH(teleporter->map),
            teleporter->x, teleporter->y,
            (teleporter->type == SHOP_MAT) ? "shop mats" : "teleporters");
        return 0;
    }

    other_teleporter = altern[RANDOM() % nrofalt];
    msp = MSP_KNOWN(other_teleporter);
    return enter_map(user, msp, teleporter, OVERLAY_FIRST_AVAILABLE | OVERLAY_SPECIAL, 0);
}

int push_roll_object(object_t * const op, int dir, const int flag)
{
    map_t *mt;
    sint16     xt,
               yt;
    msp_t  *msp;
    object_t    *this,
              *next;

    /* we check for all conditions where op can't push anything */
    if (dir <= 0 ||
        CONTR(op)->rest_mode ||
        QUERY_FLAG(op,FLAG_PARALYZED) ||
        QUERY_FLAG(op,FLAG_ROOTED) ||
        IS_AIRBORNE(op))
    {
        return 0;
    }

    mt = op->map;
    xt = op->x + OVERLAY_X(dir);
    yt = op->y + OVERLAY_Y(dir);
    msp = MSP_GET2(mt, xt, yt);

    if (!msp)
    {
        return 0;
    }

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        this = (this->head) ? this->head : this;

        if (IS_LIVE(this))
        {
            play_sound_map(MSP_KNOWN(op), SOUND_PUSH_PLAYER, SOUND_NORMAL);
            return PushLiving(this, dir, op);
        }
        else if (QUERY_FLAG(this, FLAG_CAN_ROLL))
        {
            if ((random_roll(0, this->weight / 50000 - 1) > op->stats.Str) ||
                move_ob(this, dir, op) == MOVE_RESULT_INSERTION_FAILED)
            {
                ndi(NDI_UNIQUE, 0, op, "You fail to push %s.",
                    QUERY_SHORT_NAME(this, op));
            }
            else
            {
                (void)move_ob(op, dir, NULL);
                ndi(NDI_WHITE, 0, op, "You roll %s.",
                    QUERY_SHORT_NAME(this, op));
            }
            return 0;
        }
    }

    return 0;
}

/* returns 1 if pushing invokes a attack, 0 when not */
static int PushLiving(object_t *who, sint8 dir, object_t *pusher)
{
    int     str1,
            str2;
    object_t *owner = get_owner(who);

    /* Wake up sleeping monsters that may be pushed */
    CLEAR_FLAG(who, FLAG_SLEEP);

    /* player change place with his pets or summoned creature */
    /* TODO: allow multi arch pushing. Can't be very difficult */
    if (who->more == NULL && owner == pusher)
    {
        sint16 x = pusher->x,
               y = pusher->y;

        remove_ob(who);
        if (check_walk_off(who, NULL, 0) != CHECK_WALK_OK)
            return 0;

        remove_ob(pusher);
        if (check_walk_off(pusher, NULL, 0) != CHECK_WALK_OK)
        {
            /* something is wrong, put who back */
            insert_ob_in_map(who, who->map, NULL, 0);
            return 0;
        }
        pusher->x = who->x;
        who->x = x;
        pusher->y = who->y;
        who->y = y;

        insert_ob_in_map(who, who->map, pusher, 0);
        insert_ob_in_map(pusher, pusher->map, pusher, 0);

        return 0;
    }
/* TODO: allow multi arch pushing. Can't be very difficult */
    if (who->more != NULL && owner == pusher)
    {
        remove_ob(who);
        if (check_walk_off(who, NULL, 0) != CHECK_WALK_OK)
            return 0;
        (void)move_ob(pusher, dir, pusher);
        pet_follow_owner(who);


        return 1;
    }

    /* now, lets test stand still we NEVER can push stand_still monsters. */
    if (QUERY_FLAG(who, FLAG_STAND_STILL))
    {
       ndi(NDI_UNIQUE, 0, pusher, "You can't push %s.",
           QUERY_SHORT_NAME(who, pusher));
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
     * than an ant even if they are the same level. */
    str1 = (who->stats.Str > 0 ? who->stats.Str : 9 + who->level / 10);
    str1 = str1 + who->weight / 50000 - 1;
    str2 = (pusher->stats.Str > 0 ? pusher->stats.Str : 9 + pusher->level / 10);

    if (random_roll(str1, str1 / 2 + str1 * 2) >= random_roll(str2, str2 / 2 + str2 * 2) ||
        move_ob(who, dir, pusher) != MOVE_RESULT_SUCCESS)
    {
        ndi(NDI_UNIQUE, 0, who, "%s tried to push you.",
            QUERY_SHORT_NAME(pusher, who));
        return 0;
    }

    /* If we get here, the push succeeded.  Let each now the
     * status.  I'm not sure if the second statement really needs
     * to be in an else block - the message is going to a different
     * player
     */
    (void)move_ob(pusher, dir, NULL);
    ndi(NDI_UNIQUE, 0, who, "%s pushed you.",
        QUERY_SHORT_NAME(pusher, who));
    ndi(NDI_UNIQUE, 0, pusher, "You pushed %s back.",
        QUERY_SHORT_NAME(who, pusher));
    return 1;
}

int missile_reflection_adjust(object_t *op, int flag)
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
 * the counts on the map if needed. newmap is the map pl is going to, or NULL
 * if pl is logging out. */
uint8 leave_map(player_t *pl, map_t *newmap)
{
    map_t *oldmap = pl->ob->map;
    uint8      r;

    remove_ob(pl->ob); /* TODO: hmm... never drop inv here? */
    r = check_walk_off(pl->ob, NULL, MOVE_APPLY_VANISHED);

    if (oldmap &&
        oldmap != newmap)
    {
        /* This is an anti-duping measure: when a player leaves an instance or
         * unique map, save the player file. The map is then also saved below,
         * meaning that the two object list should not get out of sync. */
        if (newmap &&
            ((oldmap->status & (MAP_STATUS_INSTANCE | MAP_STATUS_UNIQUE))))
        {
            (void)player_save(pl->ob);
        }

        if ((oldmap->status & MAP_STATUS_MULTI) ||
            map_save(oldmap))
        {
            /* When there are still players or (TODO) permanently loading mobs on
             * the map, mark it as still in memory. */
            if (oldmap->player_first ||
                oldmap->perm_load)
            {
                oldmap->in_memory = MAP_MEMORY_ACTIVE;
            }
        }
    }

    return r;
}

sint8 enter_map(object_t *who, msp_t *msp, object_t *originator, uint8 oflags, uint32 iflags)
{
    sint8   i;
    object_t *part,
           *next;
    player_t *pl = (who->type == PLAYER) ? CONTR(who) : NULL;

    if (who->head)
    {
        who = who->head;
    }

    i = overlay_find_free_by_flags(msp, who, oflags);

    /* If no spot could be found (i = -1), return insertion failed. If the
     * spot is the one at m, x, y (i = 0), msp as calculated above is still
     * valid. Otherwise (i > 0), recalculate msp (using MSP_GET() so that
     * out_of_map() is called; this is because the new x, y may be on a
     * different map). */
    if (i == -1)
    {
        return MOVE_RESULT_INSERTION_FAILED;
    }
    else if (i > 0)
    {
        map_t  *m = msp->map;
        sint16  x = msp->x + OVERLAY_X(i),
                y = msp->y + OVERLAY_Y(i);

        msp = MSP_GET(m, x, y);
    }

    /* If it is a player login, he has yet to be inserted anyplace.
     * otherwise, we need to deal with removing the object here. */
    if (!QUERY_FLAG(who, FLAG_REMOVED))
    {
        if (pl)
        {
            if (leave_map(pl, msp->map) != CHECK_WALK_OK)
            {
                return MOVE_RESULT_WHO_DESTROYED;
            }
        }
        else
        {
            remove_ob(who);

            if (check_walk_off(who, originator, 0) != CHECK_WALK_OK)
            {
                return MOVE_RESULT_WHO_DESTROYED;
            }
        }
    }

    /* set single or all part of a multi arch */
    FOREACH_PART_OF_OBJECT(part, who, next)
    {
        part->x = msp->x + part->arch->clone.x;
        part->y = msp->y + part->arch->clone.y;
    }

    if (!insert_ob_in_map(who, msp->map, originator, iflags))
    {
        return MOVE_RESULT_WHO_DESTROYED;
    }

    /* do some action special for players after we have inserted them */
    if (pl)
    {
        pl->count = 0;

        if (pl->tadoffset != who->map->tadoffset)
        {
            (void)command_time(who, "verbose");
            pl->tadoffset = who->map->tadoffset;
        }

        /* TODO: Pets, golems? */
    }

    return MOVE_RESULT_SUCCESS;
}

/* Function is used from player loader, scripts and other "direct access" situations.
 * By calling without who and/or using MAP_STATUS_NO_FALLBACK and MAP_STATUS_LOAD_ONLY,
 * will only load and return single maps.
 * There are 2 special cases:
 * When called with orig_path_sh == NULL, ready_map_name() will only check the loaded maps for
 * that path_sh and, when found, we set orig_path_sh to newmap->orig_path.
 * When called with path_sh == NULL, we create it using flags
 * RETURN: loaded map ptr or NULL */
sint8 enter_map_by_name(object_t *who, shstr_t *path_sh, shstr_t *orig_path_sh, sint16 x, sint16 y, uint32 mflags)
{
    player_t     *pl = (who && who->type == PLAYER) ? CONTR(who) : NULL;
    shstr_t      *reference;
    map_t  *m;
    msp_t   *msp;

    /* new unique maps & instance pathes must be generated by the caller (scripts,...).
     * Easy enough with the 2 helper functions create_unique_path_sh() and create_instance_path_sh() */
    if (!path_sh)
    {
        if (!orig_path_sh)
        {
            return MOVE_RESULT_INSERTION_FAILED;
        }

        if ((mflags & (MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE)))
        {
            /* Sanity checks. */
            if (!pl)
            {
                return MOVE_RESULT_INSERTION_FAILED;
            }

            if ((mflags & MAP_STATUS_UNIQUE))
            {
                path_sh = create_unique_path_sh(who->name, orig_path_sh);
            }
            else /* ATM we always get here a new instance... can't see the sense to use an old one in this case */
            {
                /* a forced instance... this is for example done by a script forcing a player in an instance! */
                pl->instance_num = MAP_INSTANCE_NUM_INVALID;
                /* the '0' as mflags are right: will automatically use
                 * the exit_ob settings for this mflags.
                 * enter_map_by_name() is called at this point only
                 * from scripts or other controllers which will do the setup after the call. */
                path_sh = create_instance_path_sh(pl, orig_path_sh, 0);
            }
        }
        else /* we can just copy orig_path_sh */
        {
            path_sh = orig_path_sh; /* not a bug: add_refcount() is not needed because we KNOW that the hash string is valid */
        }
    }
    else
    {
        /* this is our special case for instance maps
         * the caller has requested the direct access to an instance
         * ATM we only check the map is there. If the map exists, the
         * instance exists too because the "unique directory setup".
         * We can add here more tricky stuff like instances with a time limit
         * or instances which don't allow a relogin. */
        if (orig_path_sh &&
            (mflags & MAP_STATUS_INSTANCE))
        {
            if (check_path(path_sh, FALSE) == -1) /* file don't exist */
            {
                /* for non player just return with NULL...
                 * for player override with his bind point now */
                if (!pl ||
                    (mflags & (MAP_STATUS_NO_FALLBACK | MAP_STATUS_LOAD_ONLY)))
                {
                    return MOVE_RESULT_INSERTION_FAILED;
                }

                path_sh = pl->savebed_map;
                orig_path_sh = pl->orig_savebed_map;
                mflags = pl->bed_status;
                x = pl->bed_x;
                y = pl->bed_y;
            }
        }
    }

    reference = (pl) ? who->name : NULL;
    m = ready_map_name(path_sh, orig_path_sh, MAP_STATUS_TYPE(mflags), reference);

    if (!m) /* map don't exists, fallback to savebed and/or emergency when possible */
    {
        if (!who ||
            (mflags & MAP_STATUS_NO_FALLBACK) ||
            !orig_path_sh) /* we only try path_sh !  - no fallback to a different map */
        {
            return MOVE_RESULT_INSERTION_FAILED;
        }

        /* For player we first try to the bind point (aka savebed) */
        if (pl)
        {
            LOG(llevBug, "BUG:: %s:enter_map_by_name(): pathname to map does not exist! player: %s[%d] (%s)\n",
                __FILE__, STRING_OBJ_NAME(who), TAG(who), STRING_SAFE(orig_path_sh));
            m = ready_map_name(pl->savebed_map, pl->orig_savebed_map, pl->bed_status, reference);
            x = pl->bed_x;
            y = pl->bed_y;

            /* Something is wrong with our bind point... reset */
            if (!m)
            {
                MAP_SET_PLAYER_BED_INFO_DEFAULT(pl);
                m = ready_map_name(shstr_cons.emergency_mappath, shstr_cons.emergency_mappath, MAP_STATUS_MULTI, reference);

                /* If we can't load the emergency map, something is probably really screwed up, so bail out now. */
                if (!m)
                {
                    LOG(llevError, "ERROR:: %s:enter_map_by_name(): could not load emergency map? Fatal error! (player: %s[%d])\n",
                        __FILE__, STRING_OBJ_NAME(who), TAG(who));
                }
            }
        }
        else /* we NEVER use the emergency map for mobs */
        {
            return MOVE_RESULT_INSERTION_FAILED;
        }
    }

    /* TODO: Not sure why this is. */
    if (!who ||
        (mflags & MAP_STATUS_LOAD_ONLY))
    {
        return MOVE_RESULT_INSERTION_FAILED;
    }

    /* This is EXTREMLY useful for quest maps with treasure rooms. */
    if (MAP_FIXEDLOGIN(m) ||
        (mflags & MAP_STATUS_FIXED_LOGIN) ||
        OUT_OF_REAL_MAP(m, x, y))
    {
        x = MAP_ENTER_X(m);
        y = MAP_ENTER_Y(m);
    }

    if (!orig_path_sh) /* special case - we have identified the map by his dest path_sh from loaded map list */
    {
        orig_path_sh = add_refcount(m->orig_path);
        /* because we can't be sure in this case about the map status, we overrule it with the loaded map! */
        mflags = m->status;
    }

    /* enter_map_by_name() only enters the specified map, so x, y must be
     * within its boundaries (checked above). Therefore, msp can be directly
     * calculated (no need for MSP_GET()) as out_of_map() needn't and mustn't
     * be called. */
    msp = MSP_RAW(m, x, y);
    return enter_map(who, msp, NULL, OVERLAY_FIRST_AVAILABLE, INS_NO_MERGE | INS_NO_WALK_ON);
}

/* Tries to move 'who' to exit_ob.  who is the character or monster that is
* using the exit, where exit_ob is the exit object (boat, door, teleporter,
* etc.)
* This is now the one and only "use an exit" function. Every object from type EXIT
* or TELEPORTER will handled here. Beside the instance maps is the biggest change in
* map handling that we get the type of maps now with a "inheritance" concept from the
* root (caller) map. If the exit don't change explicit the map type (normal, unique, instance...)
* the type is the same as the map where the exit object is part off (or always "normal" when
* there is no root map.
* RETURN: TRUE: we have loaded and entered a map. FALSE: we failed to enter
* MT-2006 */
sint8 enter_map_by_exit(object_t *who, object_t *exit_ob)
{
    object_t  *tmp;
    shstr_t   *reference = NULL;
    uint32     mstatus;
    uint8      oflags;
    map_t     *exit_map,
              *m;
    sint16     x,
               y;
    msp_t     *msp;
    sint8      i;

    who = (who->head) ? who->head : who;

    /* Event trigger and quick exit */
    if (trigger_object_plugin_event(EVENT_TRIGGER, exit_ob, who, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
    {
        return MOVE_RESULT_INSERTION_FAILED;
    }

    /* If the destination path is nonexistent or invalid, we ain't goin'
     * nowhere. */
    if (!exit_ob->slaying ||
        check_path(exit_ob->slaying, 1) == -1)
    {
        ndi(NDI_UNIQUE, 0, who, "%s is temporarily closed.",
            QUERY_SHORT_NAME(exit_ob, who));
        return MOVE_RESULT_INSERTION_FAILED;
    }

    /* Get the map the exit is on (or the environment of the exit is on). */
    for (tmp = exit_ob; tmp->env; tmp = tmp->env)
    {
    }

    exit_map = tmp->map;

    /* We now need to generate a normalized (means absolute) path to the exit's
     * destination map. The precise destination path depends ultimately on what
     * status the destination map is being loaded with: multiplayer maps have a
     * path pointing to /maps/...; unique maps point to
     * /server/data/players/...; and instances point to
     * /server/data/instance/....
     *
     * When an exit object is first loaded on a map it's destination path (in
     * ->slaying) is normalized and rewritten to ->slaying (we'll call this the
     * original destination path). This only needs to be done once because
     * original map paths very rarely change (and never while the server is
     * running), so once normalized, the original destination path will be
     * valid almost forever, even if the exit is picked up and/or moved.
     *
     * From time to time however, original map paths DO change (eg, /maps/ is
     * reorganized). When this happens, exits in inventories and (amost always)
     * on maps will have their destination path become invalid (so 'the exit is
     * closed'). Bt this is OK as ->slaying should either have been manually
     * updated anyway as part of the reorganization or the invalidation/closure
     * was intentional.
     *
     * So the original destination path is translated, according to status
     * (->last_eat or exit_map->status), into a destination path (->race),
     *
     * TODO: Exits created/modified by scripts do not have their original
     * destination paths normalized/validated in this way so may well be
     * broken. This will be fixed in a future update. */

    /* We need to know the status with which to load the destination map. This
     * either held by the exit itself or is inherited from the exit map. */
    mstatus = (!exit_ob->last_eat) ? // means inherited
       (int)MAP_STATUS_TYPE(exit_map->status) :
       (int)MAP_STATUS_TYPE(exit_ob->last_eat);

    /* If our status is not one of these we have a problem and must bail out. */
    if (!(mstatus & (MAP_STATUS_MULTI | MAP_STATUS_STYLE | MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE)))
    {
        /* So it's an inherited status which means ecit_map is already in memory
         * with an invalid status. This should never be possible but JIC. */
        if (!exit_ob->last_eat)
        {
            LOG(llevMapbug, "MAPBUG:: %s has an unrecgnized map status: %d!\n",
                STRING_MAP_PATH(exit_map), mstatus);
        }
        /* The exit has a bad explicit status. Lets kill the mapper. */
        else
        {
            LOG(llevMapbug, "MAPBUG:: %s[%d][%s %d %d] has an unrecgnized map status: %d!\n",
                STRING_OBJ_NAME(exit_ob), TAG(exit_ob),
                STRING_MAP_PATH(exit_map), exit_ob->x, exit_ob->y,
                mstatus);
        }

        return MOVE_RESULT_INSERTION_FAILED;
    }
    /* Multiplayer maps are easy as reference is always NULL and the
     * destination path is the same as the original destination path. */
    else if ((mstatus & (MAP_STATUS_MULTI | MAP_STATUS_STYLE)))
    {
        FREE_AND_CLEAR_HASH(reference);

        if (!exit_ob->race ||
            exit_ob->env)
        {
            FREE_AND_ADD_REF_HASH(exit_ob->race, exit_ob->slaying);
        }
    }
    /* Uniques and instances are a bit more complex. */
    else
    {
        /* When the exit is to an inherited map (means to elsewhere within the
         * same unique/instanced node), the reference MUST be the same as the
         * exit map's ->reference and the desination path MUST be a mash up of
         * the exit map's ->path and the exit object's ->slaying. */
        if (!exit_ob->last_eat)
        {
            FREE_AND_ADD_REF_HASH(reference, exit_map->reference);

            if (!exit_ob->race ||
                exit_ob->env)
            {
                char buf[MAXPATHLEN];

                /* we have now this:
                * - in map->path a normalized path to /players or /instance
                * - in exit_ob->slaying the normalized path + name to the original map
                * we create now a new path out of it by using the root from ->path.
                * NOTE: the path to /maps is always part of the unique/instance map name. */
                (void)normalize_path_direct(exit_map->path, exit_ob->slaying, buf);
                FREE_AND_COPY_HASH(exit_ob->race, buf);
            }
        }
        /* Here we're going from a multiplayer map to a unique/instance. */
        else
        {
            player_t *pl = (who->type == PLAYER) ? CONTR(who) : NULL;
            uint32    flags;

            /* Only players can have uniques/instances. */
            /* TODO: ATM only players can use exits at all but this will likely
             * change soon. */
            if (!pl)
            {
                return MOVE_RESULT_INSERTION_FAILED;
            }

            /* For a unique, the destination path is in server/data/players/.
             * The reference is normally op's name (that means whoever applied
             * the exit goes to their own apt) but for egobound exits is the
             * name of the player to whom it is bound. In this way a
             * (portable) entrance to let friends into your apt is possible. */
            /* TODO: But ATM the manual apply ego check prevents players
             * applying other's ego items s anyway. This restriction is kept
             * for now as several issues need to be addressed before we allow
             * visitors in apts anyway.
             *
             * -- Smacky 20140925 */
            if ((mstatus & MAP_STATUS_UNIQUE))
            {
                /* If the exit is egoboung, the reference is the name of the
                 * player to whom it is bound. The more elaborate ego test
                 * (check_ego_item()) has already been done before this
                 * function so here it's just a flag query. */
                if (QUERY_FLAG(exit_ob, FLAG_IS_EGOBOUND))
                {
                    FREE_AND_COPY_HASH(reference, get_ego_item_name(exit_ob));
                }
                /* So the reference is the player's name. */
                else
                {
                    FREE_AND_ADD_REF_HASH(reference, who->name);
                }

                FREE_AND_COPY_HASH(exit_ob->race, create_unique_path_sh(reference, exit_ob->slaying));
            }
            /* For an instance, the destination path is in
             * server/data/instance/ */
            else if ((mstatus & MAP_STATUS_INSTANCE))
            {
                /* So the reference is the player's name. */
                FREE_AND_ADD_REF_HASH(reference, who->name);

                /* we give here a player a "temporary" instance directory inside /instance
                 * which is identified by global_instance_num (directory name  = itoa(num)).
                 * we use the normalized original map path of the exit_ob as identifier of the
                 * instanced map or map set itself. */
                if (pl->instance_name == exit_ob->slaying &&
                    pl->instance_id == global_instance_id)
                {
                    /* add here the instance validation checks for this player */
                }
                else
                {
                    pl->instance_num = MAP_INSTANCE_NUM_INVALID; /* if set, we generate a NEW instance! */
                }

                flags = (QUERY_FLAG(exit_ob, FLAG_IS_MALE)) ? MAP_INSTANCE_FLAG_NO_REENTER : 0;

                /* create_instance..() will try to load an old instance, will fallback to
                 * a new one if needed and setup all what need be done to start the instance. */
                FREE_AND_COPY_HASH(exit_ob->race, create_instance_path_sh(pl, exit_ob->slaying, flags));
            }
        }
    }

    m = ready_map_name(exit_ob->race, exit_ob->slaying, mstatus, reference);
    FREE_ONLY_HASH(reference);

    if (!m)
    {
        ndi(NDI_UNIQUE, 0, who, "%s is closed.",
            QUERY_SHORT_NAME(exit_ob, who));
        return MOVE_RESULT_INSERTION_FAILED;
    }

    /* Now we got for sure transported away.
     * IF this exit has the "neutralize instance" flag set AND the old map type
     * was instance and the new one NOT - then neutralize the instance now. */
    if ((exit_map->status & MAP_STATUS_INSTANCE) &&
        QUERY_FLAG(exit_ob, FLAG_IS_FEMALE) &&
        !(mstatus & MAP_STATUS_INSTANCE))
    {
        reset_instance_data(CONTR(who));
    }

    /* lets play a sound where we have left the map */
    if (exit_ob->sub_type1 == ST1_EXIT_SOUND &&
        exit_map)
    {
        play_sound_map(MSP_RAW(exit_map, exit_ob->x, exit_ob->y), SOUND_TELEPORT, SOUND_NORMAL);
    }

    /* Send any exit message to exiter */
    if (exit_ob->msg)
    {
        ndi(NDI_NAVY, 0, who, "%s", exit_ob->msg);
    }

    x = (exit_ob->stats.hp == -1) ? MAP_ENTER_X(m) : exit_ob->stats.hp;
    y = (exit_ob->stats.sp == -1) ? MAP_ENTER_Y(m) : exit_ob->stats.sp;

    /* At this stage, x, y should be within m boundaries; if not, return
     * insertion failed. Equally, if they are, msp can be directly calculated
     * (no need for MSP_GET() as out_of_map() needn't and mustn't be called). */
    if (OUT_OF_REAL_MAP(m, x, y))
    {
        return MOVE_RESULT_INSERTION_FAILED;
    }

    msp = MSP_RAW(m, x, y);
    oflags = OVERLAY_FIRST_AVAILABLE;

    if (exit_ob->last_heal)
    {
        oflags |= OVERLAY_FIXED;
    }
    else if (exit_ob->last_sp)
    {
        oflags |= OVERLAY_RANDOM;
    }

    if (exit_ob->last_grace)
    {
        oflags |= OVERLAY_SPECIAL;
    }

    /* enter_map() may destroy who; if so, just return that info. */
    i = enter_map(who, msp, exit_ob, oflags, 0);

    if (i != MOVE_RESULT_SUCCESS)
    {
        return i;
    }

    /* some "exits" like a pit will move you for sure to a new map -- but for a
     * price ... */
    if (exit_ob->stats.dam)
    {
        damage_ob(who, exit_ob->stats.dam, exit_ob, ENV_ATTACK_CHECK);

        /* CHECK THIS! */
        if (who->stats.hp == 0)
        {
            return MOVE_RESULT_WHO_DESTROYED;
        }
    }

    return MOVE_RESULT_SUCCESS;
}
