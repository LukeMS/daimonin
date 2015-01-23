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

    The author can be reached via e-mail to info@daimonin.org
*/

/* See map.c for module information. */

#include "global.h"

static map_t  *LoadAndLinkTiledMap(map_t *orig_map, int tile_num);

#define CHECKTILEDMAP(_OLD_, _DIR_, _NEW_) \
    if (!(_OLD_)->tiling.tile_path[(_DIR_)]) \
    { \
        return NULL; \
    } \
    else if (!((_NEW_) = (_OLD_)->tiling.tile_map[(_DIR_)]) || \
             (_NEW_)->in_memory != MAP_MEMORY_ACTIVE) \
    { \
        if (!((_NEW_) = LoadAndLinkTiledMap((_OLD_), (_DIR_)))) \
        { \
            return NULL; \
        } \
    }

/* out of map now checks all 8 possible neighbours of
* a tiled map and loads them in when needed.
*/
map_t *out_of_map(map_t *m, sint16 *x, sint16 *y)
{
    map_t *m2;

    if (!m)
    {
        return NULL;
    }

    if (*x < 0)
    {
        if (*y < 0)
        {
            CHECKTILEDMAP(m, TILING_DIRECTION_NORTHWEST, m2);
            *x += m2->width;
            *y += m2->height;
            return OUT_OF_MAP(m2, *x, *y);
        }

        if (*y >= m->height)
        {
            CHECKTILEDMAP(m, TILING_DIRECTION_SOUTHWEST, m2);
            *x += m2->width;
            *y -= m->height;
            return OUT_OF_MAP(m2, *x, *y);
        }

        CHECKTILEDMAP(m, TILING_DIRECTION_WEST, m2);
        *x += m2->width;
        return OUT_OF_MAP(m2, *x, *y);
    }

    if (*x >= m->width)
    {
        if (*y < 0)
        {
            CHECKTILEDMAP(m, TILING_DIRECTION_NORTHEAST, m2);
            *x -= m->width;
            *y += m2->height;
            return OUT_OF_MAP(m2, *x, *y);
        }

        if (*y >= m->height)
        {
            CHECKTILEDMAP(m, TILING_DIRECTION_SOUTHEAST, m2);
            *x -= m->width;
            *y -= m->height;
            return OUT_OF_MAP(m2, *x, *y);
        }

        CHECKTILEDMAP(m, TILING_DIRECTION_EAST, m2);
        *x -= m->width;
        return OUT_OF_MAP(m2, *x, *y);
    }

    if (*y < 0)
    {
        CHECKTILEDMAP(m, TILING_DIRECTION_NORTH, m2);
        *y += m2->height;
        return OUT_OF_MAP(m2, *x, *y);
    }

    if (*y >= m->height)
    {
        CHECKTILEDMAP(m, TILING_DIRECTION_SOUTH, m2);
        *y -= m->height;
        return OUT_OF_MAP(m2, *x, *y);
    }

    return NULL;
}

#undef CHECKTILEDMAP

/** Try loading the connected map tile with the given number.
 * @param orig_map base map for tiling
 * @param tile_num tile-number to connect to (not direction).
 * @return If loading _or_ tiling fails NULL is returned,
 * otherwise the loaded map neighbouring orig_map is returned.
 */
static map_t *LoadAndLinkTiledMap(map_t *orig_map, int tile_num)
{
    /* Nowadays the loader keeps track of tiling. Gecko 2006-12-31 */
    map_t *m = ready_map_name(orig_map->tiling.tile_path[tile_num],
        orig_map->tiling.orig_tile_path[tile_num],
        MAP_STATUS_TYPE(orig_map->status), orig_map->reference);

    /* If loading or linking failed */
    if (!m ||
        m != orig_map->tiling.tile_map[tile_num])
    {
        /* ensure we don't get called again over and over */
        LOG(llevMapbug, "MAPBUG: failed to connect map %s with tile no %d (%s).\n",
            STRING_MAP_PATH(orig_map), tile_num,
            STRING_MAP_TILE_PATH(orig_map, tile_num));
        FREE_AND_CLEAR_HASH(orig_map->tiling.orig_tile_path[tile_num]);
        FREE_AND_CLEAR_HASH(orig_map->tiling.tile_path[tile_num]);
        m = NULL;
    }

    return m;
}

