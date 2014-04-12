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


/* Handles misc. input request - things like hash table, malloc, maps,
 * who, etc.
 */

/* now redundant function */
int command_spell_reset(object *op, char *params)
{
    /*init_spell_param(); */
    return 0;
}

/* '/motd' displays the MOTD. GMs and SAs can also set the MOTD:
 *   '/motd default' restores the server-set MOTD (actually deletes the
 *   GMASTER-set one).
 *   '/motd <message>' sets the GMASTER-set MOTD. */
int command_motd(object *op, char *params)
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

int command_bug(object *op, char *params)
{
    char buf[MEDIUM_BUF];

    if (!params)
        return 1;

    strcpy(buf, op->name);
    strcat(buf, " bug-reports: ");
    strncat(buf, ++params, MEDIUM_BUF - strlen(buf));
    buf[MEDIUM_BUF - 1] = '\0';
    bug_report(buf);
    new_draw_info(NDI_ALL | NDI_UNIQUE, 1, NULL, "%s", buf);
    new_draw_info(NDI_UNIQUE, 0, op, "OK, thanks!");

    return 0;
}


/*
 * count_active() returns the number of objects on the list of active objects.
 */

static int count_active()
{
    int     i   = 0;
    object *tmp;
    mapstruct *map;

    for(tmp = active_objects->active_next; tmp != NULL; tmp = tmp->active_next)
        i++;
    for (map = first_map; map; map = map->next)
    {
        for(tmp = map->active_objects->active_next; tmp != NULL; tmp = tmp->active_next)
            i++;
    }
    return i;
}

void malloc_info(object *op)
{
    int             nrofmaps, fd;
    int             nrm = 0, mapmem = 0, anr, anims, sum_alloc = 0, sum_used = 0, i, tlnr, alnr;
    treasurelist   *tl;
    mapstruct      *m;
    archetype      *at;
    artifactlist   *al;

    for (tl = first_treasurelist,tlnr = 0; tl != NULL; tl = tl->next,tlnr++)
        ;
    for (al = first_artifactlist, alnr = 0; al != NULL; al = al->next, alnr++)
        ;
    for (at = first_archetype,anr = 0,anims = 0; at != NULL; at = at->more == NULL ? at->next : at->more,anr++)
        ;
    for (i = 1; i < num_animations; i++)
        anims += animations[i].num_animations;

    for (m = first_map,nrofmaps = 0; m != NULL; m = m->next,nrofmaps++)
        if (m->in_memory == MAP_ACTIVE)
        {
            mapmem += MAP_WIDTH(m) * MAP_HEIGHT(m) * (sizeof(object *) + sizeof(MapSpace));
            nrm++;
        }

    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "Sizeof: object=%ld  player=%ld  socketbuf=%ld  map=%ld",
            (long)sizeof(object), (long) (sizeof(player) + MAXSOCKBUF_IN * 2),
            (long)SOCKET_BUFSIZE_SEND+SOCKET_BUFSIZE_READ, (long)sizeof(mapstruct));
    dump_mempool_statistics(op, &sum_used, &sum_alloc);
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%4d active objects", count_active());
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%4d player(s) using buffers: %d", player_active,
            i = (player_active * (MAXSOCKBUF_IN * 2)));
    sum_alloc += i;
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%d socket(s) allocated: %d", socket_info.allocated_sockets,
            i = (socket_info.allocated_sockets * (sizeof(NewSocket) + SOCKET_BUFSIZE_SEND+SOCKET_BUFSIZE_READ)));
    sum_alloc += i;

#ifndef WIN32 /* non windows */
#ifdef HAVE_SYSCONF
    fd = sysconf(_SC_OPEN_MAX);
#else
#  ifdef HAVE_GETDTABLESIZE
    fd = getdtablesize();
#  else
    "Unable to find usable function to get max filedescriptors";
#  endif
#endif
#else
    fd = -3;
