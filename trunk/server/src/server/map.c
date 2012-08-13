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

#include <global.h>

int global_darkness_table[MAX_DARKNESS + 1] =
{
    0,
    20,
    40,
    80,
    160,
    320,
    640,
    1280
};

/* To get the reverse direction for all 8 tiled map index */
static int MapTiledReverse[TILED_MAPS] =
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

static char      *ShowMapFlags(mapstruct *m);
static mapstruct *GetLinkedMap(void);
static void       AllocateMap(mapstruct *m);
static mapstruct *LoadMap(shstr *filename, shstr *src_name,
                          uint32 flags, shstr *reference);
static mapstruct *LoadTemporaryMap(mapstruct *m);
static int        LoadMapHeader(FILE *fp, mapstruct *m, uint32 flags);
static void       FreeMap(mapstruct *m);
static void       LoadObjects(mapstruct *m, FILE *fp, int mapflags);
static void       UpdateMapTiles(mapstruct *m);
static void       SaveObjects(mapstruct *m, FILE *fp);
static void       FreeAllObjects(mapstruct *m);
#ifdef RECYCLE_TMP_MAPS
static void       WriteMapLog(void);
#endif

/* Returns the mapstruct which has a name matching the given argument oe NULL
 * if no match is found. */
mapstruct *has_been_loaded_sh(shstr *path)
{
    mapstruct *map;

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
        LOG(llevBug, "BUG:: %s/has_been_loaded_sh(): filename without start '/' or '.' (>%s<)\n",
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
int check_path(const char *name, int prepend_dir)
{
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
char *normalize_path_direct(const char *src, const char *dst, char *path)
{
    /*LOG(llevDebug,"path before normalization >%s< >%s<\n", src, dst?dst:"<no dst>");*/

    if (!src)
    {
        LOG(llevBug, "BUG:: %s/normalize_path_direct(): Called with src path = NULL! (dst:%s)\n",
            __FILE__, STRING_SAFE(dst));
        path[0] = '\0';
    }
    else
    {
        if (!dst)
        {
            dst = src;
        }

        /* First, make the dst path absolute */
        if (*dst == '/')
        {
            /* Already absolute path */
            strcpy(path, dst);
        }
        else
        {
            char *p;

           /* Combine directory part of src with dst to create absolute path */
            strcpy(path, src);

            if ((p = strrchr(path, '/')))
            {
                p[1] = '\0';
            }
            else
            {
                strcpy(path, "/");
            }

            strcat(path, dst);
        }
    }

    return path;
}

/* Dumps the header info of m to the server log and, if pl is non-NULL, prints
 * this info to the client as well.
 *
 * If list is > 0 we print a subset of the *dynamic* info (ie, not stuff you can
 * find just by checking the map file) in a brief format.
 *
 * If ref is also non-NULL we print %s Map, otherwise Map %d. The former is for
 * listing tiled maps, the latter for listing many maps.
 *
 * The amount of info given depends on the Gmaster mode of pl. If pl is a MW,
 * MM, or SA (or NULL), the full header is dumped. Otherwise just enough for
 * general info/bugfixing purposes (ie, to locate the player exactly).
 *
 * MWs/MMs/SAs/NULL also gets the number of players on the map.
 *
 * TODO: Finish listing layout. */
void dump_map(mapstruct *m, player *pl, int list, char *ref)
{
    object *ob = (pl) ? pl->ob : NULL;
    uint32  seconds = (ROUND_TAG - ROUND_TAG %
                       (long unsigned int)MAX(1, pticks_second)) /
                      pticks_second;

    if (list <= 0)
    {
        NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Name~: %s",
                STRING_SAFE(m->name));
        NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Msg~: %s",
                STRING_SAFE(m->msg));
        NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Music~: %s",
                STRING_SAFE(m->music));

        if (ob &&
            ob->map == m)
        {
            NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Position~: %d, %d",
                    ob->x, ob->y);
        }

        NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Path~: %s",
                STRING_MAP_PATH(m));
        NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Orig Path~: %s",
                STRING_MAP_ORIG_PATH(m));

        if (!pl ||
            (pl->gmaster_mode & (GMASTER_MODE_MW | GMASTER_MODE_MM | GMASTER_MODE_SA)))
        {
            NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Status~: %s%s%s (%u)",
                    ((MAP_MULTI(m)) ? "Multiplayer" :
                     ((MAP_UNIQUE(m)) ? "Unique" :
                      ((MAP_INSTANCE(m)) ? "Instance" : "UNKNOWN"))),
                    (m->reference) ? "/" : "", (m->reference) ? m->reference : "",
                    m->in_memory);
            NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Swap~: %d (%u)",
                    (sint32)(MAP_WHEN_SWAP(m) - seconds), MAP_SWAP_TIMEOUT(m));
#ifdef MAP_RESET

            if (MAP_WHEN_RESET(m) == 0)
            {
                NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Reset~: Never");
            }
            else
            {
                NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Reset~: %d (%u)",
                        (sint32)(MAP_WHEN_RESET(m) - seconds), MAP_RESET_TIMEOUT(m));
            }

#else
            NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Reset~: Never");
#endif
            NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Size~: %dx%d (%d, %d)",
                    MAP_WIDTH(m), MAP_HEIGHT(m), MAP_ENTER_X(m), MAP_ENTER_Y(m));
            NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Darkness/Light~: %d/%d%s",
                    MAP_DARKNESS(m), MAP_LIGHT_VALUE(m),
                    (MAP_OUTDOORS(m)) ? " (outdoors)" : "");
            NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Difficulty~: %d",
                    MAP_DIFFICULTY(m));

            if (m->tileset_id)
            {
                NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Tileset ID/X/Y~: %d/%d/%d",
                        m->tileset_id, m->tileset_x, m->tileset_y);
            }

            NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Flags~: %s",
                    ShowMapFlags(m));
            NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Players~: %d",
                    players_on_map(m));
        }
    }
    else
    {
        size_t len;
        char   buf[MEDIUM_BUF];

        if (!ref)
        {
            sprintf(buf, "~Map %d~: ", list);
        }
        else
        {
            sprintf(buf, "~%s Map~: ", ref);
        }

        if (m->path &&
            (len = strlen(m->path)) >= 16)
        {
            sprintf(strchr(buf, '\0'), "...%s",
                    STRING_MAP_PATH(m) + (len - 1 - 12));
        }
        else
        {
            sprintf(strchr(buf, '\0'), "%s", STRING_MAP_PATH(m));
        }

        sprintf(strchr(buf, '\0'), ": %s%s%s (%u)",
                ((MAP_MULTI(m)) ? "M" :
                 ((MAP_UNIQUE(m)) ? "U" :
                  ((MAP_INSTANCE(m)) ? "I" : "X"))),
                (m->reference) ? "/" : "", (m->reference) ? m->reference : "",
                m->in_memory);
        sprintf(strchr(buf, '\0'), ", %d (%u)",
                (sint32)(MAP_WHEN_SWAP(m) - seconds), MAP_SWAP_TIMEOUT(m));
#ifdef MAP_RESET

        if (MAP_WHEN_RESET(m) == 0)
        {
            sprintf(strchr(buf, '\0'), ", x (x)");
        }
        else
        {
            sprintf(strchr(buf, '\0'), ", %d (%u)",
                    (sint32)(MAP_WHEN_RESET(m) - seconds), MAP_RESET_TIMEOUT(m));
        }

#else
        sprintf(strchr(buf, '\0'), ", x (x)");
#endif
        sprintf(strchr(buf, '\0'), ", %d", MAP_DIFFICULTY(m));
        sprintf(strchr(buf, '\0'), ", %d/%d/%d",
                m->tileset_id, m->tileset_x, m->tileset_y);
        sprintf(strchr(buf, '\0'), ", %s", ShowMapFlags(m));
        sprintf(strchr(buf, '\0'), ", @%d", players_on_map(m));
        NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "%s", buf);
    }
}

