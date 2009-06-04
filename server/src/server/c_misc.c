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


/* Handles misc. input request - things like hash table, malloc, maps,
 * who, etc.
 */

void map_info(object *op)
{
    mapstruct  *m;
    char        buf[MAX_BUF], map_path[MAX_BUF];
    long        sec = seconds();
#ifdef MAP_RESET
    LOG(llevSystem, "Current time is: %02ld:%02ld:%02ld.\n", (sec % 86400) / 3600, (sec % 3600) / 60, sec % 60);

    new_draw_info_format(NDI_UNIQUE, 0, op, "Current time is: %02ld:%02ld:%02ld.", (sec % 86400) / 3600,
                         (sec % 3600) / 60, sec % 60);
    new_draw_info(NDI_UNIQUE, 0, op, "Path               Pl PlM IM   TO Dif Reset");
#else
    new_draw_info(NDI_UNIQUE, 0, op, "Pl Pl-M IM   TO Dif");
#endif
    for (m = first_map; m != NULL; m = m->next)
    {
#ifndef MAP_RESET
        if (m->in_memory == MAP_SWAPPED)
            continue;
#endif
        /* Print out the last 18 characters of the map name... */
        if (strlen(m->path) <= 18)
            strcpy(map_path, m->path);
        else
            strcpy(map_path, m->path + strlen(m->path) - 18);
#ifndef MAP_RESET
        sprintf(buf, "%-18.18s %c %2d   %c %4ld %2ld", map_path,
                m->in_memory ? (m->in_memory == MAP_IN_MEMORY ? 'm' : 's') : 'X', players_on_map(m), m->in_memory,
                m->timeout, m->difficulty);
#else
        LOG(llevSystem, "%s (%s) pom:%d status:%c timeout:%d diff:%d  reset:%02d:%02d:%02d\n",
            m->path, m->orig_path, players_on_map(m),
            m->in_memory ? (m->in_memory == MAP_IN_MEMORY ? 'm' : 's') : 'X', m->timeout, m->difficulty,
            (MAP_WHEN_RESET(m) % 86400) / 3600, (MAP_WHEN_RESET(m) % 3600) / 60, MAP_WHEN_RESET(m) % 60);
/*        sprintf(buf, "%-18.18s %2d   %c %4d %2d  %02d:%02d:%02d", map_path, players_on_map(m),*/
            if(!strcmp(m->path, m->orig_path))
                sprintf(buf, "%s %2d   %c %4d %2d  %02d:%02d:%02d", m->path, players_on_map(m),
                    m->in_memory ? (m->in_memory == MAP_IN_MEMORY ? 'm' : 's') : 'X', m->timeout, m->difficulty,
                    (MAP_WHEN_RESET(m) % 86400) / 3600, (MAP_WHEN_RESET(m) % 3600) / 60, MAP_WHEN_RESET(m) % 60);
            else
                sprintf(buf, "%s (%s) %2d   %c %4d %2d  %02d:%02d:%02d", m->path, m->orig_path, players_on_map(m),
                    m->in_memory ? (m->in_memory == MAP_IN_MEMORY ? 'm' : 's') : 'X', m->timeout, m->difficulty,
                    (MAP_WHEN_RESET(m) % 86400) / 3600, (MAP_WHEN_RESET(m) % 3600) / 60, MAP_WHEN_RESET(m) % 60);
#endif
        new_draw_info(NDI_UNIQUE, 0, op, buf);
    }
}

/* now redundant function */
int command_spell_reset(object *op, char *params)
{
    /*init_spell_param(); */
    return 1;
}

/* '/motd' displays the MOTD. GMs and MMs can also set the MOTD:
 *   '/motd default' restores the server-set MOTD (actually deletes the
 *   GMASTER-set one).
 *   '/motd <message>' sets the GMASTER-set MOTD. */
