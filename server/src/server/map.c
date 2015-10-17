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
    along with this program; if not, write to the Free Softwar
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The author can be reached via e-mail to info@daimonin.org
*/

/* The map module is subdivided between three file pairs: map.c/h (loads,
 * manages, and deletes individual maps in memory); msp.c/h (handled individual
 * squares on a map); and tiling.c/h (manages groups of horizontally connected
 * maps). */

#include "global.h"

/* A list of starter locations sorted by race. If a race is not
 * mentioned on this list, the server will fall back to a default.
 * NOTE: locations for each race MUST be grouped together
 * or they will be ignored. I.e. all the elf starter locations
 * must be next to each other on this list.
 */
_race_start_location race_start_locations[NUM_START_LOCATIONS] =
{
    {"human", "/planes/human_plane/castle/castle_030a", 18, 1, MAP_STATUS_MULTI}
};

/* To get the reverse direction for all 8 tiled map index */
static int MapTiledReverse[TILING_DIRECTION_NROF] =
{
    2,
    3,
    0,
    1,
    6,
    7,
    4,
    5
};

static map_t *GetLinkedMap(void);
static void   AllocateMap(map_t *m);
static map_t *LoadMap(shstr_t *path_sh, shstr_t *orig_path_sh, uint32 flags, shstr_t *reference);
static map_t *LoadTemporaryMap(map_t *m);
static int    LoadMapHeader(FILE *fp, map_t *m, uint32 flags);
static void   FreeMap(map_t *m);
static char  *PathToName(shstr_t *path_sh);
static void   LoadObjects(map_t *m, FILE *fp, int mapflags);
static void   UpdateMapTiles(map_t *m);
static void   SaveObjects(map_t *m, FILE *fp);
#ifdef RECYCLE_TMP_MAPS
static void   WriteMapLog(void);
#endif

/* Returns the map_t which has a name matching the given argument oe NULL
 * if no match is found. */
map_t *map_is_in_memory(shstr_t *path)
{
    map_t *map;

    if (!path ||
        !*path)
    {
        return NULL;
    }

    /* This IS a bug starting without '/' or '.' - this can lead in double
     * loaded maps! We don't fix it here anymore - this MUST be done by the
     * calling functions or our inheritanced map system is already broken
     * somewhere before this call. */
    if (*path != '/' &&
        *path != '.')
    {
        LOG(llevBug, "BUG:: %s/map_is_in_memory(): filename without start '/' or '.' (>%s<)\n",
            __FILE__, path);

        return NULL;
    }

    for (map = first_map; map; map = map->next)
    {
        if (path == map->path)
        {
            break;
        }
    }

    return map;
}

map_t *map_is_ready(shstr_t *path_sh)
{
    map_t *m = map_is_in_memory(path_sh);

    if (m &&
        (m->in_memory != MAP_MEMORY_LOADING &&
         m->in_memory != MAP_MEMORY_ACTIVE))
    {
         m = NULL;
    }

    return m;
}

/* This prepends LIBDIR/MAPDIR/ to the given path and returns the pointer to a
 * static array containing the result. */
char *create_mapdir_pathname(const char *name)
{
    static char buf[MAXPATHLEN];

    /* double "//" would be a problem for comparing path strings */
    if (*name == '/')
    {
        sprintf(buf, "%s%s", settings.mapdir, name);
    }
    else
    {
        sprintf(buf, "%s/%s", settings.mapdir, name);
    }

    return buf;
}

/* This function checks if a file with the given path exists. -1 is returned if
 * it fails, otherwise the mode of the file is returned. It tries out all the
 * compression suffixes listed in the uncomp[] array.
 *
 * If prepend_dir is set, then we call create_mapdir_pathname (which prepends
 * libdir & mapdir). Otherwise, we assume the name given is fully complete. */
sint8 check_path(const char *name, uint8 prepend_dir)
{
    /* Sanity check. */
    if (!name ||
        *name == '\0')
    {
        return -1;
    }

    if (prepend_dir)
    {
        char buf[MAXPATHLEN];

        strcpy(buf, create_mapdir_pathname(name));
        name = buf;
    }

    return access(name, 0);
}

/* Make path absolute and remove ".." and "." entries.
 *
 * path will become a normalized (absolute) version of the path in dst, with
 * all relative path references (".." and "." - parent directory and same
 * directory) resolved (path will not contain any ".." or "." elements, even
 * if dst did).
 *
 * If dst was not already absolute, the directory part of src will be used as
 * the base path and dst will be added to it.
 *
 * src is the already normalized file name for finding absolute path.
 *
 * dst is the path to normalize. Should be either an absolute path or a path
 * relative to src. It must be a true "source" map path, and not a path into
 * the "./instance" or "./players" directory.
 *
 * If dst is NULL, src is used for dst too.
 *
 * path is a destination buffer for normalized path. Should be at least
 * MAXPATHLEN big.
 *
 * Returns pointer to path. */
char *normalize_path(const char *src, const char *dst, char *path)
{
    char *p;
    /* char *q; */
    char  buf[MAXPATHLEN * 2];

    /*LOG(llevDebug,"path before normalization >%s< >%s<\n", src, dst?dst:"<no dst>");*/

    if (!src)
    {
        LOG(llevBug, "BUG:: %s/normalize_path(): Called with src path = NULL! (dst:%s)\n",
            __FILE__, STRING_SAFE(dst));
        path[0] = '\0';

        return path;
    }

    if (!dst)
    {
        dst = src;
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
            if (!strncmp(dst, LOCALDIR "/" INSTANCEDIR,
                         LSTRLEN(LOCALDIR "/" INSTANCEDIR)) ||
                !strncmp(dst, LOCALDIR "/" PLAYERDIR,
                         LSTRLEN(LOCALDIR "/" PLAYERDIR)))
            {
                LOG(llevBug, "BUG:: %s/normalize_path(): Called with unique/instance dst: %s!\n",
                    __FILE__, dst);
                strcpy(path, dst);

                return path;
            }
        }

        /* Combine directory part of src with dst to create absolute path */
        strcpy(buf, src);

        if ((p = strrchr(buf, '/')))
        {
            p[1] = '\0';
        }
        else
        {
            strcpy(buf, "/");
        }

        strcat(buf, dst);
    }

    /* Hmm.. This looks buggy. Meant to remove initial double slashes?
     * There will be problems if there are double slashes anywhere else in the
     * path. Gecko 2006-09-24. */
    /* Disabled to see if anything breaks. Gecko 2007-02-03 */
#if 0
    q = p = buf;

    while ((q = strstr(q, "//")))
    {
        p = ++q;
    }
#else
    p = buf;

    if (strstr(p, "//"))
    {
        LOG(llevBug, "BUG:: %s/normalize_path(): Unhandled '//' element: %s!\n",
            __FILE__, buf);
    }
#endif

    *path = '\0';

    for (p = strtok(p, "/"); p; p = strtok(NULL, "/"))
    {
        if (!strcmp(p, "."))
        {
            /* Just ignore "./" path elements */
        }
        else if (!strcmp(p, ".."))
        {
            /* Remove last inserted path element from 'path' */
            char *separator = strrchr(path, '/');

            if (separator)
            {
                *separator = '\0';
            }
            else
            {
                LOG(llevBug, "BUG:: %s/normalize_path(): Illegal path (too many \"..\" entries): %s!\n",
                    __FILE__, dst);
                *path = '\0';

                return path; /* Don't continue normalization */
            }
        }
        else
        {
            strcat(path, "/");
            strcat(path, p);
        }
    }

    /*LOG(llevDebug,"path after normalization >%s<\n", path);*/

    return path;
}

/* Same as above but here we know that src & dst was normalized before - so we
 * can just merge them without checking for ".." again. */
char *normalize_path_direct(shstr_t *src, shstr_t *dst, char *path)
{
    if (!src)
    {
        LOG(llevBug, "BUG:: %s/normalize_path_direct(): Called with src path = NULL! (dst:%s)\n",
            __FILE__, STRING_SAFE(dst));
        path[0] = '\0';
    }
    else
    {
        const char *cp;

        if (!dst)
        {
            cp = dst = src;
            *path = '\0';
        }
        else
        {
            cp = PathToName(dst);
        }

        /* cp is not absolute (guaranteed if dst != NULL). */
        if (*cp != '/')
        {
            char *p;

            /* Combine directory part of src with dst to create abs path. */
            sprintf(path, "%s", src);

            if ((p = strrchr(path, '/')))
            {
                *(p + 1) = '\0';
            }
            else
            {
                *path = '/';
                *(path + 1) = '\0';
            }
        }

        sprintf(strchr(path, '\0'), "%s", cp);
    }

    return path;
}

/* Allocates, initialises, and returns a pointer to a map_t.
 * Modified to no longer take a path option which was not being
 * used anyways.  MSW 2001-07-01 */
static map_t *GetLinkedMap(void)
{
    /* Tag counter for the memory/weakref system. Uniquely identifies this
     * instance of the map in memory. Not the same as the
     * map_tag/global_map_tag. */
    /* FIXME: I don't understand this. I think this comment must be old and
     * outdated. AFAICS ->tag and ->map_tag ARE the same (well, serve the same
     * purpose -- to uniquely globally id a map) and one should be retired and
     * replace with the other.
     *
     * Whichever survives, this is exactly analogous to object_t.count (so
     * named for historical reasons I guess) and should therefore also be
     * renamed as map_t.count.
     *
     * ATM it seems that outside of this file, ->tag is only used in plugin_lua
     * (where it is as count is to objects) and ->map_tag is not used at all.
     * Within this file, ->tag is only used on map initialisation (set to
     * non-zero) and destruction (set to zero) and ->map_tag in much the same
     * way on map load and save (swap) (although map_tag relies on
     * global_map_tag which for some inexplicable reason is randomised in
     * init.c so 0 is a valid unique value.
     *
     * Needs further investigation.
     *
     * -- Smacky 20140620 */
    static tag_t  gID = 0;
    map_t    *map = get_poolchunk(pool_map);

    if (!map)
    {
        LOG(llevError, "ERROR:: %s/GetLinkedMap(): OOM!\n", __FILE__);

        return NULL;
    }

    memset(map, 0, sizeof(map_t));
    map->tag = ++gID;

    /* lifo queue */
    if (first_map)
    {
        first_map->last = map;
    }

    map->next = first_map;
    first_map = map;
    map->in_memory = MAP_MEMORY_SWAPPED;

    /* The maps used to pick up default x and y values from the
     * map archetype. Mimic that behaviour. */
    MAP_WIDTH(map) = MAP_DEFAULT_WIDTH;
    MAP_HEIGHT(map) = MAP_DEFAULT_HEIGHT;
    MAP_RESET_TIMEOUT(map) = MAP_DEFRESET;
    MAP_SWAP_TIMEOUT(map) = MAP_DEFSWAP;
    MAP_DIFFICULTY(map) = MAP_DEFAULT_DIFFICULTY;

    /* We insert a dummy sentinel first in the activelist. This simplifies
     * work later */
    map->active_objects = get_object();
    FREE_AND_COPY_HASH(map->active_objects->name, "<map activelist sentinel>");

    /* Avoid gc of the sentinel object_t */
    insert_ob_in_ob(map->active_objects, &void_container);

    return map;
}

/* Allocates the arrays contained in a map_t.
 * This basically allocates the dynamic array of spaces for the
 * map. */
static void AllocateMap(map_t *m)
{
    sint16    x,
              y,
              xl = MAP_WIDTH(m),
              yl = MAP_HEIGHT(m);
    msp_t *msp;
    uint32    flags = 0;

    m->in_memory = MAP_MEMORY_LOADING;

    /* Log this condition and free the storage.  We could I suppose
     * realloc, but if the caller is presuming the data will be intact,
     * that is their poor assumption. */
    if (m->spaces ||
        m->bitmap)
    {
        LOG(llevBug, "\nBUG:: %s/AllocateMap(): >%s< already allocated!\n",
            __FILE__, STRING_MAP_PATH(m));
        FREE(m->spaces);
        FREE(m->bitmap);
    }

    if (m->buttons)
    {
        LOG(llevBug, "\nBUG:: %s/AllocateMap(): >%s< has already set buttons!\n",
            __FILE__, STRING_MAP_PATH(m));
    }

    MALLOC(m->spaces, xl * yl * sizeof(msp_t));
    MALLOC(m->bitmap, ((xl + 31) / 32) * yl * sizeof(uint32));
    msp = &m->spaces[0];

    /* On map load we set some msp->floor_flags for every msp depending on
     * map-wide settings. These may subsequently be toggled when floor objects
     * are loaded or during play by scripts. Be sure to turn off the map flag
     * else reloading a swapped map will reverse all the msp flags again. */
    if ((m->flags & MAP_FLAG_OUTDOOR))
    {
        m->flags &= ~MAP_FLAG_OUTDOOR;
        flags |= MSP_FLAG_DAYLIGHT;
    }

    if ((m->flags & MAP_FLAG_PVP))
    {
        m->flags &= ~MAP_FLAG_PVP;
        flags |= MSP_FLAG_PVP;
    }

    if ((m->flags & MAP_FLAG_NO_SPELLS))
    {
        m->flags &= ~MAP_FLAG_NO_SPELLS;
        flags |= MSP_FLAG_NO_SPELLS;
    }

    if ((m->flags & MAP_FLAG_NO_PRAYERS))
    {
        m->flags &= ~MAP_FLAG_NO_PRAYERS;
        flags |= MSP_FLAG_NO_PRAYERS;
    }

    if ((m->flags & MAP_FLAG_NO_HARM))
    {
        m->flags &= ~MAP_FLAG_NO_HARM;
        flags |= MSP_FLAG_NO_HARM;
    }

    for (y = 0; y < yl; y++)
    {
        for (x = 0; x < xl; x++)
        {
            msp->map = m;
            msp->x = x;
            msp->y = y;
            msp->floor_flags |= flags;
            msp++;
        }
    }
}

/* Saves a map to file.  If flag is set, it is saved into the same file it was
 * (originally) loaded from. Otherwise a temporary filename will be genarated,
 * and the file will be stored there. The temporary filename will be stored in
 * the map_ture.
 *
 * If the map is a unique/instance, we save directly to the map's path (this
 * should have been updated when first loaded)..
 *
 * The return is the map_t. This function *can* delete the map_t, so it
 * is important to always check the return. */