static char *ShowMapFlags(mapstruct *m)
{
    static char buf[12];

    buf[0] = '\0';

    if (MAP_FIXED_RESETTIME(m))
    {
        sprintf(strchr(buf, '\0'), "%c", 'R');
    }

    if (MAP_NOSAVE(m))
    {
        sprintf(strchr(buf, '\0'), "%c", '$');
    }

    if (MAP_NOMAGIC(m))
    {
        sprintf(strchr(buf, '\0'), "%c", '\\');
    }

    if (MAP_NOPRIEST(m))
    {
        sprintf(strchr(buf, '\0'), "%c", '/');
    }

    if (MAP_NOHARM(m))
    {
        sprintf(strchr(buf, '\0'), "%c", 'X');
    }

    if (MAP_NOSUMMON(m))
    {
        sprintf(strchr(buf, '\0'), "%c", '#');
    }

    if (MAP_FIXEDLOGIN(m))
    {
        sprintf(strchr(buf, '\0'), "%c", 'L');
    }

    if (MAP_PERMDEATH(m))
    {
        sprintf(strchr(buf, '\0'), "%c", '_');
    }

    if (MAP_ULTRADEATH(m))
    {
        sprintf(strchr(buf, '\0'), "%c", '-');
    }

    if (MAP_ULTIMATEDEATH(m))
    {
        sprintf(strchr(buf, '\0'), "%c", '=');
    }

    if (MAP_PVP(m))
    {
        sprintf(strchr(buf, '\0'), "%c", '!');
    }

    return buf;
}

/* Dumps the msp info of m, x, y to the server log and, if pl is non-NULL,
 * prints this info to the client as well. */
void dump_msp(mapstruct *m, int x, int y, player *pl)
{
    object *ob = (pl) ? pl->ob : NULL;
    char    buf[MEDIUM_BUF];
    int     flags;

    /* Dump the light value. */
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Light value~: %d (plus map light value of %d)",
            GET_MAP_LIGHT_VALUE(m, x, y), MAP_LIGHT_VALUE(m));

    /* Dump the flags. */
    buf[0] = '\0';
    flags = GET_MAP_FLAGS(m, x, y);

    if ((flags & P_IS_PLAYER))
    {
        sprintf(strchr(buf, '\0'), "%c", '@');
    }

    if ((flags & P_OUT_OF_MAP))
    {
        sprintf(strchr(buf, '\0'), "%c", '#');
    }

    if ((flags & P_PLAYER_ONLY))
    {
        sprintf(strchr(buf, '\0'), "%c", 'O');
    }

    if ((flags & P_IS_PVP))
    {
        sprintf(strchr(buf, '\0'), "%c", '!');
    }

    if ((flags & P_IS_ALIVE))
    {
        sprintf(strchr(buf, '\0'), "%c", '*');
    }

    if ((flags & P_IS_PLAYER_PET))
    {
        sprintf(strchr(buf, '\0'), "%c", 'd');
    }

    if ((flags & P_BLOCKSVIEW))
    {
        sprintf(strchr(buf, '\0'), "%c", 'x');
    }

    if ((flags & P_NO_PASS))
    {
        sprintf(strchr(buf, '\0'), "%c", 'X');
    }

    if ((flags & P_PASS_THRU))
    {
        sprintf(strchr(buf, '\0'), "%c", '-');
    }

    if ((flags & P_PASS_ETHEREAL))
    {
        sprintf(strchr(buf, '\0'), "%c", '=');
    }

    if ((flags & P_DOOR_CLOSED))
    {
        sprintf(strchr(buf, '\0'), "%c", '+');
    }

    if ((flags & P_NO_MAGIC))
    {
        sprintf(strchr(buf, '\0'), "%c", '\\');
    }

    if ((flags & P_NO_CLERIC))
    {
        sprintf(strchr(buf, '\0'), "%c", '/');
    }

    if ((flags & P_WALK_ON))
    {
        sprintf(strchr(buf, '\0'), "%c", '>');
    }

    if ((flags & P_WALK_OFF))
    {
        sprintf(strchr(buf, '\0'), "%c", '<');
    }

    if ((flags & P_FLY_ON))
    {
        sprintf(strchr(buf, '\0'), "%c", '}');
    }

    if ((flags & P_FLY_OFF))
    {
        sprintf(strchr(buf, '\0'), "%c", '{');
    }

    if ((flags & P_REFL_MISSILE))
    {
        sprintf(strchr(buf, '\0'), "%c", ')');
    }

    if ((flags & P_REFL_SPELLS))
    {
        sprintf(strchr(buf, '\0'), "%c", '(');
    }

    if ((flags & P_MAGIC_EAR))
    {
        sprintf(strchr(buf, '\0'), "%c", '?');
    }

    if ((flags & P_CHECK_INV))
    {
        sprintf(strchr(buf, '\0'), "%c", '_');
    }

    if ((flags & P_PLAYER_GRAVE))
    {
        sprintf(strchr(buf, '\0'), "%c", '%');
    }

    NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Flags~: %s", buf);

    /* Dump the move flags. */
    buf[0] = '\0';
    flags = GET_MAP_MOVE_FLAGS(m, x, y);

    /* TODO: I think these terrain types are from CF and are not all
     * particularly useful or used in Dai. We should rework these and possibly
     * client-side play different footstep sounds according to the terrain
     * (and no sound if the player is levitating, and swooshy sound if he is
     * flying).
     * -- Smacky 20090724 */
    while (flags)
    {
        if ((flags & 0x1))
        {
            sprintf(strchr(buf, '\0'), "Land surface");
            flags &= ~0x1;
        }
        else if ((flags & 0x2))
        {
            sprintf(strchr(buf, '\0'), "Water surface");
            flags &= ~0x2;
        }
        else if ((flags & 0x4))
        {
            sprintf(strchr(buf, '\0'), "Under water");
            flags &= ~0x4;
        }
        else if ((flags & 0x8))
        {
            sprintf(strchr(buf, '\0'), "Fire surface");
            flags &= ~0x8;
        }
        else if ((flags & 0x10))
        {
            sprintf(strchr(buf, '\0'), "Under fire");
            flags &= ~0x10;
        }
        else if ((flags & 0x20))
        {
            sprintf(strchr(buf, '\0'), "Cloud surface");
            flags &= ~0x20;
        }

        if (flags)
        {
            sprintf(strchr(buf, '\0'), " ~+~ ");
        }
    }

    NDI_LOG(llevSystem, NDI_UNIQUE, 0, ob, "~Terrain~: %s", buf);
}

/* Allocates, initialises, and returns a pointer to a mapstruct.
 * Modified to no longer take a path option which was not being
 * used anyways.  MSW 2001-07-01 */
static mapstruct *GetLinkedMap(void)
{
    /* Tag counter for the memory/weakref system. Uniquely identifies this
     * instance of the map in memory. Not the same as the
     * map_tag/global_map_tag. */
    static tag_t  gID = 0;
    mapstruct    *map = get_poolchunk(pool_map);

    if (!map)
    {
        LOG(llevError, "ERROR:: %s/GetLinkedMap(): OOM!\n", __FILE__);

        return NULL;
    }

    memset(map, 0, sizeof(mapstruct));
    map->tag = ++gID;

    /* lifo queue */
    if (first_map)
    {
        first_map->last = map;
    }

    map->next = first_map;
    first_map = map;
    map->in_memory = MAP_SWAPPED;

    /* The maps used to pick up default x and y values from the
     * map archetype. Mimic that behaviour. */
    MAP_WIDTH(map) = MAP_DEFAULT_WIDTH;
    MAP_HEIGHT(map) = MAP_DEFAULT_HEIGHT;
    MAP_RESET_TIMEOUT(map) = MAP_DEFRESET;
    MAP_SWAP_TIMEOUT(map) = MAP_DEFSWAP;
    MAP_DIFFICULTY(map) = MAP_DEFAULT_DIFFICULTY;
    set_map_darkness(map, MAP_DEFAULT_DARKNESS);

    /* We insert a dummy sentinel first in the activelist. This simplifies
     * work later */
    map->active_objects = get_object();
    FREE_AND_COPY_HASH(map->active_objects->name, "<map activelist sentinel>");

    /* Avoid gc of the sentinel object */
    insert_ob_in_ob(map->active_objects, &void_container);

    return map;
}

