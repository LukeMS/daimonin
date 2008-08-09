/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2006 Michael Toennies

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

    The author can be reached via e-mail to info@daimonin.net
*/

#include <global.h>
#include <limits.h>


/*
* This function updates various attributes about a specific space
* on the map (what it looks like, whether it blocks magic,
* has a living creatures, prevents people from passing
* through, etc)
*/
void update_position(mapstruct *m, MapSpace *mspace, int x, int y)
{
    object     *tmp;
    MapSpace   *mp;
    int         i, ii, flags;
    int         oldflags;

    if(mspace)
    {
        mp = mspace;
        flags = oldflags = (P_NO_ERROR | P_NEED_UPDATE | P_FLAGS_UPDATE);
    }
    else
    {
        if (!((oldflags = GET_MAP_FLAGS(m, x, y)) & (P_NEED_UPDATE | P_FLAGS_UPDATE)))
            LOG(llevDebug, "DBUG: update_position called with P_NEED_UPDATE|P_FLAGS_UPDATE not set: %s (%d, %d)\n", m->path, x, y);
        mp = &m->spaces[x + m->width * y];
        flags = oldflags & P_NEED_UPDATE; /* save our update flag */
    }


    /* update our flags */
    if (oldflags & P_FLAGS_UPDATE)
    {
#ifdef DEBUG_CORE
        LOG(llevDebug, "UP - FLAGS: %d,%d\n", x, y);
#endif
        /*LOG(llevDebug,"flags:: %x (%d, %d) %x (NU:%x NE:%x)\n", oldflags, x, y,P_NEED_UPDATE|P_NO_ERROR,P_NEED_UPDATE,P_NO_ERROR);*/

        /* This is a key function and highly often called - every saved tick is good. */
        if(mp->floor_flags & MAP_FLOOR_FLAG_NO_PASS)
            flags |= P_NO_PASS;
        if(mp->floor_flags & MAP_FLOOR_FLAG_PLAYER_ONLY)
            flags |= P_PLAYER_ONLY;

        mp->move_flags |= mp->floor_terrain;
        /*mp->light_value += mp->floor_light;*/

        for (tmp =mp->first; tmp; tmp = tmp->above)
        {
            /* should be floor only? let it in for now ... mt 10.2005 */
            if (QUERY_FLAG(tmp, FLAG_PLAYER_ONLY))
                flags |= P_PLAYER_ONLY;
            if (tmp->type == CHECK_INV)
                flags |= P_CHECK_INV;
            if (tmp->type == MAGIC_EAR)
                flags |= P_MAGIC_EAR;
            if (QUERY_FLAG(tmp, FLAG_IS_PLAYER))
                flags |= P_IS_PLAYER;
            if (QUERY_FLAG(tmp, FLAG_DOOR_CLOSED))
                flags |= P_DOOR_CLOSED;
            if (QUERY_FLAG(tmp, FLAG_ALIVE))
            {
                flags |= P_IS_ALIVE;
                if(tmp->type==MONSTER && OBJECT_VALID(tmp->owner, tmp->owner_count) &&
                    tmp->owner->type == PLAYER)
                    flags |= P_IS_PLAYER_PET;
            }
            if (QUERY_FLAG(tmp, FLAG_NO_MAGIC))
                flags |= P_NO_MAGIC;
            if (QUERY_FLAG(tmp, FLAG_NO_CLERIC))
                flags |= P_NO_CLERIC;
            if (QUERY_FLAG(tmp, FLAG_BLOCKSVIEW))
                flags |= P_BLOCKSVIEW;
            if (QUERY_FLAG(tmp, FLAG_CAN_REFL_SPELL))
                flags |= P_REFL_SPELLS;
            if (QUERY_FLAG(tmp, FLAG_CAN_REFL_MISSILE))
                flags |= P_REFL_MISSILE;

            if (QUERY_FLAG(tmp, FLAG_WALK_ON))
                flags |= P_WALK_ON;
            if (QUERY_FLAG(tmp, FLAG_WALK_OFF))
                flags |= P_WALK_OFF;
            if (QUERY_FLAG(tmp, FLAG_FLY_ON))
                flags |= P_FLY_ON;
            if (QUERY_FLAG(tmp, FLAG_FLY_OFF))
                flags |= P_FLY_OFF;

            if (QUERY_FLAG(tmp, FLAG_NO_PASS))
            {
                /* we also handle PASS_THRU here...
                * a.) if NO_PASS is set before, we test for PASS_THRU
                * - if we have no FLAG_PASS_THRU, we delete PASS_THRU
                * - if we have FLAG_PASS_THRU, we do nothing - other object blocks always
                * b.) if no NO_PASS is set, we set it AND set PASS_THRU if needed
                */
                if (flags & P_NO_PASS)
                {
                    if (!QUERY_FLAG(tmp, FLAG_PASS_THRU))
                        flags &= ~P_PASS_THRU; /* just fire it... always true */
                    if (!QUERY_FLAG(tmp, FLAG_PASS_ETHEREAL))
                        flags &= ~P_PASS_ETHEREAL; /* just fire it... always true */
                }
                else
                {
                    flags |= P_NO_PASS;
                    if (QUERY_FLAG(tmp, FLAG_PASS_THRU))
                        flags |= P_PASS_THRU;
                    if (QUERY_FLAG(tmp, FLAG_PASS_ETHEREAL))
                        flags |= P_PASS_ETHEREAL;
                }
            }

            /* This is set by the floor to node code now
            if (QUERY_FLAG(tmp, FLAG_IS_FLOOR))
            move_flags |= tmp->terrain_type;
            */
        } /* for stack of objects */

        if (((oldflags & ~(P_FLAGS_UPDATE | P_FLAGS_ONLY | P_NO_ERROR)) != flags) && (!(oldflags & P_NO_ERROR)))
            LOG(llevDebug, "DBUG: update_position: updated flags do not match old flags: %s (%d,%d) old:%x != %x\n",
            m->path, x, y, (oldflags & ~P_NEED_UPDATE), flags);

        mp->flags = flags;
    } /* end flag update */

    /* check we must rebuild the map layers for client view */
    if (oldflags & P_FLAGS_ONLY || !(oldflags & P_NEED_UPDATE))
        return;

#ifdef DEBUG_CORE
    LOG(llevDebug, "UP - LAYER: %d,%d\n", x, y);
#endif

    /* NOTE: The whole mlayer system will become outdated with beta 5 (smooth
    * scrolling). I will install for that client type a dynamic map protocol
    */
    mp->client_mlayer[0] = 0; /* ALWAYS is client layer 0 (cl0) a floor. force it */
    mp->client_mlayer_inv[0] = 0;

    /* disabled my floor/mask node patch
    if (mp->mask)
    {
    mp->client_mlayer[1] = 1;
    mp->client_mlayer_inv[1] = 1;
    }
    else
    */
    mp->client_mlayer_inv[1] = mp->client_mlayer[1] = -1;

    /* and 2 layers for moving stuff */
    mp->client_mlayer[2] = mp->client_mlayer[3] = -1;
    mp->client_mlayer_inv[2] = mp->client_mlayer_inv[3] = -1;

    /* THE INV FLAG CHECK IS FIRST IMPLEMENTATION AND REALLY NOT THE FASTEST WAY -
    * WE CAN AVOID IT COMPLETE BY USING A 2nd INV QUEUE
    */

    /* now we first look for a object for cl3 */
    for (i = 6; i > 1; i--)
    {
        if (mp->layer[i])
        {
            mp->client_mlayer_inv[3] = mp->client_mlayer[3] = i; /* the last*/
            i--;
            break;
        }
    }

    /* inv LAYER: perhaps we have something invisible before it*/
    for (ii = 6 + 7; ii > i + 6; ii--) /* we skip layer 7 - no invisible stuff on layer 7 */
    {
        if (mp->layer[ii])
        {
            mp->client_mlayer_inv[2] = mp->client_mlayer_inv[3];
            mp->client_mlayer_inv[3] = ii; /* the last*/
            break;
        }
    }

    /* and a last one for cl2 */
    for (; i > 1; i--)
    {
        if (mp->layer[i])
        {
            mp->client_mlayer[2] = mp->client_mlayer_inv[2] = i; /* the last*/
            break;
        }
    }

    /* in layer[2] we have now normal layer 3 or normal layer 2
    * now seek a possible inv. object to substitute normal
    */
    for (ii--; ii > 8; ii--)
    {
        if (mp->layer[ii])
        {
            mp->client_mlayer_inv[2] = ii;
            break;
        }
    }

    /* clear out all need update flags */
    SET_MAP_FLAGS(m, x, y, GET_MAP_FLAGS(m, x, y) & ~(P_NEED_UPDATE | P_FLAGS_UPDATE));
}