map_t *map_save(map_t *m)
{
    FILE   *fp;
    char    filename[MAXPATHLEN];
    int     i;

    /* if we don't do this, we leave the light mask part
     * on a possible tiled map and when we reload, the area
     * will be set with wrong light values. */
    remove_light_source_list(m);

    if ((m->status & MAP_STATUS_UNIQUE) ||
        (m->status & MAP_STATUS_INSTANCE))
    {
        char *cp;

        sprintf(filename, "%s", m->path);

        if ((cp = strrchr(filename, '/')))
        {
            *cp = '\0';

            /* When the player has been deleted we mustn't try to save the map,
             * just delete it too. */
            if (access(filename, F_OK) == -1)
            {
#ifdef DEBUG_MAP
                LOG(llevDebug, "DEBUG:: %s/map_save(): Player %s no longer exists so deleting %s map >%s<!\n",
                    __FILE__, m->reference,
                     ((m->status & MAP_STATUS_UNIQUE)) ? "unique" : "instanced",
                     STRING_MAP_PATH(m));
#endif
                delete_map(m);

                return NULL;
            }
        }
        else
        {
            LOG(llevBug, "BUG:: %s/map_save(): Invalid path >%s<!\n",
                __FILE__, STRING_MAP_PATH(m));
        }

        /* that ensures we always reload from original maps */
        if (MAP_NOSAVE(m))
        {
#ifdef DEBUG_MAP
            LOG(llevDebug, "DEBUG:: %s/map_save(): Skip map >%s< (no_save flag)\n",
                __FILE__, STRING_MAP_PATH(m));

#endif
            return m;
        }

        strcpy(filename, m->path);
    }
    else
    {
        /* create tmpname if we don't have one or our old one was used by a different map */
        if (!m->tmpname ||
            access(m->tmpname, F_OK) ==-1 )
        {
            FREE_AND_NULL_PTR(m->tmpname);
            tempnam_local_ext(settings.tmpdir, NULL, filename);
            MALLOC2(m->tmpname, filename);
        }
        else
        {
            strcpy(filename, m->tmpname);
        }
    }

    LOG(llevInfo, "INFO:: Saving map >%s< to >%s<.\n",
        STRING_MAP_PATH(m), filename);
    m->in_memory = MAP_MEMORY_SAVING;

    if (!(fp = fopen(filename, "w")))
    {
        LOG(llevBug, "BUG:: %s/map_save(): Can't open file >%s< for saving!\n",
            __FILE__, filename);

        /* Reset the in_memory flag so that delete map will also free the
         * objects with it. */
        m->in_memory = MAP_MEMORY_ACTIVE;
        delete_map(m);

        return NULL;
    }

    /* legacy */
    fprintf(fp, "arch map\n");

    if (m->name)
    {
        fprintf(fp, "name %s\n", m->name);
    }

    if (m->music)
    {
        fprintf(fp, "background_music %s\n", m->music);
    }

    if (MAP_SWAP_TIMEOUT(m))
    {
        fprintf(fp, "swap_time %d\n", MAP_SWAP_TIMEOUT(m));
    }

    if (MAP_RESET_TIMEOUT(m))
    {
        fprintf(fp, "reset_timeout %d\n", MAP_RESET_TIMEOUT(m));
    }

    if (MAP_FIXED_RESETTIME(m))
    {
        fprintf(fp, "fixed_resettime %d\n", MAP_FIXED_RESETTIME(m) ? 1 : 0);
    }

    /* we unfortunately have no idea if this is a value the creator set
     * or a difficulty value we generated when the map was first loaded.  */
    if (m->difficulty)
    {
        fprintf(fp, "difficulty %d\n", m->difficulty);
    }

    fprintf(fp, "darkness %d\n", m->ambient_darkness);
    fprintf(fp, "map_tag %d\n", m->map_tag);

    if (m->width)
    {
        fprintf(fp, "width %d\n", m->width);
    }

    if (m->height)
    {
        fprintf(fp, "height %d\n", m->height);
    }

    if (m->enter_x)
    {
        fprintf(fp, "enter_x %d\n", m->enter_x);
    }

    if (m->enter_y)
    {
        fprintf(fp, "enter_y %d\n", m->enter_y);
    }

    if (m->msg)
    {
        fprintf(fp, "msg\n%sendmsg\n", m->msg);
    }

    if ((m->status & MAP_STATUS_UNIQUE))
    {
        fputs("unique 1\n", fp);
    }

    if ((m->status & MAP_STATUS_MULTI))
    {
        fputs("multi 1\n", fp);
    }

    if ((m->status & MAP_STATUS_INSTANCE))
    {
        fputs("instance 1\n", fp);
    }

    if ((m->status & MAP_STATUS_UNIQUE) ||
        (m->status & MAP_STATUS_INSTANCE))
    {
        if (m->reference)
        {
            fprintf(fp, "reference %s\n", m->reference);
        }
        else
        {
            LOG(llevBug, "BUG:: %s/map_save(): %s map with NULL reference!\n",
                __FILE__, ((m->status & MAP_STATUS_UNIQUE)) ? "Unique" : "Instanced");
        }
    }

    if (MAP_NOSAVE(m))
    {
        fputs("no_save 1\n", fp);
    }

    if ((m->flags & MAP_FLAG_NO_SUMMON))
    {
        fputs("no_summon 1\n", fp);
    }

    if (MAP_FIXEDLOGIN(m))
    {
        fputs("fixed_login 1\n", fp);
    }

    if ((m->flags & MAP_FLAG_PERMDEATH))
    {
        fputs("perm_death 1\n", fp);
    }

    if ((m->flags & MAP_FLAG_ULTRADEATH))
    {
        fputs("ultra_death 1\n", fp);
    }

    if ((m->flags & MAP_FLAG_ULTIMATEDEATH))
    {
        fputs("ultimate_death 1\n", fp);
    }

    /* save original path */
    fprintf(fp, "orig_path %s\n", m->orig_path);

    /* Save any tiling information */
    for (i = 0; i < TILING_DIRECTION_NROF; i++)
    {
        if (m->tiling.tile_path[i])
        {
            fprintf(fp, "tile_path_%d %s\n", i + 1, m->tiling.tile_path[i]);
        }

        if (m->tiling.orig_tile_path[i])
        {
            fprintf(fp, "orig_tile_path_%d %s\n", i + 1, m->tiling.orig_tile_path[i]);
        }
    }

    /* Save any tileset information */
    if (m->tiling.tileset_id > 0)
    {
        fprintf(fp, "tileset_id %d\n", m->tiling.tileset_id);
        fprintf(fp, "tileset_x %d\n", m->tiling.tileset_x);
        fprintf(fp, "tileset_y %d\n", m->tiling.tileset_y);
    }

    fprintf(fp, "end\n");
    SaveObjects(m, fp);

    /* When there are players or (TODO) permanently loaded mobs on the map, put
     * the map back in memory. */
    if (m->player_first ||
        m->perm_load)
    {
        m->in_memory = MAP_MEMORY_ACTIVE;
    }

    fclose(fp);
    chmod(filename, SAVE_MODE);

    return m;
}

/* Frees everything allocated by the given map_ture.
 * Don't free tmpname - our caller is left to do that. */
static void FreeMap(map_t *m)
{
    int i;

    if (!m->in_memory)
    {
        LOG(llevBug, "BUG:: %s/FreeMap(): Trying to free freed map.\n",
            __FILE__);

        return;
    }

#ifdef DEBUG_MAP
    LOG(llevDebug, "DEBUG:: %s/FreeMap(): Freeing map >%s<!\n",
        __FILE__, STRING_MAP_PATH(m));
#endif

    /* remove linked spawn points (small list of objectlink_t *) */
    remove_linked_spawn_list(m);

    /* Remove buttons. */
    if (m->buttons)
    {
        objectlink_free(m->buttons);
        m->buttons = NULL;
    }

    /* Remove objects. */
    if (m->spaces)
    {
        sint16    x,
                  y,
                  xl = m->width,
                  yl = m->height;

        for (x = 0; x < xl; x++)
        {
            for (y = 0; y < yl; y++)
            {
                msp_t    *msp = MSP_RAW(m, x, y);
                object_t *this,
                         *next;

                FOREACH_OBJECT_IN_MSP(this, msp, next)
                {
                    this = (this->head) ? this->head : this;

                    /* this is important - we can't be sure after we removed
                     * all objects from the map, that the map structure will still
                     * stay in the memory. If not, the object GC will try - and obj->map
                     * will point to a free map struct... (/resetmap for example) */
                    activelist_remove(this);
                    remove_ob(this); /* technical remove - no check off */
                }
            }
        }
    }

    /* Active list sanity check.
     * At this point, FreeAllObjects() should have removed every
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
    /* I stand to be corrected, but I think the following code is
     * wrong/unnecessary. For one thing, the query of FLAG_REMOVED is made on
     * m->active_objects, which is the sentinel so of course not removed.
     * Therefore, when there are still active objects each one, regardless of
     * whether /it/ is removed will log as a bug, contributing/causing an
     * unnecessary bug flood. If we correct that (ie, query
     * m->active_objects->active_next) then why log that there is a non-removed
     * active object but not actually take the opportunity to remove it? I am
     * not clear why maps can end up with leftover active objects on them just
     * after FreeAllObjects() -- presumably a timing issue to do with the map
     * being swapped out while objects are still on the inserted_active_objects
     * temporary list -- but this frequently happens on maps with arrow-shooting
     * mobs or mobs that kill other mobs, resulting in large numbers of
     * spurious BUGs. So my replacement code logs that the freed map does have
     * active objects, if only as a reminder that this needs to be looked into,
     * but then quietly removes such leftovers.
     * -- Smacky 20101113 */
    /* TODO: Not sure this is needed at all. Needs thought.
     *
     * -- Smacky 20150805 */
#if 0
    if(m->active_objects->active_next)
    {
        LOG(llevDebug, "ACTIVEWARNING - FreeMap(): freed map has still active objects!\n");
        while(m->active_objects->active_next)
        {
            if(!QUERY_FLAG(m->active_objects, FLAG_REMOVED))
                LOG(llevBug, "ACTIVEBUG - FREE_MAP(): freed map (%s) has active non-removed object %s (%d)!\n", STRING_MAP_PATH(m), STRING_OBJ_NAME(m->active_objects->active_next), m->active_objects->active_next->count);
            activelist_remove(m->active_objects->active_next);
        }
    }
#else
    if(m->active_objects->active_next)
    {
        object_t *next;

#ifdef DEBUG_GC
        LOG(llevDebug, "DEBUG:: %s/FreeMap(): freed map (>%s<) has objects on activelist!\n",
            __FILE__, STRING_MAP_PATH(m));

#endif
        while ((next = m->active_objects->active_next))
        {
            activelist_remove(next);

            if (!QUERY_FLAG(next, FLAG_REMOVED))
            {
                remove_ob(next);
            }
        }
    }
#endif

    FREE_AND_NULL_PTR(m->spaces);
    FREE_AND_NULL_PTR(m->bitmap);

    /* Delete the backlinks in other tiled maps to our map */
    for (i = 0; i < TILING_DIRECTION_NROF; i++)
    {
        if (m->tiling.tile_map[i])
        {
            if (m->tiling.tile_map[i]->tiling.tile_map[MapTiledReverse[i]] &&
                m->tiling.tile_map[i]->tiling.tile_map[MapTiledReverse[i]] != m )
            {
                LOG(llevMapbug, "MAPBUG:: Freeing map >%s< linked to >%s< which links back to another map!\n",
                    STRING_MAP_ORIG_PATH(m),
                    STRING_MAP_ORIG_PATH(m->tiling.tile_map[i]));
            }

            m->tiling.tile_map[i]->tiling.tile_map[MapTiledReverse[i]] = NULL;
            m->tiling.tile_map[i] = NULL;
        }

        FREE_AND_CLEAR_HASH(m->tiling.tile_path[i]);
        FREE_AND_CLEAR_HASH(m->tiling.orig_tile_path[i]);
    }

    FREE_AND_CLEAR_HASH(m->name);
    FREE_AND_CLEAR_HASH(m->music);
    FREE_AND_CLEAR_HASH(m->msg);
    FREE_AND_CLEAR_HASH(m->rv_cache.path);
    FREE_AND_CLEAR_HASH(m->reference);
    m->in_memory = MAP_MEMORY_SWAPPED;
    /* Note: m->path, m->orig_path and m->tmppath are freed in delete_map */
}

/*
 * function: vanish map_t
 * m       : pointer to map_t, if NULL no action
 * this deletes all the data on the map (freeing pointers)
 * and then removes this map from the global linked list of maps.
 */

void delete_map(map_t *m)
{
    if (!m)
    {
        return;
    }

    if (m->in_memory == MAP_MEMORY_ACTIVE)
    {
        /* change to MAP_MEMORY_SAVING, even though we are not, so that remove_ob()
         * doesn't do as much work. */
        m->in_memory = MAP_MEMORY_SAVING;
        FreeMap(m);
    }
    else
    {
        remove_light_source_list(m);
    }

    /* remove m from the global server map list */
    if (m->next)
    {
        m->next->last = m->last;
    }

    if (m->last)
    {
        m->last->next = m->next;
    }
    else /* if there is no last, we are first map */
    {
        first_map = m->next;
    }

    /* Remove the list sentinel */
    remove_ob(m->active_objects);

    /* Free our pathnames (we'd like to use it above)*/
    FREE_AND_CLEAR_HASH(m->path);
    FREE_AND_CLEAR_HASH(m->orig_path);
    FREE(m->tmpname);

    m->tag = 0; /* Kill any weak references to this map */
    return_poolchunk(m, pool_map);
}

void clean_tmp_map(map_t *m)
{
    if (m->tmpname)
    {
        unlink(m->tmpname);
    }
}

void free_all_maps(void)
{
    int real_maps = 0;

    while (first_map)
    {
        /* I think some of the callers above before it gets here set this to be
         * saving, but we still want to free this data. */
        if (first_map->in_memory == MAP_MEMORY_SAVING)
        {
            first_map->in_memory = MAP_MEMORY_ACTIVE;
        }

        delete_map(first_map);
        real_maps++;
    }

    LOG(llevInfo, "INFO:: Freed %d maps\n", real_maps);
}

/* Takes path and returns NULL if it is an invalid path to a map or a
 * normalized shared string otherwise.
 *
 * This is called by scripts or any other source where we must ensure that path
 * path is legal. To this end, we do some (basic) sanity tests.
 *
 * path doesn't have to be a shared string (and usually isn't) but must be
 * absolute or a unique/instance path.
 *
 * Note that paths beginning with '.' should already have been normalized so
 * won't be normalized by this function. */
shstr_t *create_safe_path_sh(const char *path)
{
    const char *cp;
    uint16      c;
    shstr_t      *path_sh = NULL;

    /* Sanity checks. */
    if (!path ||
        (*path != '.' &&
         *path != '/'))
    {
        return NULL;
    }

    /* Do some basic checks. This is NOT foolproof. */
    for (cp = path, c = 0; *cp; cp++, c++)
    {
        if (c >= MAXPATHLEN - 6 && // path is too long
            *path != '.')          // but unique/instance paths have no limit
        {
            return NULL;
        }
        else if (isspace(*cp) || // no whitespace
                 iscntrl(*cp) || // no control chars
                 *cp == '\\')    // no windows path elements
        {
           return NULL;
        }
        else if (*cp == '.' && // a path starting with '.' is OK -- we assume
                 cp != path)   // it to be a unique/instance path
        {
            if (*path == '.' ||      // but a further '.' is bad
                (*(cp + 1) != '.' && // as is any character but another '.' or
                 *(cp + 1) != '/'))  // a '/'
            {
                return NULL;
            }
        }
    }

    /* Create our shared string. */
    if(*path == '.')
    {
        FREE_AND_COPY_HASH(path_sh, path);
    }
    else
    {
        char buf[MAXPATHLEN];

        FREE_AND_COPY_HASH(path_sh, normalize_path(path, NULL, buf));
    }

   return path_sh;
}

/* Returns a unique path_sh based on the arguments. */
shstr_t *create_unique_path_sh(shstr_t *reference, shstr_t *orig_path_sh)
{
     char     path[LARGE_BUF];
     shstr_t *path_sh = NULL;

     sprintf(path, "%s/%s/%s/%s/%s",
         settings.localdir, settings.playerdir, get_subdir(reference),
         reference, PathToName(orig_path_sh));
     FREE_AND_COPY_HASH(path_sh, path);

     return path_sh;
}

/* Returns an instance path_sh based on the arguments.
 *
 * To avoid a reenter, set the player->instance_num to MAP_INSTANCE_NUM_INVALID
 * before the call.
 *
 * This function does 2 important things:
 * 1.) creating a valid instance file path out of name
 * 2.) ensure that the (temporary) DIRECTORY of this instance exits as long as the
 * server deals with an instance to it. */
shstr_t *create_instance_path_sh(player_t *pl, shstr_t *orig_path_sh, uint32 flags)
{
    char   path[LARGE_BUF];
    int    instance_num = pl->instance_num;
    shstr_t *path_sh = NULL;

    /* REENTER PART: we have valid instance data ... remember: the important one is the directory, not the map */
    if (instance_num != MAP_INSTANCE_NUM_INVALID)
    {
        /* just a very last sanity check... never EVER use a illegal ID */
        if (pl->instance_id != global_instance_id ||
            (flags & MAP_INSTANCE_FLAG_NO_REENTER))
        {
            instance_num = MAP_INSTANCE_NUM_INVALID;
        }
        else
        {
            sprintf(path, "%s/%s/%ld/%d/%d/%s",
                    settings.localdir, settings.instancedir, pl->instance_id,
                    instance_num / 10000, instance_num, PathToName(orig_path_sh));
        }
    }

    if (instance_num == MAP_INSTANCE_NUM_INVALID)
    {
        instance_num = get_new_instance_num();

        /* create new instance directory for this instance */
        sprintf(path, "%s/%s/%ld/%d/%d/%s",
                settings.localdir, settings.instancedir, global_instance_id,
                instance_num / 10000, instance_num, PathToName(orig_path_sh));

        /* Store the instance information for the player. */
        pl->instance_flags = flags;
        pl->instance_id = global_instance_id;
        pl->instance_num = instance_num;
        FREE_AND_COPY_HASH(pl->instance_name, orig_path_sh);
    }

    FREE_AND_COPY_HASH(path_sh, path);

    /* Thats the most important part... to guarantee we have the directory!
     * If it's not there, we will run into bad problems when we try to save or
     * load the instance! */
    make_path_to_file(path);

    return path_sh;
}

/** Ready a map of the same type as another map, even for the
 * same instance if applicable.
 *
 * @param orig_map map to inherit type and instance from
 * @param new_map_path the path to the map to ready. This can be either an absolute path,
 *        or a path relative to orig_map->path. It must be a true "source" map path,
 *        and not a path into the "./instance" or "./players" directory.
 * @return pointer to loaded map, or NULL
 */
map_t *ready_inherited_map(map_t *orig_map, shstr_t *new_map_path)
{
    map_t *new_map = NULL;
    shstr_t     *new_path = NULL,
              *normalized_path = NULL;
    char       tmp_path[MAXPATHLEN];

    /* Try some quick exits first */
    if (!orig_map ||
        !new_map_path ||
        *new_map_path == '\0')
    {
        return NULL;
    }

    if (new_map_path == orig_map->path &&
        (orig_map->in_memory == MAP_MEMORY_LOADING ||
         orig_map->in_memory == MAP_MEMORY_ACTIVE))
    {
        return orig_map;
    }

    if (!MAP_STATUS_TYPE(orig_map->status))
    {
        LOG(llevBug, "BUG:: %s/ready_inherited_map(): map >%s< without status type!\n",
            __FILE__, STRING_MAP_ORIG_PATH(orig_map));

        return NULL;
    }

#if 0
/* This disabled code is the original and the alternative is a hopefully fixed
 * version. The problem was that when new_map_path is an in memory
 * instance/unique map the following still adds its munged path to
 * orig_map->path, resulting in a non-existant path, causing eg:
 *
 * BUG: normalize_path(): Called with unique/instance dst: ./data/instance/1268315636/0/1/$planes$demon_plane$drows$mainmap_0100
 * load_map: ./data/instance/1268315636/0/1/.$data$instance$1268315636$0$1$$planes$demon_plane$drows$mainmap_0100 (4)
 * Debug: Can't open map file ./data/instance/1268315636/0/1/.$data$instance$1268315636$0$1$$planes$demon_plane$drows$mainmap_0100 (./data/instance/1268315636/0/1/$planes$demon_plane$drows$mainmap_0100)
 * BUG: calc_direction_towards(): invalid destination map for 'Drow Captain'
 *
 * A,so, calling normalize_path() on an instance/unique is illegal (you should
 * use normalize_path_direct()) but this will be done by the else branch below.
 *
 * The alternative code fixes these issues by queying orig_map->status first.
 * However, I am no expert in this area which is why I have left the original
 * code here and written this enormous comment. Perhaps someone who knows this
 * code could take a look? BTW I also think new_path is an entirely unnecessary
 * variable.
 * -- Smacky 20100311 */
    /* Guesstimate whether the path was already normalized or not (for speed) */
    if(*new_map_path == '/')
        normalized_path = add_refcount(new_map_path);
    else
        normalized_path = add_string(normalize_path(orig_map->path, new_map_path, tmp_path));

    /* create the path prefix (./players/.. or ./instance/.. ) for non multi maps */
    if(orig_map->status & (MAP_STATUS_UNIQUE|MAP_STATUS_INSTANCE))
    {
        new_path = add_string(normalize_path_direct(orig_map->path,
                    normalized_path, tmp_path));
    }
#else
    if (orig_map->status & (MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE))
    {
        /* Guesstimate whether the new map is already loaded */
        if (*new_map_path == '.')
        {
            normalized_path = add_refcount(new_map_path);
        }
        else
        {
            (void)normalize_path_direct(orig_map->path, new_map_path, tmp_path);
            normalized_path = add_string(tmp_path);
        }
    }
    else
    {
        /* Guesstimate whether the path was already normalized or not (for speed) */
        if (*new_map_path == '/')
        {
            normalized_path = add_refcount(new_map_path);
        }
        else
        {
            (void)normalize_path(orig_map->path, new_map_path, tmp_path);

            normalized_path = add_string(tmp_path);
        }
    }
#endif

//     LOG(llevInfo,">>>>>>>>>>>>>>>> orig_map->path=%s\nnew_map_path=%s\nnormalized_path=%s\nnew_path=%s\n", orig_map->path,new_map_path,normalized_path,new_path);

    new_map = ready_map_name((new_path) ? new_path : normalized_path,
                             normalized_path,
                             MAP_STATUS_TYPE(orig_map->status),
                             orig_map->reference);
    FREE_ONLY_HASH(normalized_path);
    FREE_ONLY_HASH(new_path);

    return new_map;
}

