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
#ifndef WIN32 /* ---win32 exclude header */
#include <unistd.h>
#endif /* win32 */

int global_darkness_table[MAX_DARKNESS + 1] =
{
    0, 20, 40, 80, 160, 320, 640, 1280
};

/* to get the reverse direction for all 8 tiled map index */
int map_tiled_reverse[TILED_MAPS]           =
{
    2, 3, 0, 1, 6, 7, 4, 5
};

/* Tag counter for the memory/weakref system. Uniquely identifies this instance of the map
 * in memory. Not the same as the map_tag/global_map_tag. */
static tag_t global_map_id;

/*
 * Returns the mapstruct which has a name matching the given argument.
 * return NULL if no match is found. This version _requires_ a shared string as input.
 */
mapstruct *has_been_loaded_sh(const char *name)
{
    mapstruct  *map;

    if (!name || !*name)
        return NULL;

    /* this IS a bug starting without '/' or '.' - this can lead in double loaded maps!
     * We don't fix it here anymore - this MUST be done by the calling functions or our
     * inheritanced map system is already broken somewhere before this call.
     */
    if (*name != '/' && *name != '.')
    {
        LOG(llevDebug, "DEBUG: has_been_loaded_sh: filename without start '/' or '.' (%s)\n", name);
        return NULL;
    }

    for (map = first_map; map; map = map->next)
    {
        /*LOG(-1,"check map: >%s< find: >%s<\n", name, map->path);*/
        if (name == map->path)
            break;
    }

    return (map);
}

/*
 * This makes a path absolute outside the world of Crossfire.
 * In other words, it prepends LIBDIR/MAPDIR/ to the given path
 * and returns the pointer to a static array containing the result.
 * it really should be called create_mapname
 */
char * create_mapdir_pathname(const char *name)
{
    static char buf[MAXPATHLEN];

    /* double "//" would be a problem for comparing path strings */
    if (*name == '/')
        sprintf(buf, "%s%s", settings.mapdir, name);
    else
        sprintf(buf, "%s/%s", settings.mapdir, name);
    return (buf);
}

/*
 * This function checks if a file with the given path exists.
 * -1 is returned if it fails, otherwise the mode of the file
 * is returned.
 * It tries out all the compression suffixes listed in the uncomp[] array.
 *
 * If prepend_dir is set, then we call create_mapdir_pathname (which prepends
 * libdir & mapdir).  Otherwise, we assume the name given is fully
 * complete.
 * Only the editor actually cares about the writablity of this -
 * the rest of the code only cares that the file is readable.
 * when the editor goes away, the call to stat should probably be
 * replaced by an access instead (similar to the windows one, but
 * that seems to be missing the prepend_dir processing
 */

int check_path(const char *name, int prepend_dir)
{
    if (prepend_dir)
    {
        char buf[MAXPATHLEN];

        strcpy(buf, create_mapdir_pathname(name));
        name = buf;
    }

    return(access(name, 0));
}

/** Make path absolute and remove ".." and "." entries.
 * path will become a normalized (absolute) version of the path in dst, with all
 * relative path references (".." and "." - parent directory and same directory)
 * resolved (path will not contain any ".." or "." elements, even if dst did).
 * If dst was not already absolute, the directory part of src will be used
 * as the base path and dst will be added to it.
 * @param[in] src already normalized file name for finding absolute path
 * @param[in] dst path to normalize. Should be either an absolute path or a path
 * relative to src. It must be a true "source" map path, and not a path into the
 * "./instance" or "./players" directory.
 * If dst is NULL, src is used for dst too.
 * @param[out] destination buffer for normalized path. Should be at least MAXPATHLEN big.
 * @return pointer to path
 */
char *normalize_path(const char *src, const char *dst, char *path)
{
    char   *p;
    /* char *q; */
    char    buf[MAXPATHLEN*2];

    /*LOG(llevDebug,"path before normalization >%s< >%s<\n", src, dst?dst:"<no dst>");*/

    if(!dst)
        dst = src;
    if(!src)
    {
        LOG(llevDebug,"BUG: normalize_path(): Called with src path = NULL! (dst:%s)\n", STRING_SAFE(dst));
        path[0] = '\0';
        return path;
    }

    /* First, make the dst path absolute */
    if (*dst == '/')
    {
        /* Already absolute path */
        strcpy(buf, dst);
    }
    else
    {
        /* Extra safety check. Never normalize unique/instance map paths */
        if(strncmp(dst, "./", 2) == 0)
        {
            /* (Only works with default directory paths) */
            /* Actually, a settings.localdir that doesn't start with "." will probalbly
             * break instanced and unique maps... */
            if(strncmp(dst, LOCALDIR "/" INSTANCEDIR, LSTRLEN(LOCALDIR "/" INSTANCEDIR)) == 0
                    || strncmp(dst, LOCALDIR "/" PLAYERDIR, LSTRLEN(LOCALDIR "/" PLAYERDIR)) == 0)
            {
                LOG(llevDebug,"BUG: normalize_path(): Called with unique/instance dst: %s\n", dst);
                strcpy(path, dst);
                return path;
            }
        }

        /* Combine directory part of src with dst to create absolute path */
        strcpy(buf, src);
        if ((p = strrchr(buf, '/')))
            p[1] = '\0';
        else
            strcpy(buf, "/");
        strcat(buf, dst);
    }

    /* Hmm.. This looks buggy. Meant to remove initial double slashes?
     * There will be problems if there are double slashes anywhere else in the
     * path. Gecko 2006-09-24. */
    /* Disabled to see if anything breaks. Gecko 2007-02-03 */
#if 0
    q = p = buf;
    while ((q = strstr(q, "//")))
        p = ++q;
#else
    p = buf;
    if(strstr(p, "//"))
        LOG(llevBug, "BUG: map path with unhandled '//' element: %s\n", buf);
#endif

    *path = '\0';
    p = strtok(p, "/");
    while (p)
    {
        if(strcmp(p, ".") == 0)
        {
            /* Just ignore "./" path elements */
        }
        else if(strcmp(p, "..") == 0)
        {
            /* Remove last inserted path element from 'path' */
            char *separator = strrchr(path, '/');
            if (separator)
                *separator = '\0';
            else
            {
                LOG(llevBug, "BUG: Illegal path (too many \"..\" entries): %s\n", dst);
                *path = '\0';
                return path; /* Don't continue normalization */
            }
        }
        else
        {
            strcat(path, "/");
            strcat(path, p);
        }
        p = strtok(NULL, "/");
    }
    /*LOG(llevDebug,"path after normalization >%s<\n", path);*/

    return (path);
}

/* same as above but here we know that src & dst was normalized before - so
 * we can just merge them without checking for ".." again.
 */
char *normalize_path_direct(const char *src, const char *dst, char *path)
{
    /*LOG(llevDebug,"path before normalization >%s< >%s<\n", src, dst?dst:"<no dst>");*/

    if(!src)
    {
        LOG(llevBug,"normalize_path_direct(): Called with src path = NULL! (dst:%s)\n", STRING_SAFE(dst));
        path[0] = '\0';
        return path;
    }

    if(!dst)
        dst = src;

    if(dst)
    {
        /* First, make the dst path absolute */
        if (*dst == '/')
        {
            /* Already absolute path */
            strcpy(path, dst);
        }
        else
        {
            char   *p;

           /* Combine directory part of src with dst to create absolute path */
            strcpy(path, src);
            if ((p = strrchr(path, '/')))
                p[1] = '\0';
            else
                strcpy(path, "/");
            strcat(path, dst);
        }
    }
    return path;
}

/*
 * Prints out debug-information about a map.
 * Dumping these at llevError doesn't seem right, but is
 * necessary to make sure the information is in fact logged.
 */

void dump_map(mapstruct *m)
{
    LOG(llevSystem, "Map %s status: %d.\n", m->path, m->in_memory);
    LOG(llevSystem, "Size: %dx%d Start: %d,%d\n", MAP_WIDTH(m), MAP_HEIGHT(m), MAP_ENTER_X(m), MAP_ENTER_Y(m));

    if (m->msg != NULL)
        LOG(llevSystem, "Message:\n%s", m->msg);

    if (m->tmpname != NULL)
        LOG(llevSystem, "Tmpname: %s\n", m->tmpname);

    LOG(llevSystem, "Tileset: %s\n", m->tileset_id);
    if(m->tileset_id == 0)
        LOG(llevSystem, "Tileset coords: %d,%d\n", m->tileset_x, m->tileset_y);

    LOG(llevSystem, "Difficulty: %d\n", m->difficulty);
    LOG(llevSystem, "Darkness: %d\n", m->darkness);
    LOG(llevSystem, "Light: %d\n", m->light_value);
    LOG(llevSystem, "Outdoor: %d\n", MAP_OUTDOORS(m));
}

/*
 * Prints out debug-information about all maps.
 * This basically just goes through all the maps and calls
 * dump_map on each one.
 */

void dump_all_maps()
{
    mapstruct  *m;
    for (m = first_map; m != NULL; m = m->next)
    {
        dump_map(m);
    }
}

/*
 * Allocates, initialises, and returns a pointer to a mapstruct.
 * Modified to no longer take a path option which was not being
 * used anyways.  MSW 2001-07-01
 */

mapstruct * get_linked_map()
{
    mapstruct *map = get_poolchunk(pool_map);

    if (map == NULL) {
        LOG(llevError, "ERROR: get_linked_map(): OOM.\n");
        return NULL;
    }

    memset(map, 0, sizeof(mapstruct));
    map->tag = ++global_map_id;

    /* lifo queue */
    if(first_map)
        first_map->last = map;
    map->next = first_map;
    first_map = map;

    map->in_memory = MAP_SWAPPED;
    /* The maps used to pick up default x and y values from the
     * map archetype.  Mimic that behaviour.
     */
    MAP_WIDTH(map) = 16;
    MAP_HEIGHT(map) = 16;
    MAP_RESET_TIMEOUT(map) = 7200;
    MAP_TIMEOUT(map) = 300;
    /* default light of a map is full daylight */
    MAP_DARKNESS(map) = -1;
    map->light_value = global_darkness_table[MAX_DARKNESS];

    MAP_ENTER_X(map) = 1;
    MAP_ENTER_Y(map) = 1;

    /* We insert a dummy sentinel first in the activelist. This simplifies
     * work later */
    map->active_objects = get_object();
    insert_ob_in_ob(map->active_objects, &void_container); /* Avoid gc of the sentinel object */

    return map;
}

/*
 * Allocates the arrays contained in a mapstruct.
 * This basically allocates the dynamic array of spaces for the
 * map.
 */