/*
* Returns brightness of given square
* (high-res scale: 0-1280ish)
*/
int map_brightness(mapstruct *m, int x, int y)
{
    int ambient_light;

    if (!(m = out_of_map(m, &x, &y)))
        return 0;

    if (MAP_OUTDOORS(m) && global_darkness_table[world_darkness] <= m->light_value)
        ambient_light = global_darkness_table[world_darkness];
    else
        ambient_light = m->light_value;

    return GET_MAP_LIGHT_VALUE(m, x, y) + ambient_light;
}

/*
* Returns true if a wall is present in a given location.
* Calling object should do a <return>&P_PASS_THRU if it
* has CAN_PASS_THRU to see it can cross here.
* The PLAYER_ONLY flag here is analyzed without checking the
* caller type. Thats possible because player movement releated
* functions should always use blocked().
*/
int wall(mapstruct *m, int x, int y)
{
    if (!(m = out_of_map(m, &x, &y)))
        return (P_BLOCKSVIEW | P_NO_PASS | P_OUT_OF_MAP);
    return (GET_MAP_FLAGS(m, x, y) & (P_DOOR_CLOSED | P_PLAYER_ONLY | P_NO_PASS | P_PASS_THRU | P_PASS_ETHEREAL));
}

/*
* Returns true if it's impossible to see through the given coordinate
* in the given map.
*/

int blocks_view(mapstruct *m, int x, int y)
{
    mapstruct  *nm;

    if (!(nm = out_of_map(m, &x, &y)))
        return (P_BLOCKSVIEW | P_NO_PASS | P_OUT_OF_MAP);

    return (GET_MAP_FLAGS(nm, x, y) & P_BLOCKSVIEW);
}

/*
* Returns true if the given coordinate in the given map blocks magic.
*/

int blocks_magic(mapstruct *m, int x, int y)
{
    if (!(m = out_of_map(m, &x, &y)))
        return (P_BLOCKSVIEW | P_NO_PASS | P_NO_MAGIC | P_OUT_OF_MAP);
    return (GET_MAP_FLAGS(m, x, y) & P_NO_MAGIC);
}

/*
* Returns true if clerical spells cannot work here
*/
int blocks_cleric(mapstruct *m, int x, int y)
{
    if (!(m = out_of_map(m, &x, &y)))
        return (P_BLOCKSVIEW | P_NO_PASS | P_NO_CLERIC | P_OUT_OF_MAP);
    return (GET_MAP_FLAGS(m, x, y) & P_NO_CLERIC);
}

