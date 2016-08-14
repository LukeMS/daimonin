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

static void  ShowMapInfo(map_t *m, object_t *who, int list, char *ref);
static char *PrintMapFlags(map_t *m);
static void  ShowMspInfo(msp_t *msp, object_t *who);
static char *PrintMspFlags(uint32 flags);
static char *PrintMspTerrain(uint32 flags);

/* Handles misc. input request - things like hash table, malloc, maps,
 * who, etc.
 */

/* '/motd' displays the MOTD. GMs and SAs can also set the MOTD:
 *   '/motd default' restores the server-set MOTD (actually deletes the
 *   GMASTER-set one).
 *   '/motd <message>' sets the GMASTER-set MOTD. */
int command_motd(object_t *op, char *params)
{
#ifdef MOTD
    if (params &&
        (CONTR(op)->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_GM)))
    {
        char  buf[MEDIUM_BUF];
        FILE *fp;

        sprintf(buf, "%s/%s", settings.localdir, MOTD_FILE);

        if (!strcmp(params, "default"))
        {
            LOG(llevSystem, "restore default motd...\n");

            if ((fp = fopen(buf, "r")) && remove(buf))
                LOG(llevBug, "BUG: Cannot remove %s!\n", buf);
        }
        else
        {
            LOG(llevSystem, "write motd file...\n");

            if (!(fp = fopen(buf, "w")))
                LOG(llevBug, "BUG: Cannot open %s for writing!\n", buf);
            else
            {
                fprintf(fp, "%s", params);
                fclose(fp);
            }
        }
    }
#endif

    display_motd(op);

    return 0;
}

int command_bug(object_t *op, char *params)
{
    char buf[MEDIUM_BUF];

    if (!params)
        return 1;

    strcpy(buf, op->name);
    strcat(buf, " bug-reports: ");
    strncat(buf, ++params, MEDIUM_BUF - strlen(buf));
    buf[MEDIUM_BUF - 1] = '\0';
    bug_report(buf);
    ndi(NDI_ALL | NDI_UNIQUE, 1, NULL, "%s", buf);
    ndi(NDI_UNIQUE, 0, op, "OK, thanks!");

    return 0;
}


/*
 * count_active() returns the number of objects on the list of active objects.
 */

static int count_active()
{
    int     i   = 0;
    object_t *tmp;
    map_t *map;

    for(tmp = active_objects->active_next; tmp != NULL; tmp = tmp->active_next)
        i++;
    for (map = first_map; map; map = map->next)
    {
        for(tmp = map->active_objects->active_next; tmp != NULL; tmp = tmp->active_next)
            i++;
    }
    return i;
}