/* ready_map_name() attempts to ensure the map indicated by path_sh/orig_path_sh and flags is ready in memory, (re)loading from various locations depending on flags as necessary.
 *
 * path_sh and orig_path_sh must be genuine path and orig_path respectively or NULL. If both are NULL we simply return NULL (the function should not have been called in the first place).
 *
 * If a map with a matching path is already loaded into memory, there is nothing to do; it is already ready so return it.
 *
 * it will return a map pointer to the map path_sh/orig_path_sh.
 * If the map was not loaded before, the map will be loaded now.
 * orig_path_sh is ALWAYS a path to /maps = the original map path.
 * path_sh can be different and pointing to /instance or /players
 * reference needs to be a player name for UNIQUE or MULTI maps
 *
 * If orig_path_sh is NULL we will not load a map from disk, but return NULL
 * if the map wasn't in memory already.
 * If path_sh is NULL we will force a reload of the map even if it already
 * was in memory. (caller has to reset the map!) */
map_t *ready_map_name(shstr_t *path_sh, shstr_t *orig_path_sh, uint32 flags,
                          shstr_t *reference)
{
    map_t *m;

    /* Map is good to go? Just return it. */
    if ((m = map_is_in_memory((path_sh) ? path_sh : orig_path_sh)) &&
        (m->in_memory == MAP_MEMORY_LOADING ||
         m->in_memory == MAP_MEMORY_ACTIVE))
    {
#ifdef DEBUG_MAP
        LOG(llevDebug, "DEBUG:: %s/ready_map_name(): Map >%s< >%s< (%u/%u) already ready!\n",
            __FILE__, STRING_MAP_PATH(m), STRING_MAP_ORIG_PATH(m),
            m->in_memory, m->status);

#endif
        return m;
    }

    /* If it's not in memory or is but we want to load it new as unique or
     * instanced -- which always get loaded from data/players/ or
     * data/instances/. */
    if (!m ||
        (flags & (MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE)))
    {
        /* Tricky check - if we have '/' starting part, its a multi map we have here.
         * if called without orig_path_sh, we only check it in memory OR in tmp.
         * if we are here its not there or its not an multi map. */
        if (!orig_path_sh &&
            *path_sh == '/')
        {
            return  NULL;
        }

        /* The map must be SAVING or SWAPPED and we want to load it new as
         * unique or instance, so delete the old one and any temp. */
        if (m)
        {
#ifdef DEBUG_MAP
            LOG(llevDebug, "DEBUG:: %s/ready_map_name(): Cleaning up  map >%s< >%s< (%u/%u) for reload with as %s!\n",
                __FILE__, STRING_SAFE(path_sh), STRING_SAFE(orig_path_sh),
                 m->in_memory, m->status,
                 (flags & MAP_STATUS_UNIQUE) ? "unique" : "instance");
#endif
            clean_tmp_map(m);
            delete_map(m);
        }

        /* We are loading now a src map from maps/ or an unique/instance from
         * data/players/ or data/instances/. */
        m = LoadMap(path_sh, orig_path_sh, MAP_STATUS_TYPE(flags), reference);
    }
    /* If in this loop, we found a temporary map (so it was, and we are trying
     * to re-ready it as, a multi), so load it up. */
    else
    {
        m = LoadTemporaryMap(m);
    }

    return m;
}

/* Opens the file in path_sh or orig_path_sh and reads information about the
 * map from the given file, and stores it in a newly allocated map_t. A
 * pointer to this structure is returned, or NULL on failure.
 *
 * The function knows if it loads an original map from /maps or a
 * unique/instance by comparing path_sh and orig_path_sh.
 *
 * flags correspond to those in map.h. Main ones used are MAP_PLAYER_UNIQUE and
 * MAP_PLAYER_INSTANCE where path_sh != orig_path_sh. MAP_STYLE: style map -
 * don't add active objects, don't add to server managed map list.
 *
 * reference needs to be a player name for UNIQUE or MULTI maps. */
static map_t *LoadMap(shstr_t *path_sh, shstr_t *orig_path_sh, uint32 flags, shstr_t *reference)
{
    FILE      *fp;
    map_t *m;
    char       pathname[MEDIUM_BUF];

    flags &= ~MAP_STATUS_ORIGINAL;

    /* this IS a bug - because string compare will fail when it checks the loaded maps -
     * this can lead in a double load and break the server!
     * a '.' signs unique maps in fixed directories.
     * We don't fix it here anymore - this MUST be done by the calling functions or our
     * inheritanced map system is already broken somewhere before this call. */
    if ((path_sh &&
         *path_sh != '/' &&
         *path_sh != '.') ||
        (orig_path_sh &&
         *orig_path_sh != '/' &&
         *orig_path_sh != '.'))
    {
        LOG(llevBug, "BUG:: %s/LoadMap(): Filename without start '/' or '.' >%s< >%s<\n",
            __FILE__, STRING_SAFE(path_sh), STRING_SAFE(orig_path_sh));

        return NULL;
    }

    /* Here is our only "file path analyzing" trick. Our map model itself don't need it, but
     * it allows us to call this function with the DM commands like "/goto ./players/a/aa/Aa/$demo"
     * without pre-guessing the status. In fact status CAN be here invalid with 0!
     * IF status is zero here, LoadMap() will set it dynamic!
     * Checkup LoadMap() & LoadMapHeader() how it works.
     */
    if (path_sh)
    {
        if (*path_sh == '.') /* pathes to /instance and /players always start with a '.'! */
        {
            strcpy(pathname, path_sh);
        }
        else /* we have an normalized map here and the map start ALWAYS with a '/' */
        {
            strcpy(pathname, create_mapdir_pathname(path_sh)); /* we add the (...)/maps prefix path part */

            if (path_sh == orig_path_sh)
            {
                flags |= MAP_STATUS_ORIGINAL;
            }
        }
    }

    if (!path_sh ||
        !(fp = fopen(pathname, "r")))
    {
        /* this was usually a try to load a unique or instance map
         * This is RIGHT because we use fopen() here as an implicit access()
         * check. If it fails, we know we have to load the map from /maps! */
        if (orig_path_sh &&
            path_sh != orig_path_sh &&
            *orig_path_sh == '/')
        {
            strcpy(pathname, create_mapdir_pathname(orig_path_sh)); /* we add the (...)/maps prefix path part */
            flags |= MAP_STATUS_ORIGINAL;

            if (!(fp = fopen(pathname, "r")))
            {
                /* ok... NOW we are screwed with an invalid map... because it is not in /maps */
                LOG(llevBug, "BUG:: %s/LoadMap(): Can't open map file >%s< >%s<!\n",
                    __FILE__, STRING_SAFE(path_sh), STRING_SAFE(orig_path_sh));

                return NULL;
            }
        }
        else
        {
            LOG(llevBug, "BUG:: %s/LoadMap(): Can't open map file >%s< >%s<!\n",
                __FILE__, STRING_SAFE(path_sh), STRING_SAFE(orig_path_sh));

            return NULL;
        }
    }

    m = GetLinkedMap();

    if (path_sh)
    {
        FREE_AND_COPY_HASH(m->path, path_sh);
    }
    else
    {
        FREE_AND_COPY_HASH(m->path, orig_path_sh);
    }

    if (orig_path_sh) /* invalid orig_path_sh can happens when we force an explicit load of an unique map! */
    {
        FREE_AND_COPY_HASH(m->orig_path, orig_path_sh); /* orig_path will be loaded in LoadMapHeader()! */
    }

    m->map_tag = ++global_map_tag;    /* every map has an unique tag */
    LOG(llevInfo, "INFO:: %s:LoadMap(): Loading map >%s< >%s< as %s... ",
        __FILE__, STRING_MAP_PATH(m), STRING_MAP_ORIG_PATH(m),
        (flags & MAP_STATUS_UNIQUE)
        ? "unique" : ((flags & MAP_STATUS_INSTANCE) ? "instance" : "multi"));
#ifdef DEBUG_MAP
    LOG(llevInfo, "Header... ");
#endif

    if (LoadMapHeader(fp, m, flags))
    {
#ifdef DEBUG_MAP
        LOG(llevDebug, "DEBUG:: %s/LoadMap(): Error in map header, map will not be loaded!\n",
            __FILE__);
#endif
        delete_map(m);
        fclose(fp);

        return NULL;
    }

    /* Set up the reference string from function call if not stored with map */
    if(((m->status & MAP_STATUS_UNIQUE) ||
        (m->status & MAP_STATUS_INSTANCE)) &&
        !m->reference)
    {
        if (reference)
        {
            LOG(llevInfo, "Reference %s... ", reference);
            FREE_AND_ADD_REF_HASH(m->reference, reference);
        }
        else
        {
            LOG(llevBug, "BUG:: %s/LoadMap(): %s map with NULL reference parameter!\n",
                __FILE__, ((m->status & MAP_STATUS_UNIQUE)) ? "Unique" : "Instanced");
        }
    }

#ifdef DEBUG_MAP
    LOG(llevInfo, "Allocate... ");
#endif
    AllocateMap(m);
#ifdef DEBUG_MAP
    LOG(llevInfo, "Objects... ");
#endif
    LoadObjects(m, fp, flags & (MAP_STATUS_STYLE | MAP_STATUS_ORIGINAL));

#ifdef DEBUG_MAP
    /* In case other objects press some buttons down. We handle here all
     * kinds of "triggers" which are triggered permanent by objects like
     * buttons or inventory checkers. We don't check here instant stuff
     * like sacrificing altars. Because this should be handled on map
     * making side. */
    LOG(llevInfo, "Buttons.\n");
#endif
    update_buttons(m);
    fclose(fp);

    return m;
}

/* Loads a map, which has been loaded earlier, from file.
 * Return the map object we load into (this can change from the passed
 * option if we can't find the original map)
 * note: LoadMap() is called with (NULL, m->orig_path, MAP_STATUS_MULTI, NULL) when
 * tmp map loading fails because a tmp map is ALWAYS a MULTI map and when fails its
 * reloaded from /maps as new original map. */
static map_t *LoadTemporaryMap(map_t *m)
{
    FILE  *fp = NULL;
    uint8  fallback = 0;

    /* Check m->tmpname within this function as it makes the fallback logic and
     * if...else if...else chain in ready_map_name() simpler. */
    if (!m->tmpname)
    {
        LOG(llevBug, "BUG:: %s/LoadTemporaryMap(): No temporary map!\n",
            __FILE__);
        fallback = 1;
    }
    else if (!(fp = fopen(m->tmpname, "r")))
    {
        LOG(llevBug, "BUG:: %s/LoadTemporaryMap(): Can't open temporary map >%s<!\n",
            __FILE__, STRING_MAP_TMPNAME(m));
        fallback = 2;
    }
    else
    {
        LOG(llevInfo, "INFO:: Loading temporary map >%s<... ", STRING_MAP_TMPNAME(m));
#ifdef DEBUG_MAP
        LOG(llevInfo, "Header... ");
#endif

        if (LoadMapHeader(fp, m, MAP_STATUS_MULTI)) /* /tmp map = always normal multi maps */
        {
#ifdef DEBUG_MAP
            LOG(llevDebug, "DEBUG:: %s/LoadTemporaryMap(): Error in map header, map will not be loaded!\n",
                __FILE__);
#endif
            fallback = 3;
        }
    }

    if (fallback)
    {
        shstr_t *orig_path_sh = NULL;

        LOG(llevInfo, " Fallback to original!\n");
        FREE_AND_ADD_REF_HASH(orig_path_sh, m->orig_path);
        delete_map(m);
        m = LoadMap(NULL, orig_path_sh, MAP_STATUS_MULTI, NULL);
        FREE_ONLY_HASH(orig_path_sh);
    }
    else
    {
#ifdef DEBUG_MAP
        LOG(llevInfo, "Allocate... ");
#endif
        AllocateMap(m);
#ifdef DEBUG_MAP
        LOG(llevInfo, "Objects... ");
#endif
        LoadObjects(m, fp, 0);
#ifdef DEBUG_MAP
        LOG(llevInfo, "Clean... ");
#endif
        clean_tmp_map(m);
        m->in_memory = MAP_MEMORY_ACTIVE;
     
#ifdef DEBUG_MAP
        /* In case other objects press some buttons down. We handle here all
         * kinds of "triggers" which are triggered permanent by objects like
         * buttons or inventory checkers. We don't check here instant stuff
         * like sacrificing altars. Because this should be handled on map
         * making side. */
        LOG(llevInfo, "Buttons.\n");
#endif
        update_buttons(m);
    }

    if (fp)
    {
        fclose(fp);
    }
     
    return m;
}

/* This loads the header information of the map.  The header
 * contains things like difficulty, size, timeout, etc.
 * this used to be stored in the map object, but with the
 * addition of tiling, fields beyond that easily named in an
 * object structure were needed, so it just made sense to
 * put all the stuff in the map object so that names actually make
 * sense.
 *
 * This could be done in lex (like the object loader), but I think
 * currently, there are few enough fields this is not a big deal.
 * MSW 2001-07-01
 *
 * NOTE: LoadMapHeader will setup status dynamically when flags
 * has not a valid MAP_STATUS_FLAG()
 * return 0 on success, 1 on failure.  */