int command_motd(object *op, char *params)
{
#ifdef MOTD
    if (params &&
        (CONTR(op)->gmaster_mode == GMASTER_MODE_GM ||
         CONTR(op)->gmaster_mode == GMASTER_MODE_MM))
    {
        char  buf[MAX_BUF];
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

    return 1;
}

int command_bug(object *op, char *params)
{
    char    buf[MAX_BUF];

    if (params == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "what bugs?");
        return 1;
    }
    strcpy(buf, op->name);
    strcat(buf, " bug-reports: ");
    strncat(buf, ++params, MAX_BUF - strlen(buf));
    buf[MAX_BUF - 1] = '\0';
    bug_report(buf);
    new_draw_info(NDI_ALL | NDI_UNIQUE, 1, NULL, buf);
    new_draw_info(NDI_UNIQUE, 0, op, "OK, thanks!");
    return 1;
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
        if (m->in_memory == MAP_IN_MEMORY)
        {
            mapmem += MAP_WIDTH(m) * MAP_HEIGHT(m) * (sizeof(object *) + sizeof(MapSpace));
            nrm++;
        }
    sprintf(errmsg, "Sizeof: object=%ld  player=%ld  socketbuf=%ld  map=%ld", (long)sizeof(object),
            (long) (sizeof(player) + MAXSOCKBUF_IN * 2), (long)SOCKET_BUFSIZE_SEND+SOCKET_BUFSIZE_READ, (long)sizeof(mapstruct));
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);

    dump_mempool_statistics(op, &sum_used, &sum_alloc);

    sprintf(errmsg, "%4d active objects", count_active());
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);

    sprintf(errmsg, "%4d player(s) using buffers: %d", player_active,
            i = (player_active * (MAXSOCKBUF_IN * 2)));
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    sum_alloc += i;
    LOG(llevSystem, "%s\n", errmsg);

    sprintf(errmsg, "%d socket(s) allocated: %d", socket_info.allocated_sockets,
            i = (socket_info.allocated_sockets * (sizeof(NewSocket) + SOCKET_BUFSIZE_SEND+SOCKET_BUFSIZE_READ)));
    sum_alloc += i;
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);

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

    sprintf(errmsg, "socket max fd: %d  (%d %s) ncom: %d ", socket_info.max_filedescriptor, fd,
            fd != -3 ? "avaible" : "win32/ignore", socket_info.nconns);
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);

    sprintf(errmsg, "%4d maps allocated:  %d", nrofmaps, i = (nrofmaps * sizeof(mapstruct)));
    LOG(llevSystem, "%s\n", errmsg);
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    sum_alloc += i;  sum_used += nrm * sizeof(mapstruct);
    sprintf(errmsg, "%4d maps in memory:  %8d", nrm, mapmem);
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);
    sum_alloc += mapmem; sum_used += mapmem;
    sprintf(errmsg, "%4d archetypes:      %8d", anr, i = (anr * sizeof(archetype)));
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);
    sum_alloc += i; sum_used += i;
    sprintf(errmsg, "%4d animations:      %8d", anims, i = (anims * sizeof(unsigned short)));
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);
    sum_alloc += i; sum_used += i;
    sprintf(errmsg, "%4d spells:          %8d", NROFREALSPELLS, i = (NROFREALSPELLS * sizeof(spell)));
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);
    sum_alloc += i; sum_used += i;
    sprintf(errmsg, "%4d treasurelists    %8d", tlnr, i = (tlnr * sizeof(treasurelist)));
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);
    sum_alloc += i; sum_used += i;
    sprintf(errmsg, "%4ld treasures        %8d", nroftreasures, i = (nroftreasures * sizeof(treasure)));
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);
    sum_alloc += i; sum_used += i;
    sprintf(errmsg, "%4ld artifacts        %8d", nrofartifacts, i = (nrofartifacts * sizeof(artifact)));
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);
    sum_alloc += i; sum_used += i;
    sprintf(errmsg, "%4ld artifacts strngs %8d", nrofallowedstr, i = (nrofallowedstr * sizeof(linked_char)));
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);
    sum_alloc += i;sum_used += i;
    sprintf(errmsg, "%4d artifactlists    %8d", alnr, i = (alnr * sizeof(artifactlist)));
    LOG(llevSystem, "%s\n", errmsg);
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    sum_alloc += i; sum_used += i;

    sprintf(errmsg, "Total space allocated:%8d", sum_alloc);
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);
    sprintf(errmsg, "Total space used:     %8d", sum_used);
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
    LOG(llevSystem, "%s\n", errmsg);
}

void current_map_info(object *op)
{
    mapstruct  *m   = op->map;
#if 0
    /* When we remove the media tag completely, there will be no need to split
     * m->name. */

    if (!m)
        return;

    new_draw_info_format(NDI_UNIQUE, 0, op, "%s (%s)\n",
                         m->name, m->path);
#else
    char        buf[128], *tmp;

    if (!m)
        return;

    strcpy(buf, m->name);
    tmp = strchr(buf, '§');
    if (tmp)
        *tmp = 0;
    new_draw_info_format(NDI_UNIQUE, 0, op, "%s (%s)", buf, m->path);
#endif

    if (QUERY_FLAG(op, FLAG_WIZ))
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "players:%d difficulty:%d size:%dx%d start:%dx%d timeout %ld",
                             players_on_map(m), m->difficulty, MAP_WIDTH(m), MAP_HEIGHT(m), MAP_ENTER_X(m),
                             MAP_ENTER_Y(m), MAP_TIMEOUT(m));
    }
    if (m->msg)
        new_draw_info(NDI_UNIQUE, NDI_NAVY, op, m->msg);
}