void malloc_info(object_t *op)
{
    int             nrofmaps, fd;
    int             nrm = 0, mapmem = 0, anr, anims, sum_alloc = 0, sum_used = 0, i, tlnr, alnr;
    treasurelist   *tl;
    map_t      *m;
    archetype_t      *at;
    artifactlist_t   *al;

    for (tl = first_treasurelist,tlnr = 0; tl != NULL; tl = tl->next,tlnr++)
        ;
    for (al = first_artifactlist, alnr = 0; al != NULL; al = al->next, alnr++)
        ;
    for (at = first_archetype,anr = 0,anims = 0; at != NULL; at = at->more == NULL ? at->next : at->more,anr++)
        ;
    for (i = 1; i < num_animations; i++)
        anims += animations[i].num_animations;

    for (m = first_map,nrofmaps = 0; m != NULL; m = m->next,nrofmaps++)
        if (m->in_memory == MAP_MEMORY_ACTIVE)
        {
            mapmem += MAP_WIDTH(m) * MAP_HEIGHT(m) * (sizeof(object_t *) + sizeof(msp_t));
            nrm++;
        }

    ndi(NDI_UNIQUE, 0, op, "Sizeof: object=%ld  player=%ld  socketbuf=%ld  map=%ld",
            (long)sizeof(object_t), (long) (sizeof(player_t) + MAXSOCKBUF_IN * 2),
            (long)SOCKET_BUFSIZE_SEND+SOCKET_BUFSIZE_READ, (long)sizeof(map_t));
    dump_mempool_statistics(op, &sum_used, &sum_alloc);
    ndi(NDI_UNIQUE, 0, op, "%4d active objects", count_active());
    ndi(NDI_UNIQUE, 0, op, "%4d player(s) using buffers: %d", player_active,
            i = (player_active * (MAXSOCKBUF_IN * 2)));
    sum_alloc += i;
    ndi(NDI_UNIQUE, 0, op, "%d socket(s) allocated: %d", socket_info.allocated_sockets,
            i = (socket_info.allocated_sockets * (sizeof(NewSocket) + SOCKET_BUFSIZE_SEND+SOCKET_BUFSIZE_READ)));
    sum_alloc += i;

#ifndef WIN32 /* non windows */
# ifdef HAVE_SYSCONF
    fd = sysconf(_SC_OPEN_MAX);
# else
#  ifdef HAVE_GETDTABLESIZE
    fd = getdtablesize();
#  else
#   error "Unable to find usable function to get max filedescriptors"
#  endif
# endif
#else
    fd = -3;
#endif

    ndi(NDI_UNIQUE, 0, op, "socket max fd: %d  (%d %s) ncom: %d ", socket_info.max_filedescriptor, fd,
            fd != -3 ? "avaible" : "win32/ignore", socket_info.nconns);
    ndi(NDI_UNIQUE, 0, op, "%4d maps allocated:  %d", nrofmaps, i = (nrofmaps * sizeof(map_t)));
    sum_alloc += i;  sum_used += nrm * sizeof(map_t);
    ndi(NDI_UNIQUE, 0, op, "%4d maps in memory:  %8d", nrm, mapmem);
    sum_alloc += mapmem; sum_used += mapmem;
    ndi(NDI_UNIQUE, 0, op, "%4d archetypes:      %8d", anr, i = (anr * sizeof(archetype_t)));
    sum_alloc += i; sum_used += i;
    ndi(NDI_UNIQUE, 0, op, "%4d animations:      %8d", anims, i = (anims * sizeof(unsigned short)));
    sum_alloc += i; sum_used += i;
    ndi(NDI_UNIQUE, 0, op, "%4d spells:          %8d", NROFREALSPELLS, i = (NROFREALSPELLS * sizeof(spell)));
    sum_alloc += i; sum_used += i;
    ndi(NDI_UNIQUE, 0, op, "%4d treasurelists    %8d", tlnr, i = (tlnr * sizeof(treasurelist)));
    sum_alloc += i; sum_used += i;
    ndi(NDI_UNIQUE, 0, op, "%4ld treasures        %8d", nroftreasures, i = (nroftreasures * sizeof(treasure)));
    sum_alloc += i; sum_used += i;
    ndi(NDI_UNIQUE, 0, op, "%4ld artifacts        %8d", nrofartifacts, i = (nrofartifacts * sizeof(artifact)));
    sum_alloc += i; sum_used += i;
    ndi(NDI_UNIQUE, 0, op, "%4ld artifacts strngs %8d", nrofallowedstr, i = (nrofallowedstr * sizeof(shstr_linked_t)));
    sum_alloc += i;sum_used += i;
    ndi(NDI_UNIQUE, 0, op, "%4d artifactlists    %8d", alnr, i = (alnr * sizeof(artifactlist_t)));
    sum_alloc += i; sum_used += i;

    ndi(NDI_UNIQUE, 0, op, "Total space allocated:%8d", sum_alloc);
    ndi(NDI_UNIQUE, 0, op, "Total space used:     %8d", sum_used);
}

/* Lists online players, respecting privacy mode. */
int command_who(object_t *op, char *params)
{
    char *cp;

    if (!op)
    {
        return 0;
    }

    if (params)
    {
        return 1;
    }

    cp = get_online_players_info(CONTR(op), NULL, 0);

    /* Skip the time of rewrite and player numbers data. */
    while (isxdigit(*cp) ||
           isspace(*cp))
    {
        cp++;
    }

    ndi(NDI_UNIQUE, 0, op, "There %s %d player%s online.\n\n%s",
                  (player_active == 1) ? "is" : "are", player_active,
                  (player_active == 1) ? "" : "s", cp);

    return 0;
}

int command_malloc(object_t *op, char *params)
{
#ifdef MEMPOOL_TRACKING
    if (params)
    {
        int force_flag = 0, i;

        if (strcmp(params, "free") && strcmp(params, "force"))
            return 1;

        if (strcmp(params, "force") == 0)
            force_flag = 1;

        for (i = 0; i < nrof_mempools; i++)
            if (force_flag == 1 || mempools[i]->flags & MEMPOOL_ALLOW_FREEING)
                free_empty_puddles(mempools[i]);
    }
#endif

    malloc_info(op);
    return 0;
}

/* Print map info. */
int command_mapinfo(object_t *op, char *params)
{
    player_t *pl;
    map_t    *m;

    if (!op ||
        !(pl = CONTR(op)) ||
        !(m = op->map))
    {
        return 0;
    }

    /* Only MWs/MMs/SAs can use the fancy commands. */
    if (params &&
#ifdef DAI_DEVELOPMENT_CONTENT
        (pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_MM | GMASTER_MODE_MW))
#else
        (pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_MM))
