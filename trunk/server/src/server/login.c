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


/* we save the current player status to file */
int player_save(object *op)
{
    FILE   *fp;
    char    filename[MAX_BUF], tmpfilename[MAXPATHLEN], backupfile[MAX_BUF]="";
    player *pl  = CONTR(op);
    int     have_file = TRUE, i, wiz = QUERY_FLAG(op, FLAG_WIZ);
    object *force;
    archetype *at = find_archetype("drain");
    int drain_level = 0;

#ifdef USE_CHANNELS
    struct  player_channel *pl_channel;
#endif

    /* Sanity check - some stuff changes this when player is exiting */
    if (op->type != PLAYER || pl == NULL)
        return 0;

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
        new_draw_info(NDI_UNIQUE, 0, op, "Can't open file for save.");
        LOG(llevDebug, "Can't open file for save (%s).\n", tmpfilename);
        return 0;
    }

    fprintf(fp, "account %s\n", pl->account_name);

    if(pl->gmaster_mode != GMASTER_MODE_NO)
    {
        if(pl->gmaster_mode == GMASTER_MODE_VOL)
            fprintf(fp, "dm_VOL\n");
        else if(pl->gmaster_mode == GMASTER_MODE_GM)
            fprintf(fp, "dm_GM\n");
        else
            fprintf(fp, "dm_DM\n");
    }
    if(pl->mute_counter > pticks)
        fprintf(fp, "mute %d\n", (int)(pl->mute_counter-pticks)); /* should be not THAT long */

    fprintf(fp, "dm_stealth %d\nsilent_login %d\np_ver %d\nlistening %d\npickup %d\nskill_group %d %d %d\n",
                 pl->dm_stealth, pl->silent_login, pl->p_ver,
                 pl->listening, pl->mode,
                 pl->base_skill_group[0],pl->base_skill_group[1],pl->base_skill_group[2]);

    /* Match the enumerations but in string form */
    fprintf(fp, "usekeys %s\n",
            pl->usekeys == key_inventory ? "key_inventory" : (pl->usekeys == keyrings ? "keyrings" : "containers"));

    if (op->map != NULL)
        set_mappath_by_map(op);

    fprintf(fp, "map %s\n", pl->maplevel);
    if(pl->maplevel != pl->orig_map)
        fprintf(fp, "o_map %s\n", pl->orig_map);

    fprintf(fp, "savebed_map %s\n", pl->savebed_map);
    if(pl->savebed_map != pl->orig_savebed_map)
        fprintf(fp, "o_bed %s\n", pl->orig_savebed_map);

    fprintf(fp, "map_s %d\nbed_s %d\nmap_x %d\nmap_y %d\nbed_x %d\nbed_y %d\n",
                MAP_STATUS_TYPE(pl->map_status), MAP_STATUS_TYPE(pl->bed_status), pl->map_x, pl->map_y, pl->bed_x, pl->bed_y);

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
        fprintf(fp,"%s %c %d\n",pl_channel->channel->name, pl_channel->shortcut, pl_channel->mute_counter);
    }