void allocate_map(mapstruct *m)
{
#if 0
    /* These are obnoxious - presumably the caller of this function knows what it is
     * doing.  Instead of checking for load status, lets check instead to see
     * if the data has already been allocated.
     */
    if(m->in_memory != MAP_SWAPPED )
    return;
#endif
    m->in_memory = MAP_LOADING;
    /* Log this condition and free the storage.  We could I suppose
     * realloc, but if the caller is presuming the data will be intact,
     * that is their poor assumption.
     */
    if (m->spaces || m->bitmap)
    {
        LOG(llevError, "ERROR: allocate_map callled with already allocated map (%s)\n", m->path);
        if (m->spaces)
            free(m->spaces);
        if (m->bitmap)
            free(m->bitmap);
    }
    if (m->buttons)
    {
        LOG(llevBug, "Bug: allocate_map callled with allready set buttons (%s)\n", m->path);
    }

    m->spaces = calloc(1, MAP_WIDTH(m) * MAP_HEIGHT(m) * sizeof(MapSpace));

    m->bitmap = malloc(((MAP_WIDTH(m) + 31) / 32) * MAP_HEIGHT(m) * sizeof(uint32));

    if (m->spaces == NULL || m->bitmap == NULL)
        LOG(llevError, "ERROR: allocate_map(): OOM.\n");
}

/* Creatures and returns a map of the specific size.  Used
 * in random map code and the editor.
 */
mapstruct * get_empty_map(int sizex, int sizey)
{
    mapstruct  *m   = get_linked_map();
    if(! m)
        return NULL;
    m->width = sizex;
    m->height = sizey;
    m->in_memory = MAP_SWAPPED;
    allocate_map(m);
    return m;
}

/* This loads the header information of the map.  The header
 * contains things like difficulty, size, timeout, etc.
 * this used to be stored in the map object, but with the
 * addition of tiling, fields beyond that easily named in an
 * object structure were needed, so it just made sense to
 * put all the stuff in the map object so that names actually make
 * sense.
 * This could be done in lex (like the object loader), but I think
 * currently, there are few enough fields this is not a big deal.
 * MSW 2001-07-01
 * NOTE: load_map_header will setup map_status dynamically when flags
 * has not a valid MAP_STATUS_FLAG()
 * return 0 on success, 1 on failure.
 */
static int load_map_header(FILE *fp, mapstruct *m, int flags)
{
    char    buf[HUGE_BUF], msgbuf[HUGE_BUF], *key = buf, *value, *end;
    int     msgpos  = 0;
    int     got_end = 0;

    while (fgets(buf, HUGE_BUF - 1, fp) != NULL)
    {
        buf[HUGE_BUF - 1] = 0;
        key = buf;
        while (isspace(*key))
            key++;
        if (*key == 0)
            continue;    /* empty line */
        value = strchr(key, ' ');
        if (!value)
        {
            end = key + (strlen(key) - 1);
            while (isspace(*end))
                --end;
            *++end = 0;
        }
        else
        {
            *value = 0;
            value++;
            while (isspace(*value))
                value++;
            end = value + (strlen(value) - 1);
            while (isspace(*end))
                --end;
            if (*++end != '\0')
            {
                *end = '\n';
                end[1] = '\0';
            }
        }

        /* key is the field name, value is what it should be set
         * to.  We've already done the work to null terminate key,
         * and strip off any leading spaces for both of these.
         * We have not touched the newline at the end of the line -
         * these are needed for some values.  the end pointer
         * points to the first of the newlines.
         * value could be NULL!  It would be easy enough to just point
         * this to "" to prevent cores, but that would let more errors slide
         * through.
         */

        if (!strcmp(key, "arch"))
        {
            /* This is an oddity, but not something we care about much. */
            /*if (strcmp(value, "map\n"))
                LOG(llevError, "ERROR: loading map and got a non 'arch map' line(%s %s)?\n", key, value);
            */
        }
        else if (!strcmp(key, "name"))
        {
            *end = 0;
            FREE_AND_COPY_HASH(m->name, value);
        }
        else if (!strcmp(key, "msg"))
        {
            while (fgets(buf, HUGE_BUF - 1, fp) != NULL)
            {
                if (!strcmp(buf, "endmsg\n"))
                    break;
                else
                {
                    /* slightly more efficient than strcat */
                    strcpy(msgbuf + msgpos, buf);
                    msgpos += strlen(buf);
                }
            }
            /* There are lots of maps that have empty messages (eg, msg/endmsg
             * with nothing between).  There is no reason in those cases to
             * keep the empty message.  Also, msgbuf contains garbage data
             * when msgpos is zero, so copying it results in crashes
             */
            if (msgpos != 0)
                FREE_AND_COPY_HASH(m->msg, msgbuf);
        }
        else if (!strcmp(key, "enter_x"))
        {
            m->enter_x = atoi(value);
        }
        else if (!strcmp(key, "enter_y"))
        {
            m->enter_y = atoi(value);
        }
        else if (!strcmp(key, "width"))
        {
            m->width = atoi(value);
        }
        else if (!strcmp(key, "height"))
        {
            m->height = atoi(value);
        }
        else if (!strcmp(key, "reset_timeout"))
        {
            m->reset_timeout = atoi(value);
        }
        else if (!strcmp(key, "swap_time"))
        {
            m->timeout = atoi(value);
        }
        else if (!strcmp(key, "difficulty"))
        {
            m->difficulty = atoi(value);
        }
        else if (!strcmp(key, "darkness"))
        {
            MAP_DARKNESS(m) = atoi(value);
            if (MAP_DARKNESS(m) == -1)
                m->light_value = global_darkness_table[MAX_DARKNESS];
            else
            {
                if (MAP_DARKNESS(m) < 0 || MAP_DARKNESS(m) > MAX_DARKNESS)
                {
                    LOG(llevBug, "\nBug: Illegal map darkness %d, setting to %d\n", MAP_DARKNESS(m), MAX_DARKNESS);
                    MAP_DARKNESS(m) = MAX_DARKNESS;
                }
                m->light_value = global_darkness_table[MAP_DARKNESS(m)];
            }
        }
        else if (!strcmp(key, "light"))
        {
            m->light_value = atoi(value);

            /* i assume that the default map_flags settings is 0 - so we don't handle <flagset> 0 */
        }
        else if (!strcmp(key, "no_save"))
        {
            if (atoi(value))
                m->map_flags |= MAP_FLAG_NO_SAVE;
        }
        else if (!strcmp(key, "no_magic"))
        {
            if (atoi(value))
                m->map_flags |= MAP_FLAG_NOMAGIC;
        }
        else if (!strcmp(key, "no_priest"))
        {
            if (atoi(value))
                m->map_flags |= MAP_FLAG_NOPRIEST;
        }
        else if (!strcmp(key, "no_harm"))
        {
            if (atoi(value))
                m->map_flags |= MAP_FLAG_NOHARM;
        }
        else if (!strcmp(key, "no_summon"))
        {
            if (atoi(value))
                m->map_flags |= MAP_FLAG_NOSUMMON;
        }
        else if (!strcmp(key, "fixed_login"))
        {
            if (atoi(value))
                m->map_flags |= MAP_FLAG_FIXED_LOGIN;
        }
        else if (!strcmp(key, "perm_death"))
        {
            if (atoi(value))
                m->map_flags |= MAP_FLAG_PERMDEATH;
        }
        else if (!strcmp(key, "ultra_death"))
        {
            if (atoi(value))
                m->map_flags |= MAP_FLAG_ULTRADEATH;
        }
        else if (!strcmp(key, "ultimate_death"))
        {
            if (atoi(value))
                m->map_flags |= MAP_FLAG_ULTIMATEDEATH;
        }
        else if (!strcmp(key, "pvp"))
        {
            if (atoi(value))
                m->map_flags |= MAP_FLAG_PVP;
        }
        else if (!strcmp(key, "outdoor"))
        {
            if (atoi(value))
                m->map_flags |= MAP_FLAG_OUTDOOR;
        }
        else if (!strcmp(key, "map_tag"))
        {
            m->map_tag = (uint32) atoi(value);
        }
        else if (!strcmp(key, "unique"))
        {
            if (atoi(value))
                m->map_status |= MAP_STATUS_UNIQUE;
        }
        else if (!strcmp(key, "multi"))
        {
            if (atoi(value))
                m->map_status |= MAP_STATUS_MULTI;
        }
        else if (!strcmp(key, "instance"))
        {
            if (atoi(value))
                m->map_status |= MAP_STATUS_INSTANCE;
        }
        else if (!strcmp(key, "reference"))
        {
            *end = 0;
            FREE_AND_COPY_HASH(m->reference, value);
            LOG(llevDebug, "Map has reference to player '%s'\n", value);
        }
        else if (!strcmp(key, "fixed_resettime"))
        {
            if (atoi(value))
                m->map_flags |= MAP_FLAG_FIXED_RTIME;
        }
        else if (!strcmp(key, "orig_path"))
        {
            *end = 0;
            /* important override - perhaps src_path was setup with the (wrong) dest path!
             * that can happens when we force explicit the load of an old instance or
             * unique map where we want be sure that our map loader don't fallback to
             * a original source map from /maps. Because EVERY map outside /maps has its orig_path
             * stored in the header, we always have it restored here as needed.
             */
            FREE_AND_COPY_HASH(m->orig_path, value);
        }
        else if (!strncmp(key, "orig_tile_path_", 15))
        {
            int tile = atoi(key + 15);

            if (tile<1 || tile>TILED_MAPS)
            {
                LOG(llevError,  "ERROR: load_map_header: orig tile location %d out of bounds (%s) (%s)\n",
                                tile, STRING_MAP_PATH(m), STRING_SAFE(value));
            }
            else
            {
                *end = 0;
                if (m->orig_tile_path[tile - 1])
                {
                    LOG(llevError, "ERROR: load_map_header: orig tile location %d duplicated (%s) (%s)\n",
                                   tile, STRING_MAP_PATH(m), STRING_SAFE(value));
                }

                m->orig_tile_path[tile - 1] = add_string(value);
            }
        }
        else if (!strncmp(key, "tile_path_", 10))
        {
            int tile = atoi(key + 10);
            mapstruct *neighbour;
            const char *path_sh;

            if (tile<1 || tile>TILED_MAPS)
            {
                LOG(llevError, "ERROR: load_map_header: tile location %d out of bounds (%s) (%s)\n",
                               tile, STRING_MAP_PATH(m), STRING_SAFE(value));
            }
            else
            {
                *end = 0;
                if (m->tile_path[tile - 1])
                {
                    LOG(llevError, "ERROR: load_map_header: tile location %d duplicated (%s) (%s)\n",
                                   tile, STRING_MAP_PATH(m), STRING_SAFE(value));
                }

                /* note: this only works because our map saver is storing MAP_STATUS and orig_map
                 * BEFORE he saves the tile map data. NEVER change it, or the dynamic setting will fail!
                 */
                if(!MAP_STATUS_TYPE(flags)) /* synchronize dynamically the map status flags */
                {
                    flags |= m->map_status;
                    if(!MAP_STATUS_TYPE(flags)) /* still zero? then force _MULTI */
                        flags |= MAP_STATUS_MULTI;
                }

                if(flags & MAP_STATUS_ORIGINAL) /* original map... lets normalize tile_path[] to /maps */
                {
                    normalize_path(m->orig_path, value, msgbuf);
                    m->orig_tile_path[tile - 1] = add_string(msgbuf);

                    /* whatever we have opened - in m->path is the REAL path */
                    if(flags & (MAP_STATUS_UNIQUE|MAP_STATUS_INSTANCE) )
                    {
                        normalize_path_direct(m->path, path_to_name(m->orig_tile_path[tile - 1]), msgbuf);
                        path_sh = add_string(msgbuf);
                    }
                    else /* for multi maps, orig_path is the same path */
                        path_sh = add_refcount(m->orig_tile_path[tile - 1]);

                }
                else /* non original map - all the things above was done before - just load */
                    path_sh = add_string(value);

                /* If the neighbouring map tile has been loaded, set up the map pointers */
                if ((neighbour = has_been_loaded_sh(path_sh)) && (neighbour->in_memory == MAP_IN_MEMORY || neighbour->in_memory == MAP_LOADING))
                {
                    int dest_tile = map_tiled_reverse[tile - 1];

                    /* LOG(llevDebug,"add t_map %s (%d). ", path_sh, tile-1); */
                    if (neighbour->orig_tile_path[dest_tile] != m->orig_path)
                    {
                        /* Refuse tiling if anything looks suspicious, since that may leave dangling pointers and crash the server */
                        LOG(llevMapbug, "MAPBUG: map tiles incorrecly connected: %s->%s but %s->%s. Refusing to connect them!\n",
                                STRING_SAFE(m->orig_path), path_sh ? path_sh : "(no map)",
                                STRING_SAFE(neighbour->orig_path), neighbour->orig_tile_path[dest_tile] ? neighbour->orig_tile_path[dest_tile] : "(no map)" );
                        /* Disable map linking */
                        FREE_AND_CLEAR_HASH(path_sh);
                        FREE_AND_CLEAR_HASH(m->orig_tile_path[tile - 1]);
                    }
                    else
                    {
                        m->tile_map[tile - 1] = neighbour;
                        neighbour->tile_map[dest_tile] = m;
                    }
                }

                m->tile_path[tile - 1] = path_sh;
            }
        }
        else if (!strcmp(key, "tileset_id"))
            m->tileset_id = atoi(value);
        else if (!strcmp(key, "tileset_x"))
            m->tileset_x = atoi(value);
        else if (!strcmp(key, "tileset_y"))
            m->tileset_y = atoi(value);
        else if (!strcmp(key, "end"))
        {
            got_end = 1;
            break;
        }
        else
        {
            LOG(llevBug, "BUG: Got unknown value in map header: %s %s\n", key, value);
        }
    }

#ifndef PRODUCTION_SYSTEM
    /* Verify tileset_id after linking */
    {
        int i;
        for(i=0; i<8; i++) {
            if(m->tile_map[i] && m->tile_map[i]->tileset_id != m->tileset_id) {
                /* AI pathfinding reuquires consistent tileset_id */
                LOG(llevMapbug, "MAPBUG: connected maps have inconsistent tileset_ids: %s (id %d)<->%s (id %d). Pathfinding will have problems.\n",
                        STRING_SAFE(m->orig_path), m->tileset_id, STRING_SAFE(m->tile_map[i]->orig_path), m->tile_map[i]->tileset_id);
                /* TODO: also doublecheck tileset_x and tileset_y */
            }
        }
    }
#endif

    if(!MAP_STATUS_TYPE(m->map_status)) /* synchronize dynamically the map status flags */
    {
        m->map_status |= MAP_STATUS_TYPE(flags);
        if(!MAP_STATUS_TYPE(m->map_status)) /* still zero? then force _MULTI */
            m->map_status |= MAP_STATUS_MULTI;
    }
    m->map_status |= (flags & MAP_STATUS_ORIGINAL);

    if (! got_end)
    {
        LOG(llevBug, "BUG: Got premature eof on map header!\n");
        return 1;
    }

    return 0;
}