/* I total reworked the blocked functions. There was several bugs, glitches
* and loops in. The loops really scaled with bigger load very badly, slowing
* this part down for heavy traffic.
* Changes: check ALL P_xxx flags (and really all) of a tile node here. If its impossible
* to enter the tile - blocked() will tell it us.
* This included to capsule and integrate blocked_tile() in blocked().
* blocked_tile() is the function where the single objects of a node gets
* tested - for example for CHECK_INV. But i added a P_CHECK_INV flag - so its
* now only called when really needed - before it was called for EVERY moving
* object for every successful step.
* PASS_THRU check is moved in blocked() too.. This should generate for example for
* pathfinding better results. Note, that PASS_THRU only has a meaning when NO_PASS
* is set. If a object has both flags, NO_PASS can be passed when object has
* CAN_PASS_THRU. If object has PASS_THRU without NO_PASS, PASS_THRU is ignored.
* blocked() checks player vs player stuff too. No block in non pvp areas.
* Note, that blocked() is only on the first glance bigger as before - i moved stuff
* in which was in blocked_tile() or handled from calling functions around the call -
* so its less or same code but moved in blocked().
*
* Return: 0 = can be passed , elsewhere it gives one or more flags which invoke
* the block AND/OR which was not tested. (for outside check).
* MT-2003
*/
/* i added the door flag now. The trick is, that we want mark the door as possible
* to open here and sometimes not. If the object spot is in forbidden terrain, we
* don't want its possible to open it, even we stand near to it. But for example if
* it blocked by alive object, we want open it. If the spot marked as pass_thru and
* we can pass_thru, then we want skip the door (means not open it).
* MT-29.01.2004
*/
int blocked(object *op, mapstruct *m, int x, int y, int terrain)
{
    int         flags;
    MapSpace   *msp;

    flags = (msp = GET_MAP_SPACE_PTR(m, x, y))->flags;

    /* lets start... first, look at the terrain. If we don't have
    * a valid terrain flag, this is forbidden to enter.
    */
    if (msp->move_flags & ~terrain)
    {
        /* last chance ... flying & levitation allows us to stay over more terrains */
        if(op)
        {
            if(QUERY_FLAG(op,FLAG_FLYING))
                terrain |= (TERRAIN_WATERWALK|TERRAIN_CLOUDWALK);
            else if(QUERY_FLAG(op,FLAG_LEVITATE))
                terrain |= TERRAIN_WATERWALK;

            if (msp->move_flags & ~terrain)
                return (flags & (P_NO_PASS | P_IS_ALIVE | P_IS_PLAYER | P_CHECK_INV | P_PASS_THRU | P_PASS_ETHEREAL)) | P_NO_TERRAIN;
        }
        else
            return (flags & (P_NO_PASS | P_IS_ALIVE | P_IS_PLAYER | P_CHECK_INV | P_PASS_THRU | P_PASS_ETHEREAL)) | P_NO_TERRAIN;
    }

    /* the terrain is ok... whats first?
    * A.) P_IS_ALIVE - we leave without question
    * (NOTE: player objects has NO is_alive set!)
    * B.) P_NO_PASS - if set we leave here when no PASS_THRU is set
    * and/or the passer has no CAN_PASS_THRU.
    */
    if (flags & P_IS_ALIVE)
    {
        /* If this is a player pet, all players can pass it on non-pvp maps */
        if(op==NULL || op->type != PLAYER || !(flags & P_IS_PLAYER_PET)
            || flags & P_IS_PVP || m->map_flags & MAP_FLAG_PVP)
            return (flags & (P_DOOR_CLOSED | P_NO_PASS | P_IS_ALIVE | P_IS_PLAYER | P_CHECK_INV | P_PASS_THRU| P_PASS_ETHEREAL));
    }

    /* still one flag to check: perhaps P_PASS_THRU overrules NO_PASS? Or PASS_ETHEREAL? */
    if (flags & P_NO_PASS) /* i seperated it from below - perhaps we add here more tests */
    {
        /* logic is: no_pass when..
        * - no PASS_THRU... or
        * - PASS_THRU set but op==NULL (no PASS_THRU check possible)
        * - PASS_THRU set and object has no CAN_PASS_THRU
        * - the same for PASS_ETHEREAL and IS_ETHEREAL
        */
        if (!op || ( (!(flags & P_PASS_THRU) || !QUERY_FLAG(op, FLAG_CAN_PASS_THRU)) &&
            (!(flags & P_PASS_ETHEREAL) || !QUERY_FLAG(op, FLAG_IS_ETHEREAL)) ))
            return (flags & (P_DOOR_CLOSED | P_NO_PASS | P_IS_PLAYER | P_CHECK_INV | P_PASS_THRU| P_PASS_ETHEREAL));

        /* ok, NO_PASS is overruled... we go on... */
    }

    /* now.... whats left? No explicit flag can forbid us to enter anymore  - except:
    * a.) perhaps is a player in and we are a monster or the player is in a pvp area.
    * b.) we need to check_inv - which can kick us out too (checker power)
    */
    if (flags & P_IS_PLAYER)
    {
        /* ok... we leave here when
        * a.) op == NULL (because we can't check for op==PLAYER then)
        * b.) P_IS_PVP or MAP_FLAG_PVP
        */
        if (!op || flags & P_IS_PVP || m->map_flags & MAP_FLAG_PVP)
            return ((flags & (P_DOOR_CLOSED | P_IS_PLAYER | P_CHECK_INV))|P_IS_PVP);

        /* when we are here: no player pvp stuff was triggered. But:
        * a.) the tile IS blocked by a player (we still in IS_PLAYER area)
        * b.) we are not in any pvp area
        * c.) we have a op pointer to check.
        *
        * we can handle here more exclusive stuff now... Like we can pass spells
        * through player by checking owner or something... Just insert it here.
        */

        /* for now, the easiest way - if op is no player (it is a monster or somewhat
        * else "solid" object) - then no pass
        */
        if (op->type != PLAYER)
            return (flags & (P_DOOR_CLOSED | P_IS_PLAYER | P_CHECK_INV));
    }

    if (op) /* we have a object ptr - do some last checks */
    {

        if (flags & P_PLAYER_ONLY && op->type != PLAYER) /* player only space and not a player... */
            return (flags & (P_DOOR_CLOSED | P_NO_PASS | P_CHECK_INV|P_PLAYER_ONLY)); /* tell them: no pass and possible checker here */

        /* and here is our CHECK_INV ...
        * blocked_tile() is now only and exclusive called from here.
        * lets skip it, when op is NULL - so we can turn the check from outside
        * on/off (for example if we only want test size stuff)
        */
        if (flags & P_CHECK_INV)
        {
            /* we fake a NO_PASS when the checker kick us out - in fact thats
            * how it should be.
            */
            if (blocked_tile(op, m, x, y))
                return (flags & (P_DOOR_CLOSED | P_NO_PASS | P_CHECK_INV)); /* tell them: no pass and checker here */
        }
    }
    return (flags & (P_DOOR_CLOSED)); /* ah... 0 is what we want.... 0 == we can pass */
}


/*
* Returns true if the given coordinate is blocked by the
* object passed is not blocking.  This is used with
* multipart monsters - if we want to see if a 2x2 monster
* can move 1 space to the left, we don't want its own area
* to block it from moving there.
* Returns TRUE if the space is blocked by something other than the
* monster.
*/
/* why is controlling the own arch clone offsets with the new
* freearr_[] offset a good thing?
* a.) we don't must check any flags for tiles where we was before
* b.) we don't block in moving when we got teleported in a no_pass somewhere
* c.) no call to out_of_map() needed for all parts
* d.) no checks of objects in every tile node of the multi arch
* e.) no recursive call needed anymore
* f.) the multi arch are handled in maps like the single arch
* g.) no scaling by heavy map action when we move (more objects
*     on the map don't interest us anymore here)
*/
int blocked_link(object *op, int xoff, int yoff)
{
    object     *tmp, *tmp2;
    mapstruct  *m;
    int         xtemp, ytemp;

    for (tmp = op; tmp; tmp = tmp->more)
    {
        /* we search for this new position */
        xtemp = tmp->arch->clone.x + xoff;
        ytemp = tmp->arch->clone.y + yoff;
        /* lets check it match a different part of us */
        for (tmp2 = op; tmp2; tmp2 = tmp2->more)
        {
            /* if this is true, we can be sure this position is valid */
            if (xtemp == tmp2->arch->clone.x && ytemp == tmp2->arch->clone.y)
                break;
        }
        if (!tmp2) /* if this is NULL, tmp will move in a new node */
        {
            xtemp = tmp->x + xoff;
            ytemp = tmp->y + yoff;
            /* if this new node is illegal - we can skip all */
            if (!(m = out_of_map(tmp->map, &xtemp, &ytemp)))
                return -1;
            /* tricky: we use always head for tests - no need to copy any flags to the tail */
            /* we should kick in here the door test - but we need to diff we are
            * just testing here or we doing a real step!
            */
            if ((xtemp = blocked(op, m, xtemp, ytemp, op->terrain_flag)))
                return xtemp;
        }
    }
    return 0; /* when we are here - then we can move */
}

