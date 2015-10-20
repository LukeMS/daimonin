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


/* we save the current player status to file */
int player_save(object_t *op)
{
    FILE   *fp;
    char    filename[MEDIUM_BUF], tmpfilename[MAXPATHLEN], backupfile[MEDIUM_BUF]="";
    player_t *pl  = CONTR(op);
    int     tmp, have_file = TRUE, i;
    object_t *force;
    int drain_level = 0;
    Account *ac;

#ifdef USE_CHANNELS
    struct  player_channel *pl_channel;
#endif

    /* Sanity check - some stuff changes this when player is exiting */
    if (op->type != PLAYER || pl == NULL)
        return 0;

    pl->last_save_tick = ROUND_TAG;
    sprintf(filename, "%s/%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, get_subdir(op->name), op->name, op->name);

    /* if the file already is there, we don't must create the dirs but do backups */
    if (access(filename, F_OK) != 0)
    {
        have_file = FALSE;
        make_path_to_file(filename);
    }

    tempnam_local_ext(settings.tmpdir, NULL, tmpfilename);

    fp = fopen(tmpfilename, "w");
    if (!fp)
    {
        ndi(NDI_UNIQUE, 0, op, "Can't open file for save.");
        LOG(llevDebug, "Can't open file for save (%s).\n", tmpfilename);
        return 0;
    }

    fprintf(fp, "account %s\n", pl->account_name);

    /* save player state without the dynamic/session set one if there is something */
    if((tmp = (pl->state&~(ST_PLAYING|ST_ZOMBIE|ST_DEAD))))
        fprintf(fp, "state %d\n",  tmp);

    fprintf(fp, "guild_updated %d\n", pl->guild_updated);

    if (pl->gmaster_mode != GMASTER_MODE_NO)
    {
        if ((pl->gmaster_mode & GMASTER_MODE_SA))
        {
            fprintf(fp, "dm_SA\n");
        }
        else if ((pl->gmaster_mode & GMASTER_MODE_MM))
        {
            fprintf(fp, "dm_MM\n");
        }
        else if ((pl->gmaster_mode & GMASTER_MODE_MW))
        {
            fprintf(fp, "dm_MW\n");
        }
        else if ((pl->gmaster_mode & GMASTER_MODE_GM))
        {
            fprintf(fp, "dm_GM\n");
        }
        else if ((pl->gmaster_mode & GMASTER_MODE_VOL))
        {
            fprintf(fp, "dm_VOL\n");
        }
    }
    if(pl->mute_counter > pticks)
        fprintf(fp, "mute %d\n", (int)(pl->mute_counter-pticks)); /* should be not THAT long */

    fprintf(fp, "stealth %d\nprivacy %d\np_ver %d\nlistening %d\npickup %d\nskill_group %d %d %d\n",
                 pl->gmaster_stealth, pl->privacy, pl->p_ver, pl->listening, pl->mode,
                 pl->base_skill_group[0],pl->base_skill_group[1],pl->base_skill_group[2]);

    fprintf(fp, "map %s\n", pl->maplevel);
    fprintf(fp, "o_map %s\n", pl->orig_map);
    fprintf(fp, "savebed_map %s\n", pl->savebed_map);
    fprintf(fp, "o_bed %s\n", pl->orig_savebed_map);
    fprintf(fp, "map_s %d\nbed_s %d\nmap_x %d\nmap_y %d\nbed_x %d\nbed_y %d\n",
                MAP_STATUS_TYPE(pl->status), MAP_STATUS_TYPE(pl->bed_status), pl->map_x, pl->map_y, pl->bed_x, pl->bed_y);

    if(pl->instance_name)
    {
        fprintf(fp, "iname %s\ninum %d\niid %ld\niflags %d\n", pl->instance_name, pl->instance_num, pl->instance_id,pl->instance_flags);
    }

    fprintf(fp, "Str %d\nDex %d\nCon %d\nInt %d\nPow %d\nWis %d\nCha %d\n",
                pl->orig_stats.Str, pl->orig_stats.Dex, pl->orig_stats.Con, pl->orig_stats.Int,
                pl->orig_stats.Pow, pl->orig_stats.Wis, pl->orig_stats.Cha);

#ifdef USE_CHANNELS
    /* save the channel stuff */
    fprintf(fp, "Channels_On %d\n",pl->channels_on);
    fprintf(fp, "channels %d\n",pl->channel_count);
    for (pl_channel=pl->channels;pl_channel;pl_channel=pl_channel->next_channel)
    {
        fprintf(fp,"%s %c %lu\n",pl_channel->channel->name, pl_channel->shortcut, pl_channel->mute_counter);
    }
#endif

    /* check for drain so we know the proper level of the player to save the hp table with */
    if ((force = present_arch_in_ob(archetype_global._drain, op)))
        drain_level = force->level;

    /* save hp table */
    fprintf(fp, "lev_hp %d\n", op->level + drain_level);
    for (i = 1; i <= op->level + drain_level; i++)
        fprintf(fp, "%d\n", pl->levhp[i]);

    /* save sp table */
    fprintf(fp, "lev_sp %d\n", pl->skillgroup_ptr[SKILLGROUP_MAGIC]->level);
    for (i = 1; i <= pl->skillgroup_ptr[SKILLGROUP_MAGIC]->level; i++)
        fprintf(fp, "%d\n", pl->levsp[i]);

    fprintf(fp, "lev_grace %d\n", pl->skillgroup_ptr[SKILLGROUP_WISDOM]->level);
    for (i = 1; i <= pl->skillgroup_ptr[SKILLGROUP_WISDOM]->level; i++)
        fprintf(fp, "%d\n", pl->levgrace[i]);


    for (i = 0; i < pl->nrofknownspells; i++)
        fprintf(fp, "known_spell %s\n", spells[pl->known_spells[i]].name);
    fprintf(fp, "endplst\n");

    SET_FLAG(op, FLAG_NO_FIX_PLAYER);
    save_object(fp, op, 3); /* don't check and don't remove */
    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);

    if (fclose(fp) == EOF)
    {
        /* make sure the write succeeded */
        ndi(NDI_UNIQUE, 0, op, "Can't save character.");
        unlink(tmpfilename);
        return 0;
    }

    /* we back up the old file - then we try to move our new file
     * on the position of the old, just backuped one.
     * If that fails, we fallback to our backup and try to put
     * it back
     */
    if(have_file) /* backup only if there is an old file */
    {
        sprintf(backupfile, "%s.tmp", filename);
        if(rename(filename, backupfile))
            LOG(llevBug,"BUG: backupfile %s generation failed: errno %d\n", backupfile, errno);
    }

    /* lets put our new saved file in tmpfile to filename */
    if(rename(tmpfilename, filename))
    {
        struct timeval   now;

        LOG(llevBug,"BUG: tmpfile: %s to filename: %s renaming failed: errno %d\n", tmpfilename, filename, errno);
        LOG(llevDebug,"Restoring backupfile %s\n", backupfile);

        if(!have_file) /* failed to write file? no old file? give up */
            return 0;

        rename(backupfile, filename);

        /* try some last trick and neutralize the tmp file with a time tag.
         * perhaps a DM can repair this by hand...
         */
        GETTIMEOFDAY(&now);
        /* Remove unused argument, but keep the original, in case someone wants to fix the format */
        /*sprintf(filename, "%s.tmp.lu",tmpfilename, now.tv_sec);*/
        sprintf(filename, "%s.tmp.lu",tmpfilename);
        rename(tmpfilename, filename);
        unlink(tmpfilename); /* last sanity - kill this now 100% invalid tmpfilename */
    }

    if(have_file)
        unlink(backupfile); /* also 100% invalid now */
    chmod(filename, SAVE_MODE);

    /* Note - during char creation process account_update will return
     * 0, as char does not yet exist in the account. */
    if ((ac = account_get_from_object(op)))
        if (account_update(ac, op))
            if (account_save(ac, ac->name) != ACCOUNT_STATUS_OK)
                return 0;

    return 1;
}