/*
 * Saves a map to file.  If flag is set, it is saved into the same
 * file it was (originally) loaded from.  Otherwise a temporary
 * filename will be genarated, and the file will be stored there.
 * The temporary filename will be stored in the mapstructure.
 * If the map is unique, we also save to the filename in the map
 * (this should have been updated when first loaded)
 */

int new_save_map(mapstruct *m, int flag)
{
    FILE   *fp;
    char    filename[MAXPATHLEN];
    int     i;

    if (flag && !*m->path)
    {
        LOG(llevBug, "BUG: Tried to save map without path.\n");
        return -1;
    }

    if (flag || MAP_UNIQUE(m) || MAP_INSTANCE(m))
    {
        if (MAP_UNIQUE(m) || MAP_INSTANCE(m))
        {
            /* that ensures we always reload from original maps */
            if (MAP_NOSAVE(m))
            {
                LOG(llevDebug, "skip map %s (no_save flag)\n", m->path);
                return 0;
            }
            strcpy(filename, m->path);
        }
        else
            strcpy(filename, create_mapdir_pathname(m->path));

        /* /maps, /tmp should always there and player dir is created when player is loaded */
        /* make_path_to_file(filename); */
    }
    else
    {
        /* create tmpname if we don't have one or our old one was used by a different map */
        if (!m->tmpname || access(m->tmpname, F_OK) ==-1 )
        {
            FREE_AND_NULL_PTR(m->tmpname);
            tempnam_local_ext(settings.tmpdir, NULL, filename);
            m->tmpname = strdup_local(filename);
        }
        else
            strcpy(filename, m->tmpname);
    }

    LOG(llevDebug, "Saving map %s to %s\n", m->path, filename);

    m->in_memory = MAP_SAVING;

    if (!(fp = fopen(filename, "w")))
    {
        LOG(llevError, "ERROR: Can't open file %s for saving.\n", filename);
        return -1;
    }

    /* legacy */
    fprintf(fp, "arch map\n");
    if (m->name)
        fprintf(fp, "name %s\n", m->name);
    if (!flag)
        fprintf(fp, "swap_time %d\n", m->swap_time);
    if (m->reset_timeout)
        fprintf(fp, "reset_timeout %d\n", m->reset_timeout);
    if (MAP_FIXED_RESETTIME(m))
        fprintf(fp, "fixed_resettime %d\n", MAP_FIXED_RESETTIME(m) ? 1 : 0);
    /* we unfortunately have no idea if this is a value the creator set
     * or a difficulty value we generated when the map was first loaded
     */
    if (m->difficulty)
        fprintf(fp, "difficulty %d\n", m->difficulty);
    fprintf(fp, "darkness %d\n", m->darkness);
    fprintf(fp, "light %d\n", m->light_value);
    fprintf(fp, "map_tag %d\n", m->map_tag);
    if (m->width)
        fprintf(fp, "width %d\n", m->width);
    if (m->height)
        fprintf(fp, "height %d\n", m->height);
    if (m->enter_x)
        fprintf(fp, "enter_x %d\n", m->enter_x);
    if (m->enter_y)
        fprintf(fp, "enter_y %d\n", m->enter_y);
    if (m->msg)
        fprintf(fp, "msg\n%sendmsg\n", m->msg);

    if (MAP_UNIQUE(m))
        fputs("unique 1\n", fp);
    if (MAP_MULTI(m))
        fputs("multi 1\n", fp);
    if (MAP_INSTANCE(m))
        fputs("instance 1\n", fp);
    if (MAP_UNIQUE(m) || MAP_INSTANCE(m))
    {
        if(m->reference)
            fprintf(fp, "reference %s\n", m->reference);
        else
            LOG(llevBug, "BUG save_map(): instance/unique map with NULL reference!\n");
    }
    if (MAP_OUTDOORS(m))
        fputs("outdoor 1\n", fp);
    if (MAP_NOSAVE(m))
        fputs("no_save 1\n", fp);
    if (MAP_NOMAGIC(m))
        fputs("no_magic 1\n", fp);
    if (MAP_NOPRIEST(m))
        fputs("no_priest 1\n", fp);
    if (MAP_NOHARM(m))
        fputs("no_harm 1\n", fp);
    if (MAP_NOSUMMON(m))
        fputs("no_summon 1\n", fp);
    if (MAP_FIXEDLOGIN(m))
        fputs("fixed_login 1\n", fp);
    if (MAP_PERMDEATH(m))
        fputs("perm_death 1\n", fp);
    if (MAP_ULTRADEATH(m))
        fputs("ultra_death 1\n", fp);
    if (MAP_ULTIMATEDEATH(m))
        fputs("ultimate_death 1\n", fp);
    if (MAP_PVP(m))
        fputs("pvp 1\n", fp);

    /* save original path */
    fprintf(fp, "orig_path %s\n", m->orig_path);

    /* Save any tiling information */
    for (i = 0; i < TILED_MAPS; i++)
    {
        if (m->tile_path[i])
            fprintf(fp, "tile_path_%d %s\n", i + 1, m->tile_path[i]);
        if (m->orig_tile_path[i])
            fprintf(fp, "orig_tile_path_%d %s\n", i + 1, m->orig_tile_path[i]);
    }

    /* Save any tileset information */
    if (m->tileset_id > 0)
    {
        fprintf(fp, "tileset_id %d\n", m->tileset_id);
        fprintf(fp, "tileset_x %d\n", m->tileset_x);
        fprintf(fp, "tileset_y %d\n", m->tileset_y);
    }

    fprintf(fp, "end\n");

    save_objects(m, fp, 0);

    fclose(fp);
    chmod(filename, SAVE_MODE);
    return 0;
}


/*
 * Remove and free all objects in the given map.
 */

static void free_all_objects(mapstruct *m)
{
    int     i, j;
    int     yl=MAP_HEIGHT(m), xl=MAP_WIDTH(m);
    object *op;

    /*LOG(llevDebug,"FAO-start: map:%s ->%d\n", m->name?m->name:(m->tmpname?m->tmpname:""),m->in_memory);*/
    for (i = 0; i < xl; i++)
        for (j = 0; j < yl; j++)
        {
            object *previous_obj    = NULL;
            while ((op = GET_MAP_OB(m, i, j)) != NULL)
            {
                if (op == previous_obj)
                {
                    LOG(llevDebug, "free_all_objects: Link error, bailing out.\n");
                    break;
                }
                previous_obj = op;
                if (op->head != NULL)
                    op = op->head;

                /* this is important - we can't be sure after we removed
                 * all objects from the map, that the map structure will still
                 * stay in the memory. If not, the object GC will try - and obj->map
                 * will point to a free map struct... (/resetmap for example)
                 */
                activelist_remove(op);
                remove_ob(op); /* technical remove - no check off */
            }
        }
    /*LOG(llevDebug,"FAO-end: map:%s ->%d\n", m->name?m->name:(m->tmpname?m->tmpname:""),m->in_memory);*/
}