/* As above, but using an absolute coordinate (map,x,y)-triplet
* TODO: this function should really be combined with the above
* to reduce code duplication...
*/
int blocked_link_2(object *op, mapstruct *map, int x, int y)
{
    object     *tmp, *tmp2;
    int         xtemp, ytemp;
    mapstruct  *m;

    for (tmp = op; tmp; tmp = tmp->more)
    {
        /* we search for this new position */
        xtemp = x + tmp->arch->clone.x;
        ytemp = y + tmp->arch->clone.y;
        /* lets check it match a different part of us */
        for (tmp2 = op; tmp2; tmp2 = tmp2->more)
        {
            /* if this is true, we can be sure this position is valid */
            if (xtemp == tmp2->x && ytemp == tmp2->y)
                break;
        }
        if (!tmp2) /* if this is NULL, tmp will move in a new node */
        {
            /* if this new node is illegal - we can skip all */
            if (!(m = out_of_map(map, &xtemp, &ytemp)))
                return -1;
            /* tricky: we use always head for tests - no need to copy any flags to the tail */
            if ((xtemp = blocked(op, m, xtemp, ytemp, op->terrain_flag)))
                return xtemp;
        }
    }
    return 0; /* when we are here - then we can move */
}


/* blocked_tile()
* return: 0= not blocked 1: blocked
* This is used for any action which needs to browse
* through the objects of the tile node - for special objects
* like inventory checkers - or general for all what can't
* be easy handled by map flags in blocked().
*/
int blocked_tile(object *op, mapstruct *m, int x, int y)
{
    object *tmp;

    for (tmp = GET_MAP_OB(m, x, y); tmp != NULL; tmp = tmp->above)
    {
        /* This must be before the checks below.  Code for inventory checkers. */
        /* Note: we only do this check here because the last_grace cause the
        * CHECK_INV to block the space. The check_inv is called again in
        * move_apply() - there it will do the trigger and so on. This here is only
        * for testing the tile - not for invoking the check_inv power!
        */
        if (tmp->type == CHECK_INV && tmp->last_grace)
        {
            /* If last_sp is set, the player/monster needs an object,
            * so we check for it.  If they don't have it, they can't
            * pass through this space.
            */
            if (tmp->last_sp)
            {
                if (check_inv_recursive(op, tmp) == NULL)
                    return 1;
            }
            else
            {
                /* In this case, the player must not have the object -
                * if they do, they can't pass through.
                */
                if (check_inv_recursive(op, tmp) != NULL) /* player has object */
                    return 1;
            }
        } /* if check_inv */
    }
    return 0;
}

/* Testing a arch to fit in a position.
* Return: 0 == no block.-1 == out of map, else the blocking flags from blocked()
*/
/* Advanced arch_blocked() function. We CAN give a object ptr too know. If we do,
* we can test the right terrain flags AND all specials from blocked(). This is
* extremly useful for pathfinding.
*/
int arch_blocked(archetype *at, object *op, mapstruct *m, int x, int y)
{
    archetype  *tmp;
    mapstruct  *mt;
    int         xt, yt, t;

    if (op)
        t = op->terrain_flag;
    else
        t = TERRAIN_ALL;

    if (at == NULL)
    {
        if (!(m = out_of_map(m, &x, &y)))
            return -1;
        return (blocked(op, m, x, y, t));
    }
    for (tmp = at; tmp; tmp = tmp->more)
    {
        xt = x + tmp->clone.x;
        yt = y + tmp->clone.y;
        if (!(mt = out_of_map(m, &xt, &yt)))
            return -1;

        if ((xt = blocked(op, mt, xt, yt, t)))
            return xt; /* double used xt... small hack */
    }
    return 0;
}

/*
* Returns true if the given archetype can't fit into the map at the
* given spot (some part of it is outside the map-boundaries).
*/

int arch_out_of_map(archetype *at, mapstruct *m, int x, int y)
{
    archetype  *tmp;
    int         xt, yt;

    if (at == NULL)
        return out_of_map(m, &x, &y) == NULL ? 1 : 0;

    for (tmp = at; tmp != NULL; tmp = tmp->more)
    {
        xt = x + tmp->clone.x;
        yt = y + tmp->clone.y;
        if (!out_of_map(m, &xt, &yt))
            return 1;
    }
    return 0;
}

/** Try loading the connected map tile with the given number.
 * @param orig_map base map for tiling
 * @param tile_num tile-number to connect to (not direction).
 * @return If loading _or_ tiling fails NULL is returned,
 * otherwise the loaded map neighbouring orig_map is returned.
 */
static inline mapstruct * load_and_link_tiled_map(mapstruct *orig_map, int tile_num)
{
    /* Nowadays the loader keeps track of tiling. Gecko 2006-12-31 */
    mapstruct *map = ready_map_name( orig_map->tile_path[tile_num],
           orig_map->orig_tile_path[tile_num], MAP_STATUS_TYPE(orig_map->map_status),
           orig_map->reference);

    /* If loading or linking failed */
    if(map == NULL || map != orig_map->tile_map[tile_num])
    {
        /* ensure we don't get called again over and over */
        LOG(llevMapbug, "MAPBUG: failed to connect map %s with tile no %d (%s).\n", STRING_SAFE(orig_map->orig_path), tile_num, STRING_SAFE(orig_map->orig_tile_path[tile_num]));
        FREE_AND_CLEAR_HASH(orig_map->orig_tile_path[tile_num]);
        FREE_AND_CLEAR_HASH(orig_map->tile_path[tile_num]);
        return NULL;
    }

    return map;
}