#endif
        )
    {
        /* List all the loaded maps. */
        if (!strcmp(params, "all"))
        {
            int i;

            for (m = first_map, i = 1; m; m = m->next, i++)
            {
                ShowMapInfo(m, op, i, NULL);
            }
        }
        /* Detail the current map and list all the immediately tiled ones. */
        else if (!strcmp(params, "tiled"))
        {
            int   i;
            char *compass[] = { "N", "E", "S", "W", "NE", "SE", "SW", "NW" };

            ShowMapInfo(m, op, 0, NULL);

            for (i = 0; i < 8; i++)
            {
                if (!m->tiling.tile_path[i])
                {
                    continue;
                }

                if (!m->tiling.tile_map[i])
                {
                    ndi(NDI_UNIQUE, 0, op, "~%s Map~: NOT LOADED",
                        compass[i]);
                    continue;
                }

                ShowMapInfo(m->tiling.tile_map[i], op, i + 1, compass[i]);
            }
        }
    }
    /* Detail the current map (amount of detail according to gmaster_mode,
     * handled by ShowMapInfo). */
    else
    {
        ShowMapInfo(m, op, 0, NULL);
    }

    return 0;
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
static void ShowMapInfo(map_t *m, object_t *who, int list, char *ref)
{
    uint32  seconds = (ROUND_TAG - ROUND_TAG % (long unsigned int)MAX(1, pticks_second)) / pticks_second;

    if (list <= 0)
    {
        ndi(NDI_UNIQUE, 0, who, "~Name~: %s", STRING_SAFE(m->name));
        ndi(NDI_UNIQUE, 0, who, "~Msg~: %s", STRING_SAFE(m->msg));
        ndi(NDI_UNIQUE, 0, who, "~Music~: %s", STRING_SAFE(m->music));

        if (who->map == m)
        {
            ndi(NDI_UNIQUE, 0, who, "~Position~: %d, %d", who->x, who->y);
        }

        ndi(NDI_UNIQUE, 0, who, "~Path~: %s", STRING_MAP_PATH(m));
        ndi(NDI_UNIQUE, 0, who, "~Orig Path~: %s", STRING_MAP_ORIG_PATH(m));

#ifdef DAI_DEVELOPMENT_CONTENT
        if ((CONTR(who)->gmaster_mode & (GMASTER_MODE_MW | GMASTER_MODE_MM | GMASTER_MODE_SA)))
#else
        if ((CONTR(who)->gmaster_mode & (GMASTER_MODE_MM | GMASTER_MODE_SA)))
#endif
        {
            ndi(NDI_UNIQUE, 0, who, "~Status~: %s%s%s (%u)",
                (((m->status & MAP_STATUS_MULTI)) ? "Multiplayer" :
                 (((m->status & MAP_STATUS_UNIQUE)) ? "Unique" :
                  (((m->status & MAP_STATUS_INSTANCE)) ? "Instance" : "UNKNOWN"))),
                (m->reference) ? "/" : "", (m->reference) ? m->reference : "",
                m->in_memory);
            ndi(NDI_UNIQUE, 0, who, "~Swap~: %d (%u)", (sint32)(MAP_WHEN_SWAP(m) - seconds), MAP_SWAP_TIMEOUT(m));

            if (MAP_WHEN_RESET(m) == 0)
            {
                ndi(NDI_UNIQUE, 0, who, "~Reset~: Never");
            }
            else
            {
                ndi(NDI_UNIQUE, 0, who, "~Reset~: %d (%u)", (sint32)(MAP_WHEN_RESET(m) - seconds), MAP_RESET_TIMEOUT(m));
            }

            ndi(NDI_UNIQUE, 0, who, "~Size~: %dx%d (%d, %d)", MAP_WIDTH(m), MAP_HEIGHT(m), MAP_ENTER_X(m), MAP_ENTER_Y(m));
            ndi(NDI_UNIQUE, 0, who, "~Darkness/Brightness~: %d/%d", MAP_DARKNESS(m), MAP_LIGHT_VALUE(m));
            ndi(NDI_UNIQUE, 0, who, "~Difficulty~: %d", MAP_DIFFICULTY(m));

            if (m->tiling.tileset_id)
            {
                ndi(NDI_UNIQUE, 0, who, "~Tileset ID/X/Y~: %d/%d/%d", m->tiling.tileset_id, m->tiling.tileset_x, m->tiling.tileset_y);
            }

            ndi(NDI_UNIQUE, 0, who, "~Flags~: %s", PrintMapFlags(m));
            ndi(NDI_UNIQUE, 0, who, "~Players~: %d", players_on_map(m));
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
            sprintf(strchr(buf, '\0'), "...%s", STRING_MAP_PATH(m) + (len - 1 - 12));
        }
        else
        {
            sprintf(strchr(buf, '\0'), "%s", STRING_MAP_PATH(m));
        }

        sprintf(strchr(buf, '\0'), ": %s%s%s (%u)",
            (((m->status & MAP_STATUS_MULTI)) ? "M" :
             (((m->status & MAP_STATUS_UNIQUE)) ? "U" :
              (((m->status & MAP_STATUS_INSTANCE)) ? "I" : "X"))),
            (m->reference) ? "/" : "", (m->reference) ? m->reference : "",
            m->in_memory);
        sprintf(strchr(buf, '\0'), ", %d (%u)", (sint32)(MAP_WHEN_SWAP(m) - seconds), MAP_SWAP_TIMEOUT(m));

        if (MAP_WHEN_RESET(m) == 0)
        {
            sprintf(strchr(buf, '\0'), ", x (x)");
        }
        else
        {
            sprintf(strchr(buf, '\0'), ", %d (%u)", (sint32)(MAP_WHEN_RESET(m) - seconds), MAP_RESET_TIMEOUT(m));
        }

        sprintf(strchr(buf, '\0'), ", %d", MAP_DIFFICULTY(m));
        sprintf(strchr(buf, '\0'), ", %d/%d/%d", m->tiling.tileset_id, m->tiling.tileset_x, m->tiling.tileset_y);
        sprintf(strchr(buf, '\0'), ", %s", PrintMapFlags(m));
        sprintf(strchr(buf, '\0'), ", @%d", players_on_map(m));
        ndi(NDI_UNIQUE, 0, who, "%s", buf);
    }
}