#endif

    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "socket max fd: %d  (%d %s) ncom: %d ", socket_info.max_filedescriptor, fd,
            fd != -3 ? "avaible" : "win32/ignore", socket_info.nconns);
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%4d maps allocated:  %d", nrofmaps, i = (nrofmaps * sizeof(mapstruct)));
    sum_alloc += i;  sum_used += nrm * sizeof(mapstruct);
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%4d maps in memory:  %8d", nrm, mapmem);
    sum_alloc += mapmem; sum_used += mapmem;
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%4d archetypes:      %8d", anr, i = (anr * sizeof(archetype)));
    sum_alloc += i; sum_used += i;
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%4d animations:      %8d", anims, i = (anims * sizeof(unsigned short)));
    sum_alloc += i; sum_used += i;
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%4d spells:          %8d", NROFREALSPELLS, i = (NROFREALSPELLS * sizeof(spell)));
    sum_alloc += i; sum_used += i;
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%4d treasurelists    %8d", tlnr, i = (tlnr * sizeof(treasurelist)));
    sum_alloc += i; sum_used += i;
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%4ld treasures        %8d", nroftreasures, i = (nroftreasures * sizeof(treasure)));
    sum_alloc += i; sum_used += i;
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%4ld artifacts        %8d", nrofartifacts, i = (nrofartifacts * sizeof(artifact)));
    sum_alloc += i; sum_used += i;
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%4ld artifacts strngs %8d", nrofallowedstr, i = (nrofallowedstr * sizeof(linked_char)));
    sum_alloc += i;sum_used += i;
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%s\n", "%4d artifactlists    %8d", alnr, i = (alnr * sizeof(artifactlist)));
    sum_alloc += i; sum_used += i;

    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "Total space allocated:%8d", sum_alloc);
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "Total space used:     %8d", sum_used);
}

/* Lists online players, respecting privacy mode. */
int command_who(object *op, char *params)
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

    new_draw_info(NDI_UNIQUE, 0, op, "There %s %d player%s online.\n\n%s",
                  (player_active == 1) ? "is" : "are", player_active,
                  (player_active == 1) ? "" : "s", cp);

#ifdef DAI_DEVELOPMENT_CODE
    show_stream_info(&CONTR(op)->socket);
#endif

    return 0;
}

int command_malloc(object *op, char *params)
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

/* Print and log map header info. */
int command_mapinfo(object *op, char *params)
{
    mapstruct *m;
    player    *pl;

    if (!op ||
        !(pl = CONTR(op)) ||
        !(m = op->map))
        return 0;

    if (params)
    {
        /* Only MWs/MMs/SAs can use the fancy commands. */
#ifdef DAI_DEVELOPMENT_CONTENT
        if (!(pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_MM | GMASTER_MODE_MW)))
#else
        if (!(pl->gmaster_mode & (GMASTER_MODE_SA | GMASTER_MODE_MM)))
#endif
            return 1;

        /* List all the loaded maps. */
        if (!strcmp(params, "all"))
        {
            int i;

            for (m = first_map, i = 1; m; m = m->next, i++)
                dump_map(m, pl, i, NULL);

            return 0;
        }

        /* Detail the current map and list all the immediately tiled ones. */
        if (!strcmp(params, "tiled"))
        {
            int   i;
            char *compass[] = { "N", "E", "S", "W", "NE", "SE", "SW", "NW" };

            dump_map(m, pl, 0, NULL);

            for (i = 0; i < 8; i++)
            {
                if (!m->tile_path[i])
                    continue;

                if (!m->tile_map[i])
                {
                    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "~%s Map~: NOT LOADED",
                            compass[i]);

                    continue;
                }

                dump_map(m->tile_map[i], pl, i + 1, compass[i]);
            }

            return 0;
        }

        return 1;
    }

    /* Detail the current map (amount of detail according to gmaster_mode,
     * handled by dump_map). */
    dump_map(m, pl, 0, NULL);

    return 0;
}

/* Print and log msp info. */
int command_mspinfo(object *op, char *params)
{
    mapstruct *m;
    player    *pl;

    if (!op ||
        !(pl = CONTR(op)) ||
        !(m = op->map))
        return 0;

    /* Detail the current msp. */
    dump_msp(m, op->x, op->y, pl);

    return 0;
}

int command_sstable(object *op, char *params)
{
    int     flags   = 0;
    char   *tmp;

    /* any paramter: dump whole table to the logfile */
    if (params && *params != 0)
        flags = SS_DUMP_TOTALS;

    LOG(llevSystem, "HASH TABLE DUMP\n");
    ss_dump_statistics(errmsg);
    if (errmsg[0] != '\0')
    {
        NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%s", errmsg);
    }
    tmp = ss_dump_table(flags);
    NDI_LOG(llevSystem, NDI_UNIQUE, 0, op, "%s", tmp);

    return 0;
}