/* Find the distance between two map tiles on a tiled map.
* Returns true if the two tiles are part of the same map.
* the distance from the topleft (0,0) corner of map1 to the topleft corner of map2
* will be added to x and y.
*
* This function does not work well with assymetrically tiled maps.
*
* To increase efficiency, maps can have precalculated tileset_id:s and
* coordinates, which are used if available. If one or more of the two
* maps lack this data, a slow non-exhaustive breadth-first search
* is attempted.
*/
static int relative_tile_position(mapstruct *map1, mapstruct *map2, int *x, int *y)
{
    int                     i;
    static uint32           traversal_id    = 0;
    struct mapsearch_node  *first, *last, *curr, *node;
    int                     success         = FALSE;
    int                     searched_tiles  = 0;

    /* Save some time in the simplest cases ( very similar to on_same_map() )*/
    if (map1 == NULL || map2 == NULL)
        return FALSE;

    /* Precalculated tileset data available? */
    if (map1->tileset_id > 0 && map2->tileset_id > 0)
    {
        //        LOG(llevDebug, "relative_tile_position(): Could use tileset data for %s -> %s\n", map1->path, map2->path);
        if(map1->tileset_id == map2->tileset_id && in_same_instance(map1, map2))
        {
            *x += map2->tileset_x - map1->tileset_x;
            *y += map2->tileset_y - map1->tileset_y;
            return TRUE;
        } else
            return FALSE;
    }

    if (map1 == map2)
        return TRUE;

    //    LOG(llevBug, "relative_tile_position(): One or both of maps %s and %s lacks tileset data\n", map1->path, map2->path);

    /* The caching really helps when pathfinding across map tiles,
    * but not in many other cases. */
    /* Check for cached pathfinding */
    if (map1->cached_dist_map == map2->path)
    {
        *x += map1->cached_dist_x;
        *y += map1->cached_dist_y;
        return TRUE;
    }
    if (map2->cached_dist_map == map1->path)
    {
        *x -= map2->cached_dist_x;
        *y -= map2->cached_dist_y;
        return TRUE;
    }

    /* TODO: effectivize somewhat by doing bidirectional search */
    /* TODO: big project: magically make work with pre- or dynamically computed bigmap data */

    /* Avoid overflow of traversal_id */
    if (traversal_id == 4294967295U /* UINT_MAX */)
    {
        mapstruct  *m;

        LOG(llevDebug, "relative_tile_position(): resetting traversal id\n");

        for (m = first_map; m != NULL; m = m->next)
            m->traversed = 0;

        traversal_id = 0;
    }

    map1->traversed = ++traversal_id;

    /* initial queue and node values */
    first = last = NULL;
    curr = get_poolchunk(pool_map_bfs);
    curr->map = map1;
    curr->dx = curr->dy = 0;

    while (curr)
    {
        /* Expand one level */
        for (i = 0; i < TILED_MAPS; i++)
        {
            if (curr->map->tile_path[i]
            && (curr->map->tile_map[i] == NULL || curr->map->tile_map[i]->traversed != traversal_id))
            {
                if (!curr->map->tile_map[i] || curr->map->tile_map[i]->in_memory != MAP_IN_MEMORY)
                {
                    if(!load_and_link_tiled_map(curr->map, i))
                        continue; /* invalid path - we don't found the map */
                }

                /* TODO: avoid this bit of extra work if correct map */
                node = get_poolchunk(pool_map_bfs);
                node->dx = curr->dx;
                node->dy = curr->dy;
                node->map = curr->map->tile_map[i];

                /* Calc dx/dy */
                switch (i)
                {
                case 0:
                    node->dy -= MAP_HEIGHT(curr->map->tile_map[i]);  break;  /* North */
                case 1:
                    node->dx += MAP_WIDTH(curr->map); break;  /* East */
                case 2:
                    node->dy += MAP_HEIGHT(curr->map); break;  /* South */
                case 3:
                    node->dx -= MAP_WIDTH(curr->map->tile_map[i]);  break;  /* West */
                case 4:
                    node->dy -= MAP_HEIGHT(curr->map->tile_map[i]); node->dx += MAP_WIDTH(curr->map); break;  /* Northest */
                case 5:
                    node->dy += MAP_HEIGHT(curr->map); node->dx += MAP_WIDTH(curr->map); break;  /* Southest */
                case 6:
                    node->dy += MAP_HEIGHT(curr->map); node->dx -= MAP_WIDTH(curr->map->tile_map[i]); break;  /* Southwest */
                case 7:
                    node->dy -= MAP_HEIGHT(curr->map->tile_map[i]); node->dx -= MAP_WIDTH(curr->map->tile_map[i]); break;  /* Northwest */
                }

                /* Correct map? */
                if (node->map == map2)
                {
                    /* store info in cache */
                    FREE_AND_ADD_REF_HASH(map1->cached_dist_map, map2->path);
                    map1->cached_dist_x = node->dx;
                    map1->cached_dist_y = node->dy;

                    /* return result and clean up */
                    *x += node->dx;
                    *y += node->dy;
                    success = TRUE;
                    return_poolchunk(node, pool_map_bfs);
                    return_poolchunk(curr, pool_map_bfs);
                    goto out;
                }

                /* No success, add the new tile to the queue */
                node->next = NULL;
                if (first)
                {
                    last->next = node;
                    last = node;
                }
                else
                    first = last = node;
                node->map->traversed = traversal_id;
            }
        }

        return_poolchunk(curr, pool_map_bfs);

        /* Depth-limitation */
        if (++searched_tiles >= MAX_SEARCH_MAP_TILES)
        {
            LOG(llevDebug, "relative_tile_position(): reached max nrof search tiles - bailing out\n");
            break;
        }

        /* dequeue next tile to check */
        curr = first;
        if (curr)
            first = curr->next;
        else
            first = NULL;
    }

out:
    for (node = first; node; node = node->next)
        return_poolchunk(node, pool_map_bfs);

    return success;
}