static char *PrintMapFlags(map_t *m)
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

    if ((m->flags & MAP_FLAG_NO_SUMMON))
    {
        sprintf(strchr(buf, '\0'), "%c", '#');
    }

    if (MAP_FIXEDLOGIN(m))
    {
        sprintf(strchr(buf, '\0'), "%c", 'L');
    }

    if ((m->flags & MAP_FLAG_PERMDEATH))
    {
        sprintf(strchr(buf, '\0'), "%c", '_');
    }

    if ((m->flags & MAP_FLAG_ULTRADEATH))
    {
        sprintf(strchr(buf, '\0'), "%c", '-');
    }

    if ((m->flags & MAP_FLAG_ULTIMATEDEATH))
    {
        sprintf(strchr(buf, '\0'), "%c", '=');
    }

    return buf;
}

/* Print msp info. */
int command_mspinfo(object_t *op, char *params)
{
    msp_t *msp;

    if (!op ||
        !CONTR(op) ||
        !op->map)
    {
        return 0;
    }

    msp = MSP_RAW(op->map, op->x, op->y);
    ShowMspInfo(msp, op);
    return 0;
}

static void ShowMspInfo(msp_t *msp, object_t *who)
{
    ndi(NDI_UNIQUE, 0, who, "~Flooding brightness~: %d", msp->flooding_brightness);
    ndi(NDI_UNIQUE, 0, who, "~Floor darkness/brightness~: %d/%d",
        (msp->floor_flags & MSP_FLAG_DAYLIGHT) ? msp->map->tadnow->daylight_brightness : msp->map->ambient_brightness);
    ndi(NDI_UNIQUE, 0, who, "~Real brightness~: %d", MSP_GET_REAL_BRIGHTNESS(msp));
    ndi(NDI_UNIQUE, 0, who, "~Floor face~: %s", (msp->floor_face) ? msp->floor_face->name : "NONE");
    ndi(NDI_UNIQUE, 0, who, "~Fmask face~: %s", (msp->mask_face) ? msp->mask_face->name : "NONE");
    ndi(NDI_UNIQUE, 0, who, "~Floor flags~: %s", PrintMspFlags(msp->floor_flags));
    ndi(NDI_UNIQUE, 0, who, "~Flags~: %s", PrintMspFlags(msp->flags));
    ndi(NDI_UNIQUE, 0, who, "~Floor terrain~: %s", PrintMspTerrain(msp->floor_terrain));
    ndi(NDI_UNIQUE, 0, who, "~Terrain~: %s", PrintMspTerrain(msp->move_flags));
}