#endif

    /* check for drain so we know the proper level of the player to save the hp table with */
    if (!at)
        LOG(llevBug, "BUG: Couldn't find archetype drain.\n");
    else
    {
        force = present_arch_in_ob(at, op);
    
        if (force)
            drain_level = force->level;
    }

    /* save hp table */
    fprintf(fp, "lev_hp %d\n", op->level + drain_level);
    for (i = 1; i <= op->level + drain_level; i++)
        fprintf(fp, "%d\n", pl->levhp[i]);

    /* save sp table */
    fprintf(fp, "lev_sp %d\n", pl->exp_obj_ptr[SKILLGROUP_MAGIC]->level);
    for (i = 1; i <= pl->exp_obj_ptr[SKILLGROUP_MAGIC]->level; i++)
        fprintf(fp, "%d\n", pl->levsp[i]);

    fprintf(fp, "lev_grace %d\n", pl->exp_obj_ptr[SKILLGROUP_WISDOM]->level);
    for (i = 1; i <= pl->exp_obj_ptr[SKILLGROUP_WISDOM]->level; i++)
        fprintf(fp, "%d\n", pl->levgrace[i]);


    for (i = 0; i < pl->nrofknownspells; i++)
        fprintf(fp, "known_spell %s\n", spells[pl->known_spells[i]].name);
    fprintf(fp, "endplst\n");

    SET_FLAG(op, FLAG_NO_FIX_PLAYER);
    CLEAR_FLAG(op, FLAG_WIZ);

    save_object(fp, op, 3); /* don't check and don't remove */

    if (wiz)
        SET_FLAG(op, FLAG_WIZ);
    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);

    if (fclose(fp) == EOF)
    {
        /* make sure the write succeeded */
        new_draw_info(NDI_UNIQUE, 0, op, "Can't save character.");
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
        sprintf(filename, "%s.tmp.lu",tmpfilename, now.tv_sec);
        rename(tmpfilename, filename);
        unlink(tmpfilename); /* last sanity - kill this now 100% invalid tmpfilename */
    }

    if(have_file)
        unlink(backupfile); /* also 100% invalid now */
    chmod(filename, SAVE_MODE);

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
static void reorder_inventory(object *op)
{
    object *tmp, *tmp2;

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
static player *get_player_struct(void)
{
    player *p;
    int     i;

    p = (player *) get_poolchunk(pool_player);

    /* don't try any recovering here - oom means to leave ASAP */
    if (p == NULL)
        LOG(llevError, "ERROR: get_player(): out of memory\n");

    /* Initial value settings is zero ... */
    memset(p, 0, sizeof(player));

    /* ... but init some more stuff with non zero values or macros */
    p->group_id = GROUP_NO;

#ifdef AUTOSAVE
    p->last_save_tick = 9999999;
#endif

    p->gmaster_mode = GMASTER_MODE_NO;
    p->target_hp = -1;
    p->target_hp_p = -1;
    p->listening = 9;
    p->last_weapon_sp = -1;
    p->update_los = 1;

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
        p->exp_obj_ptr[i] = NULL;
        p->last_exp_obj_exp[i] = -1;
        p->last_exp_obj_level[i] = -1;
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
    player      *pl;
    object      *op;
    FILE        *fp;
    void        *mybuffer;
    char        filename[MAX_BUF];
    char        buf[MAX_BUF], bufall[MAX_BUF];
    int         i, value;
    int         correct = FALSE;
    time_t      elapsed_save_time   = 0;
    struct stat statbuf;
    object     *tmp, *tmp2;
    int         kick_loop = 0;

#ifdef USE_CHANNELS
    int     with_channels = FALSE;
#endif
#ifdef PLUGINS
    CFParm      CFP;
    int         evtid;
#endif

    sprintf(filename, "%s/%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, get_subdir(name), name, name);
    LOG(llevInfo, "PLAYER: %s\n", filename);

    while(kick_loop++ < 10)
    {
        correct = FALSE;

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
                LOG(llevBug, "\nBUG: Player file %s was saved in the future? (%d time)\n", filename, elapsed_save_time);
                elapsed_save_time = 0;
            }
        }

        /* the first line is always the password - so, we check it first. */
        if (fgets(bufall, MAX_BUF, fp) == NULL)
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

                correct = TRUE;
                /* lets check we have ghosting players - if so, kick them and retry */
                if (correct)
                {
                    char double_login_warning[] = "3 Double login! Kicking older instance!";
                    player *ptmp;

                    for (ptmp = first_player; ptmp != NULL; ptmp = ptmp->next)
                    {
                        if (ptmp->state == ST_PLAYING && ptmp->ob->name == name)
                        {
                            LOG(llevInfo, "Double login! Kicking older instance! (%d) ", kick_loop);
                            Write_String_To_Socket(ns, BINARY_CMD_DRAWINFO, double_login_warning, strlen(double_login_warning));
                            fclose(fp); /* we will rewrite the file when saving beyond! close it first */
                            player_save(ptmp->ob);
                            ptmp->state = ST_ZOMBIE;
                            ptmp->socket.status = Ns_Dead;
                            remove_ns_dead_player(ptmp);/* super hard kick! */
                            continue;
                        }
                    }
                }
            }

        } /* fgets account name */

        break; /* all fine - leave kick loop sanity */
    } /* while kickloop */

    /* sanity check for the kick loop */
    if (!correct)
    {
        LOG(llevBug, "KickLoop failed for name %s?!\n", name);
        fclose(fp);
        return ADDME_MSG_INTERNAL; /* addme_fail error with internal! */
    }
    
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
    while (fgets(bufall, MAX_BUF, fp) != NULL)
    {
        if (!strncmp(bufall, "skill_group ",12))
        {
            sscanf(bufall, "%s %d %d %d\n", buf, &pl->base_skill_group[0], &pl->base_skill_group[1], &pl->base_skill_group[2]);
            continue;
        }
        sscanf(bufall, "%s %d\n", buf, &value);
        if (!strcmp(buf, "endplst"))
            break;
        else if (!strcmp(buf, "dm_VOL"))
            pl->gmaster_mode = GMASTER_MODE_VOL;
        else if (!strcmp(buf, "dm_GM"))
            pl->gmaster_mode = GMASTER_MODE_GM;
        else if (!strcmp(buf, "dm_DM"))
            pl->gmaster_mode = GMASTER_MODE_DM;
        else if (!strcmp(buf, "mute"))
            pl->mute_counter = pticks+(unsigned long)value;
        else if (!strcmp(buf, "dm_stealth"))
            pl->dm_stealth = value;
        else if (!strcmp(buf, "silent_login"))
            pl->silent_login = value;
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
            pl->map_status = value;
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
#endif
        else if (!strcmp(buf, "usekeys"))
        {
            if (!strcmp(bufall + 8, "key_inventory\n"))
                pl->usekeys = key_inventory;
            else if (!strcmp(bufall + 8, "keyrings\n"))
                pl->usekeys = keyrings;
            else if (!strcmp(bufall + 8, "containers\n"))
                pl->usekeys = containers;
            else
                LOG(llevDebug, "load_player: got unknown usekeys type: %s\n", bufall + 8);
        }
#ifdef USE_CHANNELS
        else if (!strcmp(buf,"channels"))
        {
            char channelname[MAX_CHANNEL_NAME+1];
            char shortcut;
            unsigned long mute=0;
            with_channels=TRUE;
            for (i=1; i<= value; i++)
            {
                fscanf(fp,"%s %c %d\n",channelname,&shortcut, &mute);
                /* lets only store the channels first - lets do the login AFTER we closed the stream */
              /*  loginAddPlayerToChannel(pl, channelname, shortcut, mute);*/
            }
        }
#endif
        else if (!strcmp(buf, "lev_hp"))
        {
            int j;
            for (i = 1; i <= value; i++)
            {
                fscanf(fp, "%d\n", &j);
                pl->levhp[i] = j;
            }
        }
        else if (!strcmp(buf, "lev_sp"))
        {
            int j;
            for (i = 1; i <= value; i++)
            {
                fscanf(fp, "%d\n", &j);
                pl->levsp[i] = j;
            }
        }
        else if (!strcmp(buf, "lev_grace"))
        {
            int j;
            for (i = 1; i <= value; i++)
            {
                fscanf(fp, "%d\n", &j);
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

    /* do some sanity checks... if something looks bad force the defaults */
    if(!pl->orig_map || !pl->maplevel)
        set_mappath_by_default(pl);

    if(!pl->orig_savebed_map || !pl->savebed_map)
        set_bindpath_by_default(pl);

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
        LOG(llevBug, "BUG: Player %s was loaded without friendly flag!", query_name(op));
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

     link_player_skills(op); /* link all exp group & skill objects to the player */

     /*
    if (is_dragon_pl(op) && op->inv != NULL)
    {
        object *tmp, *abil = NULL, *skin = NULL;
        for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
        {
            if (tmp->type == FORCE)
            {
                if (tmp->arch->name == shstr_cons.dragon_ability_force)
                    abil = tmp;
                else if (tmp->arch->name == shstr_cons.dragon_skin_force)
                    skin = tmp;
            }
        }
        set_dragon_name(op, abil, skin);
    }
    */

    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
    FIX_PLAYER(op ,"check login - first fix");

#ifdef AUTOSAVE
    pl->last_save_tick = ROUND_TAG;
#endif


    /* we hook in here perm dead later - ATM we don't allow a player loaded which was dead */
    if (op->stats.hp <= 0)
    {
        if (op->stats.hp <= 0)
            op->stats.hp = 1;
    }

    /* lets check we had saved last time in a gmaster mode.
     * if so, check the setting is still allowed and if so,
     * set the player to it.
     */
    if(pl->gmaster_mode != GMASTER_MODE_NO)
    {
        int mode = pl->gmaster_mode;

        pl->gmaster_mode = GMASTER_MODE_NO;
        if(check_gmaster_list(pl, mode))
            set_gmaster_mode(pl, mode);
    }

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
    pl->socket.below_clear = 0;
    pl->socket.update_tile = 0;
    pl->socket.look_position = 0;
    pl->socket.update_tile = 0;
    pl->socket.look_position = 0;
    pl->socket.look_position_container = 0;
    pl->socket.ext_title_flag = 1;

    /* mark socket, player & connection as playing and add charcter to player queue */
    pl->socket.status = Ns_Playing;
    pl->state = ST_PLAYING;

    player_active++;
    if(player_active_meta < player_active)
        player_active_meta = player_active;

    if (!last_player)
        first_player = last_player = pl;
    else
    {
        last_player->next = pl;
        pl->prev = last_player;
        last_player = pl;
    }

    /* This command will tell the client to go in playing mode and wait for server game data */
    esrv_new_player(pl, op->weight + op->carrying);
    LOG(llevDebug, "Send new_player(): socket %d\n", ns->fd);

    esrv_send_inventory(op, op);
    send_spelllist_cmd(op, NULL, SPLIST_MODE_ADD); /* send the known spells as list to client */
    send_skilllist_cmd(op, NULL, SPLIST_MODE_ADD);

    /* we do the login script BEFORE we go to the map */
#ifdef PLUGINS
    /* GROS : Here we handle the BORN global event */
    evtid = EVENT_BORN;
    CFP.Value[0] = (void *) (&evtid);
    CFP.Value[1] = (void *) (op);
    GlobalEvent(&CFP);

    /* GROS : Here we handle the LOGIN global event */
    evtid = EVENT_LOGIN;
    CFP.Value[0] = (void *) (&evtid);
    CFP.Value[1] = (void *) (pl);
    CFP.Value[2] = (void *) (pl->socket.ip_host);
    GlobalEvent(&CFP);
#endif

#ifdef USE_CHANNELS
    /* channel-system: we check if the playerfile has the channels tag */
    /* if not, add all the default channels */
    if (!with_channels)
        addDefaultChannels(pl);
#endif

    /* and finally the player appears on the map */
    enter_map_by_name(op, pl->maplevel, pl->orig_map, pl->map_x, pl->map_y, pl->map_status);

    /* announce the login */
    if (!pl->dm_stealth)
    {
        if (!pl->silent_login) /* Inform all players of the login */
            new_draw_info_format(NDI_UNIQUE | NDI_ALL, 5, NULL, "%s has entered the game.", query_name(pl->ob));
        else    /* Inform only the DMs, GMs and VOLs that the player logged in */
        {
            char buf[MAX_BUF];
            objectlink *ol;

            sprintf(buf, "%s has entered the game.", query_name(pl->ob));

            for (ol = gmaster_list_DM; ol; ol = ol->next)
                new_draw_info(NDI_UNIQUE, 5, ol->objlink.ob, buf);
            for (ol = gmaster_list_GM; ol; ol = ol->next)
                new_draw_info(NDI_UNIQUE, 5, ol->objlink.ob, buf);
            for (ol = gmaster_list_VOL; ol; ol = ol->next)
                new_draw_info(NDI_UNIQUE, 5, ol->objlink.ob, buf);
        }
        if(gmaster_list_DM || gmaster_list_GM)
        {
            objectlink *ol;
            char buf_dm[64];

            sprintf(buf_dm, "DM: %d players now playing.", player_active);

            for(ol = gmaster_list_DM;ol;ol=ol->next)
                new_draw_info(NDI_UNIQUE, 0,ol->objlink.ob, buf_dm);

            for(ol = gmaster_list_GM;ol;ol=ol->next)
                new_draw_info(NDI_UNIQUE, 0,ol->objlink.ob, buf_dm);
        }
    }

    return ADDME_MSG_OK;
}

/* we create player >name< with a template player arch object and some setup values
 * It will return at success in pl_ret a pointer to that pl objectz.
 * That object is ready to save but NOT ready for playing.
 * We don't have set the friendly list, the socket and others.
 * If we fail, no player struct will be returned and it returns
 * with an error msg.
 */
addme_login_msg player_create(NewSocket *ns, player **pl_ret, char *name, int race, int gender, int skill_nr)
{
    player          *pl = NULL;
    object          *op = NULL;
    archetype       *p_arch;
    int             skillnr[]       = {SK_SLASH_WEAP, SK_MELEE_WEAPON, SK_CLEAVE_WEAP, SK_PIERCE_WEAP};
    char            *skillitem[]     = {"shortsword", "mstar_small", "axe_small", "dagger_large"};

    /* do some sanity checks. *ns, *pl and the name are checked by caller */ 

    /* race & gender values in range? This values MUST fit or client is hacked or damaged */
    if(race < 0 || race >= settings.player_races || gender < 0 || gender >= MAX_RACE_GENDER)
        return ADDME_MSG_DISCONNECT;

    /* check we have a valid player race object arch. MUST fit too */ 
    if(!(p_arch = player_arch_list[race].p_arch[gender]))
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
    pl->state = ST_BORN;
    *pl_ret = pl;

#ifdef AUTOSAVE
    pl->last_save_tick = ROUND_TAG;
#endif

    /* now we add our custom settings for this new character */
    FREE_AND_ADD_REF_HASH(pl->account_name, ns->pl_account.name);
    FREE_AND_COPY_HASH(op->name, name);
    pl->orig_stats.Str = op->stats.Str = (sint8) player_arch_list[race].str;
    pl->orig_stats.Dex = op->stats.Dex = (sint8) player_arch_list[race].dex;
    pl->orig_stats.Con = op->stats.Con = (sint8) player_arch_list[race].con;
    pl->orig_stats.Int = op->stats.Int = (sint8) player_arch_list[race].intel;
    pl->orig_stats.Wis = op->stats.Wis = (sint8) player_arch_list[race].wis;
    pl->orig_stats.Pow = op->stats.Pow = (sint8) player_arch_list[race].pow;
    pl->orig_stats.Cha = op->stats.Cha = (sint8) player_arch_list[race].cha;

    /* setup start point and default maps */
    set_mappath_by_default(pl);
    set_bindpath_by_default(pl);
    op->x = pl->map_x;
    op->y = pl->map_y;

    /* setup the base object structure, which are usually invisible objects in the inventory */
    link_player_skills(op);
    learn_skill(op, NULL, NULL, skillnr[skill_nr], 0);

    /* and here the initial stuff depending the treasure list of the player arch */
    give_initial_items(op, op->randomitems);

    /* some more sanity settings */
    SET_ANIMATION(op, 4 * (NUM_ANIMATIONS(op) / NUM_FACINGS(op))); /* So player faces south */
    CLEAR_FLAG(op, FLAG_WIZ);
    FREE_AND_CLEAR_HASH2(op->msg);
    op->carrying = sum_weight(op); 

    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
    /* this is more or less a fake flagging - ensure you delete it before object release */
    SET_FLAG(pl->ob, FLAG_FRIENDLY);

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
    SOCKBUF_REQUEST_FINISH(ns, BINARY_CMD_ADDME_FAIL, SOCKBUF_DYNAMIC);
}