/*
 * Frees everything allocated by the given mapstructure.
 * don't free tmpname - our caller is left to do that
 */

void free_map(mapstruct *m, int flag)
{
    int i;

    if (!m->in_memory)
    {
        LOG(llevBug, "BUG: Trying to free freed map.\n");
        return;
    }

    /* if we don't do this, we leave the light mask part
     * on a possible tiled map and when we reload, the area
     * will be set with wrong light values.
     */
    remove_light_source_list(m);

    /* remove linked spawn points (small list of objectlink *) */
    remove_linked_spawn_list(m);

    /* I put this before free_all_objects() -
     * because the link flag is now tested in destroy_object()
     * to have a clean handling of temporary/dynamic buttons on
     * a map. Because we delete now the FLAG_LINKED from the object
     * in free_objectlinkpt() we don't trigger it inside destroy_object()
     */
    if (m->buttons)
    {
        free_objectlinkpt(m->buttons);
        m->buttons = NULL;
    }

    if (flag && m->spaces)
        free_all_objects(m);

    /* Active list sanity check.
     * At this point, free_all_objects() should have removed every
     * active object from this map. If not, we will run in a bug here!
     * This map struct CAN be freed now - an object still on the list
     * will have a map ptr on this - the GC or some else server function
     * will try to access the freed map struct when it comes to handle the
     * still as "iam active on a map" marked object and crash!
     */
    /* Actually, this is a bug _only_ if FLAG_REMOVED is false for the
     * object. There are many parts in the code that just call remove_ob()
     * on objects and expect the gc to handle removal from activelists etc.
     * - Gecko 20050731
     */
    if(m->active_objects->active_next)
    {
        LOG(llevDebug, "ACTIVEWARNING - free_map(): freed map has still active objects!\n");
        while(m->active_objects->active_next)
        {
            if(!QUERY_FLAG(m->active_objects, FLAG_REMOVED))
                LOG(llevBug, "ACTIVEBUG - FREE_MAP(): freed map (%s) has active non-removed object %s (%d)!\n", STRING_MAP_PATH(m), STRING_OBJ_NAME(m->active_objects->active_next), m->active_objects->active_next->count);
            activelist_remove(m->active_objects->active_next);
        }
    }

    FREE_AND_NULL_PTR(m->spaces);
    FREE_AND_NULL_PTR(m->bitmap);

    for (i = 0; i < TILED_MAPS; i++)
    {
        /* delete the backlinks in other tiled maps to our map */
        if(m->tile_map[i])
        {
            if(m->tile_map[i]->tile_map[map_tiled_reverse[i]] && m->tile_map[i]->tile_map[map_tiled_reverse[i]] != m )
            {
                LOG(llevBug, "BUG: Freeing map %s linked to %s which links back to another map.\n",
                        STRING_SAFE(m->orig_path), STRING_SAFE(m->tile_map[i]->orig_path));
            }
            m->tile_map[i]->tile_map[map_tiled_reverse[i]] = NULL;
            m->tile_map[i] = NULL;
        }

        FREE_AND_CLEAR_HASH(m->tile_path[i]);
        FREE_AND_CLEAR_HASH(m->orig_tile_path[i]);
    }

    FREE_AND_CLEAR_HASH(m->name);
    FREE_AND_CLEAR_HASH(m->msg);
    FREE_AND_CLEAR_HASH(m->cached_dist_map);
    FREE_AND_CLEAR_HASH(m->reference);

    m->in_memory = MAP_SWAPPED;

    /* Note: m->path, m->orig_path and m->tmppath are freed in delete_map */
}

/*
 * function: vanish mapstruct
 * m       : pointer to mapstruct, if NULL no action
 * this deletes all the data on the map (freeing pointers)
 * and then removes this map from the global linked list of maps.
 */

void delete_map(mapstruct *m)
{
    if (!m)
        return;
    if (m->in_memory == MAP_IN_MEMORY)
    {
        /* change to MAP_SAVING, even though we are not,
         * so that remove_ob doesn't do as much work.
         */
        m->in_memory = MAP_SAVING;
        free_map(m, 1);
    }
    else
        remove_light_source_list(m);

    /* remove m from the global server map list */
    if(m->next)
        m->next->last = m->last;
    if(m->last)
        m->last->next = m->next;
    else /* if there is no last, we are first map */
         first_map = m->next;

    /* Remove the list sentinel */
    remove_ob(m->active_objects);

    /* Free our pathnames (we'd like to use it above)*/
    FREE_AND_CLEAR_HASH(m->path);
    FREE_AND_CLEAR_HASH(m->orig_path);
    FREE_AND_NULL_PTR(m->tmpname); /* malloc() string */

    m->tag = 0; /* Kill any weak references to this map */
    return_poolchunk(m, pool_map);
}

void clean_tmp_map(mapstruct *m)
{
    if (m->tmpname == NULL)
        return;
    unlink(m->tmpname);
}

void free_all_maps()
{
    int real_maps   = 0;

    while (first_map)
    {
        /* I think some of the callers above before it gets here set this to be
         * saving, but we still want to free this data
         */
        if (first_map->in_memory == MAP_SAVING)
            first_map->in_memory = MAP_IN_MEMORY;
        delete_map(first_map);
        real_maps++;
    }
    LOG(llevDebug, "free_all_maps: Freed %d maps\n", real_maps);
}


/* helper function to create from a normal map a unique (apartment) like map inside the player directory
 * changed it to return hash strings because we always use the generated string as hash strings.
 */
const char *create_unique_path_sh(const object * const op, const char * const name)
{
     char path[1024];

     sprintf(path, "%s/%s/%s/%s/%s", settings.localdir, settings.playerdir, get_subdir(op->name), op->name, path_to_name(name));

     return add_string(path);
}

/* same as above but more complex: helper function to create from a normal map a instanced map.
 * ONLY and really only a player can create an instance.
 * To avoid a reenter, set the player->instance_num to MAP_INSTANCE_NUM_INVALID before call.
 * This function does 2 important things:
 * 1.) creating a valid instance file path out of name
 * 2.) ensure that the (temporary) DIRECTORY of this instance exits as long as the
 * server deals with an instance to it
 */
const char *create_instance_path_sh(player * const pl, const char * const name, int flags)
{
    int instance_num = pl->instance_num;
    const char *mapname, *path_sh = NULL;
    char path[1024];

    mapname = path_to_name(name); /* create the instance map name with '$' instead of '/' */

    /* REENTER PART: we have valid instance data ... remember: the important one is the directory, not the map */
    if (instance_num != MAP_INSTANCE_NUM_INVALID)
    {
        /* just a very last sanity check... never EVER use a illegal ID */
        if(pl->instance_id != global_instance_id || flags & INSTANCE_FLAG_NO_REENTER)
            instance_num = MAP_INSTANCE_NUM_INVALID;
        else
        {
            sprintf(path, "%s/%s/%ld/%d/%d/%s", settings.localdir, settings.instancedir, pl->instance_id,
                instance_num/10000, instance_num, mapname);
        }
    }

    if (instance_num == MAP_INSTANCE_NUM_INVALID)
    {
        instance_num = get_new_instance_num();
        /* create new instance directory for this instance */
        sprintf(path, "%s/%s/%ld/%d/%d/%s", settings.localdir, settings.instancedir, global_instance_id,
            instance_num/10000, instance_num, mapname);

        /* store the instance information for the player */
        pl->instance_flags = flags;
        pl->instance_id = global_instance_id;
        pl->instance_num = instance_num;
        FREE_AND_COPY_HASH(pl->instance_name, name); /* important: our instance NAME is the real original path name */
    }

    FREE_AND_COPY_HASH(path_sh, path);
    /* thats the most important part... to garantie we have the directory!
     * if its not there, we will run in bad problems when we try to save or load the instance!
     */
    make_path_to_file (path); /* use mkdir to pre create it physical */

    return path_sh;
}


/*
* Loads a map, which has been loaded earlier, from file.
* Return the map object we load into (this can change from the passed
* option if we can't find the original map)
* note: load_map() is called with (NULL, <src_name>, MAP_STATUS_MULTI, NULL) when
* tmp map loading fails because a tmp map is ALWAYS a MULTI map and when fails its
* reloaded from /maps as new original map.
*/
static mapstruct * load_temporary_map(mapstruct *m)
{
    FILE   *fp;
    char    buf[MAX_BUF];

    if (!m->tmpname)
    {
        LOG(llevBug, "BUG: No temporary filename for map %s! fallback to original!\n", m->path);
        strcpy(buf, m->path);
        delete_map(m);
        m = load_map(NULL, buf, MAP_STATUS_MULTI, NULL);
        if (m == NULL)
            return NULL;
        return m;
    }

    LOG(llevDebug, "load_temporary_map: %s (%s) ", m->tmpname, m->path);
    if ((fp = fopen(m->tmpname,"r")) == NULL)
    {
        LOG(llevBug, "BUG: Can't open temporary map %s! fallback to original!\n", m->tmpname);
        /*perror("Can't read map file");*/
        strcpy(buf, m->path);
        delete_map(m);
        m = load_map(NULL, buf, MAP_STATUS_MULTI, NULL);
        if (m == NULL)
            return NULL;
        return m;
    }


    LOG(llevDebug, "header: ");
    if (load_map_header(fp, m, MAP_STATUS_MULTI)) /* /tmp map = always normal multi maps */
    {
        LOG(llevBug, "BUG: Error loading map header for %s (%s)! fallback to original!\n", m->path, m->tmpname);
        fclose(fp);
        strcpy(buf, m->path);
        delete_map(m);
        m = load_map(NULL, buf, MAP_STATUS_MULTI, NULL);
        if (m == NULL)
            return NULL;
        return m;
    }

    LOG(llevDebug, "alloc. ");
    allocate_map(m);

    m->in_memory = MAP_LOADING;
    LOG(llevDebug, "load objs:");
    load_objects(m, fp, 0);
    LOG(llevDebug, "close. ");
    fclose(fp);
    LOG(llevDebug, "done!\n");
    return m;
}