static char *PrintMspFlags(uint32 flags)
{
    static char buf[24];

    buf[0] = '\0';

    if ((flags & MSP_FLAG_DAYLIGHT))
    {
        sprintf(strchr(buf, '\0'), "%c", 'D');
    }

    if ((flags & MSP_FLAG_PVP))
    {
        sprintf(strchr(buf, '\0'), "%c", '!');
    }

    if ((flags & MSP_FLAG_NO_SPELLS))
    {
        sprintf(strchr(buf, '\0'), "%c", '\\');
    }

    if ((flags & MSP_FLAG_NO_PRAYERS))
    {
        sprintf(strchr(buf, '\0'), "%c", '/');
    }

    if ((flags & MSP_FLAG_NO_HARM))
    {
        sprintf(strchr(buf, '\0'), "%c", 'X');
    }

    if ((flags & MSP_FLAG_PLAYER_ONLY))
    {
        sprintf(strchr(buf, '\0'), "%c", 'O');
    }

    if ((flags & MSP_FLAG_NO_PASS))
    {
        sprintf(strchr(buf, '\0'), "%c", 'X');
    }

    if ((flags & MSP_FLAG_PASS_THRU))
    {
        sprintf(strchr(buf, '\0'), "%c", '-');
    }

    if ((flags & MSP_FLAG_PASS_ETHEREAL))
    {
        sprintf(strchr(buf, '\0'), "%c", '=');
    }

    if ((flags & MSP_FLAG_ALIVE))
    {
        sprintf(strchr(buf, '\0'), "%c", '*');
    }

    if ((flags & MSP_FLAG_PLAYER))
    {
        sprintf(strchr(buf, '\0'), "%c", '@');
    }

    if ((flags & MSP_FLAG_PLAYER_PET))
    {
        sprintf(strchr(buf, '\0'), "%c", 'd');
    }

    if ((flags & MSP_FLAG_BLOCKSVIEW))
    {
        sprintf(strchr(buf, '\0'), "%c", 'x');
    }

    if ((flags & MSP_FLAG_DOOR_CLOSED))
    {
        sprintf(strchr(buf, '\0'), "%c", '+');
    }

    if ((flags & MSP_FLAG_WALK_ON))
    {
        sprintf(strchr(buf, '\0'), "%c", '>');
    }

    if ((flags & MSP_FLAG_WALK_OFF))
    {
        sprintf(strchr(buf, '\0'), "%c", '<');
    }

    if ((flags & MSP_FLAG_FLY_ON))
    {
        sprintf(strchr(buf, '\0'), "%c", '}');
    }

    if ((flags & MSP_FLAG_FLY_OFF))
    {
        sprintf(strchr(buf, '\0'), "%c", '{');
    }

    if ((flags & MSP_FLAG_REFL_MISSILE))
    {
        sprintf(strchr(buf, '\0'), "%c", ')');
    }

    if ((flags & MSP_FLAG_REFL_CASTABLE))
    {
        sprintf(strchr(buf, '\0'), "%c", '(');
    }

    if ((flags & MSP_FLAG_MAGIC_EAR))
    {
        sprintf(strchr(buf, '\0'), "%c", '?');
    }

    if ((flags & MSP_FLAG_CHECK_INV))
    {
        sprintf(strchr(buf, '\0'), "%c", '_');
    }

    if ((flags & MSP_FLAG_PLAYER_GRAVE))
    {
        sprintf(strchr(buf, '\0'), "%c", '%');
    }

    return buf;
}

static char *PrintMspTerrain(uint32 flags)
{
    static char buf[24];

    buf[0] = '\0';

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

    return buf;
}

/* Writes the current tad to op.
 * Eventually this command will also update the client's clock. */
int command_time(object_t *op, char *params)
{
    char      *cp = params;
    sint32     offset = 0;
    int        flags = 0;
    map_t *m;

    if (!op ||
        op->type != PLAYER ||
        !CONTR(op))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

    do
    {
        char buf[MEDIUM_BUF];

        cp = get_token(cp, buf, 0);

        if (buf[0] != '\0')
       {
           int i;

            if ((i = atoi(buf)))
            {
                offset = i;
            }
            else
            {
                shstr_t *token_sh = NULL;

                SHSTR_FREE_AND_ADD_STRING(token_sh, buf);

                if (token_sh == subcommands.list)
                {
                    ndi(NDI_UNIQUE, 0, op,
                        "%u minutes per hour\n"
                        "%u hours per day\n"
                        "%u days per parweek\n"
                        "%u parweeks per week\n"
                        "%u weeks per month\n"
                        "%u months per season\n"
                        "%u seasons per year",
                        ARKHE_MES_PER_HR,
                        ARKHE_HRS_PER_DY,
                        ARKHE_DYS_PER_PK,
                        ARKHE_PKS_PER_WK,
                        ARKHE_WKS_PER_MH,
                        ARKHE_MHS_PER_SN,
                        ARKHE_SNS_PER_YR);
                }
                else if (token_sh == subcommands.verbose)
                {
                    flags |= TAD_SHOWTIME | TAD_SHOWDATE | TAD_SHOWSEASON |
                        TAD_LONGFORM;
                }
                else if (token_sh == subcommands.showtime)
                {
                    flags |= TAD_SHOWTIME;
                }
                else if (token_sh == subcommands.showdate)
                {
                    flags |= TAD_SHOWDATE;
                }
                else if (token_sh == subcommands.showseason)
                {
                    flags |= TAD_SHOWSEASON;
                }
                else
                {
                    ndi(NDI_UNIQUE, 0, op, "%s is not a valid parameter!",
                        token_sh);
                    SHSTR_FREE(token_sh);
                    return COMMANDS_RTN_VAL_SYNTAX;
                }

                SHSTR_FREE(token_sh);
            }
        }
    }
    while (cp);

    if (flags == 0)
    {
        flags = TAD_SHOWTIME | TAD_SHOWDATE | TAD_SHOWSEASON;
    }

    m = parent_map(op);
    get_tad(m->tadnow, m->tadoffset);

    if (!offset)
    {
        ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "It is %s.",
            print_tad(m->tadnow, flags));
    }
    else
    {
        timeanddate_t tad;

        memcpy(&tad, m->tadnow, sizeof(timeanddate_t));
        get_tad(&tad, m->tadoffset + offset);

        if (offset > 0)
        {
            ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "In %d hours from now it will be %s.",
                offset, print_tad(&tad, flags));
        }
        else if (offset < 0)
        {
            ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "%d hours ago it was %s.",
                ABS(offset), print_tad(&tad, flags));
        }
    }

    return COMMANDS_RTN_VAL_OK;
}