#if 1
static int spell_sort(const void *a1, const void *a2)
{
    return strcmp(spells[(int) * (sint16 *) a1].name, spells[(int) * (sint16 *) a2].name);
}
#else
static int spell_sort(const char *a1, const char *a2)
{
    LOG(llevDebug, "spell1=%d, spell2=%d\n", *(sint16 *) a1, *(sint16 *) a2);
    return strcmp(spells[(int) * a1].name, spells[(int) * a2].name);
}
#endif

/* helper function to reorder the reverse loaded
 * player inventory. This will recursive reorder
 * the container inventories.
 */
static void reorder_inventory(object_t *op)
{
    object_t *tmp, *tmp2;

    tmp2 = op->inv->below;
    op->inv->above = NULL;
    op->inv->below = NULL;

    if (op->inv->inv)
        reorder_inventory(op->inv);

    for (; tmp2;)
    {
        tmp = tmp2;
        tmp2 = tmp->below; /* save the following element */
        tmp->above = NULL;
        tmp->below = op->inv; /* resort it like in insert_ob_in_ob() */
        tmp->below->above = tmp;
        op->inv = tmp;
        if (tmp->inv)
            reorder_inventory(tmp);
    }
}

/* Helper function for player_load() and player_create()
 * get a player object and set neutral base values,
 * so the the player is virtual "valid"
 */