int command_who(object *op, char *params)
{
    player     *pl;
    int         ip = 0, il = 0;
    char        buf[MAX_BUF];
    const char *sex;

    if (!op)
        return 1;

    for (pl = first_player; pl != NULL; pl = pl->next)
    {
        if (pl->dm_stealth &&
            (CONTR(op)->gmaster_mode != GMASTER_MODE_GM &&
             CONTR(op)->gmaster_mode != GMASTER_MODE_MM))
            continue;

        if (pl->ob->map == NULL)
        {
            il++;
            continue;
        }

        ip++;
        if (pl->state & ST_PLAYING)
        {
            if (QUERY_FLAG(pl->ob, FLAG_IS_MALE))
                sex = QUERY_FLAG(pl->ob, FLAG_IS_FEMALE) ? "hermaphrodite" : "male";
            else if (QUERY_FLAG(pl->ob, FLAG_IS_FEMALE))
                sex = "female";
            else
                sex = "neuter";

            if (CONTR(op)->gmaster_mode == GMASTER_MODE_GM ||
                CONTR(op)->gmaster_mode == GMASTER_MODE_MM)
            {
                int off = 0, tmp, tmp1;
                if ((tmp = strlen(pl->ob->map->path)) > (22 - ((tmp1 = strlen(pl->ob->name)))))
                    off = tmp - (22 - tmp1);

                sprintf(buf, "%s (%d) [@%s] [%s]", pl->quick_name, pl->ob->count, pl->socket.ip_host, pl->ob->map->path + off);
            }
            else if (CONTR(op)->gmaster_mode == GMASTER_MODE_VOL)
                sprintf(buf, "%s the %s %s (lvl %d) [%s]", pl->quick_name, sex, pl->ob->race, pl->ob->level, pl->socket.ip_host);
            else
                sprintf(buf, "%s the %s %s (lvl %d)", pl->quick_name, sex, pl->ob->race, pl->ob->level);
            new_draw_info(NDI_UNIQUE, 0, op, buf);
        }
    }
    sprintf(buf, "There %s %d player%s online  (%d in login)", ip + il > 1 ? "are" : "is",ip + il,
            ip + il > 1 ? "s" : "",il);
    new_draw_info(NDI_UNIQUE, 0, op, buf);
#ifdef _TESTSERVER
    FILE *fp;
    LOG(llevSystem, "read stream file...\n");
    sprintf(buf, "%s/%s", settings.localdir, "stream");
    if ((fp = fopen(buf, "r")))
    {
        char *cp;
        if (!fgets(buf, MAX_BUF, fp))
        {
            LOG(llevBug, "BUG: error in stream file\n");
            return;
        }
        if ((cp = strchr(buf, '\n')))
            *cp = '\0';
        if (!strcmp(buf, "(null)"))
            new_draw_info(NDI_UNIQUE, 0, op, "Server compiled with trunk only.");
        else
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "Server compiled with ~%s~ stream.", buf);
            while (fgets(buf, MAX_BUF, fp))
            {
                if ((cp = strchr(buf, '\n')))
                    *cp = '\0';
                new_draw_info(NDI_UNIQUE, 0, op, buf);
            }
        }
        fclose(fp);
    }
#endif
    return 1;
}

int command_malloc(object *op, char *params)
{
#ifdef MEMPOOL_TRACKING
    if (params)
    {
        int force_flag = 0, i;

        if (strcmp(params, "free") && strcmp(params, "force"))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "Usage: /malloc [free | force]");
            return 1;
        }

        if (strcmp(params, "force") == 0)
            force_flag = 1;

        for (i = 0; i < nrof_mempools; i++)
            if (force_flag == 1 || mempools[i]->flags & MEMPOOL_ALLOW_FREEING)
                free_empty_puddles(mempools[i]);
    }
#endif

    malloc_info(op);
    return 1;
}

int command_mapinfo(object *op, char *params)
{
    current_map_info(op);
    return 1;
}

int command_maps(object *op, char *params)
{
    map_info(op);
    return 1;
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
        new_draw_info(NDI_UNIQUE, 0, op, errmsg);
        LOG(llevSystem, "%s\n", errmsg);
    }
    tmp = ss_dump_table(flags);
    LOG(llevSystem, "%s\n", tmp);
    new_draw_info(NDI_UNIQUE, 0, op, tmp);
    return 1;
}


int command_time(object *op, char *params)
{
    print_tod(op);
    return 1;
}

int command_archs(object *op, char *params)
{
    arch_info(op);
    return 1;
}

int command_debug(object *op, char *params)
{
    int     i;
    char    buf[MAX_BUF];
    if (params == NULL || !sscanf(params, "%d", &i))
    {
        sprintf(buf, "Global debug level is %d.", settings.debug);
        new_draw_info(NDI_UNIQUE, 0, op, buf);
        return 1;
    }
    if (op != NULL && !QUERY_FLAG(op, FLAG_WIZ))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Privileged command.");
        return 1;
    }
    settings.debug = (enum LogLevel) FABS(i);
    sprintf(buf, "Set debug level to %d.", i);
    new_draw_info(NDI_UNIQUE, 0, op, buf);
    return 1;
}


/*
 * Those dumps should be just one dump with good parser
 */

int command_dumpbelowfull(object *op, char *params)
{
    object *tmp;

    new_draw_info(NDI_UNIQUE, 0, op, "DUMP OBJECTS OF THIS TILE");
    new_draw_info(NDI_UNIQUE, 0, op, "-------------------");
    for (tmp = get_map_ob(op->map, op->x, op->y); tmp; tmp = tmp->above)
    {
        if (tmp == op) /* exclude the DM player object */
            continue;
        dump_object(tmp);
        new_draw_info(NDI_UNIQUE, 0, op, errmsg);
        if (tmp->above && tmp->above != op)
            new_draw_info(NDI_UNIQUE, 0, op, ">next object<");
    }
    new_draw_info(NDI_UNIQUE, 0, op, "------------------");
    return 0;
}