/*
 * Those dumps should be just one dump with good parser
 */

int command_dumpbelowfull(object_t *op, char *params)
{
    msp_t *msp = MSP_KNOWN(op);
    object_t *tmp,
           *next;

    ndi(NDI_UNIQUE, 0, op, "DUMP OBJECTS OF THIS TILE");
    ndi(NDI_UNIQUE, 0, op, "-------------------");

    FOREACH_OBJECT_IN_MSP(tmp, msp, next)
    {
        if (tmp == op) /* exclude the DM player object */
            continue;

        dump_object(tmp);
        ndi(NDI_UNIQUE, 0, op, "%s", errmsg);

        if (tmp->below && tmp->below != op)
            ndi(NDI_UNIQUE, 0, op, ">next object<");
    }

    ndi(NDI_UNIQUE, 0, op, "------------------");

    return 0;
}

int command_dumpbelow(object_t *op, char *params)
{
    msp_t *msp = MSP_KNOWN(op);
    object_t *tmp,
           *next;
    int     i   = 0;

    ndi(NDI_UNIQUE, 0, op, "DUMP OBJECTS OF THIS TILE");
    ndi(NDI_UNIQUE, 0, op, "-------------------");

    FOREACH_OBJECT_IN_MSP(tmp, msp, next)
    {
        if (tmp == op) /* exclude the DM player object */
            continue;

        ndi(NDI_UNIQUE, 0, op, "#%d  >%s<  >%s<  >%s<",
            ++i, STRING_OBJ_NAME(tmp), STRING_OBJ_ARCH_NAME(tmp),
            STRING_OBJ_NAME(tmp->env));
    }

    ndi(NDI_UNIQUE, 0, op, "------------------");

    return 0;
}

int command_dumpallobjects(object_t *op, char *params)
{
#ifdef MEMPOOL_TRACKING
    struct puddle_info *puddle = pool_object->first_puddle_info;
    unsigned int i;
    object_t *obj;

    while(puddle)
    {
        for(i=0; i<pool_object->expand_size; i++)
        {
            obj = MEM_USERDATA((char *)puddle->first_chunk + i * (sizeof(struct mempool_chunk) + pool_object->chunksize));

            if(! OBJECT_FREE(obj))
                LOG(llevDebug, "obj '%s'-(%s) %p (%d)(%s) #=%d\n", STRING_OBJ_NAME(obj),
                        STRING_OBJ_ARCH_NAME(obj), obj, obj->count,
                        QUERY_FLAG(obj, FLAG_REMOVED) ? "removed" : "in use",
                        obj->nrof);
        }
        puddle = puddle->next;
    }
#endif

    return 0;
}

int command_dumpallarchetypes(object_t *op, char *params)
{
    dump_all_archetypes();

    return 0;
}
int command_dumpactivelist(object_t *op, char *params)
{
    char    buf[1024];
    int     count   = 0;
    object_t *tmp;

    for (tmp = active_objects->active_next; tmp; tmp = tmp->active_next)
    {
        count++;
        sprintf(buf, "%s[%d] (%s) %d %f",
            STRING_OBJ_NAME(tmp), TAG(tmp), STRING_OBJ_ARCH_NAME(tmp),
            tmp->type, tmp->speed);
        /*ndi(NDI_UNIQUE, 0,op, "%s", buf); It will overflow the send buffer with many player online */
        LOG(llevSystem, "%s\n", buf);
    }

    sprintf(buf, "active objects: %d (dumped to log)", count);
    ndi(NDI_UNIQUE, 0, op, "%s", buf);
    LOG(llevSystem, "%s\n", buf);

    return 0;
}

int command_setmaplight(object_t *op, char *params)
{
    int i;

    if (!params ||
        !sscanf(params, "%d", &i))
    {
        return 1;
    }

    i = MAX(-MAX_DARKNESS, MIN(i, MAX_DARKNESS));
    op->map->ambient_darkness = i;
    op->map->ambient_brightness = (i < 0) ? -(brightness[ABS(i)]) : brightness[ABS(i)];
    ndi(NDI_UNIQUE, 0, op, "set map darkness: %d -> map:%s",
        i, STRING_MAP_PATH(op->map));
    return 0;
}