/* Allocates the arrays contained in a mapstruct.
 * This basically allocates the dynamic array of spaces for the
 * map. */
static void AllocateMap(mapstruct *m)
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
    if (m->spaces ||
        m->bitmap)
    {
        LOG(llevBug, "\nBUG:: %s/AllocateMap(): >%s< already allocated!\n",
            __FILE__, STRING_MAP_PATH(m));

        if (m->spaces)
        {
            FREE(m->spaces);
        }

        if (m->bitmap)
        {
            FREE(m->bitmap);
        }
    }

    if (m->buttons)
    {
        LOG(llevBug, "\nBUG:: %s/AllocateMap(): >%s< has already set buttons!\n",
            __FILE__, STRING_MAP_PATH(m));
    }

    MALLOC(m->spaces, MAP_WIDTH(m) * MAP_HEIGHT(m) * sizeof(MapSpace));
    MALLOC(m->bitmap, ((MAP_WIDTH(m) + 31) / 32) * MAP_HEIGHT(m) * sizeof(uint32));
}

/* Saves a map to file.  If flag is set, it is saved into the same
 * file it was (originally) loaded from.  Otherwise a temporary
 * filename will be genarated, and the file will be stored there.
 * The temporary filename will be stored in the mapstructure.
 * If the map is unique, we also save to the filename in the map
 * (this should have been updated when first loaded). */
int new_save_map(mapstruct *m, int flag)
{
    FILE   *fp;
    char    filename[MAXPATHLEN];
    int     i;

    if (flag &&
        (!m->path ||
         !*m->path))
    {
        LOG(llevBug, "BUG:: %s/new_save_map(): Tried to save map without path!\n",
            __FILE__);

        return -1;
    }

    /* if we don't do this, we leave the light mask part
     * on a possible tiled map and when we reload, the area
     * will be set with wrong light values. */
    remove_light_source_list(m);

    if (flag ||
        MAP_UNIQUE(m) ||
        MAP_INSTANCE(m))
    {
        if (MAP_UNIQUE(m) ||
            MAP_INSTANCE(m))
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
                    LOG(llevDebug, "DEBUG:: %s/new_save_map(): Player %s no longer exists so deleting %s map >%s<!\n",
                        __FILE__, m->reference,
                         (MAP_UNIQUE(m)) ? "unique" : "instanced",
                         STRING_MAP_PATH(m));
#endif
                    delete_map(m);

                    return 0;
                }
            }
            else
            {
                LOG(llevBug, "BUG:: %s/new_save_map(): Invalid path >%s<!\n",
                    __FILE__, STRING_MAP_PATH(m));
            }

            /* that ensures we always reload from original maps */
            if (MAP_NOSAVE(m))
            {
#ifdef DEBUG_MAP
                LOG(llevDebug, "DEBUG:: %s/new_save_map(): Skip map >%s< (no_save flag)\n",
                    __FILE__, STRING_MAP_PATH(m));

#endif
                return 0;
            }

            strcpy(filename, m->path);
        }
        else
        {
            strcpy(filename, create_mapdir_pathname(m->path));
        }

        /* /maps, /tmp should always there and player dir is created when player is loaded */
        /* make_path_to_file(filename); */
    }
    else
    {
        /* create tmpname if we don't have one or our old one was used by a different map */
        if (!m->tmpname ||
            access(m->tmpname, F_OK) ==-1 )
        {
            FREE_AND_NULL_PTR(m->tmpname);
            tempnam_local_ext(settings.tmpdir, NULL, filename);
            m->tmpname = strdup_local(filename);
        }
        else
        {
            strcpy(filename, m->tmpname);
        }
    }

    LOG(llevInfo, "INFO:: Saving map >%s< to >%s<.\n",
        STRING_MAP_PATH(m), filename);
    m->in_memory = MAP_SAVING;

    if (!(fp = fopen(filename, "w")))
    {
        LOG(llevBug, "BUG:: %s/new_save_map(): Can't open file >%s< for saving!\n",
            __FILE__, filename);

        return -1;
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

    if (!flag)
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

    fprintf(fp, "darkness %d\n", m->darkness);
    fprintf(fp, "light %d\n", m->light_value);
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

    if (MAP_UNIQUE(m))
    {
        fputs("unique 1\n", fp);
    }

    if (MAP_MULTI(m))
    {
        fputs("multi 1\n", fp);
    }

    if (MAP_INSTANCE(m))
    {
        fputs("instance 1\n", fp);
    }

    if (MAP_UNIQUE(m) ||
        MAP_INSTANCE(m))
    {
        if (m->reference)
        {
            fprintf(fp, "reference %s\n", m->reference);
        }
        else
        {
            LOG(llevBug, "BUG:: %s/new_save_map(): %s map with NULL reference!\n",
                __FILE__, (MAP_UNIQUE(m)) ? "Unique" : "Instanced");
        }
    }

    if (MAP_OUTDOORS(m))
    {
        fputs("outdoor 1\n", fp);
    }

    if (MAP_NOSAVE(m))
    {
        fputs("no_save 1\n", fp);
    }

    if (MAP_NOMAGIC(m))
    {
        fputs("no_magic 1\n", fp);
    }

    if (MAP_NOPRIEST(m))
    {
        fputs("no_priest 1\n", fp);
    }

    if (MAP_NOHARM(m))
    {
        fputs("no_harm 1\n", fp);
    }

    if (MAP_NOSUMMON(m))
    {
        fputs("no_summon 1\n", fp);
    }

    if (MAP_FIXEDLOGIN(m))
    {
        fputs("fixed_login 1\n", fp);
    }

    if (MAP_PERMDEATH(m))
    {
        fputs("perm_death 1\n", fp);
    }

    if (MAP_ULTRADEATH(m))
    {
        fputs("ultra_death 1\n", fp);
    }

    if (MAP_ULTIMATEDEATH(m))
    {
        fputs("ultimate_death 1\n", fp);
    }

    if (MAP_PVP(m))
    {
        fputs("pvp 1\n", fp);
    }

    /* save original path */
    fprintf(fp, "orig_path %s\n", m->orig_path);

    /* Save any tiling information */
    for (i = 0; i < TILED_MAPS; i++)
    {
        if (m->tile_path[i])
        {
            fprintf(fp, "tile_path_%d %s\n", i + 1, m->tile_path[i]);
        }

        if (m->orig_tile_path[i])
        {
            fprintf(fp, "orig_tile_path_%d %s\n", i + 1, m->orig_tile_path[i]);
        }
    }

    /* Save any tileset information */
    if (m->tileset_id > 0)
    {
        fprintf(fp, "tileset_id %d\n", m->tileset_id);
        fprintf(fp, "tileset_x %d\n", m->tileset_x);
        fprintf(fp, "tileset_y %d\n", m->tileset_y);
    }

    fprintf(fp, "end\n");
    SaveObjects(m, fp);

    /* When there are players or (TODO) permanently loaded mobs on the map, put
     * the map back in memory. */
    if (m->player_first ||
        m->perm_load)
    {
        m->in_memory = MAP_IN_MEMORY;
    }

    fclose(fp);
    chmod(filename, SAVE_MODE);

    return 0;
}

/* Frees everything allocated by the given mapstructure.
 * Don't free tmpname - our caller is left to do that. */