/* Writes the current tad to op.
 * Eventually this command will also update the client's clock. */
int command_time(object *op, char *params)
{
    timeanddate_t  tad;
    char         *pp;
    int           flags;

    if (!op || op->type != PLAYER || !CONTR(op))
        return 0;

    get_tad(&tad);

    /* This is only for testing. */
    for (pp = params, flags = 0; pp && *pp; pp++)
    {
        if (!strncmp(pp, "showtime", 8))
            flags |= TAD_SHOWTIME;
        else if (!strncmp(pp, "showdate", 8))
            flags |= TAD_SHOWDATE;
        else if (!strncmp(pp, "showseason", 10))
            flags |= TAD_SHOWSEASON;
        else if (!strncmp(pp, "longform", 8))
            flags |= TAD_LONGFORM;

        if (!(pp = strchr(pp, ' ')))
            break;
    }

    if (!flags)
        flags = TAD_SHOWTIME | TAD_SHOWDATE | TAD_SHOWSEASON | TAD_LONGFORM;

    /* Send the tad string to the player. */
    new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, "It is %s.",
                         print_tad(&tad, flags));

    return 0;
}

int command_archs(object *op, char *params)
{
    arch_info(op);

    return 0;
}

int command_debug(object *op, char *params)
{
    int i;

    if (params == NULL || !sscanf(params, "%d", &i))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Global debug level is %d.", settings.debug);

        return 0;
    }

    settings.debug = (enum LogLevel) FABS(i);
    new_draw_info(NDI_UNIQUE, 0, op, "Set debug level to %d.", i);

    return 0;
}


/*
 * Those dumps should be just one dump with good parser
 */

int command_dumpbelowfull(object *op, char *params)
{
    object *tmp;

    new_draw_info(NDI_UNIQUE, 0, op, "DUMP OBJECTS OF THIS TILE");
    new_draw_info(NDI_UNIQUE, 0, op, "-------------------");

    for (tmp = GET_MAP_OB(op->map, op->x, op->y); tmp; tmp = tmp->above)
    {
        if (tmp == op) /* exclude the DM player object */
            continue;

        dump_object(tmp);
        new_draw_info(NDI_UNIQUE, 0, op, "%s", errmsg);

        if (tmp->above && tmp->above != op)
            new_draw_info(NDI_UNIQUE, 0, op, ">next object<");
    }

    new_draw_info(NDI_UNIQUE, 0, op, "------------------");

    return 0;
}

int command_dumpbelow(object *op, char *params)
{
    object *tmp;
    int     i   = 0;

    new_draw_info(NDI_UNIQUE, 0, op, "DUMP OBJECTS OF THIS TILE");
    new_draw_info(NDI_UNIQUE, 0, op, "-------------------");

    for (tmp = GET_MAP_OB(op->map, op->x, op->y); tmp; tmp = tmp->above, i++)
    {
        if (tmp == op) /* exclude the DM player object */
            continue;

        new_draw_info(NDI_UNIQUE, 0, op, "#%d  >%s<  >%s<  >%s<",
            i, STRING_OBJ_NAME(tmp), STRING_OBJ_ARCH_NAME(tmp),
            STRING_OBJ_NAME(tmp->env));
    }

    new_draw_info(NDI_UNIQUE, 0, op, "------------------");

    return 0;
}