static int LoadMapHeader(FILE *fp, map_t *m, uint32 flags)
{
    char   buf[HUGE_BUF],
           msgbuf[HUGE_BUF],
          *key = buf,
          *value,
          *end;
    int    msgpos = 0,
           got_end = 0;
    uint8  i = 0;

    while (fgets(buf, HUGE_BUF - 1, fp) != NULL)
    {
        buf[HUGE_BUF - 1] = 0;
        key = buf;

        while (isspace(*key))
        {
            key++;
        }

        if (*key == '\0')
        {
            continue;    /* empty line */
        }

        if (!(value = strchr(key, ' ')))
        {
            end = key + (strlen(key) - 1);

            while (isspace(*end))
            {
                --end;
            }

            *++end = '\0';
        }
        else
        {
            *value++ = '\0';

            while (isspace(*value))
            {
                value++;
            }

            end = value + (strlen(value) - 1);

            while (isspace(*end))
            {
                --end;
            }

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
        else if (!strcmp(key,"background_music"))
        {
            *end = 0;
            FREE_AND_COPY_HASH(m->music, value);
        }
        else if (!strcmp(key, "msg"))
        {
            msgbuf[0] = '\0';

            while (fgets(buf, HUGE_BUF - 1, fp) != NULL)
            {
                if (!strcmp(buf, "endmsg\n"))
                    break;
                else
                {
#if 0 // SIGSEGVs
                    /* slightly more efficient than strcat */
                    strcpy(msgbuf + msgpos, buf);
                    msgpos += strlen(buf);
#else
                    /* TODO: Use snprintf to not risk buffer overflow. */
                    sprintf(strchr(msgbuf, '\0'), "%s", buf);
#endif
                }
            }

            /* There are lots of maps that have empty messages (eg, msg/endmsg
             * with nothing between).  There is no reason in those cases to
             * keep the empty message.  Also, msgbuf contains garbage data
             * when msgpos is zero, so copying it results in crashes
             */
            if (msgbuf[0])
            {
                FREE_AND_COPY_HASH(m->msg, msgbuf);
            }
        }
        /* enter_x is the default x (west-east) entry point. */
        else if (!strcmp(key, "enter_x"))
        {
            int v = atoi(value);

            m->enter_x = v;
        }
        /* enter_y is the default y (north-south) entry point. */
        else if (!strcmp(key, "enter_y"))
        {
            int v = atoi(value);

            m->enter_y = v;
        }
        /* width is the total number of x (west-east) squares. There is a
         * default which should be stuck to and any deviation on a testserver
         * will produce a debug warning. However this is not enforced and once
         * the map gets to a non-testserver the non-standard width is assumed
         * to be intentional so no warning is given. */
        else if (!strcmp(key, "width"))
        {
            int v = atoi(value);

#ifdef DAI_DEVELOPMENT_CONTENT
            if (v != MAP_DEFAULT_WIDTH)
                LOG(llevDebug, "DEBUG:: Non-standard map width %d (should be %d)!\n",
                    v, MAP_DEFAULT_WIDTH);
#endif

            m->width = v;
        }
        /* height is the total number of y (north-south) squares. There is a
         * default which should be stuck to and any deviation on a testserver
         * will produce a debug warning. However this is not enforced and once
         * the map gets to a non-testserver the non-standard height is assumed
         * to be intentional so no warning is given. */
        else if (!strcmp(key, "height"))
        {
            int v = atoi(value);

#ifdef DAI_DEVELOPMENT_CONTENT
            if (v != MAP_DEFAULT_HEIGHT)
                LOG(llevDebug, "DEBUG:: Non-standard map height %d (should be %d)!\n",
                    v, MAP_DEFAULT_HEIGHT);
#endif

            m->height = v;
        }
        else if (!strcmp(key, "reset_timeout"))
        {
#ifdef MAP_RESET
            int v = atoi(value);

            if (v == -1) // never reset
            {
                v = 0;
            }
            else if (v < 0 ||
                     v > MAP_MAXRESET) // specific time within server limits
            {
                /* Don't log temp maps. */
                if (!m->tmpname)
                {
                    LOG(llevMapbug, "MAPBUG:: >%s<: Illegal map reset time %d (must be 0 to %d, defaulting to %d)!\n",
                        STRING_MAP_ORIG_PATH(m), v, MAP_MAXRESET,
                         MAP_DEFRESET);
                }

                v = MAP_DEFRESET;
            }
            MAP_RESET_TIMEOUT(m) = v;
#else
            MAP_RESET_TIMEOUT(m) = 0;
#endif
        }
        else if (!strcmp(key, "swap_time"))
        {
#ifndef MAP_SWAP_OBJECT
            int v = atoi(value);

            if (v < MAP_MINSWAP ||
                v > MAP_MAXSWAP) // specific time within server limits
            {
                /* Don't log temp maps. */
                if (!m->tmpname)
                {
                    LOG(llevMapbug, "MAPBUG:: >%s<: Illegal map swap time %d (must be %d to %d, defaulting to %d)!\n",
                        STRING_MAP_ORIG_PATH(m), v, MAP_MINSWAP, MAP_MAXSWAP,
                        MAP_DEFSWAP);
                }

                v = MAP_DEFSWAP;
            }

            MAP_SWAP_TIMEOUT(m) = v;
#else
            MAP_SWAP_TIMEOUT(m) = 0;
#endif
        }
        /* difficulty is a 'recommended player level' for that map *for a
         * single player_t *, so follows the same restriction (1 to MAXLEVEL). It
         * is used in a number of situations to autogenerate treasure and
         * autocalculate monster stats and environment damage. The default is
         * 1. */
        else if (!strcmp(key, "difficulty"))
        {
            int v = atoi(value);

            if (v < 1 || v > MAXLEVEL)
               LOG(llevMapbug, "MAPBUG:: >%s<: Illegal map difficulty %d (must be 1 to %d, defaulting to 1)!\n",
                   STRING_MAP_ORIG_PATH(m), v, MAXLEVEL);
            else
                m->difficulty = v;
        }
        else if (!strcmp(key, "darkness"))
        {
            int v = atoi(value);

            if (v < -MAX_DARKNESS ||
                v > MAX_DARKNESS)
            {
                LOG(llevMapbug, "MAPBUG:: >%s<: Illegal map darkness %d (must be %d to %d, defaulting to %d)!\n",
                    STRING_MAP_ORIG_PATH(m), v, -MAX_DARKNESS, MAX_DARKNESS,
                    MAX(-MAX_DARKNESS, MIN(v, MAX_DARKNESS)));
                v = MAX(-MAX_DARKNESS, MIN(v, MAX_DARKNESS));
            }

            m->ambient_darkness = v;
            m->ambient_brightness = (v < 0) ? -(brightness[v]) : brightness[v];
        }
        /* No longer used, but we allow for it to suppress unknown header
         * MAPBUGs. */
        else if (!strcmp(key, "light"))
        {
        }
        else if (!strcmp(key, "no_save"))
        {
            if (atoi(value))
                m->flags |= MAP_FLAG_NO_SAVE;
        }
        else if (!strcmp(key, "fixed_login"))
        {
            if (atoi(value))
                m->flags |= MAP_FLAG_FIXED_LOGIN;
        }
        else if (!strcmp(key, "perm_death"))
        {
            if (atoi(value))
                m->flags |= MAP_FLAG_PERMDEATH;
        }
        else if (!strcmp(key, "ultra_death"))
        {
            if (atoi(value))
                m->flags |= MAP_FLAG_ULTRADEATH;
        }
        else if (!strcmp(key, "ultimate_death"))
        {
            if (atoi(value))
                m->flags |= MAP_FLAG_ULTIMATEDEATH;
        }
        else if (!strcmp(key, "outdoor"))
        {
            if (atoi(value))
                m->flags |= MAP_FLAG_OUTDOOR;
        }
        else if (!strcmp(key, "pvp"))
        {
            if (atoi(value))
                m->flags |= MAP_FLAG_PVP;
        }
        else if (!strcmp(key, "no_magic"))
        {
            if (atoi(value))
                m->flags |= MAP_FLAG_NO_SPELLS;
        }
        else if (!strcmp(key, "no_priest"))
        {
            if (atoi(value))
                m->flags |= MAP_FLAG_NO_PRAYERS;
        }
        else if (!strcmp(key, "no_harm"))
        {
            if (atoi(value))
                m->flags |= MAP_FLAG_NO_HARM;
        }
        else if (!strcmp(key, "no_summon"))
        {
            if (atoi(value))
                m->flags |= MAP_FLAG_NO_SUMMON;
        }
        else if (!strcmp(key, "map_tag"))
        {
            m->map_tag = (uint32) atoi(value);
        }
        else if (!strcmp(key, "unique"))
        {
            if (atoi(value))
                m->status |= MAP_STATUS_UNIQUE;
        }
        else if (!strcmp(key, "multi"))
        {
            if (atoi(value))
                m->status |= MAP_STATUS_MULTI;
        }
        else if (!strcmp(key, "instance"))
        {
            if (atoi(value))
                m->status |= MAP_STATUS_INSTANCE;
        }
        else if (!strcmp(key, "reference"))
        {
            *end = 0;
            FREE_AND_COPY_HASH(m->reference, value);

            if ((flags & MAP_STATUS_ORIGINAL))
            {
                LOG(llevMapbug, "MAPBUG:: >%s<: Map has reference to player %s\n",
                    STRING_MAP_ORIG_PATH(m), value);
            }
        }
        else if (!strcmp(key, "fixed_resettime"))
        {
            if (atoi(value))
                m->flags |= MAP_FLAG_FIXED_RTIME;
        }
        else if (!strcmp(key, "orig_path"))
        {
            *end = 0;
            /* important override - perhaps orig_path was setup with the (wrong) dest path!
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

            if (tile < 1 ||
                tile > TILING_DIRECTION_NROF)
            {
                LOG(llevMapbug,  "MAPBUG:: >%s<: orig tile location %d out of bounds: %s!\n",
                    STRING_MAP_PATH(m), tile, STRING_SAFE(value));
            }
            else
            {
                *end = '\0';

                if (m->tiling.orig_tile_path[tile - 1])
                {
                    LOG(llevMapbug, "MAPBUG:: >%s<: orig tile location %d duplicated: %s!\n",
                        STRING_MAP_PATH(m), tile, STRING_SAFE(value));
                }

                m->tiling.orig_tile_path[tile - 1] = add_string(value);
            }
        }
        else if (!strncmp(key, "tile_path_", 10))
        {
            int tile = atoi(key + 10);

            if (tile < 1 ||
                tile > TILING_DIRECTION_NROF)
            {
                LOG(llevMapbug,  "MAPBUG:: >%s<: tile location %d out of bounds: %s!\n",
                    STRING_MAP_PATH(m), tile, STRING_SAFE(value));
            }
            else
            {
                shstr_t *path_sh;
                map_t *neighbour;

                *end = '\0';

                if (m->tiling.tile_path[tile - 1])
                {
                    LOG(llevMapbug, "MAPBUG:: >%s<: tile location %d duplicated: %s!\n",
                        STRING_MAP_PATH(m), tile, STRING_SAFE(value));
                }

                /* note: this only works because our map saver is storing
                 * MAP_STATUS and orig_map BEFORE he saves the tile map data.
                 * NEVER change it, or the dynamic setting will fail! */
                if (!MAP_STATUS_TYPE(flags)) /* synchronize dynamically the map status flags */
                {
                    flags |= m->status;

                    if (!MAP_STATUS_TYPE(flags)) /* still zero? then force _MULTI */
                    {
                        flags |= MAP_STATUS_MULTI;
                    }
                }

                if ((flags & MAP_STATUS_ORIGINAL)) /* original map... lets normalize tile_path[] to /maps */
                {
                    normalize_path(m->orig_path, value, msgbuf);
                    m->tiling.orig_tile_path[tile - 1] = add_string(msgbuf);

                    /* If the specified map does not exist, report this and do
                     * not set the tile_path. */
                    if (check_path(m->tiling.orig_tile_path[tile - 1], 1) == -1)
                    {
                        LOG(llevMapbug, "MAPBUG:: Tile %d of map >%s< refers to non-existent file %s!\n",
                            tile, STRING_MAP_PATH(m), STRING_SAFE(m->tiling.orig_tile_path[tile - 1]));

                        continue;
                    }

                    /* whatever we have opened - in m->path is the REAL path */
                    if ((flags & (MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE)))
                    {
                        normalize_path_direct(m->path,
                                              m->tiling.orig_tile_path[tile - 1],
                                              msgbuf);
                        path_sh = add_string(msgbuf);
                    }
                    else /* for multi maps, orig_path is the same path */
                    {
                        path_sh = add_refcount(m->tiling.orig_tile_path[tile - 1]);
                    }
                }
                else /* non original map - all the things above was done before - just load */
                {
                    path_sh = add_string(value);
                }

                /* If the neighbouring map tile has been loaded, set up the map pointers */
                if ((neighbour = map_is_in_memory(path_sh)) &&
                    (neighbour->in_memory == MAP_MEMORY_ACTIVE ||
                     neighbour->in_memory == MAP_MEMORY_LOADING))
                {
                    int dest_tile = MapTiledReverse[tile - 1];

                    /* LOG(llevDebug,"add t_map >%s< (%d). ", path_sh, tile-1); */
                    if (neighbour->tiling.orig_tile_path[dest_tile] != m->orig_path)
                    {
                        /* Refuse tiling if anything looks suspicious, since that may leave dangling pointers and crash the server */
                        LOG(llevMapbug, "MAPBUG: map tiles incorrecly connected: >%s<->>%s< but >%s<->>%s<. Refusing to connect them!\n",
                                STRING_MAP_ORIG_PATH(m),
                                (path_sh) ? path_sh : "(no map)",
                                STRING_MAP_ORIG_PATH(neighbour),
                                (neighbour->tiling.orig_tile_path[dest_tile]) ? neighbour->tiling.orig_tile_path[dest_tile] : "(no map)");

                        /* Disable map linking */
                        FREE_AND_CLEAR_HASH(path_sh);
                        FREE_AND_CLEAR_HASH(m->tiling.orig_tile_path[tile - 1]);
                    }
                    else
                    {
                        m->tiling.tile_map[tile - 1] = neighbour;
                        neighbour->tiling.tile_map[dest_tile] = m;
                    }
                }

                m->tiling.tile_path[tile - 1] = path_sh;
            }
        }
        else if (!strcmp(key, "tileset_id"))
        {
            m->tiling.tileset_id = atoi(value);
        }
        else if (!strcmp(key, "tileset_x"))
        {
            m->tiling.tileset_x = atoi(value);
        }
        else if (!strcmp(key, "tileset_y"))
        {
            m->tiling.tileset_y = atoi(value);
        }
        else if (!strcmp(key, "end"))
        {
            got_end = 1;
            break;
        }
        else
        {
            LOG(llevMapbug, "MAPBUG:: >%s<: Got unknown value in map header: %s %s\n",
                STRING_MAP_ORIG_PATH(m), key, value);
        }
    }

    /* Ensure enter_x/y are sensible values. */
    if (MAP_ENTER_X(m) < 0 ||
        MAP_ENTER_X(m) >= MAP_WIDTH(m))
    {
        LOG(llevMapbug, "MAPBUG:: >%s<: enter_x out of bounds: %d (should be 0 to %d), defaulting to 0!\n",
            STRING_MAP_ORIG_PATH(m), MAP_ENTER_X(m), MAP_WIDTH(m) - 1);
        MAP_ENTER_X(m) = 0;
    }

    if (MAP_ENTER_Y(m) < 0 ||
        MAP_ENTER_Y(m) >= MAP_HEIGHT(m))
    {
        LOG(llevMapbug, "MAPBUG:: >%s<: enter_y out of bounds: %d (should be 0 to %d), defaulting to 0!\n",
            STRING_MAP_ORIG_PATH(m), MAP_ENTER_Y(m), MAP_HEIGHT(m) - 1);
        MAP_ENTER_Y(m) = 0;
    }

    /* Set times to swap/reset. */
    MAP_SET_WHEN_SWAP(m, MAP_SWAP_TIMEOUT(m));
    MAP_SET_WHEN_RESET(m, MAP_RESET_TIMEOUT(m));

#ifdef DAI_DEVELOPMENT_CONTENT
    /* Verify tileset_id after linking. */
    for(i = 0; i < 8; i++)
    {
        /* AI pathfinding reuquires consistent tileset_id. */
        if (m->tiling.tile_map[i] &&
            m->tiling.tile_map[i]->tiling.tileset_id != m->tiling.tileset_id)
        {
            LOG(llevMapbug, "MAPBUG: connected maps have inconsistent tileset_ids: >%s< (id %d)<->>%s< (id %d). Pathfinding will have problems.\n",
                STRING_MAP_ORIG_PATH(m), m->tiling.tileset_id,
                STRING_MAP_ORIG_PATH(m->tiling.tile_map[i]),
                m->tiling.tile_map[i]->tiling.tileset_id);
            /* TODO: also doublecheck tileset_x and tileset_y */
        }
    }
#endif

    if(!MAP_STATUS_TYPE(m->status)) /* synchronize dynamically the map status flags */
    {
        m->status |= MAP_STATUS_TYPE(flags);

        /* Still zero? then force _MULTI */
        if (!MAP_STATUS_TYPE(m->status))
        {
            m->status |= MAP_STATUS_MULTI;
        }
    }

    m->status |= (flags & MAP_STATUS_ORIGINAL);

    m->tadnow = &tadnow;
    m->tadoffset = 0;
    get_tad(m->tadnow, m->tadoffset);

    if (!got_end)
    {
        LOG(llevMapbug, "MAPBUG:: >%s<: Got premature eof of map header!\n",
            STRING_MAP_ORIG_PATH(m));

        return 1;
    }

    return 0;
}

/* Converts path_sh from a path to a filename. To do this, we copy path_sh to a
 * static local buffer, replace all '/' with '$' in this buffer, and return it
 * (thus the original is unchanged). */