/* out of map now checks all 8 possible neighbours of
* a tiled map and loads them in when needed.
*/
mapstruct * out_of_map(mapstruct *m, int *x, int *y)
{
    /* Simple case - coordinates are within this local map.*/
    if (!m)
        return NULL;

    if (((*x) >= 0) && ((*x) < MAP_WIDTH(m)) && ((*y) >= 0) && ((*y) < MAP_HEIGHT(m)))
        return m;

    if (*x < 0) /* thats w, nw or sw (3,7 or 6) */
    {
        if (*y < 0) /*  nw.. */
        {
            if (!m->tile_path[7])
                return NULL;
            if (!m->tile_map[7] || m->tile_map[7]->in_memory != MAP_IN_MEMORY)
            {
                if(!load_and_link_tiled_map(m, 7))
                    return NULL;
            }
            *y += MAP_HEIGHT(m->tile_map[7]);
            *x += MAP_WIDTH(m->tile_map[7]);
            return (out_of_map(m->tile_map[7], x, y));
        }

        if (*y >= MAP_HEIGHT(m)) /* sw */
        {
            if (!m->tile_path[6])
                return NULL;
            if (!m->tile_map[6] || m->tile_map[6]->in_memory != MAP_IN_MEMORY)
            {
                if(!load_and_link_tiled_map(m, 6))
                    return NULL;
            }
            *y -= MAP_HEIGHT(m);
            *x += MAP_WIDTH(m->tile_map[6]);
            return (out_of_map(m->tile_map[6], x, y));
        }


        if (!m->tile_path[3]) /* it MUST be west */
            return NULL;
        if (!m->tile_map[3] || m->tile_map[3]->in_memory != MAP_IN_MEMORY)
        {
            if(!load_and_link_tiled_map(m, 3))
                return NULL;
        }
        *x += MAP_WIDTH(m->tile_map[3]);
        return (out_of_map(m->tile_map[3], x, y));
    }

    if (*x >= MAP_WIDTH(m))  /* thatd e, ne or se (1 ,4 or 5) */
    {
        if (*y < 0) /*  ne.. */
        {
            if (!m->tile_path[4])
                return NULL;
            if (!m->tile_map[4] || m->tile_map[4]->in_memory != MAP_IN_MEMORY)
            {
                if(!load_and_link_tiled_map(m, 4))
                    return NULL;
            }
            *y += MAP_HEIGHT(m->tile_map[4]);
            *x -= MAP_WIDTH(m);
            return (out_of_map(m->tile_map[4], x, y));
        }

        if (*y >= MAP_HEIGHT(m)) /* se */
        {
            if (!m->tile_path[5])
                return NULL;
            if (!m->tile_map[5] || m->tile_map[5]->in_memory != MAP_IN_MEMORY)
            {
                if(!load_and_link_tiled_map(m, 5))
                    return NULL;
            }
            *y -= MAP_HEIGHT(m);
            *x -= MAP_WIDTH(m);
            return (out_of_map(m->tile_map[5], x, y));
        }

        if (!m->tile_path[1])
            return NULL;
        if (!m->tile_map[1] || m->tile_map[1]->in_memory != MAP_IN_MEMORY)
        {
            if(!load_and_link_tiled_map(m, 1))
                return NULL;
        }
        *x -= MAP_WIDTH(m);
        return (out_of_map(m->tile_map[1], x, y));
    }

    /* because we have tested x above, we don't need to check
    * for nw,sw,ne and nw here again.
    */
    if (*y < 0)
    {
        if (!m->tile_path[0])
            return NULL;
        if (!m->tile_map[0] || m->tile_map[0]->in_memory != MAP_IN_MEMORY)
        {
            if(!load_and_link_tiled_map(m, 0))
                return NULL;
        }
        *y += MAP_HEIGHT(m->tile_map[0]);
        return (out_of_map(m->tile_map[0], x, y));
    }
    if (*y >= MAP_HEIGHT(m))
    {
        if (!m->tile_path[2])
            return NULL;
        if (!m->tile_map[2] || m->tile_map[2]->in_memory != MAP_IN_MEMORY)
        {
            if(!load_and_link_tiled_map(m, 2))
                return NULL;
        }
        *y -= MAP_HEIGHT(m);
        return (out_of_map(m->tile_map[2], x, y));
    }
    return NULL;
}

/* this is a special version of out_of_map() - this version ONLY
* adjust to loaded maps - it will not trigger a re/newload of a
* tiled map not in memory. If out_of_map() fails to adjust the
* map positions, it will return NULL when the there is no tiled
* map and NULL when the map is not loaded.
* As special marker, x is set 0 when the coordinates are not
* in a map (outside also possible tiled maps) and to -1 when
* there is a tiled map but its not loaded.
*/
mapstruct * out_of_map2(mapstruct *m, int *x, int *y)
{
    /* Simple case - coordinates are within this local map.*/
    if (!m)
    {
        *x = 0;
        return NULL;
    }

    if (((*x) >= 0) && ((*x) < MAP_WIDTH(m)) && ((*y) >= 0) && ((*y) < MAP_HEIGHT(m)))
        return m;

    if (*x < 0) /* thats w, nw or sw (3,7 or 6) */
    {
        if (*y < 0) /*  nw.. */
        {
            if (!m->tile_path[7])
            {
                *x = 0;
                return NULL;
            }
            if (!m->tile_map[7] || m->tile_map[7]->in_memory != MAP_IN_MEMORY)
            {
                *x = -1;
                return NULL;
            }
            *y += MAP_HEIGHT(m->tile_map[7]);
            *x += MAP_WIDTH(m->tile_map[7]);
            return (out_of_map2(m->tile_map[7], x, y));
        }

        if (*y >= MAP_HEIGHT(m)) /* sw */
        {
            if (!m->tile_path[6])
            {
                *x = 0;
                return NULL;
            }
            if (!m->tile_map[6] || m->tile_map[6]->in_memory != MAP_IN_MEMORY)
            {
                *x = -1;
                return NULL;
            }
            *y -= MAP_HEIGHT(m);
            *x += MAP_WIDTH(m->tile_map[6]);
            return (out_of_map2(m->tile_map[6], x, y));
        }


        if (!m->tile_path[3]) /* it MUST be west */
        {
            *x = 0;
            return NULL;
        }
        if (!m->tile_map[3] || m->tile_map[3]->in_memory != MAP_IN_MEMORY)
        {
            *x = -1;
            return NULL;
        }
        *x += MAP_WIDTH(m->tile_map[3]);
        return (out_of_map2(m->tile_map[3], x, y));
    }

    if (*x >= MAP_WIDTH(m))  /* thatd e, ne or se (1 ,4 or 5) */
    {
        if (*y < 0) /*  ne.. */
        {
            if (!m->tile_path[4])
            {
                *x = 0;
                return NULL;
            }
            if (!m->tile_map[4] || m->tile_map[4]->in_memory != MAP_IN_MEMORY)
            {
                *x = -1;
                return NULL;
            }
            *y += MAP_HEIGHT(m->tile_map[4]);
            *x -= MAP_WIDTH(m);
            return (out_of_map2(m->tile_map[4], x, y));
        }

        if (*y >= MAP_HEIGHT(m)) /* se */
        {
            if (!m->tile_path[5])
            {
                *x = 0;
                return NULL;
            }
            if (!m->tile_map[5] || m->tile_map[5]->in_memory != MAP_IN_MEMORY)
            {
                *x = -1;
                return NULL;
            }
            *y -= MAP_HEIGHT(m);
            *x -= MAP_WIDTH(m);
            return (out_of_map2(m->tile_map[5], x, y));
        }

        if (!m->tile_path[1])
        {
            *x = 0;
            return NULL;
        }
        if (!m->tile_map[1] || m->tile_map[1]->in_memory != MAP_IN_MEMORY)
        {
            *x = -1;
            return NULL;
        }
        *x -= MAP_WIDTH(m);
        return (out_of_map2(m->tile_map[1], x, y));
    }

    /* because we have tested x above, we don't need to check
    * for nw,sw,ne and nw here again.
    */
    if (*y < 0)
    {
        if (!m->tile_path[0])
        {
            *x = 0;
            return NULL;
        }
        if (!m->tile_map[0] || m->tile_map[0]->in_memory != MAP_IN_MEMORY)
        {
            *x = -1;
            return NULL;
        }
        *y += MAP_HEIGHT(m->tile_map[0]);
        return (out_of_map2(m->tile_map[0], x, y));
    }
    if (*y >= MAP_HEIGHT(m))
    {
        if (!m->tile_path[2])
        {
            *x = 0;
            return NULL;
        }
        if (!m->tile_map[2] || m->tile_map[2]->in_memory != MAP_IN_MEMORY)
        {
            *x = -1;
            return NULL;
        }
        *y -= MAP_HEIGHT(m);
        return (out_of_map2(m->tile_map[2], x, y));
    }
    *x = 0;
    return NULL;
}