int command_dumpallobjects(object *op, char *params)
{
#ifdef MEMPOOL_TRACKING
    struct puddle_info *puddle = pool_object->first_puddle_info;
    unsigned int i;
    object *obj;

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

int command_dumpallarchetypes(object *op, char *params)
{
    dump_all_archetypes();

    return 0;
}
int command_dumpactivelist(object *op, char *params)
{
    char    buf[1024];
    int     count   = 0;
    object *tmp;

    for (tmp = active_objects->active_next; tmp; tmp = tmp->active_next)
    {
        count++;
        sprintf(buf, "%s[%d] (%s) %d %f",
            STRING_OBJ_NAME(tmp), TAG(tmp), STRING_OBJ_ARCH_NAME(tmp),
            tmp->type, tmp->speed);
        /*new_draw_info(NDI_UNIQUE, 0,op, "%s", buf); It will overflow the send buffer with many player online */
        LOG(llevSystem, "%s\n", buf);
    }

    sprintf(buf, "active objects: %d (dumped to log)", count);
    new_draw_info(NDI_UNIQUE, 0, op, "%s", buf);
    LOG(llevSystem, "%s\n", buf);

    return 0;
}

int command_setmaplight(object *op, char *params)
{
    int     i;
    char    buf[256];

    if (params == NULL || !sscanf(params, "%d", &i))
        return 1;

    if (i < -1)
        i = -1;
    if (i > MAX_DARKNESS)
        i = MAX_DARKNESS;
    op->map->darkness = i;

    if (i == -1)
        i = MAX_DARKNESS;

    op->map->light_value = global_darkness_table[i];

    sprintf(buf, "WIZ: set map darkness: %d -> map:%s (%d)", i, op->map->path, MAP_OUTDOORS(op->map));
    new_draw_info(NDI_UNIQUE, 0, op, "%s", buf);

    return 0;
}

#if 0
int command_dumpmap(object *op, char *params)
{
    if (op)
        dump_map(op->map, CONTR(op));

    return 0;
}

int command_dumpallmaps(object *op, char *params)
{
    dump_all_maps();

    return 0;
}
#endif

int command_printlos(object *op, char *params)
{
    if (op)
        print_los(op);

    return 0;
}

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

int command_listen(object *op, char *params)
{
    int i;

    if (params == NULL || !sscanf(params, "%d", &i))
        return 1;

    CONTR(op)->listening = (char) i;
    new_draw_info(NDI_UNIQUE, 0, op, "Your verbosity level is now %d.",
                         i);

    return 0;
}

/* Prints out some useful information for the character.  Everything we print
 * out can be determined by the docs, so we aren't revealing anything extra -
 * rather, we are making it convenient to find the values.  params have
 * no meaning here.
 */
int command_statistics(object *pl, char *params)
{
    if (pl->type != PLAYER || !CONTR(pl))
        return 0;

    new_draw_info(NDI_UNIQUE, 0, pl, "  Experience: %d", pl->stats.exp);
    new_draw_info(NDI_UNIQUE, 0, pl, "  Next Level: %d", GET_LEVEL_EXP(pl->level + 1));
    new_draw_info(NDI_UNIQUE, 0, pl, "\nStat       Nat/Real/Max");

    new_draw_info(NDI_UNIQUE, 0, pl, "Str         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Str, pl->stats.Str,
                         20 + pl->arch->clone.stats.Str);
    new_draw_info(NDI_UNIQUE, 0, pl, "Dex         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Dex, pl->stats.Dex,
                         20 + pl->arch->clone.stats.Dex);
    new_draw_info(NDI_UNIQUE, 0, pl, "Con         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Con, pl->stats.Con,
                         20 + pl->arch->clone.stats.Con);
    new_draw_info(NDI_UNIQUE, 0, pl, "Int         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Int, pl->stats.Int,
                         20 + pl->arch->clone.stats.Int);
    new_draw_info(NDI_UNIQUE, 0, pl, "Wis         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Wis, pl->stats.Wis,
                         20 + pl->arch->clone.stats.Wis);
    new_draw_info(NDI_UNIQUE, 0, pl, "Pow         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Pow, pl->stats.Pow,
                         20 + pl->arch->clone.stats.Pow);
    new_draw_info(NDI_UNIQUE, 0, pl, "Cha         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Cha, pl->stats.Cha,
                         20 + pl->arch->clone.stats.Cha);

    /* Can't think of anything else to print right now */
    return 0;
}

int command_fix_me(object *op, char *params)
{
    FIX_PLAYER(op ,"command fix_me");

    return 0;
}


int command_logs(object *op, char *params)
{
    int first;

    first = 1;

    if (first)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Nobody is currently logging kills.");
    }

    return 0;
}