static player_t *get_player_struct(void)
{
    player_t *p;
    int     i;

    p = (player_t *) get_poolchunk(pool_player);

    /* don't try any recovering here - oom means to leave ASAP */
    if (p == NULL)
        LOG(llevError, "ERROR: get_player(): out of memory\n");

    /* Initial value settings is zero ... */
    memset(p, 0, sizeof(player_t));

    /* ... but init some more stuff with non zero values or macros */
    p->group_id = GROUP_NO;
    p->last_save_tick = 9999999;
    p->gmaster_mode = GMASTER_MODE_NO;
    p->target_hp = -1;
    p->target_hp_p = -1;
    p->listening = 9;
    p->last_weapon_sp = -1;
    p->update_los = 1;
    p->update_target = 1;

    /* Disable static socket: important setting to tell the engine the socket is not valid */
    p->socket.status = Ns_Disabled;

    /* Would be better of '0' was not a defined spell */
    for (i = 0; i < NROFREALSPELLS; i++)
        p->known_spells[i] = -1;

    /* we need to clear these to -1 and not zero - otherwise,
    * if a player quits and starts a new character, we wont
    * send new values to the client, as things like exp start
    * at zero.
    */
    for (i = 0; i < NROFSKILLGROUPS; i++)
    {
        p->skillgroup_ptr[i] = NULL;
        p->last_skillgroup_exp[i] = -1;
        p->last_skillgroup_level[i] = -1;
    }

    p->set_skill_weapon = NO_SKILL_READY; /* quick skill reminder for select hand weapon */
    p->set_skill_archery = NO_SKILL_READY;
    p->last_stats.exp = -1;

    p->name_changed = 1;
    p->p_ver = PLAYER_FILE_VERSION_DEFAULT;

#ifdef USE_CHANNELS
    p->channels_on=TRUE;
#endif

    return p;
}

/* this whole player loading routine is REALLY not optimized -
 * just look for all these scanf()
 */