/** Initializes a rv to sane values if no vector could be found. */
static inline int fail_rangevector(rv_vector *rv)
{
    rv->distance_x = rv->distance_y = rv->distance = UINT_MAX;
    rv->direction = 0;
    rv->part = NULL;
    return FALSE;
}

/** Get distance and direction between two objects.
* TODO: this should probably be replaced with a macro or an inline function
* Note: this function was changed from always calculating euclidian distance to
* defaulting to calculating manhattan distance. Gecko 20050714
* @see get_rangevector_full
*/
int get_rangevector(object *op1, object *op2, rv_vector *retval, int flags)
{
    return get_rangevector_full(
        op1, op1->map, op1->x, op1->y,
        op2, op2->map, op2->x, op2->y,
        retval, flags);
}

/** Get distance and direction between two coordinates.
* Never adjusts for multipart objects (since objects are unknown)
* TODO: this should probably be replaced with a macro or an inline function
* @see get_rangevector_full
*/
int get_rangevector_from_mapcoords(
                                   mapstruct *map1, int x1, int y1,
                                   mapstruct *map2, int x2, int y2,
                                   rv_vector *retval, int flags)
{
    return get_rangevector_full(NULL, map1, x1, y1, NULL, map2, x2, y2, retval, flags);
}

/** Get distance and direction between two points.
* This is the base for all get_rangevector_* functions. It can compute the
* rangevector between any two points on any maps, with or without adjusting
* for multipart objects.
*
* op1 and op2 are optional, but are required (separately or together) for multipart
* object handling. (Currently op2 is ignored but might be used in the future)
*
* Returns (through retval):
*  distance_x/y are distance away, which can be negative.
*  direction is the daimonin direction scheme from p1 to p2.
*  part is the part of op1 that is closest to p2. (can be NULL)
*  distance is an absolute distance value according to the selected algorithm.
*
* If the objects are not on maps, results are likely to be unexpected or fatal
*
* Flags:
*  RV_IGNORE_MULTIPART   - don't translate for closest body part.
*  RV_RECURSIVE_SEARCH   - handle separate maps better (slow and does still not
*                                search the whole mapset).
*  RV_MANHATTAN_DISTANCE - Calculate manhattan distance (dx+dy)  (fast)
*  RV_EUCLIDIAN_DISTANCE - straight line distance (slowest)
*  RV_FAST_EUCLIDIAN_DISTANCE - squared straight line distance (slow)
*  RV_DIAGONAL_DISTANCE  - diagonal (max(dx + dy)) distance (fast) (default)
*  RV_NO_DISTANCE        - don't calculate distance (or direction) (fastest)
*
* @return FALSE if the function fails (because of the maps being separate), and the rangevector will not be touched. Otherwise it will return TRUE.
*
*  TODO: support multipart->multipart handling
*/
int get_rangevector_full(
                         object *op1, mapstruct *map1, int x1, int y1,
                         object *op2, mapstruct *map2, int x2, int y2,
                         rv_vector *retval, int flags)
{
    /* Common calculations for almost all cases */
    retval->distance_x = x2 - x1;
    retval->distance_y = y2 - y1;

    if (map1 == map2)
    {
        /* Most common case. We are actually done */
    }
    else if (map1->tileset_id > 0 && map2->tileset_id > 0)
    {
        if(map1->tileset_id == map2->tileset_id && in_same_instance(map1, map2))
        {
            retval->distance_x += map2->tileset_x - map1->tileset_x;
            retval->distance_y += map2->tileset_y - map1->tileset_y;
        } 
        else
            return fail_rangevector(retval);
    }
    else if (map1->tile_map[0] == map2) /* North */
        retval->distance_y -= MAP_HEIGHT(map2);
    else if (map1->tile_map[1] == map2) /* East */
        retval->distance_x += MAP_WIDTH(map1);
    else if (map1->tile_map[2] == map2) /* South */
        retval->distance_y += MAP_HEIGHT(map1);
    else if (map1->tile_map[3] == map2) /* West */
        retval->distance_x -= MAP_WIDTH(map2);
    else if (map1->tile_map[4] == map2) /* Northeast */
    {
        retval->distance_x += MAP_WIDTH(map1);
        retval->distance_y -= MAP_HEIGHT(map2);
    }
    else if (map1->tile_map[5] == map2) /* Southeast */
    {
        retval->distance_x += MAP_WIDTH(map1);
        retval->distance_y += MAP_HEIGHT(map1);
    }
    else if (map1->tile_map[6] == map2) /* Southwest */
    {
        retval->distance_x -= MAP_WIDTH(map2);
        retval->distance_y += MAP_HEIGHT(map1);
    }
    else if (map1->tile_map[7] == map2) /* Northwest */
    {
        retval->distance_x -= MAP_WIDTH(map2);
        retval->distance_y -= MAP_HEIGHT(map2);
    }
    else if (flags & RV_RECURSIVE_SEARCH) /* Search */
    {
        if (!relative_tile_position(map1, map2, &(retval->distance_x), &(retval->distance_y)))
        {
            /*LOG(llevDebug,"DBUG: get_rangevector_from_mapcoords: No tileset path between maps '%s' and '%s'\n", map1->path, map2->path);*/
            return fail_rangevector(retval);
        }
    }
    else
    {
        /*LOG(llevDebug,"DBUG: get_rangevector_from_mapcoords: objects not on adjacent maps\n");*/
        return fail_rangevector(retval);
    }

    retval->part = op1;
    /* If this is multipart, find the closest part now */
    if (!(flags & RV_IGNORE_MULTIPART) && op1 && op1->more)
    {
        object *tmp, *best = NULL;
        int best_distance = retval->distance_x*retval->distance_x + retval->distance_y*retval->distance_y;
        int tmpi;

        /* we just take the offset of the piece to head to figure
        * distance instead of doing all that work above again
        * since the distance fields we set above are positive in the
        * same axis as is used for multipart objects, the simply arithemetic
        * below works.
        */
        for (tmp = op1->more; tmp; tmp = tmp->more)
        {
            tmpi =
                (retval->distance_x - tmp->arch->clone.x) * (retval->distance_x - tmp->arch->clone.x) +
                (retval->distance_y - tmp->arch->clone.y) * (retval->distance_y - tmp->arch->clone.y);
            if (tmpi < best_distance)
            {
                best_distance = tmpi;
                best = tmp;
            }
        }

        if (best)
        {
            retval->distance_x -= best->arch->clone.x;
            retval->distance_y -= best->arch->clone.y;
            retval->part = best;
        }
    }

    /* Calculate approximate direction */
    retval->direction = find_dir_2(-retval->distance_x, -retval->distance_y);

    /* Calculate distance */
    switch (flags & (0x04 | 0x08 | 0x10))
    {
    case RV_MANHATTAN_DISTANCE:
        retval->distance = abs(retval->distance_x) + abs(retval->distance_y);
        break;
    case RV_EUCLIDIAN_DISTANCE:
        retval->distance = isqrt(retval->distance_x * retval->distance_x + retval->distance_y * retval->distance_y);
        break;
    case RV_FAST_EUCLIDIAN_DISTANCE:
        retval->distance = retval->distance_x * retval->distance_x + retval->distance_y * retval->distance_y;
        break;
    case RV_DIAGONAL_DISTANCE:
        retval->distance = MAX(abs(retval->distance_x), abs(retval->distance_y));
        break;
    case RV_NO_DISTANCE:
        retval->distance = UINT_MAX;
        break;
    }

    return TRUE;
}