static char *PathToName(shstr_t *path_sh)
{
    static char  buf[MAXPATHLEN];
    char        *cp;

    strncpy(buf, path_sh, MAXPATHLEN - 1);
    buf[MAXPATHLEN - 1] = '\0';

    for (cp = buf; *cp != '\0'; cp++)
    {
        if (*cp == '/')
        {
            *cp = '$';
        }
    }

    return buf;
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
static void LoadObjects(map_t *m, FILE *fp, int mapflags)
{
    int         i;
    archetype_t  *tail;
    void       *mybuffer;
    object_t     *op,
               *prev = NULL,
               *last_more = NULL,
               *tmp;

    op = get_object();
    op->map = m; /* To handle buttons correctly */
    m->flags |= MAP_FLAG_NO_UPDATE; /* be sure to avoid tile updating in the loop below */
    mybuffer = create_loader_buffer(fp);

    while ((i = load_object(fp, op, mybuffer, LO_REPEAT, mapflags)))
    {
        msp_t *msp = MSP_KNOWN(op);

        /* atm, we don't need and handle multi arches saved with tails! */
        if (i == LL_MORE)
        {
            LOG(llevMapbug, "MAPBUG:: %s[>%s< %d %d]: Object is a tail!\n",
                STRING_OBJ_NAME(op), STRING_MAP_PATH(m), op->x, op->y);
            goto next;
        }

        /* should not happen because we catch invalid arches now as singularities */
        if (!op->arch)
        {
            LOG(llevMapbug, "MAPBUG:: %s[>%s< %d %d]: Invalid archetype!\n",
                STRING_OBJ_NAME(op), STRING_MAP_PATH(m), op->x, op->y);
            goto next;
        }

        /* important pre set for the animation/face of a object_t */
        if (QUERY_FLAG(op, FLAG_IS_TURNABLE) ||
            QUERY_FLAG(op, FLAG_ANIMATE))
        {
            /* If a bad animation is set, we will get div by zero */
            if(NUM_FACINGS(op) == 0)
            {
                LOG(llevMapbug, "MAPBUG:: %s[>%s< %d %d]: NUM_FACINGS == 0. Bad animation?\n",
                    STRING_OBJ_NAME(op), STRING_MAP_PATH(m), op->x, op->y);
                goto next;
            }

            /* its an invalid animation offset (can trigger wrong memory access)
             * we try to repair it. This is a wrong setting in the arch file or,
             * more common, a map maker bug. */
            if (op->direction < 0 ||
                op->direction >= NUM_FACINGS(op))
            {
                LOG(llevMapbug, "MAPBUG:: %s[>%s< %d %d]: NUM_FACINGS < direction(%d) + state(%d)!\n",
                    STRING_OBJ_NAME(op), STRING_MAP_PATH(m), op->x, op->y,
                    op->direction, op->state);
                op->direction = NUM_FACINGS(op) - 1;
            }

            SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + op->state);
        }

        switch (op->type)
        {
            case FLOOR:
                /* Be sure that floor is a.) always single arch and b.) always uses
                 * "in map" offsets (no multi arch tricks). */
                msp->floor_face = op->face;
                msp->floor_terrain = op->terrain_type;
                msp->floor_direction_block = op->block_movement;
#ifdef USE_TILESTRETCHER
                msp->floor_z = op->z;
#endif

                if (op->last_sp)
                {
                    sint16 v = MAX(-MAX_DARKNESS, MIN(op->last_sp, MAX_DARKNESS));

                    msp->floor_darkness = v;
                    msp->floor_brightness = (v < 0) ? -(brightness[ABS(v)]) : brightness[ABS(v)];
                }

                /* FLAG_CHANGING toggles MSP_FLAG_DAYLIGHT. */
                if (QUERY_FLAG(op, FLAG_CHANGING))
                {
                    if ((m->flags & MAP_FLAG_OUTDOOR))
                    {
                        msp->floor_flags &= ~MSP_FLAG_DAYLIGHT;
                    }
                    else
                    {
                        msp->floor_flags |= MSP_FLAG_DAYLIGHT;
                    }
                }

                /* FLAG_HITBACK toggles MSP_FLAG_PVP. */
                if (QUERY_FLAG(op, FLAG_HITBACK))
                {
                    if ((m->flags & MAP_FLAG_PVP))
                    {
                        msp->floor_flags &= ~MSP_FLAG_PVP;
                    }
                    else
                    {
                        msp->floor_flags |= MSP_FLAG_PVP;
                    }
                }

                if (QUERY_FLAG(op, FLAG_NO_SPELLS))
                {
                    if ((m->flags & MAP_FLAG_NO_SPELLS))
                    {
                        msp->floor_flags &= ~MSP_FLAG_NO_SPELLS;
                    }
                    else
                    {
                        msp->floor_flags |= MSP_FLAG_NO_SPELLS;
                    }
                }

                if (QUERY_FLAG(op, FLAG_NO_PRAYERS))
                {
                    if ((m->flags & MAP_FLAG_NO_PRAYERS))
                    {
                        msp->floor_flags &= ~MSP_FLAG_NO_PRAYERS;
                    }
                    else
                    {
                        msp->floor_flags |= MSP_FLAG_NO_PRAYERS;
                    }
                }

                /* FLAG_NO_ATTACK toggles MSP_FLAG_NO_HARM. */
                if (QUERY_FLAG(op, FLAG_NO_ATTACK))
                {
                    if ((m->flags & MAP_FLAG_NO_HARM))
                    {
                        msp->floor_flags &= ~MSP_FLAG_NO_HARM;
                    }
                    else
                    {
                        msp->floor_flags |= MSP_FLAG_NO_HARM;
                    }
                }

                if (QUERY_FLAG(op, FLAG_NO_PASS))
                {
                    msp->floor_flags |= MSP_FLAG_NO_PASS;
                }

                if (QUERY_FLAG(op, FLAG_PLAYER_ONLY))
                {
                    msp->floor_flags |= MSP_FLAG_PLAYER_ONLY;
                }

                goto next;

            case TYPE_FLOORMASK:
                /* We save floor masks direct over a generic mask arch/object
                 * and don't need to store the direction. A mask will not turn
                 * ingame - thats just for the editor and to have one arch. */
                msp->mask_face = op->face;
                goto next;

            case CONTAINER:
                /* Unlink containers. */
                op->attacked_by = NULL;
                op->attacked_by_count = 0;
                break;

            case SPAWN_POINT:
                /* Link spawns where necessary. */
                if (op->slaying)
                {
                    add_linked_spawn(op);
                }

                break;

            case EXIT:
            case TELEPORTER:
            case PIT:
            case TRAPDOOR:
                /* Check for invalid exit path. */
                if (op->slaying)
                {
                    char buf[MAXPATHLEN];

                    /* Normalize ->slaying and then rewrite it. This is the
                     * original destination path. */
                    (void)normalize_path(m->orig_path, op->slaying, buf);
                    FREE_AND_COPY_HASH(op->slaying, buf);

                    /* If it does not exist, shout about it. */
                    if (check_path(buf, 1) == -1)
                    {
                        LOG(llevMapbug, "MAPBUG:: %s[%s %d %d] destination map %s does not exist!\n",
                            STRING_OBJ_NAME(op), STRING_MAP_PATH(m), op->x,
                            op->y, buf);
                    }
                }

                break;
        }

        /* XXX: This means too - NO double use of ->carrying! */
        op->carrying = (op->inv &&
                        !QUERY_FLAG(op, FLAG_SYS_OBJECT)) ? sum_weight(op) : 0;

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
        }

        insert_ob_in_map(op, m, op, INS_NO_MERGE | INS_NO_WALK_ON);

        if (op->glow_radius)
        {
            adjust_light_source(msp, op->glow_radius);
        }

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
    UpdateMapTiles(m);
    m->flags &= ~MAP_FLAG_NO_UPDATE; /* turn tile updating on again */
    m->in_memory = MAP_MEMORY_ACTIVE;

    /* this is the only place we can insert this because the
    * recursive nature of LoadObjects().
    */
    check_light_source_list(m);
}

/* helper func for LoadObjects()
 * This help function will loop through the map and set the nodes. */
static void UpdateMapTiles(map_t *m)
{
    sint16    x = 0,
              y = 0,
              xl = MAP_WIDTH(m),
              yl = MAP_HEIGHT(m);
    msp_t *msp = &m->spaces[0];

    for (x = 0; x < xl; x++)
    {
        for (y = 0; y < yl; y++)
        {
            msp_update(m, msp++, x, y);
        }
    }
}

#define REMOVE_OBJECT(_O_, _C_) \
    activelist_remove((_O_)); \
    remove_ob((_O_)); \
    if ((_C_)) \
    { \
        check_walk_off((_O_), NULL, MOVE_APPLY_VANISHED | MOVE_APPLY_SAVING); \
    }

#define VALIDATE_NEXT(_T_, _N_, _P_, _L_) \
    if ((_N_) && \
        (QUERY_FLAG((_N_), FLAG_REMOVED) || \
         OBJECT_FREE((_N_)))) \
    { \
        if (!QUERY_FLAG((_T_), FLAG_REMOVED) && \
            !OBJECT_FREE((_T_))) \
        { \
            (_N_) = (_T_)->above; \
        } \
        else if ((_P_)) \
        { \
            (_N_) = (_P_)->above; \
        } \
        else \
        { \
            (_N_) = (_L_); /* should be really rare */ \
        } \
    }

/* This saves all the objects on the map in a (most times) non destructive fashion.
* Except spawn point/mobs and multi arches - see below.
* Modified by MSW 2001-07-01 to do in a single pass - reduces code,
* and we only save the head of multi part objects - this is needed
* in order to do map tiling properly.
* The function/engine is now multi arch/tiled map save - put on the
* map what you like. MT-07.02.04 */
static void SaveObjects(map_t *m, FILE *fp)
{
    static object_t *floor_g = NULL,
                  *fmask_g = NULL;
    uint16         x,
                   y;
    uint8          keep = (m->player_first ||
                           m->perm_load) ? 1 : 0;

    /* FIXME: These 2 checks should/could probably be done once at server
     * init with global_archetypes. */
    /* ensure we have our "template" objects for saving floors & masks */
    if(!floor_g)
    {
        if (!(floor_g = get_archetype("floor_g")))
        {
            LOG(llevError, "ERROR:: Can't find 'floor_g' arch\n");
        }

        insert_ob_in_ob(floor_g, &void_container); /* Avoid gc */
        FREE_AND_CLEAR_HASH(floor_g->name);
    }

    if(!fmask_g)
    {
        if (!(fmask_g = get_archetype("fmask_g")))
        {
            LOG(llevError, "ERROR:: Cant'find 'fmask_g' arch\n");
        }

        insert_ob_in_ob(fmask_g, &void_container); /* Avoid gc */
    }

    for (x = 0; x < MAP_WIDTH(m); x++)
    {
        for (y = 0; y < MAP_HEIGHT(m); y++)
        {
            msp_t *msp = MSP_RAW(m, x, y);
            object_t   *this,
                     *next,
                     *prev,
                     *head;

            /* save first the floor and mask from the node */
            if(msp->floor_face)
            {
                floor_g->terrain_type = msp->floor_terrain;
                floor_g->face = msp->floor_face;
                floor_g->x = x;
                floor_g->y = y;
#ifdef USE_TILESTRETCHER
                floor_g->z = msp->floor_z;
#endif
                floor_g->last_sp = msp->floor_darkness;
                SET_OR_CLEAR_FLAG(floor_g, FLAG_CHANGING, (msp->floor_flags & MSP_FLAG_DAYLIGHT));
                SET_OR_CLEAR_FLAG(floor_g, FLAG_HITBACK, (msp->floor_flags & MSP_FLAG_PVP));
                SET_OR_CLEAR_FLAG(floor_g, FLAG_NO_SPELLS, (msp->floor_flags & MSP_FLAG_NO_SPELLS));
                SET_OR_CLEAR_FLAG(floor_g, FLAG_NO_PRAYERS, (msp->floor_flags & MSP_FLAG_NO_PRAYERS));
                SET_OR_CLEAR_FLAG(floor_g, FLAG_NO_ATTACK, (msp->floor_flags & MSP_FLAG_NO_HARM));
                SET_OR_CLEAR_FLAG(floor_g, FLAG_NO_PASS, (msp->floor_flags & MSP_FLAG_NO_PASS));
                SET_OR_CLEAR_FLAG(floor_g, FLAG_PLAYER_ONLY, (msp->floor_flags & MSP_FLAG_PLAYER_ONLY));

                /* black object magic... don't do this in the "normal" server code */
                save_object(fp, floor_g, 3);
            }

            if(msp->mask_face)
            {
                fmask_g->face = msp->mask_face;
                fmask_g->x = x;
                fmask_g->y = y;
#ifdef USE_TILESTRETCHER
                floor_g->z = msp->floor_z; // FIXME: Why?
#endif
                save_object(fp, fmask_g, 3);
            }


            /* Now we go through every object on the square. Each object falls
             * into one of three categories:
             *
             * (1) player objects;
             * (2) 'dynamic' objects such as spell effects or mobs; or
             * (3) other objects. */
            for (this = msp->first; this; this = next)
            {
                next = this->above;
                prev = this->below;
                head = (this->head) ? this->head : this;

                /* Category (1) player objects.
                 *
                 * The server performs periodic scheduled saves (ie, when no
                 * player has been on the map for X time). Therefore we should
                 * not find such an object. But if we do, no matter, simply
                 * skip over it.
                 *
                 * Scripts can call map:Save() at any time, and we also save a
                 * unique/instanced map if a player /saves in it or when he
                 * leaves (no guarantee he's not invited friends round for tea
                 * and they're still there). So these unscheduled saves are
                 * likely to have players on the map. Again, no matter, we
                 * simply skip. */
                if (this->type == PLAYER)
                {
                    continue;
                }

                /* Category (2) 'dynamic' objects.
                 *
                 * When a map is saved with no players on it it is subsequently
                 * deleted from memory so we'll remove these objects now too.
                 *
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
                 * does an action IMMEDIATELY unless it is a static effect (like we put a wall
                 * in somewhere).
                 *
                 * Absolutely forbidden are dynamic effect like instant spawns of mobs on other maps
                 * point - then our map will get messed up again. Teleporters are a bit critical here
                 * and i fear the code and callings in move_apply() will need some more carefully
                 * examination.
                 *
                 * But when there are players on the map we simply skip over these
                 * object (so they are not saved but do remain in memory). */
                if (!keep)
                {
                    /* here we remove all "dynamic" content which are around on the map.
                    * ATM i remove some spell effects with it.
                    * For things like permanent counterspell walls or something we should
                    * use and create special objects. */
                    if (QUERY_FLAG(head, FLAG_NO_SAVE))
                    {
                        REMOVE_OBJECT(head, 1);
                        VALIDATE_NEXT(this, next, prev, msp->first);

                        continue;
                    }
                    /* here we handle the mobs of a spawn point - called spawn mobs.
                    * We *never* save spawn mobs - not even if they are on the same map.
                    * We remove them and tell the spawn point to generate them new in the next tick.
                    * (In case of the saved map it is the reloading).
                    * If reloaded, the spawn point will restore a new mob of same kind on
                    * the default position. */
                    else if (QUERY_FLAG(head, FLAG_SPAWN_MOB))
                    {
                        /* sanity check for the mob structures & ptr */
                        if (!MOB_DATA(head) ||
                            !MOB_DATA(head)->spawn_info)
                        {
                            LOG(llevBug, "BUG:: %s:SaveObjects(): Spawn mob %s %s [%d] without SPAWN INFO %s or MOB_DATA %p.\n",
                                __FILE__, STRING_OBJ_ARCH_NAME(head),
                                STRING_OBJ_NAME(head), TAG(head),
                                (MOB_DATA(head)) ? STRING_OBJ_NAME(MOB_DATA(head)->spawn_info) : ">NULL<",
                                (MOB_DATA(head)) ? MOB_DATA(head) : NULL);
                        }
                        else
                        {
                            object_t *info = MOB_DATA(head)->spawn_info;

                            /* spawn info is ok - check the spawn point attached to it */
                            if (info->owner &&
                                info->owner->type == SPAWN_POINT)
                            {
                                /* Found spawn point. Tell the source spawn point to respawn this deleted object.
                                * It can be here OR on a different map. */
                                info->owner->stats.sp = info->owner->last_sp; /* force a pre spawn setting */
                                info->owner->speed_left += 1.0f; /* we force a active spawn point */
                                info->owner->enemy = NULL;
                            }
                            else
                            {
                                LOG(llevBug, "BUG:: %s:SaveObjects(): Spawn mob %s %s [%d] has SPAWN INFO with illegal owner: %s [%d]!\n",
                                    __FILE__, STRING_OBJ_ARCH_NAME(head),
                                    STRING_OBJ_NAME(head), TAG(head),
                                    STRING_OBJ_NAME(info->owner), TAG(info->owner));
                            }
                        }

                        /* and remove the mob itself */
                        REMOVE_OBJECT(head, 1);
                        VALIDATE_NEXT(this, next, prev, msp->first);

                        continue;
                    }
                    /* This is for homeless mobs. */
                    else if (QUERY_FLAG(head, FLAG_HOMELESS_MOB))
                    {
                        REMOVE_OBJECT(head, 1);
                        VALIDATE_NEXT(this, next, prev, msp->first);

                        continue;
                    }
                    /* Fired/thrown items are not saved. */
                    else if (QUERY_FLAG(head, FLAG_IS_MISSILE))
                    {
                        REMOVE_OBJECT(head, 1);
                        VALIDATE_NEXT(this, next, prev, msp->first);

                        continue;
                    }
                    else if (head->type == SPAWN_POINT)
                    {
                        /* If the spawn point has spawned... */
                        if (head->enemy &&
                            head->enemy_count == head->enemy->count &&
                            !QUERY_FLAG(head->enemy, FLAG_REMOVED) &&
                            !OBJECT_FREE(head->enemy))
                        {
                             /* ...Set the pre-spawn value to last mob so we
                              * restore our creature when we reload the map. */
                            head->stats.sp = head->last_sp;
                            head->speed_left += 1.0f;

                            /* ...If the spawn is on the same map as the spawn
                             * point, delete the mob. Otherwise, turn him into
                             * a homeless mob. */
                            if (head->enemy->map == head->map)
                            {
                                REMOVE_OBJECT(head->enemy, 1);
                                VALIDATE_NEXT(this, next, prev, msp->first);
                                head->enemy = NULL;
                                head->enemy_count = 0;
                            }
                            else
                            {
                                make_mob_homeless(head->enemy);
                            }
                        }
                    }

                    /* TODO: Needs thought.
                     * -- Smacky 20120725 */
                    /* we will delete here all temporary owner objects.
                    * We talk here about spell effects, pets, golems and
                    * other "dynamic" objects.
                    * What NOT should be deleted are throw objects and other
                    * permanent items which has a owner setting! (if they have) */
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
                        * manipulate the map here as more secure we are! */
                        if (head->type == GOLEM) /* a golem needs a valid release from the player... */
                        {
                            send_golem_control(head, GOLEM_CTR_RELEASE);
                            REMOVE_OBJECT(head, 1);
                            VALIDATE_NEXT(this, next, prev, msp->first);

                            continue;
                        }

                        LOG(llevDebug, "DEBUG:: %s:SaveObjects(): obj with owner (%s [%d]) on map >%s< (%d %d)\n",
                            __FILE__, STRING_OBJ_NAME(this), TAG(this),
                            STRING_MAP_PATH(m), this->x, this->y);
                        head->owner = NULL;

                        continue;
                    }
                }
                else
                {
                    if (QUERY_FLAG(head, FLAG_NO_SAVE) ||
                        QUERY_FLAG(head, FLAG_SPAWN_MOB) ||
                        QUERY_FLAG(head, FLAG_HOMELESS_MOB) ||
                        QUERY_FLAG(head, FLAG_IS_MISSILE) ||
                        head->owner)
                    {
                        continue;
                    }
                }

                /* Category (3) other objects.
                 *
                 * These objects can be saved, although for multiparts we
                 * first do some magic.
                 *
                 * The magic is, that when we find a tail, we
                 * save the head and give it the x/y
                 * position basing on this tail position and its
                 * x/y clone arch default multi tile offsets!
                 * With this trick, we even can generate negative
                 * map positions - and thats exactly what we want
                 * when our head is on a different map as this tail!
                 * insert_ob() and the map loader will readjust map and
                 * positions and load the other map when needed!
                 * we must save x/y or remove_ob() will fail.
                 * */
                if (!keep)
                {
                    if (this->head) // its a tail...
                    {
                        int x2,
                            y2;

                        x2 = head->x;
                        y2 = head->y;
                        head->x = this->x - this->arch->clone.x;
                        head->y = this->y - this->arch->clone.y;
                        save_object(fp, head, 3);
                        head->x = x2;
                        head->y = y2;

                        /* remember: if we have remove for example 2 or more objects above, the
                         * this->above WILL be still valid - remove_ob() will handle it right.
                         * IF we get here a valid ptr, ->above WILL be valid too. Always. */
                        REMOVE_OBJECT(head, 0);
                        VALIDATE_NEXT(this, next, prev, msp->first);

                        continue;
                    }

                    save_object(fp, this, 3);

                    if (this->more) // its a head (because we had tails tested before)
                    {
                        REMOVE_OBJECT(this, 0);
                        VALIDATE_NEXT(this, next, prev, msp->first);
                    }
                }
                else
                {
                    if (this->head) // its a tail...
                    {
                        int x2,
                            y2;

                        x2 = head->x;
                        y2 = head->y;
                        head->x = this->x - this->arch->clone.x;
                        head->y = this->y - this->arch->clone.y;
                        save_object(fp, head, 3);
                        head->x = x2;
                        head->y = y2;

                        continue;
                    }

                    save_object(fp, this, 3);
                }
            } /* for this space */
        } /* for this y */
    }
}