#if 0
int command_dumpmap(object_t *op, char *params)
{
    if (op)
        dump_map(op->map, CONTR(op));

    return 0;
}

int command_dumpallmaps(object_t *op, char *params)
{
    dump_all_maps();

    return 0;
}
#endif

void bug_report(char *reportstring)
{
    /*
     FILE * fp;
     if((fp = fopen( BUG_LOG , "a")) != NULL){
         fprintf(fp,"%s\n", reportstring);
         fclose(fp);
     } else {
         perror(BUG_LOG);
     }
     */
}

/*
 * Actual commands.
 * Those should be in small separate files (c_object.c, c_wiz.c, cmove.c,...)
 */

#if 0
static void help_topics(object_t *op, int what)
{
    DIR            *dirp;
    struct dirent  *de;
    char            filename[MEDIUM_BUF], line[80];
    int             namelen, linelen = 0;

    switch (what)
    {
        case 1:
          sprintf(filename, "%s/wizhelp", HELPDIR);
          ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "\nWiz commands:");
          break;
        case 3:
          sprintf(filename, "%s/help", HELPDIR);
          ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "Topics:");
          break;
        default:
          sprintf(filename, "%s/commands", HELPDIR);
          ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "Commands:");
          break;
    }
    if (!(dirp = opendir(filename)))
        return;

    line[0] = '\0';
    for (de = readdir(dirp); de; de = readdir(dirp))
    {
        namelen = NAMLEN(de);
        if (!strcmp(de->d_name, "CVS") || (*de->d_name == '.'))
            continue;
        linelen += namelen + 1;
        if (linelen > 42)
        {
            ndi(NDI_UNIQUE, 0, op, "%s", line);
            sprintf(line, " %s", de->d_name);
            linelen = namelen + 1;
            continue;
        }
        strcat(line, " ");
        strcat(line, de->d_name);
    }
    ndi(NDI_UNIQUE, 0, op, "%s", line);
    closedir(dirp);
}
#endif

int command_resting(object_t *op, char *params)
{
    player_t *pl = CONTR(op);

    /* sitting is the way we enter resting */
    if (pl->rest_sitting) /* we allready sit? stand up! */
    {
        ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "You stop resting.");
        pl->rest_mode = pl->rest_sitting = 0;
        pl->resting_reg_timer = 0;
    }
    else
    {
        ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "You start resting.");
        remove_food_force(op); /* sitting will interrupt our eating - we enter resting */
        pl->food_status = 1000;
        pl->damage_timer = 0;

        /* force a combat mode leave... we don't fight when sitting on our butt! */
        pl->combat_mode = 1;
        command_combat(op, NULL);

        pl->rest_mode = pl->rest_sitting = 1;
        pl->resting_reg_timer = RESTING_DEFAULT_SEC_TIMER;
    }

    return 0;
}

static void show_help(char *fname, player_t *pl)
{
    FILE *fp;
    char  buf[MEDIUM_BUF];

    if ((fp = fopen(fname, "r")) == NULL)
    {
        ndi(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "No more available!");

        return;
    }

    while (fgets(buf, (int)sizeof(buf), fp))
    {
        int len;

        len = (int)strlen(buf) - 1;

        if (buf[len] == '\n')
            buf[len] = (len) ? '\0' : ' ';

        ndi(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "%s", buf);
    }

    fclose(fp);
}

static void show_commands(player_t *pl)
{
    char         name[7][TINY_BUF] = { "", "", "", "", "", "", "" };
    CommArray_s *ap[7] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    int          size[7] = { -1, -1, -1, -1, -1, -1, -1 },
                 i = 0;

    sprintf(name[i], "Normal Commands");
    ap[i] = Commands;
    size[i++] = CommandsSize;
    sprintf(name[i], "Emotes");
    ap[i] = EmoteCommands;
    size[i++] = EmoteCommandsSize;

    if ((pl->gmaster_mode & GMASTER_MODE_SA))
    {
        sprintf(name[i], "SA Commands");
        ap[i] = CommandsSA;
        size[i++] = CommandsSASize;
    }

    if ((pl->gmaster_mode & GMASTER_MODE_MM))
    {
        sprintf(name[i], "MM Commands");
        ap[i] = CommandsMM;
        size[i++] = CommandsMMSize;
    }

    if ((pl->gmaster_mode & GMASTER_MODE_MW))
    {
        sprintf(name[i], "MW Commands");
        ap[i] = CommandsMW;
        size[i++] = CommandsMWSize;
    }

    if ((pl->gmaster_mode & GMASTER_MODE_GM))
    {
        sprintf(name[i], "GM Commands");
        ap[i] = CommandsGM;
        size[i++] = CommandsGMSize;
    }

    if ((pl->gmaster_mode & GMASTER_MODE_VOL))
    {
        sprintf(name[i], "VOL Commands");
        ap[i] = CommandsVOL;
        size[i++] = CommandsVOLSize;
    }

    for (i = 0; i < 6 && ap[i]; i++)
    {
        int  j;
        char buf[MEDIUM_BUF] = "";

        ndi(NDI_UNIQUE | NDI_YELLOW, 0, pl->ob, "\n%s", name[i]);

        for (j = 0; j < size[i]; j++)
        {
            /* TODO: This calculation can be removed, and the following
             * ndi() moved to inside this for loop by having the
             * client handle NDI_UNIQUE properly (well, at all).
             * -- Smacky 20090604 */
            if (strlen(buf) + strlen(ap[i][j].name) > 42)
            {
                ndi(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "%s", buf);
                buf[0] = '\0';
            }

            sprintf(strchr(buf, '\0'), " /%s ~+~", ap[i][j].name);
        }

        ndi(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "%s", buf);
    }
}