#define CHECKTILEDMAP(_OLD_, _DIR_, _NEW_, _X_) \
    if (!(_OLD_)->tiling.tile_path[(_DIR_)]) \
    { \
        *(_X_) = 0; \
        return NULL; \
    } \
    else if (!((_NEW_) = (_OLD_)->tiling.tile_map[(_DIR_)]) || \
             (_NEW_)->in_memory != MAP_MEMORY_ACTIVE) \
    { \
        *(_X_) = -1; \
        return NULL; \
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
map_t *out_of_map2(map_t *m, sint16 *x, sint16 *y)
{
    map_t *m2;

    if (!m)
    {
        return NULL;
    }

    if (*x < 0)
    {
        if (*y < 0)
        {
            CHECKTILEDMAP(m, TILING_DIRECTION_NORTHWEST, m2, x);
            *x += m2->width;
            *y += m2->height;
            return OUT_OF_MAP2(m2, *x, *y);
        }

        if (*y >= m->height)
        {
            CHECKTILEDMAP(m, TILING_DIRECTION_SOUTHWEST, m2, x);
            *x += m2->width;
            *y -= m->height;
            return OUT_OF_MAP2(m2, *x, *y);
        }

        CHECKTILEDMAP(m, TILING_DIRECTION_WEST, m2, x);
        *x += m2->width;
        return OUT_OF_MAP2(m2, *x, *y);
    }

    if (*x >= m->width)
    {
        if (*y < 0)
        {
            CHECKTILEDMAP(m, TILING_DIRECTION_NORTHEAST, m2, x);
            *x -= m->width;
            *y += m2->height;
            return OUT_OF_MAP2(m2, *x, *y);
        }

        if (*y >= m->height)
        {
            CHECKTILEDMAP(m, TILING_DIRECTION_SOUTHEAST, m2, x);
            *x -= m->width;
            *y -= m->height;
            return OUT_OF_MAP2(m2, *x, *y);
        }

        CHECKTILEDMAP(m, TILING_DIRECTION_EAST, m2, x);
        *x -= m->width;
        return OUT_OF_MAP2(m2, *x, *y);
    }

    if (*y < 0)
    {
        CHECKTILEDMAP(m, TILING_DIRECTION_NORTH, m2, x);
        *y += m2->height;
        return OUT_OF_MAP2(m2, *x, *y);
    }

    if (*y >= m->height)
    {
        CHECKTILEDMAP(m, TILING_DIRECTION_SOUTH, m2, x);
        *y -= m->height;
        return OUT_OF_MAP2(m2, *x, *y);
    }

    *x = 0;
    return NULL;
}

#undef CHECKTILEDMAP

/* on_same_tileset() compares m1 and m2 and returns 1 or 0 to indicate whether
 * they are or are not on the same tileset. */
sint8 on_same_tileset(map_t *m1, map_t *m2)
{
    /* If either m1 or m2 is NULL. they can't be on the same tileset. */
    if (!m1 ||
        !m2)
    {
        return 0;
    }
    /* But if they're the same, they must be on the same tileset, */
    else if (m1 == m2)
    {
        return 1;
    }

    /* This is the fallback in case the tileset data is unavailable. If they're
     * DIRECTLY connected tiled maps, they must be on the same tileset. Note
     * that this only works for such direct connections; any recursive idea
     * here will kill the server in a big tileset. */
    if (m1->tiling.tileset_id == 0 ||
        m2->tiling.tileset_id == 0)
    {
        int i;

        for (i = 0; i < TILING_DIRECTION_NROF; i++)
        {
            if (m1->tiling.tile_map[i] == m2)
            {
                return 1;
            }
        }
    }
    /* If ->tiling.tileset_id is the same and they're on the same tiling instance (see
     * below for definition), they're on the same tileset. */
    else if (m1->tiling.tileset_id == m2->tiling.tileset_id &&
             on_same_instance(m1, m2))
    {
        return 1;
    }

    /* If we reach here, they aren't on the same tileset. */
    return 0;
}

/* on_same_instance() compares m1 and m2 and returns 1 or 0 to indicate whether
 * they are or are not on the same tiling instance.
 *
 * A tiling instance is, for unique maps, a tileset 'belonging' to the same
 * player. For instanced maps, the definition is the same. But for multiplayer
 * maps, all tilesets are on the same singleton tiling instance. */
sint8 on_same_instance(map_t *m1, map_t *m2)
{
    /* Common case: both maps are on the singleton MULTI "instance" */
    if ((m1->status & MAP_STATUS_MULTI) &&
        (m2->status & MAP_STATUS_MULTI))
    {
        return 1;
    }

    /* Instance maps? */
    if ((m1->status & MAP_STATUS_INSTANCE) &&
        (m2->status & MAP_STATUS_INSTANCE))
    {
        /* A player can only have a single active instance, so we
         * assume that if we have two valid map pointers to instance maps,
         * that belong to the same player they are on the same instance */
        if (m1->reference == m2->reference)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    /* Unique maps? */
    if ((m1->status & MAP_STATUS_UNIQUE) &&
        (m2->status & MAP_STATUS_UNIQUE))
    {
        /* A unique map is always referenced by a single player */
        if (m1->reference == m2->reference)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    /* Different types */
    return 0;
}

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

    The author can be reached via e-mail to info@daimonin.org
*/

/* See map.c for module information. */

#include "global.h"

static int Fail(rv_t *rv);
static int RelativeTilePosition(map_t *map1, map_t *map2, rv_t *rv);

/** Get distance and direction between two objects.
* TODO: this should probably be replaced with a macro or an inline function
* Note: this function was changed from always calculating euclidian distance to
* defaulting to calculating manhattan distance. Gecko 20050714
* @see get_rangevector_full
*/
int get_rangevector(object_t *op1, object_t *op2, rv_t *rv, int flags)
{
    return get_rangevector_full(
        op1, op1->map, op1->x, op1->y,
        op2, op2->map, op2->x, op2->y,
        rv, flags);
}

/** Get distance and direction between two coordinates.
* Never adjusts for multipart objects (since objects are unknown)
* TODO: this should probably be replaced with a macro or an inline function
* @see get_rangevector_full
*/
int get_rangevector_from_mapcoords(
                                   map_t *map1, int x1, int y1,
                                   map_t *map2, int x2, int y2,
                                   rv_t *rv, int flags)
{
    return get_rangevector_full(NULL, map1, x1, y1, NULL, map2, x2, y2, rv, flags);
}

/** Get distance and direction between two points.
* This is the base for all get_rangevector_* functions. It can compute the
* rangevector between any two points on any maps, with or without adjusting
* for multipart objects.
*
* op1 and op2 are optional, but are required (separately or together) for multipart
* object handling. (Currently op2 is ignored but might be used in the future)
*
* Returns (through rv):
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
                         object_t *op1, map_t *map1, int x1, int y1,
                         object_t *op2, map_t *map2, int x2, int y2,
                         rv_t *rv, int flags)
{
    /* Common calculations for almost all cases */
    rv->distance_x = x2 - x1;
    rv->distance_y = y2 - y1;

    if (map1 == map2)
    {
        /* Most common case. We are actually done */
    }
    else if (map1->tiling.tileset_id > 0 && map2->tiling.tileset_id > 0)
    {
        if(map1->tiling.tileset_id == map2->tiling.tileset_id && on_same_instance(map1, map2))
        {
            rv->distance_x += map2->tiling.tileset_x - map1->tiling.tileset_x;
            rv->distance_y += map2->tiling.tileset_y - map1->tiling.tileset_y;
        }
        else
            return Fail(rv);
    }
    else if (map1->tiling.tile_map[TILING_DIRECTION_NORTH] == map2) /* North */
        rv->distance_y -= map2->height;
    else if (map1->tiling.tile_map[TILING_DIRECTION_EAST] == map2) /* East */
        rv->distance_x += map1->width;
    else if (map1->tiling.tile_map[TILING_DIRECTION_SOUTH] == map2) /* South */
        rv->distance_y += map1->height;
    else if (map1->tiling.tile_map[TILING_DIRECTION_WEST] == map2) /* West */
        rv->distance_x -= map2->width;
    else if (map1->tiling.tile_map[TILING_DIRECTION_NORTHEAST] == map2) /* Northeast */
    {
        rv->distance_x += map1->width;
        rv->distance_y -= map2->height;
    }
    else if (map1->tiling.tile_map[TILING_DIRECTION_SOUTHEAST] == map2) /* Southeast */
    {
        rv->distance_x += map1->width;
        rv->distance_y += map1->height;
    }
    else if (map1->tiling.tile_map[TILING_DIRECTION_SOUTHWEST] == map2) /* Southwest */
    {
        rv->distance_x -= map2->width;
        rv->distance_y += map1->height;
    }
    else if (map1->tiling.tile_map[TILING_DIRECTION_NORTHWEST] == map2) /* Northwest */
    {
        rv->distance_x -= map2->width;
        rv->distance_y -= map2->height;
    }
    else if (flags & RV_RECURSIVE_SEARCH) /* Search */
    {
        if (!RelativeTilePosition(map1, map2, rv))
        {
            /*LOG(llevDebug,"DBUG: get_rangevector_from_mapcoords: No tileset path between maps '%s' and '%s'\n", map1->path, map2->path);*/
            return Fail(rv);
        }
    }
    else
    {
        /*LOG(llevDebug,"DBUG: get_rangevector_from_mapcoords: objects not on adjacent maps\n");*/
        return Fail(rv);
    }

    rv->part = op1;
    /* If this is multipart, find the closest part now */
    if (!(flags & RV_IGNORE_MULTIPART) && op1 && op1->more)
    {
        object_t *part,
               *next,
               *best = NULL;
        int best_distance = rv->distance_x*rv->distance_x + rv->distance_y*rv->distance_y;
        int tmpi;

        /* we just take the offset of the piece to head to figure
        * distance instead of doing all that work above again
        * since the distance fields we set above are positive in the
        * same axis as is used for multipart objects, the simply arithemetic
        * below works.
        */
        FOREACH_PART_OF_OBJECT(part, op1->more, next)
        {
            tmpi =
                (rv->distance_x - part->arch->clone.x) * (rv->distance_x - part->arch->clone.x) +
                (rv->distance_y - part->arch->clone.y) * (rv->distance_y - part->arch->clone.y);
            if (tmpi < best_distance)
            {
                best_distance = tmpi;
                best = part;
            }
        }

        if (best)
        {
            rv->distance_x -= best->arch->clone.x;
            rv->distance_y -= best->arch->clone.y;
            rv->part = best;
        }
    }

    /* Calculate approximate direction */
    rv->direction = find_dir_2(-rv->distance_x, -rv->distance_y);

    /* Calculate distance */
    switch (flags & (0x04 | 0x08 | 0x10))
    {
    case RV_MANHATTAN_DISTANCE:
        rv->distance = abs(rv->distance_x) + abs(rv->distance_y);
        break;
    case RV_EUCLIDIAN_DISTANCE:
        rv->distance = isqrt(rv->distance_x * rv->distance_x + rv->distance_y * rv->distance_y);
        break;
    case RV_FAST_EUCLIDIAN_DISTANCE:
        rv->distance = rv->distance_x * rv->distance_x + rv->distance_y * rv->distance_y;
        break;
    case RV_DIAGONAL_DISTANCE:
        rv->distance = MAX(abs(rv->distance_x), abs(rv->distance_y));
        break;
    case RV_NO_DISTANCE:
        rv->distance = UINT_MAX;
        break;
    }

    return TRUE;
}

/** Initializes a rv to sane values if no vector could be found. */
static int Fail(rv_t *rv)
{
    rv->distance_x = rv->distance_y = rv->distance = UINT_MAX;
    rv->direction = 0;
    rv->part = NULL;
    return FALSE;
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
* is attempted. */
static int RelativeTilePosition(map_t *map1, map_t *map2, rv_t *rv)
{
    int                    i;
    static uint32          traversal_id = 0;
    struct mapsearch_node *first,
                          *last,
                          *curr,
                          *node;
    int                    success = FALSE;
    int                    searched_tiles = 0;

    /* Save some time in the simplest cases ( very similar to on_same_map() )*/
    if (!map1 ||
        !map2)
    {
        return FALSE;
    }

    /* Precalculated tileset data available? */
    if (map1->tiling.tileset_id > 0 &&
        map2->tiling.tileset_id > 0)
    {
//        LOG(llevDebug, "RelativeTilePosition(): Could use tileset data for %s -> %s\n", map1->path, map2->path);
        if (map1->tiling.tileset_id == map2->tiling.tileset_id &&
            on_same_instance(map1, map2))
        {
            rv->distance_x += map2->tiling.tileset_x - map1->tiling.tileset_x;
            rv->distance_y += map2->tiling.tileset_y - map1->tiling.tileset_y;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    if (map1 == map2)
    {
        return TRUE;
    }

//    LOG(llevBug, "RelativeTilePosition(): One or both of maps %s and %s lacks tileset data\n", map1->path, map2->path);

    /* Check for cached pathfinding */
    /* The caching really helps when pathfinding across map tiles but not in many other cases. */
    if (map1->rv_cache.path == map2->path)
    {
        rv->distance_x += map1->rv_cache.x;
        rv->distance_y += map1->rv_cache.y;
        return TRUE;
    }

    if (map2->rv_cache.path == map1->path)
    {
        rv->distance_x -= map2->rv_cache.x;
        rv->distance_y -= map2->rv_cache.y;
        return TRUE;
    }

    /* TODO: effectivize somewhat by doing bidirectional search */
    /* TODO: big project: magically make work with pre- or dynamically computed bigmap data */

    /* Avoid overflow of traversal_id */
    if (traversal_id == 4294967295U /* UINT_MAX */)
    {
        map_t  *m;

        LOG(llevDebug, "RelativeTilePosition(): resetting traversal id\n");

        for (m = first_map; m; m = m->next)
        {
            m->traversed = 0;
        }

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
        for (i = 0; i < TILING_DIRECTION_NROF; i++)
        {
            if (curr->map->tiling.tile_path[i] &&
                (!curr->map->tiling.tile_map[i] ||
                 curr->map->tiling.tile_map[i]->traversed != traversal_id))
            {
                if (!curr->map->tiling.tile_map[i] ||
                    curr->map->tiling.tile_map[i]->in_memory != MAP_MEMORY_ACTIVE)
                {
                    if (!LoadAndLinkTiledMap(curr->map, i))
                    {
                        continue; /* invalid path - we don't found the map */
                    }
                }

                /* TODO: avoid this bit of extra work if correct map */
                node = get_poolchunk(pool_map_bfs);
                node->dx = curr->dx;
                node->dy = curr->dy;
                node->map = curr->map->tiling.tile_map[i];

                /* Calc dx/dy */
                switch (i)
                {
                    case 0:  /* North */
                    node->dy -= curr->map->tiling.tile_map[i]->height;
                    break;

                    case 1:  /* East */
                    node->dx += curr->map->width;
                    break;

                    case 2:  /* South */
                    node->dy += curr->map->height;
                    break;

                    case 3:  /* West */
                    node->dx -= curr->map->tiling.tile_map[i]->width;
                    break;

                    case 4:  /* Northest */
                    node->dy -= curr->map->tiling.tile_map[i]->height;
                    node->dx += curr->map->width;
                    break;

                    case 5:  /* Southest */
                    node->dy += curr->map->height;
                    node->dx += curr->map->width;
                    break;

                    case 6:  /* Southwest */
                    node->dy += curr->map->height;
                    node->dx -= curr->map->tiling.tile_map[i]->width;
                    break;

                    case 7:  /* Northwest */
                    node->dy -= curr->map->tiling.tile_map[i]->height;
                    node->dx -= curr->map->tiling.tile_map[i]->width;
                    break;
                }

                /* Correct map? */
                if (node->map == map2)
                {
                    /* store info in cache */
                    FREE_AND_ADD_REF_HASH(map1->rv_cache.path, map2->path);
                    map1->rv_cache.x = node->dx;
                    map1->rv_cache.y = node->dy;

                    /* return result and clean up */
                    rv->distance_x += node->dx;
                    rv->distance_y += node->dy;
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
                {
                    first = last = node;
                }

                node->map->traversed = traversal_id;
            }
        }

        return_poolchunk(curr, pool_map_bfs);

        /* Depth-limitation */
        if (++searched_tiles >= MAX_SEARCH_MAP_TILES)
        {
            LOG(llevDebug, "RelativeTilePosition(): reached max nrof search tiles - bailing out\n");
            break;
        }

        /* dequeue next tile to check */
        curr = first;
        first = (curr) ? curr->next : NULL;
    }

out:
    for (node = first; node; node = node->next)
    {
        return_poolchunk(node, pool_map_bfs);
    }

    return success;
}