#undef REMOVE_OBJECT
#undef VALIDATE_NEXT

/* Remove all players from a map and set a marker. If path_sh is non-NULL, this
 * is the path of the map the player will subsequently be linked to.
 *
 * XXX: map_player_link() MUST be called after this or the server has a
 * problem. */
uint16 map_player_unlink(map_t *m, shstr_t *path_sh)
{
    object_t *op;
    uint16  num = 0;

    if (!m)
    {
        return 0;
    }

    while ((op = m->player_first))
    {
        /* With the new activelist, any player on a reset map
         * was somehow forgotten. This seems to fix it. The
         * problem isn't analyzed, though. Gecko 20050713 */
        activelist_remove(op);
        remove_ob(op); // changes m->player_first
        FREE_AND_ADD_REF_HASH(CONTR(op)->temp_removal_map,
                              (path_sh) ? path_sh : m->path);
        num++;
    }

    return num;
}

/* Reinsert players on a map after they were removed with map_player_unlink().
 * If m is NULL use the savebed (or if flag, the emergency map). Otherwise, if
 * x or y == -1 they will overrule the player map position.
 *
 * If m is NULL then *all* temp removed players, regardless of where they were
 * removed from, will be restored to their savebed or the emergency map. */
void map_player_link(map_t *m, sint16 x, sint16 y, uint8 flag)
{
    player_t *pl;

    for (pl = first_player; pl; pl = pl->next)
    {
        uint8 relinked = 0;

        if (pl->temp_removal_map)
        {
            if (m)
            {
                if (pl->temp_removal_map == m->path)
                {
                    sint16    x2 = (x == -1) ? pl->ob->x : x,
                              y2 = (y == -1) ? pl->ob->y : y;
                    msp_t *msp = MSP_RAW(m, x2, y2);

                    (void)enter_map(pl->ob, msp, NULL, OVERLAY_FIRST_AVAILABLE | OVERLAY_FIXED, INS_NO_MERGE | INS_NO_WALK_ON);
                    relinked = 1;
                }
            }
            else if (!flag)
            {
                (void)enter_map_by_name(pl->ob, pl->savebed_map, pl->orig_savebed_map,
                    pl->bed_x, pl->bed_y, pl->bed_status);
                relinked = 1;
            }
            else
            {
                (void)enter_map_by_name(pl->ob, shstr_cons.emergency_mappath, shstr_cons.emergency_mappath,
                     -1, -1, MAP_STATUS_MULTI);
                relinked = 1;
            }

            if (relinked)
            {
                FREE_AND_CLEAR_HASH(pl->temp_removal_map);
                ndi(NDI_UNIQUE, 0, pl->ob, "You have a distinct feeling of deja vu.");
            }
        }
    }
}

#ifdef RECYCLE_TMP_MAPS
/* This writes out information on all the temporary maps.  It is called by
 * swap_map below. */
/* TODO: Don't know if thid works (current servers do not recycle).
 * -- Smacky 20120726 */
static void WriteMapLog(void)
{
    FILE       *fp;
    map_t  *map;
    char        buf[MEDIUM_BUF];

    sprintf(buf, "%s/temp.maps", settings.localdir);
    if (!(fp = fopen(buf, "w")))
    {
        LOG(llevBug, "BUG: Could not open %s for writing\n", buf);
        return;
    }
    for (map = first_map; map != NULL; map = map->next)
    {
        /* If tmpname is null, it is probably a unique player map,
         * so don't save information on it.
         */
        if (map->in_memory != MAP_MEMORY_ACTIVE &&
            map->tmpname &&
            strncmp(map->path, "/random", 7))
        {
            /* the 0 written out is a leftover from the lock number for
               * unique items and second one is from encounter maps.
               * Keep using it so that old temp files continue
               * to work.
               */
            fprintf(fp, "%s:%s:%ld:0:0:%d:0:%d\n",
                    map->path, map->tmpname, MAP_RESET_TIMEOUT(map),
                    map->difficulty, map->ambient_darkness);
        }
    }
    fclose(fp);
}
#endif

void read_map_log(void)
{
    FILE *fp;
    char  buf[MEDIUM_BUF];

    sprintf(buf, "%s/temp.maps", settings.localdir);

    if (!(fp = fopen(buf, "r")))
    {
        LOG(llevInfo, "INFO:: Could not open %s for reading!\n", buf);

        return;
    }

    while (fgets(buf, MEDIUM_BUF, fp))
    {
        char      *cp,
                  *cp1;
        map_t *m = GetLinkedMap();
        int        do_los,
                   darkness,
                   difficulty,
                   lock;

        /* scanf doesn't work all that great on strings, so we break
         * out that manually.  strdup is used for tmpname, since other
         * routines will try to free that pointer. */
        cp = strchr(buf, ':');
        *cp++ = '\0';
        FREE_AND_COPY_HASH(m->path, buf);
        cp1 = strchr(cp, ':');
        *cp1++ = '\0';
        MALLOC2(m->tmpname, cp);

        /* Lock is left over from the lock items - we just toss it now.
         * We use it twice - second one is from encounter, but as we
         * don't care about the value, this works fine. */
        sscanf(cp1, "%d:%d:%d:%d:%d:%d\n",
               &MAP_RESET_TIMEOUT(m), &lock, &lock, &difficulty, &do_los, &darkness);

        m->in_memory = MAP_MEMORY_SWAPPED;
        m->difficulty = difficulty;
        m->ambient_darkness = darkness;
        m->ambient_brightness = brightness[ABS(darkness)];
    }

    fclose(fp);
}

/* if on the map and the direct attached maps no player and no perm_load
 * flag set, we can safely swap them out! */
void swap_map(map_t *map, int force_flag)
{
    /* lets check some legal things... */
    if (map->in_memory != MAP_MEMORY_ACTIVE &&
        map->in_memory != MAP_MEMORY_SAVING)
    {
        LOG(llevBug, "BUG:: %s/swap_map(): Tried to swap out map which was not in memory: %s!\n",
            __FILE__, STRING_MAP_PATH(map));

        return;
    }

    if (!force_flag) /* test for players! */
    {
        if (map->player_first ||
            map->perm_load) /* player nor perm_loaded marked */
        {
            force_flag = 1;
        }
        else
        {
            uint8 i;

            for (i = 0; i < TILING_DIRECTION_NROF; i++)
            {
                /* if there is a map, is load AND in memory and players on OR perm_load flag set, then... */
                if (map->tiling.tile_map[i] &&
                    map->tiling.tile_map[i]->in_memory == MAP_MEMORY_ACTIVE &&
                    (map->tiling.tile_map[i]->player_first ||
                     map->tiling.tile_map[i]->perm_load))
                {
                    force_flag = 1;

                    break;
                }
            }
        }

        /* If force_flag has been set, do not swap the map. */
        if (force_flag)
        {
#ifdef DEBUG_MAP
            LOG(llevDebug, "DEBUG:: %s/swap_map(): Map >%s< is still busy so will not be swapped!\n",
                __FILE__, STRING_MAP_PATH(map));

#endif
            return;
        }
    }

    /* when we are here, map is save to swap! */
    remove_all_pets(map); /* Give them a chance to follow */

    /* Update the reset time.  Only do this is STAND_STILL is not set */
    if (!MAP_FIXED_RESETTIME(map))
    {
        MAP_SET_WHEN_RESET(map, MAP_RESET_TIMEOUT(map));
    }

    /* Only save the map if we do reset maps and it is immediate reset time, or
     * if we don't reset maps at all (so saved maps are very important to
     * persistency). */
    if (
#ifdef MAP_RESET
        !MAP_WHEN_RESET(map) ||
         MAP_WHEN_RESET(map) > (ROUND_TAG - ROUND_TAG % (long unsigned int)MAX(1, pticks_second)) / pticks_second
#else
        1
#endif
        )
    {
        if (map->in_memory != MAP_MEMORY_SAVING && // do not save twice
            !map_save(map))
        {
            LOG(llevBug, "BUG:: %s/swap_map(): Failed to swap map!\n", __FILE__);

            return;
        }
        else
        {
            FreeMap(map);
        }

#ifdef RECYCLE_TMP_MAPS
        WriteMapLog();
#endif
    }
}

/* Count the player on a map, using the local map player list. */
int players_on_map(map_t *m)
{
    object_t *tmp = m->player_first;
    int     count = 0;

    for (; tmp; tmp = CONTR(tmp)->map_above)
    {
        count++;
    }

    return count;
}

/* Go through the list of maps, swapping or resetting those that need it. */
/*TODO: This will be the one and only function to handle map swaps/resets, but
 * ATM this functionality is still scattered through the code a bit.
 *
 * -- Smacky 20120810 */