/** Ready a map of the same type as another map, even for the
 * same instance if applicable.
 *
 * @param orig_map map to inherit type and instance from
 * @param new_map_path the path to the map to ready. This can be either an absolute path,
 *        or a path relative to orig_map->path. It must be a true "source" map path,
 *        and not a path into the "./instance" or "./players" directory.
 * @param flags        1 to never load unloaded or swapped map, i.e. only return maps
 *                     already in memory.
 * @return pointer to loaded map, or NULL
 */
mapstruct *ready_inherited_map(mapstruct *orig_map, shstr *new_map_path, int flags)
{
    mapstruct *new_map = NULL;
    shstr *new_path = NULL;
    shstr *normalized_path = NULL;
    char tmp_path[MAXPATHLEN];

    /* Try some quick exits first */
    if(orig_map == NULL || new_map_path == NULL || *new_map_path == '\0')
        return NULL;
    if(new_map_path == orig_map->path && (orig_map->in_memory == MAP_LOADING || orig_map->in_memory == MAP_IN_MEMORY))
        return orig_map;

    if(! MAP_STATUS_TYPE(orig_map->map_status))
    {
        LOG(llevBug, "ready_inherited_map(): map %s without status type\n", STRING_MAP_ORIG_PATH(orig_map));
        return NULL;
    }

    /* Guesstimate whether the path was already normalized or not (for speed) */
    if(*new_map_path == '/')
        normalized_path = add_refcount(new_map_path);
    else
        normalized_path = add_string(normalize_path(orig_map->path, new_map_path, tmp_path));

    /* create the path prefix (./players/.. or ./instance/.. ) for non multi maps */
    if(orig_map->map_status & (MAP_STATUS_UNIQUE|MAP_STATUS_INSTANCE))
    {
        new_path = add_string(normalize_path_direct(orig_map->path,
                    path_to_name(normalized_path), tmp_path));
    }

    if(flags & 1)
    {
        /* Just check if it has in memory */
        new_map = has_been_loaded_sh( new_path ? new_path : normalized_path );
        if (new_map && (new_map->in_memory != MAP_LOADING && new_map->in_memory != MAP_IN_MEMORY))
            new_map = NULL;
    } else
    {
        /* Load map if necesseary */
        new_map = ready_map_name(new_path?new_path:normalized_path, normalized_path,
                MAP_STATUS_TYPE(orig_map->map_status), orig_map->reference);
    }

    FREE_ONLY_HASH(normalized_path);
    if(new_path)
        FREE_ONLY_HASH(new_path);

    return new_map;
}

/* ready_map_name() will return a map pointer to the map name_path/src_path.
 * If the map was not loaded before, the map will be loaded now.
 * src_path is ALWAYS a path to /maps = the original map path.
 * name_path can be different and pointing to /instance or /players
 * reference needs to be a player name for UNIQUE or MULTI maps
 *
 * If src_path is NULL we will not load a map from disk, but return NULL
 * if the map wasn't in memory already.
 * If name_path is NULL we will force a reload of the map even if it already
 * was in memory. (caller has to reset the map!)
 */
mapstruct * ready_map_name(const char *name_path, const char *src_path, int flags, shstr *reference)
{
     mapstruct  *m;

    /* Map is good to go? so just return it */
     m = has_been_loaded_sh( name_path ? name_path : src_path );
     if (m && (m->in_memory == MAP_LOADING || m->in_memory == MAP_IN_MEMORY))
     return m;

     /* unique maps always get loaded from their original location, and never from a temp location. */
     if (!m || (flags & (MAP_STATUS_UNIQUE|MAP_STATUS_INSTANCE)) )
     {
        /* tricky check - if we have '/' starting part, its a multi map we have here.
         * if called without src_path, we only check it in memory OR in tmp.
         * if we are here its not there or its not an multi map.
         */
        if(!src_path && *name_path == '/')
            return  NULL;

        if (m)
        {
            /* after the instance map patch we should look in the map swap system...
             * it seems unique maps are not 100% right implemented there
             */
            LOG(llevDebug, "NOTE: ready_map_name(): unique/instanced map as tmp map? (stats:%d) (%s) (%s)",
                    m->in_memory, STRING_SAFE(name_path), STRING_SAFE(src_path) );
            clean_tmp_map(m);
            delete_map(m);
            m = NULL;
        }

        /* we are loading now a src map from /maps or an instance/unique from /instance or /players */
        if(!(m = load_map(name_path, src_path, MAP_STATUS_TYPE(flags), reference)))
            return NULL;
     }
     else
     {
         /* If in this loop, we found a temporary map, so load it up. */
         m = load_temporary_map(m);
         if (m == NULL)
             return NULL;

         LOG(llevDebug, "clean. ");
         clean_tmp_map(m);
         m->in_memory = MAP_IN_MEMORY;
     }

     /* Below here is stuff common to both first time loaded maps and
      * temp maps.
      */

     /* In case other objects press some buttons down.
      * We handle here all kind of "triggers" which are triggered
      * permanent by objects like buttons or inventory checkers.
      * We don't check here instant stuff like sacrificing altars.
      * Because this should be handled on map making side.
      */
     LOG(llevDebug, "buttons. ");
     update_buttons(m);
     LOG(llevDebug, "end ready-map_name(%s)\n", m->path ? m->path : "<nopath>");
     return m;
}



/*
* Opens the file "filename" or "src_name" and reads information about the map
* from the given file, and stores it in a newly allocated
* mapstruct.  A pointer to this structure is returned, or NULL on failure.
* flags correspond to those in map.h.  Main ones used are
* MAP_PLAYER_UNIQUE and MAP_PLAYER_INSTANCE where filename is != src_name.
* MAP_STYLE: style map - don't add active objects, don't add to server
* managed map list. The function knows it loads a "real" original map from /maps
* or and unique/instance by comparing filename and src_name.
* reference needs to be a player name for UNIQUE or MULTI maps
*/
mapstruct * load_map(const char *filename, const char *src_name, int flags, shstr *reference)
{
    FILE       *fp;
    mapstruct  *m;
    char       pathname[MAX_BUF];

    flags &= ~MAP_STATUS_ORIGINAL;

    /* this IS a bug - because string compare will fail when it checks the loaded maps -
     * this can lead in a double load and break the server!
     * a '.' signs unique maps in fixed directories.
     * We don't fix it here anymore - this MUST be done by the calling functions or our
     * inheritanced map system is already broken somewhere before this call.
     */
    if ((filename && *filename != '/' && *filename != '.') || (src_name && *src_name != '/' && *src_name != '.'))
    {
        LOG(llevDebug, "DEBUG: load_map: filename without start '/' or '.' (%s) (%s)\n", STRING_SAFE(filename), STRING_SAFE(src_name));
        return NULL;
    }

    /* Here is our only "file path analyzing" trick. Our map model itself don't need it, but
     * it allows us to call this function with the DM commands like "/goto ./players/a/aa/Aa/$demo"
     * without pre-guessing the map_status. In fact map_status CAN be here invalid with 0!
     * IF map_status is zero here, load_map() will set it dynamic!
     * Checkup load_map() & load_map_header() how it works.
     */
    if(filename)
    {
        if (*filename == '.') /* pathes to /instance and /players always start with a '.'! */
        {
            LOG(llevDebug, "load_map: %s (%x)\n", filename, flags);
            strcpy(pathname, filename);
        }
        else /* we have an normalized map here and the map start ALWAYS with a '/' */
        {
            LOG(llevDebug, "load_map (orig): %s (%x) ", filename, flags);
            strcpy(pathname, create_mapdir_pathname(filename)); /* we add the (...)/maps prefix path part */

            if(filename == src_name)
                flags |= MAP_STATUS_ORIGINAL;
        }
    }

    if (!filename || (fp = fopen(pathname, "r")) == NULL)
    {
        /* this was usually a try to load a unique or instance map
         * This is RIGHT because we use fopen() here as an implicit access()
         * check. If it fails, we know we have to load the map from /maps!
         */
        if(src_name && filename != src_name && *src_name == '/')
        {
            LOG(llevDebug, "load_map: original %s (%x) ", src_name, flags);
            strcpy(pathname, create_mapdir_pathname(src_name)); /* we add the (...)/maps prefix path part */
            flags |= MAP_STATUS_ORIGINAL;

            if ((fp = fopen(pathname, "r")) == NULL)
            {
                /* ok... NOW we are screwed with an invalid map... because it is not in /maps */
                LOG(llevBug, "Debug: Can't open map file %s (%s)\n", STRING_SAFE(filename), STRING_SAFE(src_name));
                return (NULL);
            }
        }
        else
        {
            LOG(llevBug, "Debug: Can't open map file %s (%s)\n", STRING_SAFE(filename), STRING_SAFE(src_name));
            return (NULL);
        }
    }

    LOG(llevDebug, "link map. ");
    m = get_linked_map();

    LOG(llevDebug, "header: ");

    if(filename)
    {
        FREE_AND_COPY_HASH(m->path, filename);
    }
    else
    {
        FREE_AND_COPY_HASH(m->path, src_name);
    }
    if(src_name) /* invalid src_name can happens when we force an explicit load of an unique map! */
        FREE_AND_COPY_HASH(m->orig_path, src_name); /* orig_path will be loaded in load_map_header()! */

    m->map_tag = ++global_map_tag;    /* every map has an unique tag */
    if (load_map_header(fp, m, flags))
    {
        LOG(llevBug, "BUG: Failure loading map header for %s, flags=%d\n",
                flags&MAP_STATUS_ORIGINAL?m->orig_path:m->path, flags);
        delete_map(m);
        fclose(fp);
        return NULL;
    }

    /* Set up the reference string from function call if not stored with map */
    if((MAP_UNIQUE(m) || MAP_INSTANCE(m)) && m->reference == NULL)
    {
        if(reference)
        {
            FREE_AND_ADD_REF_HASH(m->reference, reference);
        } else
        {
            LOG(llevBug, "BUG: load_map() unique/instance map with NULL reference parameter\n");
        }
    }

    LOG(llevDebug, "alloc. ");
    allocate_map(m);

    m->in_memory = MAP_LOADING;

    LOG(llevDebug, "load objs:");
    load_objects(m, fp, flags & (MAP_STATUS_STYLE|MAP_STATUS_ORIGINAL));
    LOG(llevDebug, "close. ");
    fclose(fp);
    LOG(llevDebug, "post set. ");
    if (!MAP_DIFFICULTY(m))
    {
        /*LOG(llevBug, "BUG: Map %s has difficulty 0. Changing to 1 (non special item area).\n", filename);*/
        MAP_DIFFICULTY(m) = 1;
    }
    set_map_reset_time(m);
    LOG(llevDebug, "done!\n");
    return (m);
}