/* Returns true of op1 and op2 are on the same tileset
* (if there is a way to move between map tiles from op1 to op2)
*/
int on_same_tileset(object *op1, object *op2)
{
    if (!op1->map || !op2->map)
        return FALSE;

    /* This is the fallback in case the tileset data is
    * unavailable */
    if (op1->map->tileset_id == 0 || op2->map->tileset_id == 0)
        return on_same_map(op1, op2);

    return op1->map->tileset_id == op2->map->tileset_id
        && in_same_instance(op1->map, op2->map);
}

/* Returns true of op1 and op2 are effectively on the same map
* (as related to map tiling).
* this will ONLY work if op1 and op2 are on a DIRECT connected
* tiled map. Any recursive idea here will kill in a big tiled
* world map the server.
*/
int on_same_map(object *op1, object *op2)
{
    if (!op1->map || !op2->map)
        return FALSE;

    if (op1->map == op2->map
        || op1->map->tile_map[0] == op2->map
        || op1->map->tile_map[1] == op2->map
        || op1->map->tile_map[2] == op2->map
        || op1->map->tile_map[3] == op2->map
        || op1->map->tile_map[4] == op2->map
        || op1->map->tile_map[5] == op2->map
        || op1->map->tile_map[6] == op2->map
        || op1->map->tile_map[7] == op2->map)
        return TRUE;

    return FALSE;
}

/** Returns true if the two maps are in the same instance */
int in_same_instance(mapstruct *m1, mapstruct *m2)
{
    /* Common case: both maps are on the singleton MULTI "instance" */
    if(MAP_MULTI(m1) && MAP_MULTI(m2))
        return TRUE;

    /* Instance maps? */
    if(MAP_INSTANCE(m1) && MAP_INSTANCE(m2))
    {
        /* A player can only have a single active instance, so we
         * assume that if we have two valid map pointers to instance maps,
         * that belong to the same player they are on the same instance */
        if(m1->reference == m2->reference)
            return TRUE;
        else
            return FALSE;
    }

    /* Unique maps? */
    if(MAP_UNIQUE(m1) && MAP_UNIQUE(m2))
    {
        /* A unique map is always referenced by a single player */
        if(m1->reference == m2->reference)
            return TRUE;
        else
            return FALSE;
    }

    /* Different types */
    return FALSE;
}

/* transfer all items from one instance apartment to another.
* put them on spot x,y
*/
void map_transfer_apartment_items(mapstruct *map_old, mapstruct * map_new, int x, int y)
{
    int        i, j;
    object     *op, *tmp, *tmp2, *tmp3;

    for (i = 0; i < MAP_WIDTH(map_old); i++)
    {
        for (j = 0; j < MAP_HEIGHT(map_old); j++)
        {
            for (op = get_map_ob(map_old, i, j); op; op = tmp2)
            {
                tmp2 = op->above;
                /* if thats true, the player can't get it - no sense to transfer it! */
                if (QUERY_FLAG(op, FLAG_SYS_OBJECT))
                    continue;

                if (!QUERY_FLAG(op, FLAG_NO_PICK))
                {
                    remove_ob(op);
                    op->x = x;
                    op->y = y;
                    insert_ob_in_map(op, map_new, NULL, INS_NO_MERGE | INS_NO_WALK_ON);
                }
                else /* this is a fixed part of the map */
                {
                    /* now we test we have a container type object.
                    * The player can have items stored in it.
                    * If so, we remove them too.
                    * we don't check inv of non container object.
                    * The player can't store in normal sense items
                    * in them, so the items in them (perhaps special
                    * marker of forces) should not be transfered.
                    */

                    for (tmp = op->inv; tmp; tmp = tmp3)
                    {
                        tmp3 = tmp->below;
                        /* well, non pickup container in non pickup container? no no... */
                        if (QUERY_FLAG(tmp, FLAG_SYS_OBJECT) || QUERY_FLAG(tmp, FLAG_NO_PICK))
                            continue;
                        remove_ob(tmp);
                        tmp->x = x;
                        tmp->y = y;
                        insert_ob_in_map(tmp, map_new, NULL, INS_NO_MERGE | INS_NO_WALK_ON);
                    }
                }
            }
        }
    }
}