int command_dumpbelow(object *op, char *params)
{
    object *tmp;
    char    buf[5 * 1024];
    int     i   = 0;

    new_draw_info(NDI_UNIQUE, 0, op, "DUMP OBJECTS OF THIS TILE");
    new_draw_info(NDI_UNIQUE, 0, op, "-------------------");
    for (tmp = get_map_ob(op->map, op->x, op->y); tmp; tmp = tmp->above, i++)
    {
        if (tmp == op) /* exclude the DM player object */
            continue;
        sprintf(buf, "#%d  >%s<  >%s<  >%s<", i, query_name(tmp),
                tmp->arch ? (tmp->arch->name ? tmp->arch->name : "no arch name") : "NO ARCH",
                tmp->env ? query_name(tmp->env) : "");
        new_draw_info(NDI_UNIQUE, 0, op, buf);
    }
    new_draw_info(NDI_UNIQUE, 0, op, "------------------");
    return 0;
}

int command_wizpass(object *op, char *params)
{
    int i;

    if (!op)
        return 0;

    if (!params)
        i = (QUERY_FLAG(op, FLAG_WIZPASS)) ? 0 : 1;
    else
        i = onoff_value(params);

    if (i)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You will now walk through walls.\n");
        SET_FLAG(op, FLAG_WIZPASS);
    }
    else
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You will now be stopped by walls.\n");
        CLEAR_FLAG(op, FLAG_WIZPASS);
    }
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
                LOG(llevDebug, "obj '%s'-(%s) %x (%d)(%s) #=%d\n", STRING_OBJ_NAME(obj),
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


int command_dm_dev(object *op, char *params)
{
    if (op->type != PLAYER)
        return -1;

    command_goto(op, "/dev/testmaps/testmap_main 2 2");

    return 0;
}

/* '/dm_invis' toggles intrinsic invisibility and see invisible for the DM, but
 * only while in DM mode. It must be turned on every time the player enters DM
 * mode/logs in. */
int command_dm_invis(object *op, char *params)
{
    if (op->type != PLAYER)
        return -1;

    CONTR(op)->dm_invis = (CONTR(op)->dm_invis) ? 0 : 1;
    new_draw_info_format(NDI_UNIQUE, 0, op, "toggled dm_invis to %d",
                         CONTR(op)->dm_invis);
    FIX_PLAYER(op, "command dm_invis");

    return 0;
}

/* '/dm_stealth' toggles whether the DM's presence is announced to players (ie,
 * on login, in /who, and as feedback in /tell) AND whether mobs can sense the
 * DM's presence. The former works also when a dm logs in WITHOUT /dm set or
 * when the player leave DM mode, but the latter is only effective while DM
 * mode is active. */
int command_dm_stealth(object *op, char *params)
{
    if (op->type != PLAYER)
        return -1;

   if (CONTR(op)->dm_stealth)
   {
       new_draw_info_format(NDI_UNIQUE | NDI_ALL, 5, NULL, "%s has entered the game.",
                            query_name(op));
       CONTR(op)->dm_stealth = 0;
   }
   else
       CONTR(op)->dm_stealth = 1;

   new_draw_info_format(NDI_UNIQUE, 0, op, "toggled dm_stealth to %d",
                        CONTR(op)->dm_stealth);

#ifdef USE_CHANNELS
    channel_dm_stealth(CONTR(op), CONTR(op)->dm_stealth);
#endif

    return 0;
}

/* '/dm_light x' switches the map master's personal light to
 * 1 <= x <= MAX_DARKNESS, or turns it off (x=0).
 * '/dm_light' toggles personal_light between off (0) and fullbeams
 * (MAX_DARKNESS). */
int command_dm_light(object *op, char *params)
{
    player *pl;
    int     personal_light;

    if (op->type != PLAYER)
        return -1;

    pl = CONTR(op);
    personal_light = (pl->personal_light) ? 0 : MAX_DARKNESS;

    if (params)
        sscanf(params, "%d", &personal_light);

    set_personal_light(pl, personal_light);
    new_draw_info_format(NDI_UNIQUE, 0, op, "Switch personal light %s (%d).",
                         (pl->personal_light) ? "to" : "off", pl->personal_light);

    return 0;
}

/* TODO: logic changed with account patch! */
#if 0
int command_dm_password (object *op, char *params)
{
    player *pl;
    FILE *fp, *fpout;
    const char *name_hash;
    char pfile[MAX_BUF], bufall[MAX_BUF], outfile[MAX_BUF];
    char name[MAX_BUF]="", pwd[MAX_BUF]="";

    if(params==NULL || !sscanf(params, "%s %s", name, pwd) || name[0] == 0 || pwd[0]== 0)
    {
        new_draw_info(NDI_UNIQUE, 0,op, "dm_pwd: missing/invalid parameter\nUsage: /dm_pwd <playername> <new password>");
        return 0;
    }
    transform_name_string(name);

    /* we have now 2 strings - name and password - lets check there is a player file for that name */
    sprintf(pfile, "%s/%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, get_subdir(name), name, name);
    if (access(pfile, F_OK)==-1)
    {
        new_draw_info_format(NDI_UNIQUE, 0,op, "dm_pwd: player %s don't exists or has no player file!", name);
        return 0;
    }

    /* All ok - player exists and player file can be altered - load in the player file */
    strcpy(outfile, pfile);
    strcat(outfile, ".tmp");

    /* lets do a safe read/write in a temp. file */
    if((fp=fopen(pfile,"r"))==NULL)
    {
        new_draw_info_format(NDI_UNIQUE, 0,op, "dm_pwd: error open file %s!", pfile);
        return 0;
    }

    if((fpout=fopen(outfile,"w"))==NULL)
    {
        new_draw_info_format(NDI_UNIQUE, 0,op, "dm_pwd: error open file %s!", outfile);
        return 0;
    }

    while (fgets(bufall,MAX_BUF-1,fp) != NULL)
    {
        if(!strncmp(bufall,"password ",9))
            fprintf(fpout,"password %s\n", crypt_string(pwd));
        else
            fputs(bufall, fpout);
    }

    /* now, this is important - perhaps the player is online!
     * be sure we change the password in the player struct too!
     */
/* TODO: we must check accounts here */
    if((name_hash = find_string(name)))
    {
        for(pl=first_player;pl!=NULL;pl=pl->next)
        {
            /* we don't care about removed or such - just force to be sure the change
             * in the player* struct.
             */
            if(pl->ob && pl->ob->name == name_hash)
            {
        FIXME->  strcpy(pl->socket.account.pwd, crypt_string(pwd));
                break;
            }
        }
    }

    fclose(fp);
    fclose(fpout);

    /* delete the original file and move the tmp file */
    unlink(pfile);
    rename(outfile, pfile);

    new_draw_info_format(NDI_UNIQUE, 0,op, "Done. Changed password of %s to %s!", name, pwd);

    return 0;
}
#endif

int command_dumpactivelist(object *op, char *params)
{
    char    buf[1024];
    int     count   = 0;
    object *tmp;

    for (tmp = active_objects->active_next; tmp; tmp = tmp->active_next)
    {
        count++;
        sprintf(buf, "%08d %03d %f %s (%s)", tmp->count, tmp->type, tmp->speed, query_short_name(tmp, NULL),
                tmp->arch->name ? tmp->arch->name : "<NA>");
        /*new_draw_info(NDI_UNIQUE, 0,op, buf); It will overflow the send buffer with many player online */
        LOG(llevSystem, "%s\n", buf);
    }
    sprintf(buf, "active objects: %d (dumped to log)", count);
    new_draw_info(NDI_UNIQUE, 0, op, buf);
    LOG(llevSystem, "%s\n", buf);

    return 0;
}

/* Reboots the server (recompile code, update arches and maps).
 * On a test server the command is available for MWs and MMs on a 30 second
 * counter. A stream name may be given -- ie, /restart stream -- to compile the
 * trunk server with that stream added.
 * On a normal server it's a MM-only command on a 5 min counter with no param.
 */
int command_restart(object *ob, char *params)
{
#ifdef _TESTSERVER
    char  buf[MAX_BUF];
    FILE *fp;

    if (ob->type != PLAYER)
        return -1;

    LOG(llevSystem,"write stream file...\n");
    sprintf(buf, "%s/%s", settings.localdir, "stream");

    if ((fp = fopen(buf, "w")) == NULL)
    {
        LOG(llevBug, "BUG: Cannot open %s for writing\n", buf);

        return -1;
    }

    if (params)
        fprintf(fp, "%s", params);
    else
        fprintf(fp, "(null)");

    fclose(fp);

    sprintf(buf, "'/restart%s%s' issued by %s\nServer will recompile and arches and maps will be updated!",
            (params) ? " " : "", (params) ? params : "", STRING_OBJ_NAME(ob));
    LOG(llevSystem, buf);
    shutdown_agent(30, EXIT_RESETMAP, buf);

    return 0;
#else
    char buf[MAX_BUF];

    if (ob->type != PLAYER)
        return -1;

    sprintf(buf, "'/restart' issued by %s\nServer will recompile and arches and maps will be updated!",
            STRING_OBJ_NAME(ob));
    LOG(llevSystem, buf);
    shutdown_agent(300, EXIT_RESETMAP, buf);

    return 0;
#endif
}

int command_start_shutdown(object *op, char *params)
{
    char   *bp  = NULL;
    int     i   = -2;

    if (params == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "DM usage: /start_shutdown <-1 ... x>");
        return 0;
    }

    sscanf(params, "%d ", &i);
    if ((bp = strchr(params, ' ')) != NULL)
        bp++;

    if (bp && bp == 0)
        bp = NULL;

    if (i < -1)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "DM usage: /start_shutdown <-1 ... x>");
        return 0;
    }

    LOG(llevSystem, "Shutdown Agent started!\n");
    shutdown_agent(i, EXIT_SHUTODWN, bp);
    new_draw_info_format(NDI_UNIQUE | NDI_GREEN, 0, op, "shutdown agent started! (timer set to %d seconds).", i);

    return 0;
}