/* path_to_name() takes a path and replaces all / with '$'
* We do a strcpy so that we do not change the original string.
*/
const char *path_to_name(const char *file)
{
    static char newpath[MAXPATHLEN], *cp;

    strncpy(newpath, file, MAXPATHLEN - 1);
    newpath[MAXPATHLEN - 1] = '\0';
    for (cp = newpath; *cp != '\0'; cp++)
    {
        if (*cp == '/')
            *cp = '$';
    }
    return newpath;
}

/* initialize the player struct map & bind pathes */
void set_mappath_by_default(player *pl)
{
    FREE_AND_ADD_REF_HASH(pl->maplevel, shstr_cons.start_mappath);
    FREE_AND_ADD_REF_HASH(pl->orig_map, shstr_cons.start_mappath);
    pl->map_status = START_MAP_STATUS;
    pl->map_x = START_MAP_X;
    pl->map_y = START_MAP_Y;
}

void set_mappath_by_map(object* op)
{
    player *pl = CONTR(op);

    if(!pl)
        return;

    FREE_AND_ADD_REF_HASH(pl->maplevel, op->map->path);
    FREE_AND_ADD_REF_HASH(pl->orig_map, op->map->orig_path);
    pl->map_status = op->map->map_status;
    pl->map_x = op->x;
    pl->map_y = op->y;
}

void set_mappath_by_name(player *pl, const char *dst, const char *src, int status, int x, int y)
{
    if(!dst)
        dst = src;

    FREE_AND_ADD_REF_HASH(pl->maplevel, dst);
    FREE_AND_ADD_REF_HASH(pl->orig_map, src);
    pl->map_status = status;
    pl->map_x = x;
    pl->map_y = y;
}

void set_bindpath_by_default(player *pl)
{
    FREE_AND_ADD_REF_HASH(pl->savebed_map, shstr_cons.bind_mappath);
    FREE_AND_ADD_REF_HASH(pl->orig_savebed_map, shstr_cons.bind_mappath);
    pl->bed_status = BIND_MAP_STATUS;
    pl->bed_x = BIND_MAP_X;
    pl->bed_y = BIND_MAP_Y;
}

void set_bindpath_by_name(player *pl, const char *dst, const char *src, int status, int x, int y)
{
    if(!dst)
        dst = src;

    FREE_AND_ADD_REF_HASH(pl->savebed_map, dst);
    FREE_AND_ADD_REF_HASH(pl->orig_savebed_map, src);
    pl->bed_status = status;
    pl->bed_x = x;
    pl->bed_y = y;
}

/* helper func for load_objects()
* This help function will loop through the map and set the nodes
*/
static inline void update_map_tiles(mapstruct *m)
{
    int i,j, yl=MAP_HEIGHT(m), xl=MAP_WIDTH(m);
    MapSpace *msp = GET_MAP_SPACE_PTR(m,0,0);

    for (i = 0; i < xl; i++)
    {
        for (j = 0; j < yl; j++)
        {
            msp->update_tile++;
            update_position(m, msp++, i, j);
        }
    }
}

/* now, this function is the very deep core of the whole server map &
* object handling. To understand the tiled map handling, you have to
* understand the flow of this function. It can now called recursive
* and it will call themself recursive if the insert_object_in_map()
* function below has found a multi arch reaching in a different map.
* Then the out_of_map() call in insert_object_in_map() will trigger a
* new map load and a recursive call of this function. This will work
* without any problems. Note the restore_light_source_list() call at
* the end of the list. Adding overlapping light sources for tiled map
* in this recursive structure was the hard part but it will work now
* without problems. MT-25.02.2004
*/
void load_objects(mapstruct *m, FILE *fp, int mapflags)
{
    int         i;
    archetype  *tail;
    void       *mybuffer;
    object     *op, *prev = NULL, *last_more = NULL, *tmp;

    op = get_object();
    op->map = m; /* To handle buttons correctly */
    m->map_flags |= MAP_FLAG_NO_UPDATE; /* be sure to avoid tile updating in the loop below */

    mybuffer = create_loader_buffer(fp);
    while ((i = load_object(fp, op, mybuffer, LO_REPEAT, mapflags)))
    {
        /* atm, we don't need and handle multi arches saved with tails! */
        if (i == LL_MORE)
        {
            LOG(llevDebug, "BUG: load_objects(%s): object %s - its a tail!.\n", m->path ? m->path : ">no map<",
                query_short_name(op, NULL));
            goto next;
        }
        /* should not happen because we catch invalid arches now as singularities */
        if (op->arch == NULL)
        {
            LOG(llevDebug, "BUG:load_objects(%s): object %s (%d)- invalid archetype. (pos:%d,%d)\n",
                m->path ? m->path : ">no map<", query_short_name(op, NULL), op->type, op->x, op->y);
            goto next;
        }
        else if (op->type == FLOOR)
        {
            /* be sure that floor is a.) always single arch and b.) always use "in map" offsets (no multi arch tricks) */
            MapSpace *msp = GET_MAP_SPACE_PTR(m,op->x,op->y);

            msp->floor_terrain = op->terrain_type;
            msp->floor_light = op->last_sp;
#ifdef USE_TILESTRETCHER
            msp->floor_z = op->z;
#endif

            if(QUERY_FLAG(op,FLAG_NO_PASS))
                msp->floor_flags |= MAP_FLOOR_FLAG_NO_PASS;
            if(QUERY_FLAG(op,FLAG_PLAYER_ONLY))
                msp->floor_flags |= MAP_FLOOR_FLAG_PLAYER_ONLY;

            /* we don't animate floors at the moment. But perhaps turnable for adjustable pictures */
            if (QUERY_FLAG(op, FLAG_IS_TURNABLE) || QUERY_FLAG(op, FLAG_ANIMATE))
            {
                if(NUM_FACINGS(op) == 0)
                {
                    LOG(llevDebug, "BUG:load_objects(%s): object %s (%d)- NUM_FACINGS == 0. Bad animation? (pos:%d,%d)\n",
                        m->path ? m->path : ">no map<", query_short_name(op, NULL), op->type, op->x, op->y);
                    goto next;
                }
                SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + op->state);
            }
            /* we save floor masks direct over a generic mask arch/object and don't need to store the direction.
            * a mask will not turn ingame - thats just for the editor and to have one arch
            */
            msp->floor_face = op->face;

            goto next;
        }
        else if (op->type == TYPE_FLOORMASK)
        {
            /* we have never animated floor masks, but perhaps turnable for adjustable pictures */
            if (QUERY_FLAG(op, FLAG_IS_TURNABLE) || QUERY_FLAG(op, FLAG_ANIMATE))
            {
                if(NUM_FACINGS(op) == 0)
                {
                    LOG(llevDebug, "BUG:load_objects(%s): object %s (%d)- NUM_FACINGS == 0. Bad animation? (pos:%d,%d)\n",
                        m->path ? m->path : ">no map<", query_short_name(op, NULL), op->type, op->x, op->y);
                    goto next;
                }
                SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + op->state);
            }
            /* we save floor masks direct over a generic mask arch/object and don't need to store the direction.
            * a mask will not turn ingame - thats just for the editor and to have one arch
            */
            SET_MAP_FACE_MASK(m,op->x,op->y,op->face);
            goto next;
        }
        else if (op->type == CONTAINER) /* do some safety for containers */
        {
            op->attacked_by = NULL; /* used for containers as link to players viewing it */
            op->attacked_by_count = 0;
        }
        else if(op->type == SPAWN_POINT && op->slaying)
        {
            add_linked_spawn(op);
        }

        /* important pre set for the animation/face of a object */
        if (QUERY_FLAG(op, FLAG_IS_TURNABLE) || QUERY_FLAG(op, FLAG_ANIMATE))
        {
            /* If a bad animation is set, we will get div by zero */
            if(NUM_FACINGS(op) == 0)
            {
                LOG(llevDebug, "\n**BUG:load_objects(%s): object %s (%d)- NUM_FACINGS == 0. Bad animation? (pos:%d,%d)\n",
                    m->path ? m->path : ">no map<", query_short_name(op, NULL), op->type, op->x, op->y);
                goto next;
            }
			else if( op->direction < 0 || op->direction >= NUM_FACINGS(op))
			{
				LOG(llevDebug, "\n**BUG:load_objects(%s): object %s (%d)- NUM_FACINGS < op->direction(%d) + op->state(%d) - (pos:%d,%d)\n",
					m->path ? m->path : ">no map<", query_short_name(op, NULL), op->type, op->direction, op->state, op->x, op->y);
				/* its an invalid animation offset (can trigger wrong memory access)
				 * we try to repair it. This is a wrong setting in the arch file or,
				 * more common, a map maker bug.
				 */
				op->direction = NUM_FACINGS(op)-1;
			}

            SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + op->state);
        }

        if(op->inv && !QUERY_FLAG(op,FLAG_SYS_OBJECT) )
            op->carrying = sum_weight(op);              /* ensure right weight for inventories */
        else
            op->carrying = 0; /* sanity setting... this means too - NO double use of ->carrying! */

        /* expand a multi arch - we have only the head saved in a map!
        * the *real* fancy point is, that our head/tail don't must fit
        * in this map! insert_ob will take care about it and loading the needed
        * map - then this function and the map loader is called recursive!
        */
        if (op->arch->more) /* we have a multi arch head? */
        {
            /* an important note: we have sometimes the head of a multi arch
            * object in the inventory of objects - for example mobs
            * which changed type in spawn points and in the mob itself
            * as TYPE_BASE_INFO. As long as this arches are not on the map,
            * we will not come in trouble here because load_object() will them
            * load on the fly. That means too, that multi arches in inventories
            * are always NOT expanded - means no tail.
            */
            tail = op->arch->more;
            prev = op,last_more = op;

            /* then clone the tail using the default arch */
            do
            {
                tmp = get_object();
                copy_object(&tail->clone, tmp);

                tmp->x += op->x;
                tmp->y += op->y;
                tmp->map = op->map;

                /* adjust the single object specific data except flags. */
                tmp->type = op->type;
                tmp->layer = op->layer;

                /* link the tail object... */
                tmp->head = prev,last_more->more = tmp,last_more = tmp;
            }
            while ((tail = tail->more));

            /* now some tricky stuff again:
            * to speed up some core functions like moving or remove_ob()/insert_ob
            * and because there are some "arch depending and not object depending"
            * flags, we init the tails with some of the head settings.
            * NOTE / TODO : is it not possible and easier to copy simply the WHOLE
            * flag block from head to tail? in theorie, the tail can safely have
            * all the flags too... Perhaps we must change one or two code parts
            * for it but it should work MT-10.2005
            */
            if (QUERY_FLAG(op, FLAG_SYS_OBJECT))
                SET_MULTI_FLAG(op->more, FLAG_SYS_OBJECT)
            else
            CLEAR_MULTI_FLAG(tmp->more, FLAG_SYS_OBJECT);
            if (QUERY_FLAG(op, FLAG_NO_APPLY))
                SET_MULTI_FLAG(op->more, FLAG_NO_APPLY)
            else
            CLEAR_MULTI_FLAG(tmp->more, FLAG_NO_APPLY);
            if (QUERY_FLAG(op, FLAG_IS_INVISIBLE))
                SET_MULTI_FLAG(op->more, FLAG_IS_INVISIBLE)
            else
            CLEAR_MULTI_FLAG(tmp->more, FLAG_IS_INVISIBLE);
            if (QUERY_FLAG(op, FLAG_IS_ETHEREAL))
                SET_MULTI_FLAG(op->more, FLAG_IS_ETHEREAL)
            else
            CLEAR_MULTI_FLAG(tmp->more, FLAG_IS_ETHEREAL);
            if (QUERY_FLAG(op, FLAG_CAN_PASS_THRU))
                SET_MULTI_FLAG(op->more, FLAG_CAN_PASS_THRU)
            else
            CLEAR_MULTI_FLAG(tmp->more, FLAG_CAN_PASS_THRU);
            if (QUERY_FLAG(op, FLAG_FLYING))
                SET_MULTI_FLAG(op->more, FLAG_FLYING)
            else
            CLEAR_MULTI_FLAG(tmp->more, FLAG_FLYING);
            if (QUERY_FLAG(op, FLAG_LEVITATE))
                SET_MULTI_FLAG(op->more, FLAG_LEVITATE)
            else
            CLEAR_MULTI_FLAG(tmp->more, FLAG_LEVITATE);

            if (QUERY_FLAG(op, FLAG_BLOCKSVIEW))
                SET_MULTI_FLAG(op->more, FLAG_BLOCKSVIEW)
            else
            CLEAR_MULTI_FLAG(tmp->more, FLAG_BLOCKSVIEW);
        }

        insert_ob_in_map(op, m, op, INS_NO_MERGE | INS_NO_WALK_ON);

        if (op->glow_radius)
            adjust_light_source(op->map, op->x, op->y, op->glow_radius);

        /* this is from fix_auto_apply() which is removed now */
        if (QUERY_FLAG(op, FLAG_AUTO_APPLY))
            auto_apply(op); /* auto_apply() will remove the flag_auto_apply after first use */
        else if ((mapflags & MAP_STATUS_ORIGINAL) && op->randomitems) /* for fresh maps, create treasures */
        {
            if (op->type == MONSTER)
                create_treasure_list( op->randomitems, op, op->type != TREASURE ? GT_APPLY : 0,
                op->level ? op->level : m->difficulty, ART_CHANCE_UNSET, 0);
            else
                create_treasure_list( op->randomitems, op, op->type != TREASURE ? GT_APPLY : 0,
                op->level ? op->level : m->difficulty, ART_CHANCE_UNSET, 0);
        }

        if (op->type == MONSTER)
            fix_monster(op);