addme_login_msg player_load(NewSocket *ns, const char *name)
{
    player_t      *pl;
    object_t      *op;
    FILE        *fp;
    void        *mybuffer;
    char        filename[MEDIUM_BUF];
    char        buf[MEDIUM_BUF], bufall[MEDIUM_BUF];
    int         i, value;
    time_t      elapsed_save_time   = 0;
    struct stat statbuf;
    object_t     *tmp, *tmp2;
    int         mode_id = GMASTER_MODE_NO;
#ifdef USE_CHANNELS
    int     with_channels = FALSE;
    int     channelcount=0;
    /* we limit channels in save file for now to 256 entrys */
    char    chantemp[256][MEDIUM_BUF];

    char channelname[MAX_CHANNEL_NAME+1];
    char shortcut;
    unsigned long mute=0;
#endif
#ifdef PLUGINS
    CFParm      CFP;
    int         evtid;
#endif

    sprintf(filename, "%s/%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, get_subdir(name), name, name);
    LOG(llevInfo, "PLAYER: %s\n", filename);

    if ((fp = fopen(filename, "r")) == NULL)
    {
        return ADDME_MSG_UNKNOWN; /* player not found - return addme fails with no player */
    }

    /* do some sanity checks with the the file */
    if (fstat(fileno(fp), &statbuf))
    {
        LOG(llevBug, "\nBUG: Unable to stat %s?\n", filename);
        elapsed_save_time = 0;
    }
    else
    {
        elapsed_save_time = time(NULL) - statbuf.st_mtime;
        if (elapsed_save_time < 0)
        {
            LOG(llevBug, "\nBUG: Player file %s was saved in the future? (%d time)\n",
                filename, (int)elapsed_save_time);
            elapsed_save_time = 0;
        }
    }

    /* the first line is always the password - so, we check it first. */
    if (fgets(bufall, MEDIUM_BUF, fp) == NULL)
    {
        LOG(llevDebug, "\nBUG: Corrupt player file %s!\n", filename);
        fclose(fp);
        return ADDME_MSG_CORRUPT; /* addme fails - can't load player */
    }
    else
    {
        if (sscanf(bufall, "account %s\n", buf))
        {
            /* TODO: check double login of char under different accounts.
             * can be when a DM force a player load to his connection
             * for maintance
             */
            /* simple sanity check - player is owned by this account? */
            if(strcmp(ns->pl_account.name, buf))
            {
                fclose(fp);
                return ADDME_MSG_ACCOUNT;
            }
        }
    } /* fgets account name */

    /* Here we go.. at this point we will put this player on the map, whatever happens
     * If needed we overrule bogus loaded stats but we WILL put a valid player in the
     * game. That also means we don't deal with freeing pl or objects here anymore.
     * We put a valid player in the game and after this the higher engine function
     * will take care about this player.
     */

    pl = get_player_struct(); /* will return the struct with safe base values! */
    FREE_AND_ADD_REF_HASH(pl->account_name, ns->pl_account.name);

    /* we have here the classic problem with fgets():
     * fgets() reads in a string and puts the \0 after the 0x0a.
     * The problem is, that when we have saved the file, we have added
     * the 0x0a to the original string as end marker - thats the way how
     * you save a text file. fgets is WRONG here - what we would need
     * is gets(), wich does it right by exchanging 0x0a through \0.
     * but gets() only works on stdin... Thats the reason we use sscanf() to
     * get a string parameter. What we need is a self coded fgets() which works
     * like gets(). MT-10/2006
     */
    while (fgets(bufall, MEDIUM_BUF, fp) != NULL)
    {
        if (!strncmp(bufall, "skill_group ",12))
        {
            sscanf(bufall, "%s %d %d %d\n", buf, &pl->base_skill_group[0], &pl->base_skill_group[1], &pl->base_skill_group[2]);
            continue;
        }
        sscanf(bufall, "%s %d\n", buf, &value);
        if (!strcmp(buf, "endplst"))
            break;
        else if (!strcmp(buf, "guild_updated"))
            pl->guild_updated = value;
        else if (!strcmp(buf, "dm_SA"))
            mode_id = GMASTER_MODE_SA;
        else if (!strcmp(buf, "dm_MM"))
            mode_id = GMASTER_MODE_MM;
        else if (!strcmp(buf, "dm_MW"))
            mode_id = GMASTER_MODE_MW;
        else if (!strcmp(buf, "dm_GM"))
            mode_id = GMASTER_MODE_GM;
        else if (!strcmp(buf, "dm_VOL"))
            mode_id = GMASTER_MODE_VOL;
        else if (!strcmp(buf, "mute"))
            pl->mute_counter = pticks+(unsigned long)value;
        else if (!strcmp(buf, "state"))
            pl->state = value; /* be sure to do all other state flag settings after this load */
        else if (!strcmp(buf, "stealth"))
            pl->gmaster_stealth = value;
        else if (!strcmp(buf, "privacy"))
            pl->privacy = value;
        else if (!strcmp(buf, "p_ver"))
            pl->p_ver = value;
        else if (!strcmp(buf, "listening"))
            pl->listening = value;
        else if (!strcmp(buf, "pickup"))
            pl->mode = value;
        else if (!strcmp(buf, "iflags"))
            pl->instance_flags = value;
        else if (!strcmp(buf, "iid"))
            pl->instance_id = value;
        else if (!strcmp(buf, "inum"))
            pl->instance_num = value;
        else if (!strcmp(buf, "iname"))
        {
            sscanf(bufall, "iname %s", buf );
            FREE_AND_COPY_HASH(pl->instance_name, buf);
        }
        else if (!strcmp(buf, "map"))
        {
            sscanf(bufall, "map %s", buf );
            FREE_AND_COPY_HASH(pl->maplevel, buf);
        }
        else if (!strcmp(buf, "o_map"))
        {
            sscanf(bufall, "o_map %s", buf );
            FREE_AND_COPY_HASH(pl->orig_map, buf);
        }
        else if (!strcmp(buf, "savebed_map"))
        {
            sscanf(bufall, "savebed_map %s", buf );
            FREE_AND_COPY_HASH(pl->savebed_map, buf);
        }
        else if (!strcmp(buf, "o_bed"))
        {
            sscanf(bufall, "o_bed %s", buf );
            FREE_AND_COPY_HASH(pl->orig_savebed_map, buf);
        }
        else if (!strcmp(buf, "map_s"))
            pl->status = value;
        else if (!strcmp(buf, "bed_s"))
            pl->bed_status = value;
        else if (!strcmp(buf, "map_x"))
            pl->map_x = value;
        else if (!strcmp(buf, "map_y"))
            pl->map_y = value;
        else if (!strcmp(buf, "bed_x"))
            pl->bed_x = value;
        else if (!strcmp(buf, "bed_y"))
            pl->bed_y = value;
        else if (!strcmp(buf, "Str"))
            pl->orig_stats.Str = value;
        else if (!strcmp(buf, "Dex"))
            pl->orig_stats.Dex = value;
        else if (!strcmp(buf, "Con"))
            pl->orig_stats.Con = value;
        else if (!strcmp(buf, "Int"))
            pl->orig_stats.Int = value;
        else if (!strcmp(buf, "Pow"))
            pl->orig_stats.Pow = value;
        else if (!strcmp(buf, "Wis"))
            pl->orig_stats.Wis = value;
        else if (!strcmp(buf, "Cha"))
            pl->orig_stats.Cha = value;
#ifdef USE_CHANNELS
        else if (!strcmp(buf, "Channels_On"))
            pl->channels_on=value;
        else if (!strcmp(buf,"channels"))
        {
            with_channels = TRUE;
            channelcount=value;
            for (i=1; (i<= value) && (i<256); i++)
            {
                char *dummy; // purely to suppres GCC's warn_unused_result

                dummy = fgets(chantemp[i-1], MEDIUM_BUF, fp);
            }
        }
#endif
        else if (!strcmp(buf, "lev_hp"))
        {
            int j;
            for (i = 1; i <= value; i++)
            {
                if (fscanf(fp, "%d\n", &j) != 1)
                {
                    LOG(llevBug, "BUG: %s/player_load(): Corrupt lev_hp value in save file '%s'\n",
                        __FILE__, filename);
                }

                pl->levhp[i] = j;
            }
        }
        else if (!strcmp(buf, "lev_sp"))
        {
            int j;
            for (i = 1; i <= value; i++)
            {
                if (fscanf(fp, "%d\n", &j) != 1)
                {
                    LOG(llevBug, "BUG: %s/player_load(): Corrupt lev_sp value in save file '%s'\n",
                        __FILE__, filename);
                }

                pl->levsp[i] = j;
            }
        }
        else if (!strcmp(buf, "lev_grace"))
        {
            int j;
            for (i = 1; i <= value; i++)
            {
                if (fscanf(fp, "%d\n", &j) != 1)
                {
                    LOG(llevBug, "BUG: %s/player_load(): Corrupt lev_grace value in save file '%s'\n",
                        __FILE__, filename);
                }

                pl->levgrace[i] = j;
            }
        }
        else if (!strcmp(buf, "known_spell"))
        {
            char   *cp  = strchr(bufall, '\n');
            *cp = '\0';
            cp = strchr(bufall, ' ');
            cp++;
            for (i = 0; i < NROFREALSPELLS; i++)
                if (!strcmp(spells[i].name, cp))
                {
                    pl->known_spells[pl->nrofknownspells++] = i;
                    break;
                }
            if (i == NROFREALSPELLS)
                LOG(llevDebug, "Error: unknown spell (%s) for player %s\n", cp, name);
        }
        else
            LOG(llevDebug, "Debug: load_player(%s) unknown line in player file: %s\n", name, bufall);
    } /* End of loop loading the character file */

    /* Ensure we have a valid map we can the player kick put in later.
     * when needed fallback to the defined start maps. */
    if (!pl->maplevel)
    {
        MAP_SET_PLAYER_MAP_INFO_DEFAULT(pl);
    }
    else if (!pl->orig_map)
    {
        FREE_AND_ADD_REF_HASH(pl->orig_map, pl->maplevel);
    }

    if (!pl->savebed_map)
    {
        MAP_SET_PLAYER_BED_INFO_DEFAULT(pl);
    }
    else if (!pl->orig_savebed_map)
    {
        FREE_AND_ADD_REF_HASH(pl->orig_savebed_map, pl->savebed_map);
    }

    LOG(llevDebug, "load obj for player: %s\n", name);
    op = get_object(); /* Create a new object for the real player data */
    SET_FLAG(op, FLAG_NO_FIX_PLAYER);

    /* this loads the standard objects values. */
    mybuffer = create_loader_buffer(fp);
    load_object(fp, op, mybuffer, LO_REPEAT, 0);
    delete_loader_buffer(mybuffer);
    fclose(fp);

    op->custom_attrset = pl;
    pl->ob = op;
    pl->name_changed = 1;
    op->type = PLAYER;

    if (!QUERY_FLAG(op, FLAG_FRIENDLY)) /* ensure we are on friendly list */
    {
        LOG(llevBug, "BUG: Player %s was loaded without friendly flag!", STRING_OBJ_NAME(op));
        SET_FLAG(op, FLAG_FRIENDLY);
    }

    /* File Version Change */
    if(pl->p_ver != PLAYER_FILE_VERSION_DEFAULT)
    {
        pl->p_ver = PLAYER_FILE_VERSION_DEFAULT; /* new version */
        /* do some stuff here is needed */
    }

    /* at this moment, the inventory is reverse loaded.
     * Lets exchange it here.
     * Caution: We do it on the hard way here without
     * calling remove/insert again.
     */
    if (op->inv)
    {
        tmp2 = op->inv->below;
        op->inv->above = NULL;
        op->inv->below = NULL;

        if (op->inv->inv)
            reorder_inventory(op->inv);

        for (; tmp2;)
        {
            tmp = tmp2;
            tmp2 = tmp->below; /* save the following element */
            tmp->above = NULL;
            tmp->below = op->inv; /* resort it like in insert_ob_in_ob() */
            tmp->below->above = tmp;
            op->inv = tmp;
            if (tmp->inv)
                reorder_inventory(tmp);
        }
    }

    op->carrying = sum_weight(op); /* sanity calc for inventory weight of loaded players */

    link_player_skills(pl); /* link all exp group & skill objects to the player */

    /* A little hack to remove guild forces from players which were created before new
     * guild system, which may cause bugs with backwards-incompatibility. This
     * code and pl->guild_updated should be removed when we have another player wipe.
     */
    if (!pl->guild_updated)
    {
        object_t *obj;
        archetype_t *arch;

        while (obj = present_arch_in_ob(archetype_global._guild_force, op))
            if (obj)
                remove_ob(obj);

        arch = archetype_global._guild_force;
        obj = arch_to_object(arch);
        pl->guild_force = insert_ob_in_ob(obj, pl->ob);

        pl->guild_updated = 1;
        LOG(llevInfo, "INFO:: %s/player_load(): Guild forces removed and replaced for the new guild system for %s.\n",
            __FILE__, STRING_OBJ_NAME(op));
    }

    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
    pl->last_save_tick = ROUND_TAG;

    /* This seems to compile without warnings now.  Don't know if it works
     * on SGI's or not, however.
     */
    qsort((void *) pl->known_spells, pl->nrofknownspells, sizeof(pl->known_spells[0]), (void *) (int (*) ()) spell_sort);

    /* put the player in "playing" mode and add it to the player list... */
    /* enable the player socket by moving the login socket to it */
    /* WARNING: delete this reference by the caller. Its a marker to show the socket is now owned by pl */
    ns->pl = pl;
    FREE_AND_CLEAR_HASH(ns->pl_account.create_name);
    memcpy(&pl->socket, ns, sizeof(NewSocket));
    ns->pl_account.name = NULL;
    pl->socket.look_position = 0;
    pl->socket.look_position_container = 0;
    pl->socket.ext_title_flag = 1;
    pl->socket.status = Ns_Playing;
    pl->state |= ST_PLAYING;
    player_active++;

    if (player_active_meta < player_active)
    {
        player_active_meta = player_active;
    }

    if (!last_player)
    {
        first_player = last_player = pl;
    }
    else
    {
        last_player->next = pl;
        pl->prev = last_player;
        last_player = pl;
    }

    /* This command will tell the client to go in playing mode and wait for server game data */
    esrv_new_player(pl, op->weight + op->carrying);
    write_socket_buffer(&pl->socket); // ensure we send this cmd immediately
    LOG(llevDebug, "Send new_player(): socket %d\n", ns->fd);
    send_spelllist_cmd(op, NULL, SPLIST_MODE_ADD); /* send the known spells as list to client */
    send_skilllist_cmd(pl, -1, SPLIST_MODE_ADD);
    LOS_SET_TARGET(pl, op, LOS_TARGET_SELF, 0);

    /* we do the login script BEFORE we go to the map */
#ifdef PLUGINS
    if(pl->state & ST_BORN)
    {
        /* GROS : Here we handle the BORN global event */
        evtid = EVENT_BORN;
        CFP.Value[0] = (void *) (&evtid);
        CFP.Value[1] = (void *) (op);
        GlobalEvent(&CFP);
    }

    /* GROS : Here we handle the LOGIN global event */
    evtid = EVENT_LOGIN;
    CFP.Value[0] = (void *) (&evtid);
    CFP.Value[1] = (void *) (pl);
    CFP.Value[2] = (void *) (pl->socket.ip_host);
    GlobalEvent(&CFP);
#endif

    /* If pl is new, announce the newpl login to all players else if the pl has not requested privacy, announce the login to all players. */
    if (!pl->privacy)
    {
        ndi(NDI_UNIQUE | NDI_ALL, 0, NULL, "%s has entered the game%s.",
            QUERY_SHORT_NAME(pl->ob, NULL),
            ((pl->state & ST_BORN)) ? " for the first time" : "");
    }

    /* lets check we had saved last time in a gmaster mode.
     * if so, check the setting is still allowed and if so,
     * set the player to it.
     */
    if (mode_id != GMASTER_MODE_NO)
    {
        set_gmaster_mode(pl, mode_id);
    }
    else
    {
        pl->gmaster_mode = GMASTER_MODE_NO;
    }

    if (!(pl->gmaster_mode & GMASTER_MODE_SA))
    {
        esrv_send_inventory(pl, op);
        esrv_send_below(pl);
    }

#ifdef USE_CHANNELS
    /* channel-system: we check if the playerfile has the channels tag */
    /* if not, add all the default channels */
    /* ALSO we check for the BORN flag, if its set the player is freshly
       created, so we add the default channels as well */
    if (!with_channels ||
        (pl->state & ST_BORN))
    {
        addDefaultChannels(pl);
    }

    /* defered channeljoin */
    for (i = 1; i <= channelcount && i <= 255; i++)
    {
        sscanf(chantemp[i - 1], "%s %c %lu\n", channelname, &shortcut, &mute);
        loginAddPlayerToChannel(pl, channelname, shortcut, mute);
    }
#endif

    /* if we add more BORN, "first time used / first time loaded" stuff, do it before this line */
    pl->state &= ~ST_BORN;
    (void)enter_map_by_name(op, pl->maplevel, pl->orig_map, pl->map_x, pl->map_y, pl->status);

    if (pl->tadoffset == 0)
    {
        (void)command_time(op, "verbose");
    }

    FIX_PLAYER(op ,"check login - first fix");

    /* Extra info for VOLs, GMs, and SAs (if any are online), but not if pl is
     * a privacy-seeking SA*/
    if ((gmaster_list_VOL ||
         gmaster_list_GM ||
         gmaster_list_SA) &&
        !((pl->gmaster_mode & GMASTER_MODE_SA) &&
          pl->privacy))
    {
        char        buf[MEDIUM_BUF];
        objectlink_t *ol;

        buf[0] = '\0';

        /* There is no privacy from VOLs, GMs, and SA */
        if (pl->privacy)
        {
            sprintf(buf, "|%s| has entered the game (~privacy mode~).\n",
                pl->quick_name);
        }

        sprintf(strchr(buf, '\0'), "  ~IP~: %s.\n", pl->socket.ip_host);
        sprintf(strchr(buf, '\0'), "  ~Account~: %s.\n", pl->account_name);
        sprintf(strchr(buf, '\0'), "Players now playing: %d.", player_active);

        for (ol = gmaster_list_VOL; ol; ol = ol->next)
        {
            ndi(NDI_UNIQUE, 0, ol->objlink.ob, "%s", buf);
        }

        for (ol = gmaster_list_GM; ol; ol = ol->next)
        {
            ndi(NDI_UNIQUE, 0, ol->objlink.ob, "%s", buf);
        }

        for (ol = gmaster_list_SA; ol; ol = ol->next)
        {
            ndi(NDI_UNIQUE, 0, ol->objlink.ob, "%s", buf);
        }
    }

    /* Report any scheduled shutdown to the new player. */
    shutdown_agent(-1, SERVER_EXIT_NORMAL, pl, NULL);

    (void)get_online_players_info(NULL, pl, 1);

    return ADDME_MSG_OK;
}

/* we create player >name< with a template player arch object and some setup values
 * It will return at success in pl_ret a pointer to that pl objectz.
 * That object is ready to save but NOT ready for playing.
 * We don't have set the friendly list, the socket and others.
 * If we fail, no player struct will be returned and it returns
 * with an error msg.
 */
addme_login_msg player_create(NewSocket *ns, player_t **pl_ret, char *name, int race, int gender, int skill_nr)
{
    player_t          *pl = NULL;
    object_t          *op = NULL;
    archetype_t       *p_arch;
    int             skillnr[]       = {SK_MELEE_BASIC_SLASH, SK_MELEE_BASIC_IMPACT, SK_MELEE_BASIC_CLEAVE, SK_MELEE_BASIC_PIERCE};
/*    char            *skillitem[]     = {"shortsword", "mstar_small", "axe_small", "dagger_large"}; */ /* unused */

    /* do some sanity checks. *ns, *pl and the name are checked by caller */

    /* race & gender values in range? This values MUST fit or client is hacked or damaged */
    if(race < 0 || race >= settings.player_races || gender < 0 || gender >= MAX_RACE_GENDER)
        return ADDME_MSG_DISCONNECT;

    /* check we have a valid player race object arch. MUST fit too */
    if(!(p_arch = player_template[race].p_arch[gender]))
        return ADDME_MSG_DISCONNECT;

    /* now check the skill ID */
    if(skill_nr < 0 || skill_nr >= MAX_START_SKILLS)
        return ADDME_MSG_DISCONNECT;

    /* santiy checks done - all values are fine and initilized, start character generation */
    /* setup the base structure */
    pl = get_player_struct();
    op = get_object();
    copy_object_data(&p_arch->clone, op); /* copy without active list stuff and such */
    SET_FLAG(op, FLAG_NO_FIX_PLAYER);
    op->custom_attrset = pl;
    pl->ob = op;
    op->type = PLAYER;
    pl->state |= ST_BORN;
    *pl_ret = pl;
    pl->last_save_tick = ROUND_TAG;

    /* now we add our custom settings for this new character */
    FREE_AND_ADD_REF_HASH(pl->account_name, ns->pl_account.name);
    FREE_AND_COPY_HASH(op->name, name);
    pl->orig_stats.Str = op->stats.Str = player_template[race].str;
    pl->orig_stats.Dex = op->stats.Dex = player_template[race].dex;
    pl->orig_stats.Con = op->stats.Con = player_template[race].con;
    pl->orig_stats.Int = op->stats.Int = player_template[race].intel;
    pl->orig_stats.Wis = op->stats.Wis = player_template[race].wis;
    pl->orig_stats.Pow = op->stats.Pow = player_template[race].pow;
    pl->orig_stats.Cha = op->stats.Cha = player_template[race].cha;

    /* setup start point and default maps */
    MAP_SET_PLAYER_MAP_INFO_DEFAULT(pl);
    MAP_SET_PLAYER_BED_INFO_DEFAULT(pl);
    op->x = pl->map_x;
    op->y = pl->map_y;

    /* setup the base object structure, which are usually invisible objects in the inventory */
    learn_skill(op, skillnr[skill_nr]);
    validate_skills(pl);

    /* and here the initial stuff depending the treasure list of the player arch */
    give_initial_items(op, op->randomitems);

    /* some more sanity settings */
    SET_ANIMATION(op, 4 * (NUM_ANIMATIONS(op) / NUM_FACINGS(op))); /* So player faces south */
    FREE_AND_CLEAR_HASH2(op->msg);
    op->carrying = sum_weight(op);

    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
    /* this is more or less a fake flagging - ensure you delete it before object release */
    SET_FLAG(pl->ob, FLAG_FRIENDLY);

    pl->guild_updated = 1;

 return ADDME_MSG_OK;
}


/* tell the player something is wrong - and let him try another char
* we don't kill the socket here and we stay in the current socket mode
* mostly for another addme but also a new character is possible
*/
void player_addme_failed(NewSocket *ns, int error_msg)
{
    sockbuf_struct	*sbptr;

    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_SMALL);
    sbptr = ACTIVE_SOCKBUF(ns);
    SockBuf_AddChar(sbptr, error_msg);
    SOCKBUF_REQUEST_FINISH(ns, SERVER_CMD_ADDME_FAIL, SOCKBUF_DYNAMIC);
}