int command_setmaplight(object *op, char *params)
{
    int     i;
    char    buf[256];

    if (params == NULL || !sscanf(params, "%d", &i))
        return 0;

    if (i < -1)
        i = -1;
    if (i > MAX_DARKNESS)
        i = MAX_DARKNESS;
    op->map->darkness = i;

    if (i == -1)
        i = MAX_DARKNESS;

    op->map->light_value = global_darkness_table[i];

    sprintf(buf, "WIZ: set map darkness: %d -> map:%s (%d)", i, op->map->path, MAP_OUTDOORS(op->map));
    new_draw_info(NDI_UNIQUE, 0, op, buf);

    return 0;
}

int command_dumpmap(object *op, char *params)
{
    if (op)
        dump_map(op->map);
    return 0;
}

int command_dumpallmaps(object *op, char *params)
{
    dump_all_maps();
    return 0;
}

int command_printlos(object *op, char *params)
{
    if (op)
        print_los(op);
    return 0;
}


int command_version(object *op, char *params)
{
    version(op);
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
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "Set listen to what (presently %d)?", CONTR(op)->listening);
        return 1;
    }
    CONTR(op)->listening = (char) i;
    new_draw_info_format(NDI_UNIQUE, 0, op, "Your verbose level is now %d.", i);
    return 1;
}