next:
        /* We always need a fresh object for the next iteration */
        op = get_object();
        op->map = m;
    }

    delete_loader_buffer(mybuffer);

    /* this MUST be set here or check_light_source_list()
    * will fail. If we set this to early, the recursive called map
    * will add a light source to the caller and then the caller itself
    * here again...
    */
    update_map_tiles(m);
    m->map_flags &= ~MAP_FLAG_NO_UPDATE; /* turn tile updating on again */
    m->in_memory = MAP_IN_MEMORY;

    /* this is the only place we can insert this because the
    * recursive nature of load_objects().
    */
    check_light_source_list(m);
}

/* This saves all the objects on the map in a (most times) non destructive fashion.
* Except spawn point/mobs and multi arches - see below.
* Modified by MSW 2001-07-01 to do in a single pass - reduces code,
* and we only save the head of multi part objects - this is needed
* in order to do map tiling properly.
* The function/engine is now multi arch/tiled map save - put on the
* map what you like. MT-07.02.04
*/
void save_objects(mapstruct *m, FILE *fp, int flag)
{
    static object *floor_g=NULL, *fmask_g=NULL;
    int    yl=MAP_HEIGHT(m), xl=MAP_WIDTH(m);
    int     i, j = 0;
    object *head, *op, *otmp, *tmp, *last_valid;

    /* ensure we have our "template" objects for saving floors & masks */
    if(!floor_g)
    {
        floor_g = get_archetype("floor_g");
        if(!floor_g)
            LOG(llevError, "ERROR: Cant'find 'floor_g' arch\n");
        insert_ob_in_ob(floor_g, &void_container); /* Avoid gc */
        FREE_AND_CLEAR_HASH(floor_g->name);
    }
    if(!fmask_g)
    {
        fmask_g = get_archetype("fmask_g");
        insert_ob_in_ob(fmask_g, &void_container); /* Avoid gc */
        if(!fmask_g)
            LOG(llevError, "ERROR: Cant'find 'fmask_g' arch\n");
    }

    /* first, we have to remove all dynamic objects from this map.
    * from spell effects with owners (because the owner can't
    * be restored after a save) or from spawn points generated mobs.
    * We need to remove them in a right way - perhaps our spawn mob
    * was sitting on a button and we need to save then the unpressed
    * button - when not removed right our map get messed up when reloaded.
    *
    * We need to be a bit careful here.
    *
    * We will give the move_apply() code (which handles object changes when
    * something is removed) the MOVE_APPLY_VANISHED flag - we MUST
    * take care in all called function about it.
    *
    * a example: A button which is pressed will call a spawn point "remove object x"
    * and unpressed "spawn onject x".
    * This is ok in the way, the button only set a flag/value in the spawn point
    * so in the next game tick the spawn point can do action. Because we will save
    * now, that action will be called when the map is reloaded. All ok.
    * NOT ok is, that the button then (or any other from move apply called object)
    * does a action IMMIDIALTY except it is a static effect (like we put a wall
    * in somewhere).
    * Absolut forbidden are dynamic effect like instant spawns of mobs on other maps
    * point - then our map will get messed up again. Teleporters are a bit critical here
    * and i fear the code and callings in move_apply() will need some more carefully
    * examination.
    */
    for (i = 0; i < xl; i++)
    {
        for (j = 0; j < yl; j++)
        {
            for (op = get_map_ob(m, i, j); op; op = otmp)
            {
                otmp = op->above;
                last_valid = op->below; /* thats NULL OR a valid ptr - it CAN'T be a non valid
                                        * or we had remove it before AND reseted the ptr then right.
                                        */
                if (op->type == PLAYER) /* ok, we will *never* save maps with player on */
                {
                    LOG(llevDebug, "SemiBUG: Tried to save map with player on!(%s (%s))\n", query_name(op), m->path);
                    continue;
                }

                head = op->head ? op->head : op;
                /* here we remove all "dynamic" content which are around on the map.
                * ATM i remove some spell effects with it.
                * For things like permanent counterspell walls or something we should
                * use and create special objects.
                */
                if (QUERY_FLAG(head, FLAG_NO_SAVE))
                {
                    activelist_remove(head);
                    remove_ob(head);
                    check_walk_off(head, NULL, MOVE_APPLY_VANISHED | MOVE_APPLY_SAVING);

                    if (otmp && (QUERY_FLAG(otmp, FLAG_REMOVED) || OBJECT_FREE(otmp))) /* invalid next ptr! */
                    {
                        if (!QUERY_FLAG(op, FLAG_REMOVED) && !OBJECT_FREE(op))
                            otmp = op->above;
                        else if (last_valid)
                            otmp = last_valid->above;
                        else
                            otmp = get_map_ob(m, i, j); /* should be really rare */
                    }
                    continue;
                }
                /* here we handle the mobs of a spawn point - called spawn mobs.
                * We *never* save spawn mobs - not even if they are on the same map.
                * We remove them and tell the spawn point to generate them new in the next tick.
                * (In case of the saved map it is the reloading).
                * If reloaded, the spawn point will restore a new mob of same kind on
                * the default position.
                */
                else if (QUERY_FLAG(head, FLAG_SPAWN_MOB))
                {
                    /* sanity check for the mob structures & ptr */
                    if(!MOB_DATA(head) || !MOB_DATA(head)->spawn_info)
                    {
                        LOG( llevBug, "BUG: Spawn mob (%s %s) without SPAWN INFO (%s) or MOB_DATA(%x).\n",
                            STRING_SAFE(head->arch->name), query_name(head),
                            MOB_DATA(head)?query_name(MOB_DATA(head)->spawn_info):"NULL",
                            MOB_DATA(head)?MOB_DATA(head):0x00);
                    }
                    else
                    {
                        tmp = MOB_DATA(head)->spawn_info;
                        /* spawn info is ok - check the spawn point attached to it */
                        if (tmp->owner && tmp->owner->type == SPAWN_POINT)
                        {
                            /* Found spawn point. Tell the source spawn point to respawn this deleted object.
                            * It can be here OR on a different map.
                            */
                            tmp->owner->stats.sp = tmp->owner->last_sp; /* force a pre spawn setting */
                            tmp->owner->speed_left += 1.0f; /* we force a active spawn point */
                            tmp->owner->enemy = NULL;
                        }
                        else
                        {
                            LOG( llevBug, "BUG: Spawn mob (%s (%s)) has SPAWN INFO with illegal owner: (%s)!\n",
                                STRING_SAFE(head->arch->name), query_name(head), query_name(tmp->owner));
                        }
                    }

                    /* and remove the mob itself */
                    activelist_remove(head);
                    remove_ob(head);
                    check_walk_off(head, NULL, MOVE_APPLY_VANISHED | MOVE_APPLY_SAVING);
                    if (otmp && (QUERY_FLAG(otmp, FLAG_REMOVED) || OBJECT_FREE(otmp))) /* invalid next ptr! */
                    {
                        if (!QUERY_FLAG(op, FLAG_REMOVED) && !OBJECT_FREE(op))
                            otmp = op->above;
                        else if (last_valid)
                            otmp = last_valid->above;
                        else
                            otmp = get_map_ob(m, i, j); /* should be really rare */
                    }
                    continue;
                }
                else if (op->type == SPAWN_POINT)
                {
                    /* Handling of the spawn points is much easier as handling the mob.
                    * if the spawn point still control some mobs, we delete the mob  - where ever
                    * it is. Also, set pre spawn value to last mob - so we restore our creature
                    * when we reload this map.
                    */
                    if (op->enemy)
                    {
                        if (op->enemy_count == op->enemy->count &&  /* we have a legal spawn? */
                            !QUERY_FLAG(op->enemy, FLAG_REMOVED) && !OBJECT_FREE(op->enemy))
                        {
                            op->stats.sp = op->last_sp; /* force a pre spawn setting */
                            op->speed_left += 1.0f;
                            /* and delete the spawn */
                            /* note: because a spawn point always is on a map, its safe to
                            * have the activelist_remove() inside here
                            */
                            activelist_remove(op->enemy);
                            remove_ob(op->enemy);
                            check_walk_off(op->enemy, NULL, MOVE_APPLY_VANISHED | MOVE_APPLY_SAVING);
                            op->enemy = NULL;

                            if (otmp && (QUERY_FLAG(otmp, FLAG_REMOVED) || OBJECT_FREE(otmp))) /* invalid next ptr! */
                            {
                                if (!QUERY_FLAG(op, FLAG_REMOVED) && !OBJECT_FREE(op))
                                    otmp = op->above;
                                else if (last_valid)
                                    otmp = last_valid->above;
                                else
                                    otmp = get_map_ob(m, i, j); /* should be really rare */
                            }
                        }
                    }
                }

                /* we will delete here all temporary owner objects.
                * We talk here about spell effects, pets, golems and
                * other "dynamic" objects.
                * What NOT should be deleted are throw objects and other
                * permanent items which has a owner setting! (if they have)
                */
                if (head->owner)
                {
                    /* perhaps we should add here a flag for pets...
                    * But the pet code needs a rework so or so.
                    * ATM we simply delete GOLEMS and clearing
                    * from all other spells/stuff the owner tags.
                    * SPAWN MOBS are not here so we only speak about
                    * spell effects
                    * we *can* delete them here too - but then i would
                    * prefer a no_save flag. Only reason to save them is
                    * to reset for example buttons or avoiding side effects
                    * like a fireball saved with neutral owner which does then
                    * something evil - but that CAN always catched in the code
                    * and scripts so lets go the easy way here - as less we
                    * manipulate the map here as more secure we are!
                    */
                    if (head->type == GOLEM) /* a golem needs a valid release from the player... */
                    {
                        send_golem_control(head, GOLEM_CTR_RELEASE);
                        activelist_remove(head);
                        remove_ob(head);
                        check_walk_off(head, NULL, MOVE_APPLY_VANISHED | MOVE_APPLY_SAVING);

                        if (otmp && (QUERY_FLAG(otmp, FLAG_REMOVED) || OBJECT_FREE(otmp))) /* invalid next ptr! */
                        {
                            if (!QUERY_FLAG(op, FLAG_REMOVED) && !OBJECT_FREE(op))
                                otmp = op->above;
                            else if (last_valid)
                                otmp = last_valid->above;
                            else
                                otmp = get_map_ob(m, i, j); /* should be really rare */
                        }
                        continue;
                    }

                    LOG(llevDebug, "WARNING (only debug): save_obj(): obj w. owner. map:%s obj:%s (%s) (%d,%d)\n",
                        m->path, query_name(op), op->arch->name ? op->arch->name : "<no arch name>", op->x, op->y);
                    head->owner = NULL;
                    continue;
                }
            } /* for this space */
        } /* for this j */
    }


    /* The map is now cleared from non static objects on this or other maps
    * (when the source was from this map). Now all can be saved as a legal
    * snapshot of the map state mashine.
    * That means all button/mashine relations are correct.
    */

    for (i = 0; i < xl; i++)
    {
        for (j = 0; j < yl; j++)
        {
            MapSpace *mp = &m->spaces[i + m->width * j];

            /* save first the floor and mask from the node */
            if(mp->floor_face)
            {
                floor_g->terrain_type = mp->floor_terrain;
                floor_g->last_sp = mp->floor_light;
                floor_g->face = mp->floor_face;
                floor_g->x = i;
                floor_g->y = j;
#ifdef USE_TILESTRETCHER
                floor_g->z = mp->floor_z;
#endif

                if(mp->floor_flags & MAP_FLOOR_FLAG_NO_PASS )
                    SET_FLAG(floor_g, FLAG_NO_PASS);
                else
                    CLEAR_FLAG(floor_g, FLAG_NO_PASS);

                if(mp->floor_flags & MAP_FLOOR_FLAG_PLAYER_ONLY)
                    SET_FLAG(floor_g, FLAG_PLAYER_ONLY);
                else
                    CLEAR_FLAG(floor_g, FLAG_PLAYER_ONLY);

                /* black object magic... don't do this in the "normal" server code */
                save_object(fp, floor_g, 3);
            }

            if(mp->mask_face)
            {
                fmask_g->face = mp->mask_face;
                fmask_g->x = i;
                fmask_g->y = j;
#ifdef USE_TILESTRETCHER
                floor_g->z = mp->floor_z;
#endif
                save_object(fp, fmask_g, 3);
            }

            for (op = mp->first; op; op = otmp)
            {
                otmp = op->above;
                last_valid = op->below; /* thats NULL OR a valid ptr - it CAN'T be a non valid
                                        * or we had remove it before AND reseted the ptr then right.
                                        */

                /* do some testing... */
                if (op->type == PLAYER) /* ok, we will *never* save maps with player on */
                    continue; /* warning was given before */

                /* here we do the magic! */
                if (op->head) /* its a tail... */
                {
                    int xt, yt;

                    /* the magic is, that we have a tail here, but we
                    * save the head now and give it the x/y
                    * position basing on this tail position and its
                    * x/y clone arch default multi tile offsets!
                    * With this trick, we even can generate negative
                    * map positions - and thats exactly what we want
                    * when our head is on a different map as this tail!
                    * insert_ob() and the map loader will readjust map and
                    * positions and load the other map when needed!
                    * we must save x/y or remove_ob() will fail.
                    */
                    tmp = op->head;
                    xt = tmp->x;
                    yt = tmp->y;
                    tmp->x = op->x - op->arch->clone.x;
                    tmp->y = op->y - op->arch->clone.y;

                    save_object(fp, tmp, 3);

                    tmp->x = xt;
                    tmp->y = yt;
                    activelist_remove(tmp);
                    remove_ob(tmp); /* this is only a "trick" remove - no walk off check.
                                    * Remember: don't put important triggers near tiled map borders!
                                    */

                    if (otmp && (QUERY_FLAG(otmp, FLAG_REMOVED) || OBJECT_FREE(otmp))) /* invalid next ptr! */
                    {
                        /* remember: if we have remove for example 2 or more objects above, the
                        * op->above WILL be still valid - remove_ob() will handle it right.
                        * IF we get here a valid ptr, ->above WILL be valid too. Always.
                        */
                        if (!QUERY_FLAG(op, FLAG_REMOVED) && !OBJECT_FREE(op))
                            otmp = op->above;
                        else if (last_valid)
                            otmp = last_valid->above;
                        else
                            otmp = mp->first; /* should be really rare */
                    }
                    continue;
                }

                save_object(fp, op, 3);

                if (op->more) /* its a head (because we had tails tested before) */
                {
                    activelist_remove(op);
                    remove_ob(op); /* only a "trick" remove - no move_apply() changes or something */

                    if (otmp && (QUERY_FLAG(otmp, FLAG_REMOVED) || OBJECT_FREE(otmp))) /* invalid next ptr! */
                    {
                        if (!QUERY_FLAG(op, FLAG_REMOVED) && !OBJECT_FREE(op))
                            otmp = op->above;
                        else if (last_valid)
                            otmp = last_valid->above;
                        else
                            otmp = mp->first; /* should be really rare */
                    }
                }
            } /* for this space */
        } /* for this j */
    }
}