int command_help(object_t *op, char *params)
{
    player_t *pl;
    int     len;

    if (!op ||
        !(pl = CONTR(op)))
        return 0;

    /* Main help page */
    if (!params)
    {
        char buf[MEDIUM_BUF];

        sprintf(buf, "%s/index", HELPDIR);
        show_help(buf, pl);

        return 0;
    }

    /* Individual /command */
    if (params[0] == '/')
    {
        CommArray_s *csp;
        char         buf[MEDIUM_BUF];

//        if (strpbrk(params + 1, " ./\\"))
//        {
//            ndi(NDI_UNIQUE, 0, op, "Illegal characters in '%s'", params);
//
//            return 0;
//        }
        ndi(NDI_UNIQUE | NDI_YELLOW, 0, pl->ob, "Help for command %s:",
                             params);

        if (!(csp = find_command(params + 1, NULL)))
        {
            ndi(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "Unrecognised command!");

            return 0;
        }

        sprintf(buf, "%s/commands/%s", HELPDIR, params + 1);
        show_help(buf, pl);

        return 0;
    }

    len = (int)strlen(params);

    /* TODO: Categories list */
    if (!strncasecmp("categories", params, len))
        return 0;

    /* Commands list */
    if (!strncasecmp("commands", params, len))
    {
        show_commands(pl);

        return 0;
    }

    /* TODO: Individual category. */

    /* Unknown topic */
    ndi(NDI_UNIQUE, 0, pl->ob, "No help available on '%s'",
                         params);

    return 0;
}

int command_privacy(object_t *op, char *params)
{
    int new_status = !CONTR(op)->privacy;

    CONTR(op)->privacy = new_status;

    if (new_status)
        ndi(NDI_UNIQUE, 0, op, "Privacy enabled.");
    else
        ndi(NDI_UNIQUE, 0, op, "Privacy disabled.");

#ifdef USE_CHANNELS
    channel_privacy(CONTR(op), CONTR(op)->privacy);
#endif

    (void)get_online_players_info(NULL, CONTR(op), 1);

    return 0;
}

/* get_subdir creates the directory for the player file structure
 * e.g. Zergus -> z/ze
 * '_' is used for non-letters */
char *get_subdir(const char *name)
{
    static char subdir[5];
    const char rest_dir = '_';

    if(isalpha(name[0]))
        subdir[0] = tolower(name[0]);
    else
        subdir[0] = rest_dir;
    subdir[1] = '/';
    subdir[2] = subdir[0];
    if(isalpha(name[1]))
        subdir[3] = tolower(name[1]);
    else
        subdir[3] = rest_dir;
    subdir[4] = '\0';
    return subdir;
}

/* '/stuck' teleports the player to a location near the beginning, just outside
 * of the unofficial maps lobby.
 *
 * '/stuck name' teleports the player to the named MWs private lobby. If name
 * is not a MW then no map will be found, which goto will handle. */
int command_stuck(object_t *op, char *params)
{
    if (params)
    {
        char buf[TINY_BUF];

        sprintf(buf, "/unofficial/%s/private_lobby", params);

        return command_goto(op, buf);
    }
    else
    {
        return command_goto(op, "/planes/human_plane/castle/castle_030a 5 11");
    }
}

int command_level(object_t *op, char *params)
{
    player_t *pl;

    if (!params)
    {
        ndi(NDI_UNIQUE, 0, op, "You are level ~%d~.", op->level);
    }
    else if (!(pl = find_player(params)))
    {
        ndi(NDI_UNIQUE, 0, op, "No such player.");

        return COMMANDS_RTN_VAL_ERROR;
    }
    else
    {
        ndi(NDI_UNIQUE, 0, op, "|%s| is level ~%d~.",
            pl->quick_name, pl->ob->level);
    }

    return COMMANDS_RTN_VAL_OK;
}