/* Prints out some useful information for the character.  Everything we print
 * out can be determined by the docs, so we aren't revealing anything extra -
 * rather, we are making it convenient to find the values.  params have
 * no meaning here.
 */
int command_statistics(object *pl, char *params)
{
    if (pl->type != PLAYER || !CONTR(pl))
        return 1;
    new_draw_info_format(NDI_UNIQUE, 0, pl, "  Experience: %d", pl->stats.exp);
    new_draw_info_format(NDI_UNIQUE, 0, pl, "  Next Level: %d", GET_LEVEL_EXP(pl->level + 1));
    new_draw_info(NDI_UNIQUE, 0, pl, "\nStat       Nat/Real/Max");

    new_draw_info_format(NDI_UNIQUE, 0, pl, "Str         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Str, pl->stats.Str,
                         20 + pl->arch->clone.stats.Str);
    new_draw_info_format(NDI_UNIQUE, 0, pl, "Dex         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Dex, pl->stats.Dex,
                         20 + pl->arch->clone.stats.Dex);
    new_draw_info_format(NDI_UNIQUE, 0, pl, "Con         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Con, pl->stats.Con,
                         20 + pl->arch->clone.stats.Con);
    new_draw_info_format(NDI_UNIQUE, 0, pl, "Int         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Int, pl->stats.Int,
                         20 + pl->arch->clone.stats.Int);
    new_draw_info_format(NDI_UNIQUE, 0, pl, "Wis         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Wis, pl->stats.Wis,
                         20 + pl->arch->clone.stats.Wis);
    new_draw_info_format(NDI_UNIQUE, 0, pl, "Pow         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Pow, pl->stats.Pow,
                         20 + pl->arch->clone.stats.Pow);
    new_draw_info_format(NDI_UNIQUE, 0, pl, "Cha         %2d/ %3d/%3d", CONTR(pl)->orig_stats.Cha, pl->stats.Cha,
                         20 + pl->arch->clone.stats.Cha);

    /* Can't think of anything else to print right now */
    return 0;
}

int command_fix_me(object *op, char *params)
{
    FIX_PLAYER(op ,"command fix_me");
    return 1;
}


int command_logs(object *op, char *params)
{
    int first;

    first = 1;

    if (first)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Nobody is currently logging kills.");
    }
    return 1;
}

int command_usekeys(object *op, char *params)
{
    usekeytype      oldtype = CONTR(op)->usekeys;
    static char    *types[] =
    {
        "inventory", "keyrings", "containers"
    };

    if (!params)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "usekeys is set to %s", types[CONTR(op)->usekeys]);
        return 1;
    }

    if (!strcmp(params, "inventory"))
        CONTR(op)->usekeys = key_inventory;
    else if (!strcmp(params, "keyrings"))
        CONTR(op)->usekeys = keyrings;
    else if (!strcmp(params, "containers"))
        CONTR(op)->usekeys = containers;
    else
    {
        new_draw_info_format(NDI_UNIQUE, 0, op,
                             "usekeys: Unknown options %s, valid options are inventory, keyrings, containers", params);
             return 0;
    }
    new_draw_info_format(NDI_UNIQUE, 0, op, "usekeys %s set to %s", (oldtype == CONTR(op)->usekeys ? "" : "now"),
                         types[CONTR(op)->usekeys]);
    return 1;
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

        new_draw_info_format(NDI_UNIQUE, 0, op, "%-20s %+5d", attack_name[i], op->resist[i]);
    }
    return 0;
}
/*
 * Actual commands.
 * Those should be in small separate files (c_object.c, c_wiz.c, cmove.c,...)
 */


static void help_topics(object *op, int what)
{
    DIR            *dirp;
    struct dirent  *de;
    char            filename[MAX_BUF], line[80];
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
            new_draw_info(NDI_UNIQUE, 0, op, line);
            sprintf(line, " %s", de->d_name);
            linelen = namelen + 1;
            continue;
        }
        strcat(line, " ");
        strcat(line, de->d_name);
    }
    new_draw_info(NDI_UNIQUE, 0, op, line);
    closedir(dirp);
}