/* function will remove all player from a map and set a marker.
 * use CAREFUL - this is called from functions who do a forced
 * map resets and such. map_to_player_link() MUST be called after
 * this or the server has a problem.
 */
int map_to_player_unlink(mapstruct *m)
{
    player *pl;
    int num = 0;

    if(m)
    {
        for (pl = first_player; pl != NULL; pl = pl->next)
        {
            if (pl->ob->map == m)
            {
                num++;

                /* With the new activelist, any player on a reset map
                * was somehow forgotten. This seems to fix it. The
                * problem isn't analyzed, though. Gecko 20050713 */
                activelist_remove(pl->ob);
                remove_ob(pl->ob); /* no walk off check */

                pl->dm_removed_from_map = 1;
            }
        }

    }
    return num;
}

/* Reinsert players on a map which was removed with
 * map_to_player_unlink(). If m is NULL use the bind point.
 * if x or y != -1 they will overrule the player map position.
 */
void map_to_player_link(mapstruct *m, int x, int y, int flag)
{
    player *pl;

   for (pl = first_player; pl != NULL; pl = pl->next)
    {
        if (pl->dm_removed_from_map)
        {
            pl->dm_removed_from_map = 0;
            if(m)
                enter_map(pl->ob, NULL, m, x==-1?pl->ob->x:x, y==-1?pl->ob->y:y, m->map_status);
            else if(!flag)
                enter_map_by_name(pl->ob, pl->savebed_map, pl->orig_savebed_map, pl->bed_x, pl->bed_y, pl->bed_status);
            else /* if flag == TRUE move to emergency map! */
                enter_map_by_name(pl->ob, pl->savebed_map, pl->orig_savebed_map, pl->bed_x, pl->bed_y, pl->bed_status);
        }
    }
}

/* FIXME: what is the relevance of the use of the word "mapname" instead
 * of map path here? Gecko 2007-02-03. */
/** Validate and normalize a map name.
 * This is called by scripts or any other source where we must ensure
 * that our mapname is legal. We do some sanity tests to ensure we
 * have a valid and working name.
 * Note that mapnames beginning with "." should already have been normalized
 * and won't be normalized by this function.
 * @param mapname the mapname to validate and normalize. Doesn't have to be a shared string.
 * Should be an absolute path (or an instance/unique path).
 * @return normalized legal map name as shared string, or NULL if the mapname is illegal
 */
const char* create_safe_mapname_sh(char const *mapname)
{
    char path[MAXPATHLEN];
    const char *p;
    int c=0;

    if(mapname)
    {
        for(p=mapname;;p++) /* count and check the string */
        {
            if(*p=='\0') /* string ends with 0 - all is fine */
                break;

            if(++c >= MAXPATHLEN-6) /* string is to long */
                return NULL;

            /* control chars, 0x0a or windows path elements? forget it */
            if(isspace(*p) || iscntrl(*p) || *p=='\\')
               return NULL;

            /* check for valid '.' use - we mark as invalid if either:
             * - in a path starting with '.' another '.' is found ("./xxx" pathes are always normalized)
             * - after a '.' anything but another '.' or a '/' is found
             */
            if(*p == '.' && p != mapname)
            {
                if(*mapname == '.' || (*(p+1) != '.' && *(p+1) != '/'))
                    return NULL;
            }
        }
    }

    if(*mapname == '.') /* direct unique or instance string - should already be normalized */
        p = add_string(mapname);
    else
        p = add_string(normalize_path(mapname, NULL, path));

   return p;
}