void map_check_in_memory(map_t *m)
{
    map_t *this,
          *next;
#ifdef MAP_SWAP_OBJECT
    static uint32 threshold = MAP_MAXOBJECTS;
#endif

    for (this = (m) ? m : first_map; this; this = (!m) ? next : NULL)
    {
        next = this->next;

        /* When doing a manual reset no need to look at player activity or swap
         * the maps. */
        if (!(this->status & MAP_STATUS_MANUAL_RESET))
        {
            /* When there are players or (TODO) permanently loading mobs on the
             * map, do not swap/reset but reset the times. */
            if (this->player_first ||
                this->perm_load)
            {
                MAP_SET_WHEN_SWAP(this, MAP_SWAP_TIMEOUT(this));

                if (!MAP_FIXED_RESETTIME(this))
                {
                    MAP_SET_WHEN_RESET(this, MAP_RESET_TIMEOUT(this));
                }

                continue;
            }

            /* Map may need swapping. */
            if (this->in_memory == MAP_MEMORY_ACTIVE ||
                this->in_memory == MAP_MEMORY_SAVING)
            {
#ifdef MAP_SWAP_OBJECT
                sint32 objs = (sint32)(pool_object->nrof_allocated[0] - pool_object->nrof_free[0]);

                if (objs <= (sint32)threshold)
                {
                    threshold = MAP_MAXOBJECTS;
                }
                else
                {
                    uint8 i,
                          noswap = 0;

                    threshold = MAP_MINOBJECTS;

                    /* TODO: The old code used to loop through the map list again
                     * to try to find the map nearest it's actual swap time and
                     * swap that one out. Perhaps more sensibly, given the intent
                     * here, we should swap out the map with the most objects on it
                     * first (though ATM map_t does not keep a tally). For now
                     * anyway we just swap out this.
                     *
                     * -- Smacky 20120811 */
#else
                /* So it's swap time. */
                if (MAP_WHEN_SWAP(this) <= (ROUND_TAG - ROUND_TAG %
                                            (long unsigned int)MAX(1, pticks_second)) /
                                            pticks_second)
                {
                    uint8 i,
                          noswap = 0;

#endif
                    /* Check adjacent maps for players or (TODO) permanently
                     * loading mobs. */
                    for (i = 0; i < TILING_DIRECTION_NROF; i++)
                    {
                        if (this->tiling.tile_map[i] &&
                            this->tiling.tile_map[i]->in_memory == MAP_MEMORY_ACTIVE &&
                            (this->tiling.tile_map[i]->player_first ||
                             this->tiling.tile_map[i]->perm_load))
                        {
                            noswap = 1;

                            break;
                        }
                    }

                    /* Now swap the map. */
                    if (!noswap)
                    {
#ifdef MAP_SWAP_OBJECT
# ifdef DEBUG_MAP
                        LOG(llevDebug, "DEBUG:: %s/map_check_in_memory(): Swapping map >%s< (%u) before its time (%u of %u).\n", 
                            __FILE__, STRING_MAP_PATH(this), this->in_memory, objs,
                            threshold);
# endif
                        swap_map(this, 1);
                        object_gc(); // keep mempool uptodate
#else
# ifdef DEBUG_MAP
                        LOG(llevDebug, "DEBUG:: %s/map_check_in_memory(): Swapping map >%s< (%u)!\n",
                            __FILE__, STRING_MAP_PATH(this), this->in_memory);
# endif
                        swap_map(this, 1);
#endif
                    }
                }
            }
        }

        /* When doing a delayed manual reset, give a countdown to everyone on
         * the map. */
        if ((this->status & MAP_STATUS_MANUAL_RESET) &&
            MAP_WHEN_RESET(this))
        {
            sint32 countdown = MAP_WHEN_RESET(this) - (ROUND_TAG - ROUND_TAG %
                                                       (uint32)MAX(1, pticks_second)) /
                                                      pticks_second;

             if (countdown > 0 &&      // not for 0
                 (!(countdown % 30) || // every 30s
                  countdown <= 10))    // every s for the last 10
            {
                ndi_map(NDI_UNIQUE | NDI_NAVY, MSP_RAW(this, 0, 0), MAP_INFO_ALL, NULL, NULL, "Only ~%d~ second%s to map reset!",
                    countdown, (countdown != 1) ? "s" : "");
            }
        }

        /* Map may need resetting. */
        if ((this->status & MAP_STATUS_MANUAL_RESET) ||
            this->in_memory == MAP_MEMORY_SWAPPED)
        {
            /* per player unique maps are never really reset.  However, we do want
             * to perdiocially remove the entries in the list of active maps - this
             * generates a cleaner listing if a player issues the map commands, and
             * keeping all those swapped out per player unique maps also has some
             * memory and cpu consumption.
             * We do the cleanup here because there are lots of places that call
             * swap map, and doing it within swap map may cause problems as
             * the functions calling it may not expect the map list to change
             * underneath them. */
            if (((this->status & MAP_STATUS_UNIQUE) ||
                 (this->status & MAP_STATUS_INSTANCE)) ||
                ((this->status & MAP_STATUS_MANUAL_RESET) ||
                 this->tmpname) &&
                MAP_WHEN_RESET(this) &&
                MAP_WHEN_RESET(this) <= (ROUND_TAG - ROUND_TAG %
                                          (long unsigned int)MAX(1, pticks_second)) /
                                          pticks_second)
            {
                /* On a manual reset we may want to immediately reload the
                 * source map which also means we need to juggle any players on
                 * it. */
                if ((this->status & MAP_STATUS_MANUAL_RESET))
                {
#ifdef DEBUG_MAP
                    LOG(llevDebug, "DEBUG:: %s/map_check_in_memory(): Manually resetting%s map >%s<!\n",
                        __FILE__, ((this->status & MAP_STATUS_RELOAD)) ? " and reloading" : "",
                        STRING_MAP_PATH(this));

#endif
                    /* We need to 'save' the objects on map so spawns from
                     * other maps disappear and the spawn point on the other
                     * map knows to respawn, etc. On uniques/instances we in
                     * fact do save the map to disk to prevent item loss. But
                     * on multis there is no need for slow disk access (we're
                     * resetting to source remember) so in fact we're just
                     * pruning such dynamic objects. */
                    if ((this->status & MAP_STATUS_UNIQUE) ||
                        (this->status & MAP_STATUS_INSTANCE))
                    {
                        if (!map_save(this))
                        {
                            continue;
                        }
                    }
                    else
                    {
                        SaveObjects(this, NULL);
                    }

                    if ((this->status & MAP_STATUS_RELOAD))
                    {
                        uint8  anyplayers;
                        shstr_t *path_sh = NULL,
                              *orig_path_sh = NULL,
                              *reference_sh = NULL;
                        uint32 status;

                        /* If there are any players on the map, temp remove them. */
                        if ((anyplayers = (this->player_first) ? 1 : 0))
                        {
                            (void)map_player_unlink(this, NULL);
                        }

                        /* Remember a few details so we can reload the map. */
                        FREE_AND_ADD_REF_HASH(path_sh, this->path);
                        FREE_AND_ADD_REF_HASH(orig_path_sh, this->orig_path);

                        if (this->reference)
                        {
                            FREE_AND_ADD_REF_HASH(reference_sh, this->reference);
                        }

                        status = MAP_STATUS_TYPE(this->status);

                        /* Delete the map from memory and reload it. */
                        clean_tmp_map(this);
                        delete_map(this);
                        this = ready_map_name(path_sh, orig_path_sh, status,
                                              reference_sh);

                        /* Put any removed players back on the map (or emergency if
                         * for some reason it has failed to be ready).*/
                        if (anyplayers)
                        {
                            map_player_link(this, -1, -1, 1);
                        }

                        /* Tidy up. */
                        FREE_ONLY_HASH(path_sh);
                        FREE_ONLY_HASH(orig_path_sh);
                        FREE_ONLY_HASH(reference_sh);
                    }
                    else
                    {
                        uint8 anyplayers;

                        /* If there are any players on the map, temp remove them. */
                        if ((anyplayers = (this->player_first) ? 1 : 0))
                        {
                            (void)map_player_unlink(this,
                                                    shstr_cons.emergency_mappath);
                        }

                        /* Delete the map from memory. */
                        clean_tmp_map(this);
                        delete_map(this);

                        /* Put any removed players back on the emergency map - why
                         * emergency? Because our bind point CAN be the same map we
                         * just reset!
                         *
                         * By explicitly readying emergency here we ensure that
                         * only THESE players are relinked. */
                        if (anyplayers)
                        {
                            map_t *m = ready_map_name(shstr_cons.emergency_mappath,
                                                          shstr_cons.emergency_mappath,
                                                          MAP_STATUS_MULTI, NULL);

                            map_player_link(m, -1, -1, 1);
                        }
                    }
                }
                /* Otherwise we know there's no-one here, so just delete it. */
                else
                {
#ifdef DEBUG_MAP
                    LOG(llevDebug, "DEBUG:: %s/map_check_in_memory(): Resetting map >%s<!\n",
                        __FILE__, STRING_MAP_PATH(this));
#endif
                    clean_tmp_map(this);
                    delete_map(this);
                }
            }
        }
    }
}

/* transfer all items from one instance apartment to another.
* put them on spot x,y
*/
void map_transfer_apartment_items(map_t *mold, map_t *mnew, sint16 xnew, sint16 ynew)
{
    sint16    xold,
              yold;

    for (xold = 0; xold < mold->width; xold++)
    {
        for (yold = 0; yold < mold->height; yold++)
        {
            msp_t    *msp = MSP_RAW(mold, xold, yold);
            object_t *this,
                     *next;

            FOREACH_OBJECT_IN_MSP(this, msp, next)
            {
                /* The player can't get it so no sense to transfer it! Also,
                 * because only other system objects can be below this one, we
                 * may as well break the loop and move to the next msp. */
                if (QUERY_FLAG(this, FLAG_SYS_OBJECT))
                {
                    break;
                }

                /* It is not nailed down, so take it with us! */
                if (!QUERY_FLAG(this, FLAG_NO_PICK))
                {
                    remove_ob(this);
                    this->x = xnew;
                    this->y = ynew;
                    insert_ob_in_map(this, mnew, NULL, INS_NO_MERGE | INS_NO_WALK_ON);
                }
                /* This container is a fixed part of the map but lets take its
                 * contents. */
                else if (this->type == CONTAINER)
                {
                    object_t *that,
                             *next2;

                    FOREACH_OBJECT_IN_OBJECT(that, this, next2)
                    {
                        /* well, non pickup container in non pickup container? no no... */
                        if (QUERY_FLAG(that, FLAG_SYS_OBJECT) ||
                            QUERY_FLAG(that, FLAG_NO_PICK))
                        {
                            continue;
                        }

                        remove_ob(that);
                        that->x = xnew;
                        that->y = ynew;
                        insert_ob_in_map(that, mnew, NULL, INS_NO_MERGE | INS_NO_WALK_ON);
                    }
                }
            }
        }
    }
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

static uint32 IsBlocked(msp_t *msp, object_t *what);

/* Sets the slayers for msp according to various attributes of op. If insert is
 * non-zero, this means op has been or is being newly (re)inserted so will
 * become the new head of the relevant slayers. Otherwise, op has been or is
 * being removed from the square so if it was the head of a slayer that slayer
 * will be reassigned to the next appropriate object on the square (or NULL).
 *
 * Each square has three slayer arrays: visible, invisible, and gmaster.
 * Visible is for visible objects only. Invisible is for visible and
 * invisible objects. Gmaster is for visible, invisible, and gmaster_invis
 * objects. This means gmaster_layer is the true representation of the head of
 * the list of objects in a square. */
/* TODO: Rewrite these comments (these refer to an old single-function,
 * pre-slice version of the following). */
void msp_rebuild_slices_without(msp_t *msp, object_t *op)
{
    msp_slayer_t  s = op->layer - MSP_SLAYER_UNSLICED - 1;
    object_t     *this,
                 *next;

    if (!IS_GMASTER_INVIS(op))
    {
        if (!QUERY_FLAG(op, FLAG_IS_INVISIBLE))
        {
            if (msp->slayer[MSP_SLICE_VISIBLE][s] == op)
            {
                /* Cannot guarantee the order of objects beyond that
                 * MSP_SLAYER_SYSTEM layers are at the ->first end of the msp
                 * and other layers are at the ->last end. So browse from
                 * ->last until we find a MSP_SLAYER_SYSTEM. */
                FOREACH_OBJECT_IN_MSP(this, msp, next)
                {
                    if (this->layer == MSP_SLAYER_SYSTEM)
                    {
                        this = NULL;
                    }
                    else if (this == op)
                    {
                        continue;
                    }
                    else if (this->layer == op->layer &&
                             !IS_GMASTER_INVIS(this) &&
                             !QUERY_FLAG(this, FLAG_IS_INVISIBLE))
                    {
                        break;
                    }
                }

                msp->slayer[MSP_SLICE_VISIBLE][s] = this;
                msp->slices_synced &= ~(1 << MSP_SLICE_VISIBLE);
            }
        }

        if (msp->slayer[MSP_SLICE_INVISIBLE][s] == op)
        {
            FOREACH_OBJECT_IN_MSP(this, msp, next)
            {
                if (this->layer == MSP_SLAYER_SYSTEM)
                {
                    this = NULL;
                }
                else if (this == op)
                {
                    continue;
                }
                else if (this->layer == op->layer &&
                         !IS_GMASTER_INVIS(this))
                {
                    break;
                }
            }

            msp->slayer[MSP_SLICE_INVISIBLE][s] = this;
            msp->slices_synced &= ~(1 << MSP_SLICE_INVISIBLE);
        }
    }

    if (msp->slayer[MSP_SLICE_GMASTER][s] == op)
    {
        FOREACH_OBJECT_IN_MSP(this, msp, next)
        {
            if (this->layer == MSP_SLAYER_SYSTEM)
            {
                this = NULL;
            }
            else if (this == op)
            {
                continue;
            }
            else if (this->layer == op->layer &&
                     !IS_GMASTER_INVIS(this))
            {
                break;
            }
        }

        msp->slayer[MSP_SLICE_GMASTER][s] = this;
        msp->slices_synced &= ~(1 << MSP_SLICE_GMASTER);
    }
}

void msp_rebuild_slices_with(msp_t *msp, object_t *op)
{
    msp_slayer_t  s = op->layer - MSP_SLAYER_UNSLICED - 1;

    if (!IS_GMASTER_INVIS(op))
    {
        if (!QUERY_FLAG(op, FLAG_IS_INVISIBLE))
        {
            msp->slayer[MSP_SLICE_VISIBLE][s] = op;
            msp->slices_synced &= ~(1 << MSP_SLICE_VISIBLE);
        }

        msp->slayer[MSP_SLICE_INVISIBLE][s] = op;
        msp->slices_synced &= ~(1 << MSP_SLICE_INVISIBLE);
    }

    msp->slayer[MSP_SLICE_GMASTER][s] = op;
    msp->slices_synced &= ~(1 << MSP_SLICE_GMASTER);
}

/* This function updates various attributes about a specific space
 * on the map (what it looks like, whether it blocks magic,
 * has a living creatures, prevents people from passing
 * through, etc) */
void msp_update(map_t *m, msp_t *mspace, sint16 x, sint16 y)
{
    object_t   *tmp;
    msp_t *msp;
    uint32    flags,
              oldflags;

    if (mspace)
    {
        msp = mspace;
        flags = oldflags = (MSP_FLAG_NO_ERROR | MSP_FLAG_UPDATE);
    }
    else
    {
        msp = MSP_GET2(m, x, y);
        flags = 0;
        oldflags = msp->flags;

        if (!(oldflags & MSP_FLAG_UPDATE))
        {
            LOG(llevInfo, "INFO:: msp_update(): MSP_FLAG_UPDATE not set: %s (%d, %d)\n",
                m->path, x, y);
        }
    }


    /* update our flags */
    if (oldflags & MSP_FLAG_UPDATE)
    {
#ifdef DEBUG_CORE
        LOG(llevDebug, "UP - FLAGS: %d,%d\n", x, y);
#endif
        /*LOG(llevDebug,"flags:: %x (%d, %d) NE:%x\n", oldflags, x, y,MSP_FLAG_NO_ERROR);*/

        /* This is a key function and highly often called - every saved tick is good. */
        flags |= msp->floor_flags;
        msp->move_flags |= msp->floor_terrain;

        for (tmp =msp->first; tmp; tmp = tmp->above)
        {
            if (QUERY_FLAG(tmp, FLAG_NO_PASS))
            {
                /* we also handle PASS_THRU here...
                * a.) if NO_PASS is set before, we test for PASS_THRU
                * - if we have no FLAG_PASS_THRU, we delete PASS_THRU
                * - if we have FLAG_PASS_THRU, we do nothing - other object blocks always
                * b.) if no NO_PASS is set, we set it AND set PASS_THRU if needed
                */
                if (flags & MSP_FLAG_NO_PASS)
                {
                    if (!QUERY_FLAG(tmp, FLAG_PASS_THRU))
                        flags &= ~MSP_FLAG_PASS_THRU; /* just fire it... always true */
                    if (!QUERY_FLAG(tmp, FLAG_PASS_ETHEREAL))
                        flags &= ~MSP_FLAG_PASS_ETHEREAL; /* just fire it... always true */
                }
                else
                {
                    flags |= MSP_FLAG_NO_PASS;
                    if (QUERY_FLAG(tmp, FLAG_PASS_THRU))
                        flags |= MSP_FLAG_PASS_THRU;
                    if (QUERY_FLAG(tmp, FLAG_PASS_ETHEREAL))
                        flags |= MSP_FLAG_PASS_ETHEREAL;
                }
            }

            MSP_SET_FLAGS_BY_OBJECT(flags, tmp);
        }

        if ((oldflags & ~(MSP_FLAG_UPDATE | MSP_FLAG_NO_ERROR)) != flags &&
            !(oldflags & MSP_FLAG_NO_ERROR))
        {
            LOG(llevDebug, "DBUG: msp_update: updated flags do not match old flags: %s (%d,%d) old:%x != %x\n",
                m->path, x, y, oldflags, flags);
        }

        msp->flags = flags;
    } /* end flag update */
}

/* msp_blocked() returns non-zero (some combination of MSP_FLAG_FOO flags) if the
 * specified location is blocked in some way, or zero if it is not.
 *
 * In fact, despite the function name, msp_blocked() does not take a msp as a
 * parameter. In fact, it takes an object, what, a map_t, m, and two
 * coordinates, x and y.
 *
 * what may be a singlepart or a multipart or NULL. If a multipart it must be
 * the head of the multipart; to save on redundant checks, it is the
 * responsibility of the caller to ensure this is so.
 *
 * m may be a map or NULL (as long as what is neither NULL nor an object not
 * currently on a map).
 *
 * x and y are numbers. If m is not NULL, these are absolute coordinates to an
 * msp within m (or on a tiled map). If m is NULL, these are offsets from
 * what's current position (that is the msp at what->map, what->x, what->y).
 *
 * Therefore this function can query if what (non-NULL) can legally occupy a
 * specified msp, or more generally (what is NULL) if a specified msp is
 * blocked or not.
 *
 * When what is non-NULL, an offset (m is NULL) is used to query possible
 * movement of what from msp A to B (usually, but not necessarily, two directly
 * adjacent msps) on the same/tiled maps, while absolute coordinates (m is
 * non-NULL) are used to query possible placement of what at an arbitrary msp
 * on an arbitrary map regardless of where what was before (typically for
 * example when what emerges from an exit to a new map node).
 *
 * Why? Essentially this is so we can treat multiparts the same as singleparts;
 * the code handles them internally as efficiently as possible. While offsets
 * only really make an efficiency gain for multiparts and in fact absolute
 * coordinates will work (less efficiently) in lieu, you should nevertheless
 * use either calling mode whether what is singlepart or multipart as outlined
 * in the previous paragraph. */
/* why is controlling the own arch clone offsets with the overlay_x/y[]
 * offsets a good thing?
 * a.) we don't must check any flags for tiles where we was before
 * b.) we don't block in moving when we got teleported in a no_pass somewhere
 * c.) no call to out_of_map() needed for all parts
 * d.) no checks of objects in every tile node of the multi arch
 * e.) no recursive call needed anymore
 * f.) the multi arch are handled in maps like the single arch
 * g.) no scaling by heavy map action when we move (more objects
 *     on the map don't interest us anymore here)
 * .  This is used with
 * multipart monsters - if we want to see if a 2x2 monster
 * can move 1 space to the left, we don't want its own area
 * to block it from moving there.
 * If <map> is NULL, <x> and <y> are taken as offsets, else absolute values.
 * Returns TRUE if the space is blocked by something other than the
 * monster. */
uint32 msp_blocked(object_t *what, map_t *m, sint16 x, sint16 y)
{
    uint32 block = 0;

   /* Here we deal with multiparts (remember, the calling function must ensure
    * what is ->head). */
    if (what &&
        what->more)
    {
        object_t *part,
               *next;

        /* Step through each part of what, setting x2 and y2 to the part's arch
         * x and y offsets plus the x and y parameters passed to this
         * function. The pointer part is the old (current) position of that
         * part of the object, and x2/y2 are the coordinates of where that part
         * is hoping to move to. */
        FOREACH_PART_OF_OBJECT(part, what, next)
        {
            map_t *m2;
            sint16     x2 = part->arch->clone.x + x,
                       y2 = part->arch->clone.y + y;
            object_t    *part2,
                      *next2;

            /* Now step through each part of what again. The pointer part2 is
             * still the old (current) position of that part of the object.
             * Remember, this function does not actually relocate the object
             * but checks if such an action would be legal. So part2 represents
             * an overlapping part of the object between its old (current)
             * position and the new (proposed) position. */
            /* If m is NULL, x and y are offsets so break if x2 and y2
             * are equal to part2's arch offsets. This means that the
             * proposed move would cause part to overlap part2. */
            if (!m)
            {
                FOREACH_PART_OF_OBJECT(part2, what, next2)
                {
                    if (x2 == part2->arch->clone.x &&
                        y2 == part2->arch->clone.y)
                    {
                        break;
                    }
                }
            }
            else // x and y are absolute
            {
                FOREACH_PART_OF_OBJECT(part2, what, next2)
                {
                    if (1 &&//m == what->map &&
                        x2 == part->x &&
                        y2 == part->y)
                    {
                        break;
                    }
                }
            }

            if (!part2) /* if this is NULL, part will move in a new node */
            {
                msp_t *msp;

                if (!m)
                {
                    m2 = what->map;
                    x2 = part->x + x;
                    y2 = part->y + y;
                    msp = MSP_GET2(m2, x2, y2);
                }
                else
                {
                    msp = MSP_GET2(m, x, y);
                }

                block |= IsBlocked(msp, what);

                /* If all that's blocking this part is a closed door and what
                 * can generally open doors... */
                if (block == MSP_FLAG_DOOR_CLOSED &&
                    QUERY_FLAG(what, FLAG_CAN_OPEN_DOOR))
                {
                    /* ...check if what can open THIS door. If not, return with
                     * the MSP_FLAG_NO_PASS flag. */
                    if (!open_door(what, msp, 0))
                    {
                        return block | MSP_FLAG_NO_PASS;
                    }
                }
                /* Otherwise, if there's a block, return it. */
                else if (block)
                {
                    return block;
                }
            }
        }
    }
    else
    {
        msp_t *msp;

        if (!what)
        {
            if (!m)
            {
                // TODO: BUG
            }
        }
        else if (!m)
        {
            m = what->map;
            x = what->x + x;
            y = what->y + y;
        }

        msp = MSP_GET2(m, x, y);
        block = IsBlocked(msp, what);

        /* If all that's blocking this part is a closed door and what
         * can generally open doors... */
        if (block == MSP_FLAG_DOOR_CLOSED &&
            what &&
            QUERY_FLAG(what, FLAG_CAN_OPEN_DOOR))
        {
            /* ...check if what can open THIS door. If not, return with
             * the MSP_FLAG_NO_PASS flag. */
            if (!open_door(what, msp, 0))
            {
                return block | MSP_FLAG_NO_PASS;
            }
        }
        /* Otherwise, if there's a block, return it. */
        else if (block)
        {
            return block;
        }
    }

    return block; /* when we are here - then we can move */
}

/* I total reworked the blocked functions. There was several bugs, glitches
* and loops in. The loops really scaled with bigger load very badly, slowing
* this part down for heavy traffic.
* Changes: check ALL MSP_FLAG_xxx flags (and really all) of a tile node here. If its impossible
* to enter the tile - blocked() will tell it us.
* This included to capsule and integrate blocked_tile() in blocked().
* blocked_tile() is the function where the single objects of a node gets
* tested - for example for CHECK_INV. But i added a MSP_FLAG_CHECK_INV flag - so its
* now only called when really needed - before it was called for EVERY moving
* object for every successful step.
* PASS_THRU check is moved in blocked() too.. This should generate for example for
* pathfinding better results. Note, that PASS_THRU only has a meaning when NO_PASS
* is set. If a object has both flags, NO_PASS can be passed when object has
* CAN_PASS_THRU. If object has PASS_THRU without NO_PASS, PASS_THRU is ignored.
* blocked() checks player vs player stuff too. No block in non pvp areas.
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
static uint32 IsBlocked(msp_t *msp, object_t *what)
{
    /* if this new node is illegal - we can skip all */
    if (!msp)
    {
        return MSP_FLAG_OUT_OF_MAP;
    }
    /* tricky: we use always head for tests - no need to copy any flags to the tail */
    /* we should kick in here the door test - but we need to diff we are
     * just testing here or we doing a real step!  */
    else
    {
        uint32 inflags = msp->flags,
               outflags = MSP_FLAG_ALIVE | MSP_FLAG_PLAYER | MSP_FLAG_NO_PASS | MSP_FLAG_PASS_THRU | MSP_FLAG_PASS_ETHEREAL | MSP_FLAG_PVP | MSP_FLAG_PLAYER_ONLY | MSP_FLAG_PLAYER_GRAVE | MSP_FLAG_CHECK_INV | MSP_FLAG_DOOR_CLOSED;
        int    terrain;

        if (what)
        {
            if (msp->floor_direction_block &&
                (msp->floor_direction_block & (1 << absdir(what->direction))))
            {
                return (inflags & outflags) | MSP_FLAG_NO_PASS;
            }

            /* Flying/levitating allows us to stay over more terrains */
            /* TODO: Terrain types will change. */
            terrain = what->terrain_flag;

            if (QUERY_FLAG(what, FLAG_FLYING))
            {
                terrain |= (TERRAIN_WATERWALK | TERRAIN_CLOUDWALK);
            }
            else if (QUERY_FLAG(what, FLAG_LEVITATE))
            {
                terrain |= TERRAIN_WATERWALK;
            }
        }
        else
        {
            terrain = TERRAIN_ALL;
        }

        /* If we don't have a valid terrain flag, msp is forbidden to enter. */
        if ((msp->move_flags & ~terrain))
        {
            return (inflags & outflags) | MSP_FLAG_NO_PASS;
        }

        /* A.) MSP_FLAG_ALIVE - we leave without question. (NOTE: player objects has NO is_alive set!). '*/
        if ((inflags & MSP_FLAG_ALIVE))
        {
            /* If this is a player pet, all players can pass it on non-pvp maps */
            if (!what ||
                what->type != PLAYER ||
                !(inflags & MSP_FLAG_PLAYER_PET) ||
                (inflags & MSP_FLAG_PVP))
            {
                return (inflags & outflags);
            }
        }

        outflags &= ~MSP_FLAG_ALIVE;

        /* a.) perhaps is a player in and we are a monster or the player is in a pvp area. */
        if ((inflags & MSP_FLAG_PLAYER))
        {
            /* ok... we leave here when
            * a.) what == NULL (because we can't check for what==PLAYER then)
            * b.) MSP_FLAG_PVP
            */
            if (!what ||
                (inflags & MSP_FLAG_PVP))
              
            {
                return (inflags & outflags);
            }

            outflags &= ~MSP_FLAG_PVP;

            if (what->type != PLAYER &&
                what->type != GRAVESTONE)
            {
                return (inflags & outflags);
            }
        }

        outflags &= ~MSP_FLAG_PLAYER;

        /* B.) MSP_FLAG_NO_PASS - if set we leave here when no PASS_THRU is set and/or the passer has no CAN_PASS_THRU. */
        if ((inflags & MSP_FLAG_NO_PASS))
        {
            /* logic is: no_pass when..
            * - no PASS_THRU... or
            * - PASS_THRU set but what==NULL (no PASS_THRU check possible)
            * - PASS_THRU set and object has no CAN_PASS_THRU
            * - the same for PASS_ETHEREAL and IS_ETHEREAL
            */
            if (!what ||
                ((!(inflags & MSP_FLAG_PASS_THRU) ||
                  !QUERY_FLAG(what, FLAG_CAN_PASS_THRU)) &&
                 (!(inflags & MSP_FLAG_PASS_ETHEREAL) ||
                  !QUERY_FLAG(what, FLAG_IS_ETHEREAL))))
            {
                return (inflags & outflags);
            }
        }

        outflags &= ~(MSP_FLAG_NO_PASS | MSP_FLAG_PASS_THRU | MSP_FLAG_PASS_ETHEREAL);

        if (what) /* we have a object ptr - do some last checks */
        {
            /* player only space and not a player... */
            if ((inflags & MSP_FLAG_PLAYER_ONLY) &&
                what->type != PLAYER)
            {
               return (inflags & outflags);
            }

            outflags &= ~MSP_FLAG_PLAYER_ONLY;

            /* already a gravestone here and try to insert another */
            if ((inflags & MSP_FLAG_PLAYER_GRAVE) &&
                what->type == GRAVESTONE)
            {
               return (inflags & outflags);
            }

            outflags &= ~MSP_FLAG_PLAYER_GRAVE;

            /* and here is our CHECK_INV. */
                    /* Note: we only do this check here because the last_grace cause the
                     * CHECK_INV to block the space. The check_inv is called again in
                     * move_apply() - there it will do the trigger and so on. This here is only
                     * for testing the tile - not for invoking the check_inv power! */
            if ((inflags & MSP_FLAG_CHECK_INV))
            {
                object_t *this,
                         *next;

                FOREACH_OBJECT_IN_MSP(this, msp, next)
                {
                    if (this->type == CHECK_INV &&
                        this->last_grace)
                    {
                        /* If last_sp is set, the player/monster needs an object,
                         * so we check for it.  If they don't have it, they can't
                         * pass through this space. */
                        /* In this case, the player must not have the object -
                         * if they do, they can't pass through. */
                        if ((this->last_sp &&
                             !check_inv_recursive(what, this)) ||
                            (!this->last_sp &&
                             check_inv_recursive(what, this)))
                        {
                            return (inflags & outflags);
                        }
                    }
                }
            }

            outflags &= ~MSP_FLAG_CHECK_INV;
        }
        else
        {
            outflags &= ~(MSP_FLAG_PLAYER_ONLY | MSP_FLAG_PLAYER_GRAVE | MSP_FLAG_CHECK_INV);
        }

        return (inflags & outflags);
    }
}

/* TODO: Below here is stuff for the overlay system. This will eventually be
 * moved to its own module. */
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

#include "global.h"

/* The overlay_x and overlay_y arrays contain the x and y offsets for up to a
 * 9x9 grid of squares. The indices map to the squares as follows:
 *  77 78 79 80 49 50 51 52 53
 *  76 46 47 48 25 26 27 28 54
 *  75 45 23 24 09 10 11 29 55
 *  74 44 22 08 01 02 12 30 56
 *  73 44 21 07 00 03 13 31 57
 *  72 42 20 06 05 04 14 32 58
 *  71 41 19 18 17 16 15 33 59
 *  70 40 39 38 37 36 35 34 60
 *  69 68 67 66 65 64 63 62 61
 * Remember though that Daimonin uses an isometric display so in game this map
 * is rotated 45 degrees clockwise (ie, a diamond not a square). */
sint8 overlay_x[OVERLAY_MAX] =
{
    0,
    0, 1, 1, 1, 0, -1, -1, -1,
    0, 1, 2, 2, 2, 2, 2, 1, 0, -1, -2, -2, -2, -2, -2, -1,
    0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -3, -3, -3, -3, -2, -1,
    0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0, -1, -2, -3, -4, -4, -4, -4, -4, -4, -4, -4, -4, -3, -2, -1
};

sint8 overlay_y[OVERLAY_MAX] =
{
    0,
    -1, -1, 0, 1, 1, 1, 0, -1,
    -2, -2, -2, -1, 0, 1, 2, 2, 2, 2, 2, 1, 0, -1, -2, -2,
    -3, -3, -3, -3, -2, -1, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3,
    -4, -4, -4, -4, -4, -3, -2, -1, 0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0, -1, -2, -3, -4, -4, -4, -4
};

static sint8 Dir[OVERLAY_MAX] =
{
    0,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 2, 2, 3, 4, 4, 4, 5, 6, 6, 6, 7, 8, 8, 8,
    1, 2, 2, 2, 2, 2, 3, 4, 4, 4, 4, 4, 5, 6, 6, 6, 6, 6, 7, 8, 8, 8, 8, 8,
    1, 2, 2, 2, 2, 2, 2, 2, 3, 4, 4, 4, 4, 4, 4, 4, 5, 6, 6, 6, 6, 6, 6, 6, 7, 8, 8, 8, 8, 8, 8, 8
};

static sint8 Back[OVERLAY_MAX][2] =
{
    {0, 0},
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {1, 1}, {1, 2}, {2, 2}, {3, 2}, {3, 3}, {3, 4}, {4, 4}, {5, 4}, {5, 5}, {5, 6}, {6, 6}, {7, 6}, {7, 7}, {7, 8}, {8, 8}, {1, 8},
    {9, 9}, {9, 10}, {10, 11}, {11, 11}, {12, 11}, {13, 12}, {13, 13}, {13, 14}, {14, 15}, {15, 15}, {16, 15}, {17, 16}, {17, 17}, {17, 18}, {18, 19}, {19, 19}, {20, 19}, {21, 20}, {21, 21}, {21, 22}, {22, 23}, {23, 23}, {24, 23}, {9, 24},
    {25, 25}, {25, 26}, {26, 27}, {27, 28}, {28, 28}, {29, 28}, {30, 29}, {31, 30}, {31, 31}, {31, 32}, {32, 33}, {33, 34}, {34, 34}, {35, 34}, {36, 35}, {37, 36}, {37, 37}, {37, 38}, {38, 39}, {39, 40}, {40, 40}, {41, 40}, {42, 41}, {43, 42}, {43, 43}, {43, 44}, {44, 45}, {45, 46}, {46, 46}, {47, 46}, {48, 47}, {25, 48}
};

sint8 overlay_find_free(msp_t *msp, object_t *what, sint8 start, sint8 stop, uint8 flags)
{
    sint8        i,
                 index = 0;
    static sint8 a[OVERLAY_MAX];
    uint32       terrain;

    /* Terrain is tested within msp_blocked() which is called when
     * OVERLAY_FORCE is not set. So as the test is external to this function,
     * we temporarilt overwrite what->terrain_flag if OVERLAY_IGNORE_TERRAIN is
     * set before the loop and restore it afterwards. */
    if (what &&
        (flags & OVERLAY_IGNORE_TERRAIN))
    {
        terrain = what->terrain_flag;
        what->terrain_flag = TERRAIN_ALL;
    }

    /* Loop through each msp between start and stop. */
    for (i = start; i < stop; i++)
    {
        map_t *m2 = msp->map;
        sint16     x2 = msp->x + OVERLAY_X(i),
                   y2 = msp->y + OVERLAY_Y(i);
        msp_t  *msp2 = MSP_GET2(m2, x2, y2);

        /* If the map will not accommodate at, move on to the next msp. This
         * works when what is NULL, or a singlepart, or for the had of a
         * multipart. */
        if (!msp2)
        {
            continue;
        }
        /* For multiparts we also check if the body would be out of map. */
        /* TODO: This can be done more efficiently because as maps and
         * multiparts are always square/rectangular with preknown heights and
         * widths we can therefore refer to this info to work out if any part
         * of what would be out of map with between 0-3 calls to out_of_map()
         * whatever the size of what, rather than potentially a call to
         * out_of_map() for every part of what.
         *
         * -- Smacky 20140725 */
        else if (what &&
                 what->more)
        {
            object_t *part;

            for (part = what->more; part; part = part->more)
            {
                map_t *m3 = m2;
                sint16     x3 = x2 + part->arch->clone.x,
                           y3 = y2 + part->arch->clone.y;

                if (!OUT_OF_MAP(m3, x3, y3))
                {
                    continue;
                }
            }
        }

        /* If we're looking beyond the 3x3 overlay and we only consider msps
         * within LOS and there's a view block in the way, move on to the next
         * msp. */
        if (i >= OVERLAY_3X3 &&
            (flags & OVERLAY_WITHIN_LOS) &&
            overlay_is_back_blocked(i, msp2, MSP_FLAG_BLOCKSVIEW))
        {
            continue;
        }

        /* If we're not forcing this placement and there's a block in the way,
         * move on to the next msp. Remember that terrain is tested within
         * msp_blocked() (actually within blocked() which is called by
         * msp_blocked()) hence the special handling of OVERLAY_IGNORE_TERRAIN
         * above. */ 
        if (!(flags & OVERLAY_FORCE) &&
            what &&
            msp_blocked(what, m2, x2, y2))
        {
            continue;
        }

        a[index++] = i;

        if ((flags & OVERLAY_FIRST_AVAILABLE))
        {
            break;
        }
    }

    if (what &&
        (flags & OVERLAY_IGNORE_TERRAIN))
    {
        what->terrain_flag = terrain;
    }

    if (index)
    {
        return a[random_roll(0, index - 1)];
    }

    return -1;
}

/* There are 3 flags, OVERLAY_FIXED, OVERLAY_RANDOM, and OVERLAY_SPECIAL. The
 * first two are mutually exclusive (if both are specified, OVERLAY_FIXED takes
 * precedence) and OVERLAY_SPECIAL is a modifier for either (or no) flag.
 *
 * OVERLAY_FIXED forces insertion of who at msp unless OVERLAY_SPECIAL is set
 * (IOW OVERLAY_FIXED | OVERLAY_SPECIAL will try to insert at msp but may fail
 * due to terrain, etc but OVERLAY_FIXED on its own will force the insertion).
 *
 * OVERLAY_RANDOM inserts who at msp, or if not free the first free of the 8
 * surrounding squares, or if not free the first free of the next 16
 * surrounding squares, or if not free the first free of the next 24
 * surrounding squares. If none are free insertion is forced at msp unless
 * OVERLAY_SPECIAL is set.
 *
 * If neither are set, who is inserted at msp, or if not free the first free of
 * the 8 surrounding squares. If none are free insertion is forced at msp
 * unless OVERLAY_SPECIAL is set. */
sint8 overlay_find_free_by_flags(msp_t *msp, object_t *who, uint8 oflags)
{
    sint8 start = 0,
          stop = OVERLAY_3X3,
          i;

    /* According to oflags work out exactly where who is reinserted. */
    if ((oflags & OVERLAY_FIXED))
    {
        if (!(oflags & OVERLAY_SPECIAL))
        {
            oflags |= OVERLAY_IGNORE_TERRAIN | OVERLAY_FORCE;
        }

        stop = 1;
        i = overlay_find_free(msp, who, start, stop, oflags);
    }
    else if ((oflags & OVERLAY_RANDOM))
    {
        stop = OVERLAY_7X7;
        i = overlay_find_free(msp, who, start, stop, oflags);

        if (i == -1 &&
            !(oflags & OVERLAY_SPECIAL))
        {
            i = 0;
        }
    }
    else
    {
        i = overlay_find_free(msp, who, start, stop, oflags);

        if (i == -1 &&
            !(oflags & OVERLAY_SPECIAL))
        {
            i = 0;
        }
    }

    return i;
}

sint8 overlay_find_dir(msp_t *msp, object_t *exclude)
{
    sint8 i;

    if (exclude &&
        exclude->head)
    {
        exclude = exclude->head;
    }

    for (i = 1; i < OVERLAY_MAX; i++)
    {
        map_t *m2 = msp->map;
        sint16     x2 = msp->x + OVERLAY_X(i),
                   y2 = msp->y + OVERLAY_Y(i);
        msp_t  *msp2 = MSP_GET2(m2, x2, y2);
        object_t    *this,
                  *next;

        if (!msp2 ||
            (msp2->flags & MSP_FLAG_BLOCKSVIEW))
        {
            continue;
        }

        if (i >= OVERLAY_3X3 &&
            overlay_is_back_blocked(i, msp2, MSP_FLAG_BLOCKSVIEW))
        {
            continue;
        }

        FOREACH_OBJECT_IN_MSP(this, msp2, next)
        {
            object_t *head = (this->head) ? this->head : this;

            if (head != exclude &&
                IS_LIVE(head))
            {
                return Dir[i];
            }
        }
    }

    return 0;
}

uint32 overlay_is_back_blocked(sint8 index, msp_t *msp, uint32 flags)
{
    sint8      i;
    map_t *m2;
    sint16     x2,
               y2;
    msp_t  *msp2;

    for (i = Back[index][0]; i; i = Back[i][0])
    {
        m2 = msp->map;
        x2 = msp->x + OVERLAY_X(i);
        y2 = msp->y + OVERLAY_Y(i);
        msp2 = MSP_GET2(m2, x2, y2);

        if (!msp2)
        {
            return MSP_FLAG_OUT_OF_MAP;
        }
        else if ((msp2->flags & flags))
        {
            return (msp2->flags & flags);
        }
    }

    m2 = msp->map;
    x2 = msp->x + OVERLAY_X(Back[index][1]);
    y2 = msp->y + OVERLAY_Y(Back[index][1]);
    msp2 = MSP_GET2(m2, x2, y2);

    if (!msp2)
    {
        return MSP_FLAG_OUT_OF_MAP;
    }
    else if ((msp2->flags & flags))
    {
        return (msp2->flags & flags);
    }

    return 0;
}