static void show_commands(object *op)
{
    CommArray_s *ap[6];
    int          size[6],
                 i;

    for (i = 0; i < 6; i++)
    {
        ap[i] = NULL;
        size[i] = -1;
    }

    switch (CONTR(op)->gmaster_mode)
    {
        case GMASTER_MODE_VOL:
            ap[0] = Commands;
            ap[1] = CommunicationCommands;
            ap[2] = CommandsVOL;
            size[0] = CommandsSize;
            size[1] = CommunicationCommandsSize;
            size[2] = CommandsVOLSize;

            break;

        case GMASTER_MODE_GM:
            ap[0] = Commands;
            ap[1] = CommunicationCommands;
            ap[2] = CommandsVOL;
            ap[3] = CommandsGM;
            size[0] = CommandsSize;
            size[1] = CommunicationCommandsSize;
            size[2] = CommandsVOLSize;
            size[3] = CommandsGMSize;

            break;

        case GMASTER_MODE_MW:
            ap[0] = Commands;
            ap[1] = CommunicationCommands;
            ap[2] = CommandsMW;
            size[0] = CommandsSize;
            size[1] = CommunicationCommandsSize;
            size[2] = CommandsMWSize;

            break;

        case GMASTER_MODE_MM:
            ap[0] = Commands;
            ap[1] = CommunicationCommands;
            ap[2] = CommandsVOL;
            ap[3] = CommandsGM;
            ap[4] = CommandsMW;
            ap[5] = CommandsMM;
            size[0] = CommandsSize;
            size[1] = CommunicationCommandsSize;
            size[2] = CommandsVOLSize;
            size[3] = CommandsGMSize;
            size[4] = CommandsMWSize;
            size[5] = CommandsMMSize;

            break;

        default:
            ap[0] = Commands;
            ap[1] = CommunicationCommands;
            size[0] = CommandsSize;
            size[1] = CommunicationCommandsSize;
    }

    for (i = 0; i < 6 && ap[i]; i++)
    {
        int  j;
        char buf[MAX_BUF];

        if (ap[i] == Commands)
            new_draw_info(NDI_UNIQUE | NDI_YELLOW, 0, op, "\nNormal Commands");
        else if (ap[i] == CommunicationCommands)
            new_draw_info(NDI_UNIQUE | NDI_YELLOW, 0, op, "\nEmotes");
        else if (ap[i] == CommandsVOL)
            new_draw_info(NDI_UNIQUE | NDI_YELLOW, 0, op, "\nVOL Commands");
        else if (ap[i] == CommandsGM)
            new_draw_info(NDI_UNIQUE | NDI_YELLOW, 0, op, "\nGM Commands");
        else if (ap[i] == CommandsMW)
            new_draw_info(NDI_UNIQUE | NDI_YELLOW, 0, op, "\nMW Commands");
        else if (ap[i] == CommandsMM)
            new_draw_info(NDI_UNIQUE | NDI_YELLOW, 0, op, "\nMM Commands");
        else
        {
            LOG(llevDebug, "DEBUG:: %s/show_commands(): Unknown command structure!\n",
                __FILE__);

            return;
        }

        for (j = 0, buf[0] = '\0'; j < size[i]; j++)
        {
            /* TODO: This calculation can be removed, and the following
             * new_draw_info() moved to inside this for loop by having the
             * client handle NDI_UNIQUE properly (well, at all).
             * -- Smacky 20090604 */
            if (strlen(buf) + strlen(ap[i][j].name) > 42)
            {
                new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, buf);
                buf[0] = '\0';
            }

            sprintf(strchr(buf, '\0'), " /%s ~+~", ap[i][j].name);
        }

        new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, buf);
    }
}

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

int command_help(object *op, char *params)
{
    struct stat st;
    FILE       *fp;
    char        filename[MAX_BUF], line[MAX_BUF];
    int         len;

    /* Main help page?  */
    if (!params)
    {
        sprintf(filename, "%s/def_help", HELPDIR);

        if ((fp = fopen(filename, "r")) == NULL)
        {
            LOG(llevBug, "BUG: Can't open %s\n", filename);
            /*perror("Can't read default help");*/

            return 0;
        }

        while (fgets(line, MAX_BUF, fp))
        {
            line[MAX_BUF - 1] = '\0';
            len = strlen(line) - 1;

            if (line[len] == '\n')
                line[len] = '\0';
            new_draw_info(NDI_UNIQUE, 0, op, line);
        }
        fclose(fp);

        return 0;
    }
    /* Topics list */
    else if (!strcmp(params, "list"))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "\n**** list of help topics ****");
        help_topics(op, 3);
        help_topics(op, 0);

        if (QUERY_FLAG(op, FLAG_WIZ))
            help_topics(op, 1);

        return 0;
    }
    /* Commands list */
    else if (!strcmp(params, "commands"))
    {
        show_commands(op);

        return 0;
    }
    /* /command */
    else if (params[0] == '/')
    {
//        if (strpbrk(params + 1, " ./\\"))
//        {
//            sprintf(line, "Illegal characters in '%s'", params);
//            new_draw_info(NDI_UNIQUE, 0, op, line);
//
//            return 0;
//        }
        new_draw_info_format(NDI_UNIQUE | NDI_YELLOW, 0, op, "Help for command %s:",
                             params);

        if (!find_command(params + 1, CONTR(op)))
        {
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "Unrecognised command!");

            return 0;
        }

        sprintf(filename, "%s/commands%s", HELPDIR, params);

        if ((fp = fopen(filename, "r")) == NULL)
        {
            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, "None available!");

            return 0;
        }

        while (fgets(line, MAX_BUF, fp))
        {
            len = (int)strlen(line) - 1;

            if (line[len] == '\n')
                line[len] = '\0';

            new_draw_info(NDI_UNIQUE | NDI_WHITE, 0, op, line);
        }

        fclose(fp);

        return 0;
    }

    /* Unknown topic */
    new_draw_info_format(NDI_UNIQUE, 0, op, "No help available on '%s'",
                         params);

    return 0;