static void FreeMap(mapstruct *m)
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

    /* remove linked spawn points (small list of objectlink *) */
    remove_linked_spawn_list(m);

    /* I put this before FreeAllObjects() -
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

    if (m->spaces)
    {
        FreeAllObjects(m);
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
        object *next;

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
    for (i = 0; i < TILED_MAPS; i++)
    {
        if (m->tile_map[i])
        {
            if (m->tile_map[i]->tile_map[MapTiledReverse[i]] &&
                m->tile_map[i]->tile_map[MapTiledReverse[i]] != m )
            {
                LOG(llevMapbug, "MAPBUG:: Freeing map >%s< linked to >%s< which links back to another map!\n",
                    STRING_MAP_ORIG_PATH(m),
                    STRING_MAP_ORIG_PATH(m->tile_map[i]));
            }

            m->tile_map[i]->tile_map[MapTiledReverse[i]] = NULL;
            m->tile_map[i] = NULL;
        }

        FREE_AND_CLEAR_HASH(m->tile_path[i]);
        FREE_AND_CLEAR_HASH(m->orig_tile_path[i]);
    }

    FREE_AND_CLEAR_HASH(m->name);
    FREE_AND_CLEAR_HASH(m->music);
    FREE_AND_CLEAR_HASH(m->msg);
    FREE_AND_CLEAR_HASH(m->cached_dist_map);
    FREE_AND_CLEAR_HASH(m->reference);
    m->in_memory = MAP_SWAPPED;
    /* Note: m->path, m->orig_path and m->tmppath are freed in delete_map */
}

/*
 * Remove and free all objects in the given map.
 */

static void FreeAllObjects(mapstruct *m)
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
                    LOG(llevBug, "BUG:: %s/FreeAllObjects(): Link error, bailing out.\n",
                        __FILE__);

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
 * function: vanish mapstruct
 * m       : pointer to mapstruct, if NULL no action
 * this deletes all the data on the map (freeing pointers)
 * and then removes this map from the global linked list of maps.
 */

void delete_map(mapstruct *m)
{
    if (!m)
    {
        return;
    }

    if (m->in_memory == MAP_IN_MEMORY)
    {
        /* change to MAP_SAVING, even though we are not, so that remove_ob()
         * doesn't do as much work. */
        m->in_memory = MAP_SAVING;
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
    FREE_AND_NULL_PTR(m->tmpname); /* malloc() string */

    m->tag = 0; /* Kill any weak references to this map */
    return_poolchunk(m, pool_map);
}

void clean_tmp_map(mapstruct *m)
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
        if (first_map->in_memory == MAP_SAVING)
        {
            first_map->in_memory = MAP_IN_MEMORY;
        }

        delete_map(first_map);
        real_maps++;
    }

    LOG(llevInfo, "INFO:: Freed %d maps\n", real_maps);
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
mapstruct *ready_inherited_map(mapstruct *orig_map, shstr *new_map_path,
                               uint32 flags)
{
    mapstruct *new_map = NULL;
    shstr     *new_path = NULL,
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
        (orig_map->in_memory == MAP_LOADING ||
         orig_map->in_memory == MAP_IN_MEMORY))
    {
        return orig_map;
    }

    if (!MAP_STATUS_TYPE(orig_map->map_status))
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
    if(orig_map->map_status & (MAP_STATUS_UNIQUE|MAP_STATUS_INSTANCE))
    {
        new_path = add_string(normalize_path_direct(orig_map->path,
                    path_to_name(normalized_path), tmp_path));
    }