int command_resistances(object *op, char *params)
{
    int i;

    if (!op)
        return 0;

    for (i = 0; i < NROFATTACKS; i++)
    {
        if (i == ATNR_INTERNAL)
            continue;

        new_draw_info(NDI_UNIQUE, 0, op, "%-20s %+5d",
            attack_name[i].name, op->resist[i]);
    }

    return 0;
}
/*
 * Actual commands.
 * Those should be in small separate files (c_object.c, c_wiz.c, cmove.c,...)
 */

#if 0
static void help_topics(object *op, int what)
{
    DIR            *dirp;
    struct dirent  *de;
    char            filename[MEDIUM_BUF], line[80];
    int             namelen, linelen = 0;

    switch (what)
    {
        case 1:
          sprintf(filename, "%s/wizhelp", HELPDIR);
          new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, "\nWiz commands:");
          break;
        case 3:
          sprintf(filename, "%s/help", HELPDIR);
          new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, "Topics:");
          break;
        default:
          sprintf(filename, "%s/commands", HELPDIR);
          new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, "Commands:");
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
            new_draw_info(NDI_UNIQUE, 0, op, "%s", line);
            sprintf(line, " %s", de->d_name);
            linelen = namelen + 1;
            continue;
        }
        strcat(line, " ");
        strcat(line, de->d_name);
    }
    new_draw_info(NDI_UNIQUE, 0, op, "%s", line);
    closedir(dirp);
}
#endif

int command_resting(object *op, char *params)
{
    player *pl = CONTR(op);

    /* sitting is the way we enter resting */
    if (pl->rest_sitting) /* we allready sit? stand up! */
    {
        new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, "You stop resting.");
        pl->rest_mode = pl->rest_sitting = 0;
        pl->resting_reg_timer = 0;
    }
    else
    {
        new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, "You start resting.");
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

static void show_help(char *fname, player *pl)
{
    FILE *fp;
    char  buf[MEDIUM_BUF];

    if ((fp = fopen(fname, "r")) == NULL)
    {
        new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "No more available!");

        return;
    }

    while (fgets(buf, (int)sizeof(buf), fp))
    {
        int len;

        len = (int)strlen(buf) - 1;

        if (buf[len] == '\n')
            buf[len] = (len) ? '\0' : ' ';

        new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "%s", buf);
    }

    fclose(fp);
}

static void show_commands(player *pl)
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

        new_draw_info(NDI_UNIQUE | NDI_YELLOW, 0, pl->ob, "\n%s", name[i]);

        for (j = 0; j < size[i]; j++)
        {
            /* TODO: This calculation can be removed, and the following
             * new_draw_info() moved to inside this for loop by having the
             * client handle NDI_UNIQUE properly (well, at all).
             * -- Smacky 20090604 */
            if (strlen(buf) + strlen(ap[i][j].name) > 42)
            {
                new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "%s", buf);
                buf[0] = '\0';
            }

            sprintf(strchr(buf, '\0'), " /%s ~+~", ap[i][j].name);
        }

        new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "%s", buf);
    }
}

int command_help(object *op, char *params)
{
    player *pl;
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
//            new_draw_info(NDI_UNIQUE, 0, op, "Illegal characters in '%s'", params);
//
//            return 0;
//        }
        new_draw_info(NDI_UNIQUE | NDI_YELLOW, 0, pl->ob, "Help for command %s:",
                             params);

        if (!(csp = find_command(params + 1, NULL)))
        {
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, pl->ob, "Unrecognised command!");

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
    new_draw_info(NDI_UNIQUE, 0, pl->ob, "No help available on '%s'",
                         params);

    return 0;
}

int command_privacy(object *op, char *params)
{
    int new_status = !CONTR(op)->privacy;

    CONTR(op)->privacy = new_status;

    if (new_status)
        new_draw_info(NDI_UNIQUE, 0, op, "Privacy enabled.");
    else
        new_draw_info(NDI_UNIQUE, 0, op, "Privacy disabled.");

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
int command_stuck(object *op, char *params)
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

int command_level(object *op, char *params)
{
    player *pl;

    if (!params)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You are level ~%d~.", op->level);
    }
    else if (!(pl = find_player(params)))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "No such player.");

        return COMMANDS_RTN_VAL_ERROR;
    }
    else
    {
        new_draw_info(NDI_UNIQUE, 0, op, "|%s| is level ~%d~.",
            pl->quick_name, pl->ob->level);
    }

    return COMMANDS_RTN_VAL_OK;
}