#if 0
    sprintf(filename, "%s/commands/%s", HELPDIR, params);
    if (stat(filename, &st) || !S_ISREG(st.st_mode))
    {
        if (op)
        {
            sprintf(filename, "%s/help/%s", HELPDIR, params);
            if (stat(filename, &st) || !S_ISREG(st.st_mode))
            {
                if (QUERY_FLAG(op, FLAG_WIZ))
                {
                    sprintf(filename, "%s/wizhelp/%s", HELPDIR, params);
                    if (stat(filename, &st) || !S_ISREG(st.st_mode))
                        goto nohelp;
                }
                else
                    goto nohelp;
            }
        }
    }

    /*
     * Found that. Just cat it to screen.
     */
    if ((fp = fopen(filename, "r")) == NULL)
    {
        LOG(llevBug, "BUG: Can't open %s\n", filename);
        /*perror("Can't read helpfile");*/
        return 0;
    }
    while (fgets(line, MAX_BUF, fp))
    {
        line[MAX_BUF - 1] = '\0';
        len = strlen(line) - 1;
        if (line[len] == '\n')
            line[len] = '\0';
        new_draw_info(NDI_UNIQUE, 0, op, line);
    }
    fclose(fp);
    return 0;
#endif
}


int onoff_value(char *line)
{
    int i;

    if (sscanf(line, "%d", &i))
        return (i != 0);
    switch (line[0])
    {
        case 'o':
          switch (line[1])
          {
              case 'n':
                return 1;       /* on */
              default:
                return 0;       /* o[ff] */
          }
        case 'y':
          /* y[es] */
        case 'k':
          /* k[ylla] */
        case 's':
        case 'd':
          return 1;
        case 'n':
          /* n[o] */
        case 'e':
          /* e[i] */
        case 'u':
        default:
          return 0;
    }
}

int command_save(object *op, char *params)
{
    if (blocks_cleric(op->map, op->x, op->y))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can not save on unholy ground");
    }
#if 0 // no longer needed with accounts patch
    else if (!op->stats.exp)
    {
        new_draw_info(NDI_UNIQUE, 0, op,
                "To avoid too many unused player accounts you must get some exp before you can save!");
    }
#endif
    else
    {
        if (player_save(op))
            new_draw_info(NDI_UNIQUE, 0, op, "You have been saved.");
        else
            new_draw_info(NDI_UNIQUE, 0, op, "SAVE FAILED!");

        /* with the new code we should NOT save "active" maps.
         * we do a kind of neutralizing when we save now that can have
         * strange effects when saving!
        if(op->map && !strncmp(op->map->path, settings.localdir, strlen(settings.localdir)))
        {
            new_save_map(op->map,0);
            op->map->in_memory=MAP_IN_MEMORY;
        }*/
    }
    return 1;
}


int command_style_map_info(object *op, char *params)
{
    extern mapstruct               *styles;
    mapstruct  *mp;
    int         maps_used = 0, mapmem = 0, objects_used = 0, x, y;
    object     *tmp;

    for (mp = styles; mp != NULL; mp = mp->next)
    {
        maps_used++;
        mapmem += MAP_WIDTH(mp) * MAP_HEIGHT(mp) * (sizeof(object *) + sizeof(MapSpace)) + sizeof(mapstruct);
        for (x = 0; x < MAP_WIDTH(mp); x++)
        {
            for (y = 0; y < MAP_HEIGHT(mp); y++)
            {
                for (tmp = get_map_ob(mp, x, y); tmp != NULL; tmp = tmp->above)
                    objects_used++;
            }
        }
    }
    new_draw_info_format(NDI_UNIQUE, 0, op, "Style maps loaded:    %d", maps_used);
    new_draw_info(NDI_UNIQUE, 0, op, "Memory used, not");
    new_draw_info_format(NDI_UNIQUE, 0, op, "including objects:    %d", mapmem);
    new_draw_info_format(NDI_UNIQUE, 0, op, "Style objects:        %d", objects_used);
    new_draw_info_format(NDI_UNIQUE, 0, op, "Mem for objects:      %d", objects_used * sizeof(object));
    return 0;
}

int command_silent_login(object *op, char *params)
{
    int new_status = !CONTR(op)->silent_login;

    CONTR(op)->silent_login = new_status;

    if (new_status)
        new_draw_info(NDI_UNIQUE, 0, op, "Silent login enabled.");
    else
        new_draw_info(NDI_UNIQUE, 0, op, "Silent login disabled.");
    return 1;
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


int command_stuck(object *op, char *params)
{
    if (op->type == PLAYER && CONTR(op))
    {
        command_goto(op, "/planes/human_plane/castle/castle_030a 5 11");
	}
    return 0;
}