#else
    if (orig_map->map_status & (MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE))
    {
        /* Guesstimate whether the new map is already loaded */
        if (*new_map_path == '.')
        {
            normalized_path = add_refcount(new_map_path);
        }
        else
        {
            (void)normalize_path_direct(orig_map->path,
                                        path_to_name(new_map_path), tmp_path);

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

    if ((flags & 1))
    {
        /* Just check if it has in memory */
        if ((new_map = has_been_loaded_sh((new_path) ? new_path : normalized_path)) &&
            (new_map->in_memory != MAP_LOADING &&
             new_map->in_memory != MAP_IN_MEMORY))
        {
            new_map = NULL;
        }
    }
    else
    {
        /* Load map if necesseary */
        new_map = ready_map_name((new_path) ? new_path : normalized_path,
                                 normalized_path,
                                 MAP_STATUS_TYPE(orig_map->map_status),
                                 orig_map->reference);
    }

    FREE_ONLY_HASH(normalized_path);

    if (new_path)
    {
        FREE_ONLY_HASH(new_path);
    }

    return new_map;
}

/* ready_map_name() attempts to ensure the map indicated by name_path/src_path and flags is ready in memory, (re)loading from various locations depending on flags as necessary.
 *
 * name_path and src_path must be genuine path and orig_path respectively or NULL. If both are NULL we simply return NULL (the function should not have been called in the first place).
 *
 * If a map with a matching path is already loaded into memory, there is nothing to do; it is already ready so return it.
 *
 * 
 * it will return a map pointer to the map name_path/src_path.
 * If the map was not loaded before, the map will be loaded now.
 * src_path is ALWAYS a path to /maps = the original map path.
 * name_path can be different and pointing to /instance or /players
 * reference needs to be a player name for UNIQUE or MULTI maps
 *
 * If src_path is NULL we will not load a map from disk, but return NULL
 * if the map wasn't in memory already.
 * If name_path is NULL we will force a reload of the map even if it already
 * was in memory. (caller has to reset the map!) */
mapstruct *ready_map_name(shstr *name_path, shstr *src_path, uint32 flags,
                          shstr *reference)
{
    mapstruct *m;

/* Sanity checks: These extra checks are for debugging/tracking how the map
 * system works in practice. They probably/hopefully will never come up in
 * actuality.
 *
 * -- Smacky 20120807 */
#ifdef DEBUG_MAP
    LOG(llevDebug, "DEBUG:: %s/ready_map_name(): >%s< >%s< %u >%s<!\n",
        __FILE__, STRING_SAFE(name_path), STRING_SAFE(src_path), flags,
        STRING_SAFE(reference));

    /* No paths = nothing to do. */
    if (!name_path &&
        !src_path)
    {
        LOG(llevDebug, "  * Both name_path/src_path = NULL!\n");
    }

    if ((name_path &&
         (*name_path != '.' &&
          (flags & (MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE))) ||
         (*name_path != '/' &&
          !(flags & (MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE)))) ||
        (src_path &&
         *src_path != '/'))
    {
        LOG(llevDebug, "  * Either name_path or src_path are incorrect according to flags!\n");
    }

    /* This would not make sense. */
    if (!name_path &&
        src_path &&
        (flags & (MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE)))
    {
        LOG(llevDebug, "  * name_path = NULL and flags indicate we are trying "
            "to ready a unique or instanced map. However if a multi version "
            "of this map is already ready we will just return it and not load "
            "the new status version!\n");
    }
#endif

    /* Map is good to go? Just return it. */
    if ((m = has_been_loaded_sh((name_path) ? name_path : src_path)) &&
        (m->in_memory == MAP_LOADING ||
         m->in_memory == MAP_IN_MEMORY))
    {
#ifdef DEBUG_MAP
        LOG(llevDebug, "DEBUG:: %s/ready_map_name(): Map >%s< >%s< (%u/%u) already ready!\n",
            __FILE__, STRING_MAP_PATH(m), STRING_MAP_ORIG_PATH(m),
            m->in_memory, m->map_status);

#endif
        return m;
    }
    /* If it's not in memory or is but we want to load it new as unique or
     * instanced -- which always get loaded from data/players/ or
     * data/instances/. */
    else if (!m ||
             (flags & (MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE)))
    {
        /* Tricky check - if we have '/' starting part, its a multi map we have here.
         * if called without src_path, we only check it in memory OR in tmp.
         * if we are here its not there or its not an multi map. */
        if (!src_path &&
            *name_path == '/')
        {
            return  NULL;
        }

        /* The map must be SAVING or SWAPPED and we want to load it new as
         * unique or instance, so delete the old one and any temp. */
        if (m)
        {
#ifdef DEBUG_MAP
            LOG(llevDebug, "DEBUG:: %s/ready_map_name(): Cleaning up  map >%s< >%s< (%u/%u) for reload with as %s!\n",
                __FILE__, STRING_SAFE(name_path), STRING_SAFE(src_path),
                 m->in_memory, m->map_status,
                 (flags & MAP_STATUS_UNIQUE) ? "unique" : "instance");
#endif
            clean_tmp_map(m);
            delete_map(m);
        }

        /* We are loading now a src map from maps/ or an unique/instance from
         * data/players/ or data/instances/. */
        m = LoadMap(name_path, src_path, MAP_STATUS_TYPE(flags), reference);
    }
    /* If in this loop, we found a temporary map (so it was, and we are trying
     * to re-ready it as, a multi), so load it up. */
    else
    {
        m = LoadTemporaryMap(m);
    }

    return m;
}

/* Opens the file "filename" or "src_name" and reads information about the map
 * from the given file, and stores it in a newly allocated mapstruct. A pointer
 * to this structure is returned, or NULL on failure.
 *
 * The function knows if it loads an original map from /maps or a
 * unique/instance by comparing filename and src_name.
 *
 * flags correspond to those in map.h. Main ones used are MAP_PLAYER_UNIQUE and
 * MAP_PLAYER_INSTANCE where filename is != src_name. MAP_STYLE: style map -
 * don't add active objects, don't add to server managed map list.
 *
 * reference needs to be a player name for UNIQUE or MULTI maps. */
static mapstruct *LoadMap(shstr *filename, shstr *src_name, uint32 flags, shstr *reference)
{
    FILE      *fp;
    mapstruct *m;
    char       pathname[MEDIUM_BUF];

    flags &= ~MAP_STATUS_ORIGINAL;

    /* this IS a bug - because string compare will fail when it checks the loaded maps -
     * this can lead in a double load and break the server!
     * a '.' signs unique maps in fixed directories.
     * We don't fix it here anymore - this MUST be done by the calling functions or our
     * inheritanced map system is already broken somewhere before this call. */
    if ((filename &&
         *filename != '/' &&
         *filename != '.') ||
        (src_name &&
         *src_name != '/' &&
         *src_name != '.'))
    {
        LOG(llevBug, "BUG:: %s/LoadMap(): Filename without start '/' or '.' (>%s<) (>%s<)\n",
            __FILE__, STRING_SAFE(filename), STRING_SAFE(src_name));

        return NULL;
    }

    /* Here is our only "file path analyzing" trick. Our map model itself don't need it, but
     * it allows us to call this function with the DM commands like "/goto ./players/a/aa/Aa/$demo"
     * without pre-guessing the map_status. In fact map_status CAN be here invalid with 0!
     * IF map_status is zero here, LoadMap() will set it dynamic!
     * Checkup LoadMap() & LoadMapHeader() how it works.
     */
    if (filename)
    {
        if (*filename == '.') /* pathes to /instance and /players always start with a '.'! */
        {
            strcpy(pathname, filename);
        }
        else /* we have an normalized map here and the map start ALWAYS with a '/' */
        {
            strcpy(pathname, create_mapdir_pathname(filename)); /* we add the (...)/maps prefix path part */

            if (filename == src_name)
            {
                flags |= MAP_STATUS_ORIGINAL;
            }
        }
    }

    if (!filename ||
        !(fp = fopen(pathname, "r")))
    {
        /* this was usually a try to load a unique or instance map
         * This is RIGHT because we use fopen() here as an implicit access()
         * check. If it fails, we know we have to load the map from /maps! */
        if (src_name &&
            filename != src_name &&
            *src_name == '/')
        {
            strcpy(pathname, create_mapdir_pathname(src_name)); /* we add the (...)/maps prefix path part */
            flags |= MAP_STATUS_ORIGINAL;

            if (!(fp = fopen(pathname, "r")))
            {
                /* ok... NOW we are screwed with an invalid map... because it is not in /maps */
                LOG(llevBug, "BUG:: %s/LoadMap(): Can't open map file >%s< (>%s<)!\n",
                    __FILE__, STRING_SAFE(filename), STRING_SAFE(src_name));

                return NULL;
            }
        }
        else
        {
            LOG(llevBug, "BUG:: %s/LoadMap(): Can't open map file >%s< (>%s<)!\n",
                __FILE__, STRING_SAFE(filename), STRING_SAFE(src_name));

            return NULL;
        }
    }

    m = GetLinkedMap();

    if (filename)
    {
        FREE_AND_COPY_HASH(m->path, filename);
    }
    else
    {
        FREE_AND_COPY_HASH(m->path, src_name);
    }

    if (src_name) /* invalid src_name can happens when we force an explicit load of an unique map! */
    {
        FREE_AND_COPY_HASH(m->orig_path, src_name); /* orig_path will be loaded in LoadMapHeader()! */
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
    if((MAP_UNIQUE(m) ||
        MAP_INSTANCE(m)) &&
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
                __FILE__, (MAP_UNIQUE(m)) ? "Unique" : "Instanced");
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
 * note: LoadMap() is called with (NULL, <src_name>, MAP_STATUS_MULTI, NULL) when
 * tmp map loading fails because a tmp map is ALWAYS a MULTI map and when fails its
 * reloaded from /maps as new original map. */
static mapstruct *LoadTemporaryMap(mapstruct *m)
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
        char buf[MEDIUM_BUF];

        LOG(llevInfo, " Fallback to original!\n");
        sprintf(buf, "%s", m->orig_path);
        delete_map(m);
        m = LoadMap(NULL, buf, MAP_STATUS_MULTI, NULL);
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
        m->in_memory = MAP_IN_MEMORY;
     
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
 * NOTE: LoadMapHeader will setup map_status dynamically when flags
 * has not a valid MAP_STATUS_FLAG()
 * return 0 on success, 1 on failure.  */
static int LoadMapHeader(FILE *fp, mapstruct *m, uint32 flags)
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
            int v = atoi(value);

#ifdef MAP_RESET
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
#endif
            MAP_RESET_TIMEOUT(m) = v;
        }
        else if (!strcmp(key, "swap_time"))
        {
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
        }
        /* difficulty is a 'recommended player level' for that map *for a
         * single player*, so follows the same restriction (1 to MAXLEVEL). It
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
        /* darkness is a 'coarse' way to set a map's ambient light levels. As a
         * special case, darkness can be -1 which means full daylight (ie, the
         * mapper does not need to know the value of MAX_DARKNESS and the map
         * will cope if this value ever changes in the server); darkness -1 is
         * *not* the same as outdoor 1 (see below). The default is -1. */
        else if (!strcmp(key, "darkness"))
        {
            int v = atoi(value);

            if (v < -1 || v > MAX_DARKNESS)
            {
                LOG(llevMapbug, "MAPBUG:: >%s<: Illegal map darkness %d (must be -1 to %d, defaulting to -1)!\n",
                    STRING_MAP_ORIG_PATH(m), v, MAX_DARKNESS);
            }
            else if (v != MAP_DEFAULT_DARKNESS)
            {
                set_map_darkness(m, v);
            }
        }
        /* light is a 'finer' way to set a map's ambient light levels. Mappers
         * shouldn't explicitly set light as darkness takes care of it. */
        /* TODO: I think perhaps the following else if block should even be
         * removed -- Smacky 20081228 */
        else if (!strcmp(key, "light"))
        {
            int v = atoi(value);

            if (v < 0)
                LOG(llevMapbug, "MAPBUG:: >%s<: Illegal map light %d (must be positive, defaulting to 0)!\n",
                    STRING_MAP_ORIG_PATH(m), v);

            MAP_LIGHT_VALUE(m) = v;

            /* I assume that the default map_flags settings is 0 - so we don't handle <flagset> 0 */
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
        /* outdoor 1 means the map has variable ambient light over the course of
         * a day, up to a maximum as given in darkness (see above). */
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

            if ((flags & MAP_STATUS_ORIGINAL))
            {
                LOG(llevMapbug, "MAPBUG:: >%s<: Map has reference to player %s\n",
                    STRING_MAP_ORIG_PATH(m), value);
            }
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

            if (tile < 1 ||
                tile > TILED_MAPS)
            {
                LOG(llevMapbug,  "MAPBUG:: >%s<: orig tile location %d out of bounds: %s!\n",
                    STRING_MAP_PATH(m), tile, STRING_SAFE(value));
            }
            else
            {
                *end = '\0';

                if (m->orig_tile_path[tile - 1])
                {
                    LOG(llevMapbug, "MAPBUG:: >%s<: orig tile location %d duplicated: %s!\n",
                        STRING_MAP_PATH(m), tile, STRING_SAFE(value));
                }

                m->orig_tile_path[tile - 1] = add_string(value);
            }
        }
        else if (!strncmp(key, "tile_path_", 10))
        {
            int tile = atoi(key + 10);

            if (tile < 1 ||
                tile > TILED_MAPS)
            {
                LOG(llevMapbug,  "MAPBUG:: >%s<: tile location %d out of bounds: %s!\n",
                    STRING_MAP_PATH(m), tile, STRING_SAFE(value));
            }
            else
            {
                shstr *path_sh;
                mapstruct *neighbour;

                *end = '\0';

                if (m->tile_path[tile - 1])
                {
                    LOG(llevMapbug, "MAPBUG:: >%s<: tile location %d duplicated: %s!\n",
                        STRING_MAP_PATH(m), tile, STRING_SAFE(value));
                }

                /* note: this only works because our map saver is storing
                 * MAP_STATUS and orig_map BEFORE he saves the tile map data.
                 * NEVER change it, or the dynamic setting will fail! */
                if (!MAP_STATUS_TYPE(flags)) /* synchronize dynamically the map status flags */
                {
                    flags |= m->map_status;

                    if (!MAP_STATUS_TYPE(flags)) /* still zero? then force _MULTI */
                    {
                        flags |= MAP_STATUS_MULTI;
                    }
                }

                if ((flags & MAP_STATUS_ORIGINAL)) /* original map... lets normalize tile_path[] to /maps */
                {
                    normalize_path(m->orig_path, value, msgbuf);
                    m->orig_tile_path[tile - 1] = add_string(msgbuf);

                    /* If the specified map does not exist, report this and do
                     * not set the tile_path. */
                    if (check_path(m->orig_tile_path[tile - 1], 1) == -1)
                    {
                        LOG(llevMapbug, "MAPBUG:: Tile %d of map >%s< refers to non-existent file %s!\n",
                            tile, STRING_MAP_PATH(m), STRING_SAFE(m->orig_tile_path[tile - 1]));

                        continue;
                    }

                    /* whatever we have opened - in m->path is the REAL path */
                    if ((flags & (MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE)))
                    {
                        normalize_path_direct(m->path,
                                              path_to_name(m->orig_tile_path[tile - 1]),
                                              msgbuf);
                        path_sh = add_string(msgbuf);
                    }
                    else /* for multi maps, orig_path is the same path */
                    {
                        path_sh = add_refcount(m->orig_tile_path[tile - 1]);
                    }
                }
                else /* non original map - all the things above was done before - just load */
                {
                    path_sh = add_string(value);
                }

                /* If the neighbouring map tile has been loaded, set up the map pointers */
                if ((neighbour = has_been_loaded_sh(path_sh)) &&
                    (neighbour->in_memory == MAP_IN_MEMORY ||
                     neighbour->in_memory == MAP_LOADING))
                {
                    int dest_tile = MapTiledReverse[tile - 1];

                    /* LOG(llevDebug,"add t_map >%s< (%d). ", path_sh, tile-1); */
                    if (neighbour->orig_tile_path[dest_tile] != m->orig_path)
                    {
                        /* Refuse tiling if anything looks suspicious, since that may leave dangling pointers and crash the server */
                        LOG(llevMapbug, "MAPBUG: map tiles incorrecly connected: >%s<->>%s< but >%s<->>%s<. Refusing to connect them!\n",
                                STRING_MAP_ORIG_PATH(m),
                                (path_sh) ? path_sh : "(no map)",
                                STRING_MAP_ORIG_PATH(neighbour),
                                (neighbour->orig_tile_path[dest_tile]) ? neighbour->orig_tile_path[dest_tile] : "(no map)");

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
        {
            m->tileset_id = atoi(value);
        }
        else if (!strcmp(key, "tileset_x"))
        {
            m->tileset_x = atoi(value);
        }
        else if (!strcmp(key, "tileset_y"))
        {
            m->tileset_y = atoi(value);
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
        if (m->tile_map[i] &&
            m->tile_map[i]->tileset_id != m->tileset_id)
        {
            LOG(llevMapbug, "MAPBUG: connected maps have inconsistent tileset_ids: >%s< (id %d)<->>%s< (id %d). Pathfinding will have problems.\n",
                STRING_MAP_ORIG_PATH(m), m->tileset_id,
                STRING_MAP_ORIG_PATH(m->tile_map[i]),
                m->tile_map[i]->tileset_id);
            /* TODO: also doublecheck tileset_x and tileset_y */
        }
    }
#endif

    if(!MAP_STATUS_TYPE(m->map_status)) /* synchronize dynamically the map status flags */
    {
        m->map_status |= MAP_STATUS_TYPE(flags);

        /* Still zero? then force _MULTI */
        if (!MAP_STATUS_TYPE(m->map_status))
        {
            m->map_status |= MAP_STATUS_MULTI;
        }
    }

    m->map_status |= (flags & MAP_STATUS_ORIGINAL);

    if (!got_end)
    {
        LOG(llevMapbug, "MAPBUG:: >%s<: Got premature eof of map header!\n",
            STRING_MAP_ORIG_PATH(m));

        return 1;
    }

    return 0;
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
static void LoadObjects(mapstruct *m, FILE *fp, int mapflags)
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
        MapSpace *msp;

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

        /* important pre set for the animation/face of a object */
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
                msp = GET_MAP_SPACE_PTR(m,op->x,op->y);
                msp->floor_face = op->face;
                msp->floor_terrain = op->terrain_type;
                msp->floor_light = op->last_sp;
                msp->floor_direction_block = op->block_movement;
#ifdef USE_TILESTRETCHER
                msp->floor_z = op->z;
#endif

                if (QUERY_FLAG(op, FLAG_NO_PASS))
                {
                    msp->floor_flags |= MAP_FLOOR_FLAG_NO_PASS;
                }

                if (QUERY_FLAG(op, FLAG_PLAYER_ONLY))
                {
                    msp->floor_flags |= MAP_FLOOR_FLAG_PLAYER_ONLY;
                }

                goto next;

            case TYPE_FLOORMASK:
                /* We save floor masks direct over a generic mask arch/object
                 * and don't need to store the direction. A mask will not turn
                 * ingame - thats just for the editor and to have one arch. */
                SET_MAP_FACE_MASK(m,op->x,op->y,op->face);

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
                if (EXIT_PATH(op))
                {
                    char buf[MAXPATHLEN];

                    /* Absolute path? */
                    if (*op->slaying == '/')
                    {
                        sprintf(buf, "%s", op->slaying);
                    }
                    else
                    {
                        (void)normalize_path(m->orig_path, op->slaying, buf);
                    }

                    if (check_path(buf, 1) == -1)
                    {
                        LOG(llevMapbug, "MAPBUG:: %s[>%s< %d %d] destination map %s does not exist!\n",
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
    UpdateMapTiles(m);
    m->map_flags &= ~MAP_FLAG_NO_UPDATE; /* turn tile updating on again */
    m->in_memory = MAP_IN_MEMORY;

    /* this is the only place we can insert this because the
    * recursive nature of LoadObjects().
    */
    check_light_source_list(m);
}

/* helper func for LoadObjects()
 * This help function will loop through the map and set the nodes. */
static void UpdateMapTiles(mapstruct *m)
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
static void SaveObjects(mapstruct *m, FILE *fp)
{
    static object *floor_g = NULL,
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
            MapSpace *mp = &m->spaces[x + m->width * y];
            object   *this,
                     *next,
                     *prev,
                     *head;

            /* save first the floor and mask from the node */
            if(mp->floor_face)
            {
                floor_g->terrain_type = mp->floor_terrain;
                floor_g->last_sp = mp->floor_light;
                floor_g->face = mp->floor_face;
                floor_g->x = x;
                floor_g->y = y;
#ifdef USE_TILESTRETCHER
                floor_g->z = mp->floor_z;
#endif

                if ((mp->floor_flags & MAP_FLOOR_FLAG_NO_PASS))
                {
                    SET_FLAG(floor_g, FLAG_NO_PASS);
                }
                else
                {
                    CLEAR_FLAG(floor_g, FLAG_NO_PASS);
                }

                if ((mp->floor_flags & MAP_FLOOR_FLAG_PLAYER_ONLY))
                {
                    SET_FLAG(floor_g, FLAG_PLAYER_ONLY);
                }
                else
                {
                    CLEAR_FLAG(floor_g, FLAG_PLAYER_ONLY);
                }

                /* black object magic... don't do this in the "normal" server code */
                save_object(fp, floor_g, 3);
            }

            if(mp->mask_face)
            {
                fmask_g->face = mp->mask_face;
                fmask_g->x = x;
                fmask_g->y = y;
#ifdef USE_TILESTRETCHER
                floor_g->z = mp->floor_z;
#endif
                save_object(fp, fmask_g, 3);
            }


            /* Now we go through every object on the square. Each object falls
             * into one of three categories:
             *
             * (1) player objects;
             * (2) 'dynamic' objects such as spell effects or mobs; or
             * (3) other objects. */
            for (this = mp->first; this; this = next)
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
                        VALIDATE_NEXT(this, next, prev, mp->first);

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
                                (MOB_DATA(head)) ? query_name(MOB_DATA(head)->spawn_info) : ">NULL<",
                                (MOB_DATA(head)) ? MOB_DATA(head) : NULL);
                        }
                        else
                        {
                            object *info = MOB_DATA(head)->spawn_info;

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
                        VALIDATE_NEXT(this, next, prev, mp->first);

                        continue;
                    }
                    /* This is for mobs whose spawn has been interrupted by a
                     * script so do not have SPAWN_INFO. */
                    else if (QUERY_FLAG(head, FLAG_SCRIPT_MOB))
                    {
                        REMOVE_OBJECT(head, 1);
                        VALIDATE_NEXT(this, next, prev, mp->first);

                        continue;
                    }
                    /* Fired/thrown items are not saved. */
                    else if (QUERY_FLAG(head, FLAG_IS_MISSILE))
                    {
                        REMOVE_OBJECT(head, 1);
                        VALIDATE_NEXT(this, next, prev, mp->first);

                        continue;
                    }
                    else if (head->type == SPAWN_POINT)
                    {
                        /* Handling of the spawn points is much easier as handling the mob.
                        * if the spawn point still control some mobs, we delete the mob  - where ever
                        * it is. Also, set pre spawn value to last mob - so we restore our creature
                        * when we reload this map. */
                        if (head->enemy)
                        {
                            if (head->enemy_count == head->enemy->count &&  /* we have a legal spawn? */
                                !QUERY_FLAG(head->enemy, FLAG_REMOVED) &&
                                !OBJECT_FREE(head->enemy))
                            {
                                head->stats.sp = head->last_sp; /* force a pre spawn setting */
                                head->speed_left += 1.0f;
                                REMOVE_OBJECT(head->enemy, 1);
                                VALIDATE_NEXT(this, next, prev, mp->first);
                                head->enemy = NULL;
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
                            VALIDATE_NEXT(this, next, prev, mp->first);

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
                        QUERY_FLAG(head, FLAG_SCRIPT_MOB) ||
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
                        int xt,
                            yt;

                        xt = head->x;
                        yt = head->y;
                        head->x = this->x - this->arch->clone.x;
                        head->y = this->y - this->arch->clone.y;
                        save_object(fp, head, 3);
                        head->x = xt;
                        head->y = yt;

                        /* remember: if we have remove for example 2 or more objects above, the
                         * this->above WILL be still valid - remove_ob() will handle it right.
                         * IF we get here a valid ptr, ->above WILL be valid too. Always. */
                        REMOVE_OBJECT(head, 0);
                        VALIDATE_NEXT(this, next, prev, mp->first);

                        continue;
                    }

                    save_object(fp, this, 3);

                    if (this->more) // its a head (because we had tails tested before)
                    {
                        REMOVE_OBJECT(this, 0);
                        VALIDATE_NEXT(this, next, prev, mp->first);
                    }
                }
                else
                {
                    if (this->head) // its a tail...
                    {
                        int xt,
                            yt;

                        xt = head->x;
                        yt = head->y;
                        head->x = this->x - this->arch->clone.x;
                        head->y = this->y - this->arch->clone.y;
                        save_object(fp, head, 3);
                        head->x = xt;
                        head->y = yt;

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

/* Remove all players from a map and set a marker.
 *
 * XXX: map_player_link() MUST be called after this or the server has a
 * problem. */
uint16 map_player_unlink(mapstruct *m)
{
    player *pl;
    uint16  num = 0;

    if (!m)
    {
        return 0;
    }

    for (pl = first_player; pl; pl = pl->next)
    {
        if (pl->ob->map == m)
        {
            /* With the new activelist, any player on a reset map
            * was somehow forgotten. This seems to fix it. The
            * problem isn't analyzed, though. Gecko 20050713 */
            activelist_remove(pl->ob);
            remove_ob(pl->ob); /* no walk off check */
            pl->dm_removed_from_map = 1;
            num++;
        }
    }

    return num;
}

/* Reinsert players on a map after they were removed with map_player_unlink().
 * If m is NULL use the savebed (or if flag, the emergency map). Otherwise, if
 * x or y == -1 they will overrule the player map position. */
void map_player_link(mapstruct *m, sint16 x, sint16 y, uint8 flag)
{
    player *pl;

    for (pl = first_player; pl; pl = pl->next)
    {
        if (pl->dm_removed_from_map)
        {
            if (m)
            {
                enter_map(pl->ob, NULL, m, (x == -1) ? pl->ob->x : x,
                          (y == -1) ? pl->ob->y : y, m->map_status, 0);
            }
            else if (!flag)
            {
                enter_map_by_name(pl->ob, pl->savebed_map,
                                  pl->orig_savebed_map, pl->bed_x, pl->bed_y,
                                  pl->bed_status);
            }
            else
            {
                enter_map_by_name(pl->ob, shstr_cons.emergency_mappath,
                                  shstr_cons.emergency_mappath, -1, -1,
                                  MAP_STATUS_MULTI);
            }

            pl->dm_removed_from_map = 0;
            new_draw_info(NDI_UNIQUE, 0, pl->ob, "You have a distinct feeling of deja vu.");
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

#ifdef RECYCLE_TMP_MAPS
/* This writes out information on all the temporary maps.  It is called by
 * swap_map below. */
/* TODO: Don't know if thid works (current servers do not recycle).
 * -- Smacky 20120726 */
static void WriteMapLog(void)
{
    FILE       *fp;
    mapstruct  *map;
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
        if (map->in_memory != MAP_IN_MEMORY && (map->tmpname != NULL) && (strncmp(map->path, "/random", 7)))
        {
            /* the 0 written out is a leftover from the lock number for
               * unique items and second one is from encounter maps.
               * Keep using it so that old temp files continue
               * to work.
               */
            fprintf(fp, "%s:%s:%ld:0:0:%d:0:%d\n",
                    map->path, map->tmpname, MAP_RESET_TIMEOUT(map),
                    map->difficulty, map->darkness);
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
        mapstruct *m = GetLinkedMap();
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
        m->tmpname = strdup_local(cp);

        /* Lock is left over from the lock items - we just toss it now.
         * We use it twice - second one is from encounter, but as we
         * don't care about the value, this works fine. */
        sscanf(cp1, "%d:%d:%d:%d:%d:%d\n",
               &MAP_RESET_TIMEOUT(m), &lock, &lock, &difficulty, &do_los, &darkness);

        m->in_memory = MAP_SWAPPED;
        m->darkness = darkness;
        m->difficulty = difficulty;

        if (darkness == -1)
        {
            darkness = MAX_DARKNESS;
        }

        m->light_value = global_darkness_table[MAX_DARKNESS];
    }

    fclose(fp);
}

/* if on the map and the direct attached maps no player and no perm_load
 * flag set, we can safely swap them out! */
void swap_map(mapstruct *map, int force_flag)
{
    /* lets check some legal things... */
    if (map->in_memory != MAP_IN_MEMORY &&
        map->in_memory != MAP_SAVING)
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

            for (i = 0; i < TILED_MAPS; i++)
            {
                /* if there is a map, is load AND in memory and players on OR perm_load flag set, then... */
                if (map->tile_map[i] &&
                    map->tile_map[i]->in_memory == MAP_IN_MEMORY &&
                    (map->tile_map[i]->player_first ||
                     map->tile_map[i]->perm_load))
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
        if (map->in_memory != MAP_SAVING && // do not save twice
            new_save_map(map, 0) == -1)
        {
            LOG(llevBug, "BUG:: %s/swap_map(): Failed to swap map %s!\n",
                __FILE__, STRING_MAP_PATH(map));
            /* Reset the in_memory flag so that delete map will also free the
             * objects with it. */
            map->in_memory = MAP_IN_MEMORY;
            delete_map(map);
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
int players_on_map(mapstruct *m)
{
    object *tmp = m->player_first;
    int     count = 0;

    for (; tmp; tmp = CONTR(tmp)->map_above)
    {
        count++;
    }

    return count;
}

/* Sets m->darkness to 0 <= value <= MAX_DARKNESS and m->light_value to
 * global_darkness_table[value]. */
void set_map_darkness(mapstruct *m, int value)
{
    if (value < 0 ||
        value > MAX_DARKNESS)
    {
        value = MAX_DARKNESS;
    }

    MAP_DARKNESS(m) = (sint32)value;
    MAP_LIGHT_VALUE(m) = (sint32)global_darkness_table[value];
}

/* Go through the list of maps, swapping or resetting those that need it. */
/*TODO: This will be the one and only function to handle map swaps/resets, but
 * ATM this functionality is still scattered through the code a bit.
 *
 * -- Smacky 20120810 */
void map_check_active(void)
{
    mapstruct *this,
              *next;
#ifdef MAP_MAXOBJECTS
    static uint32 threshold = MAP_MAXOBJECTS;
#endif

    for (this = first_map; this; this = next)
    {
        next = this->next;

        /* When doing a manual reset no need to look at player activity or swap
         * the maps. */
        if (!MAP_MANUAL_RESET(this))
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

            if (this->in_memory == MAP_IN_MEMORY ||
                this->in_memory == MAP_SAVING)
            {
#ifdef MAP_MAXOBJECTS
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
                     * first (though ATM mapstruct does not keep a tally). For now
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
                    for (i = 0; i < TILED_MAPS; i++)
                    {
                        if (this->tile_map[i] &&
                            this->tile_map[i]->in_memory == MAP_IN_MEMORY &&
                            (this->tile_map[i]->player_first ||
                             this->tile_map[i]->perm_load))
                        {
                            noswap = 1;

                            break;
                        }
                    }

                    /* Now swap the map. */
                    if (!noswap)
                    {
#ifdef MAP_MAXOBJECTS
# ifdef DEBUG_MAP
                        LOG(llevDebug, "DEBUG:: %s/map_check_active(): Swapping map >%s< (%u) before its time (%u of %u).\n", 
                            __FILE__, STRING_MAP_PATH(this), this->in_memory, objs,
                            threshold);
# endif
                        swap_map(this, 1);
                        object_gc(); // keep mempool uptodate
#else
# ifdef DEBUG_MAP
                        LOG(llevDebug, "DEBUG:: %s/map_check_active(): Swapping map >%s< (%u)!\n",
                            __FILE__, STRING_MAP_PATH(this), this->in_memory);
# endif
                        swap_map(this, 1);
#endif
                    }
                }
            }
        }

        if (MAP_MANUAL_RESET(this) &&
            MAP_WHEN_RESET(this))
        {
            sint32 countdown = MAP_WHEN_RESET(this) - (ROUND_TAG - ROUND_TAG %
                                                       (uint32)MAX(1, pticks_second)) /
                                                      pticks_second;

             if (countdown > 0 &&
                 (!(countdown % 30) ||
                  countdown <= 10))
            {
                new_info_map(NDI_UNIQUE | NDI_NAVY, this, 0, 0, MAP_INFO_ALL, "Only ~%u~ second%s to map reset!",
                             countdown, (countdown != 1) ? "s" : "");
            }
        }

        if (MAP_MANUAL_RESET(this) ||
            this->in_memory == MAP_SWAPPED)
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
            if ((MAP_UNIQUE(this) ||
                 MAP_INSTANCE(this)) ||
                (MAP_MANUAL_RESET(this) ||
                 this->tmpname) &&
                MAP_WHEN_RESET(this) &&
                MAP_WHEN_RESET(this) <= (ROUND_TAG - ROUND_TAG %
                                          (long unsigned int)MAX(1, pticks_second)) /
                                          pticks_second)
            {
#ifdef DEBUG_MAP
                LOG(llevDebug, "DEBUG:: %s/map_check_active(): Resetting map >%s<!\n",
                    __FILE__, STRING_MAP_PATH(this));
#endif
                clean_tmp_map(this);

                /* On a manual reset we want to immediately reload the source
                 * map which also means we need to juggle any players on it. */
                if (MAP_MANUAL_RESET(this))
                {
                    uint8  anyplayers = (this->player_first) ? 1 : 0;
                    shstr *path_sh = NULL,
                          *orig_path_sh = NULL,
                          *reference_sh = NULL;
                    uint32 status;

                    /* If there are any players on the map, temp remove them. */
                    if (anyplayers)
                    {
                        (void)map_player_unlink(this);
                    }

                    /* We now need to 'save' the objects on map so spawns from
                     * other maps disappear and the spawn point on the other
                     * map knows to respawn, etc. On uniques/instances we in
                     * fact do save the map to disk to prevent item loss. But
                     * on multis there is no need for slow disk access (we're
                     * resetting to source remember) so in fact we're just
                     * pruning such dynamic objects. */
                    if (MAP_UNIQUE(this) ||
                        MAP_INSTANCE(this))
                    {
                        new_save_map(this, 0);
                    }
                    else
                    {
                        SaveObjects(this, NULL);
                    }

                    /* Remember a few details so we can reload the map. */
                    FREE_AND_ADD_REF_HASH(path_sh, this->path);
                    FREE_AND_ADD_REF_HASH(orig_path_sh, this->orig_path);

                    if (this->reference)
                    {
                        FREE_AND_ADD_REF_HASH(reference_sh, this->reference);
                    }

                    status = MAP_STATUS_TYPE(this->map_status);

                    /* Delete the map from memory and reload it. */
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
                /* Otherwise we know there's no-one here, so just delete it. */
                else
                {
                    delete_map(this);
                }
            }
        }
    }
}
