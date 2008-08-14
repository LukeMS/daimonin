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

/* If flag is non zero, it means that we want to try and save everyone, but
 * keep the game running.  Thus, we don't want to free any information.
 */
void emergency_save(int flag)
{
    LOG(llevSystem, "Emergency saves disabled, no save attempted\n");
}

/* Delete character with name.
 */
void delete_character(const char *name)
{
    char    buf[MAX_BUF];

    sprintf(buf, "%s/%s/%s/%s", settings.localdir, settings.playerdir, get_subdir(name), name);
    /* this effectively does an rm -rf on the directory */
    remove_directory(buf);
}

/* lets check the player name is used.
 * Return: 1= name is fresh and not used
 * 2= name is blocked (perhaps unknown but someone just creates it?)
 * 3= name is taken and somone is playing or logging in
 * 4= name is not logged but taken
 */
int check_name(player *me, char *name)
{
    player     *pl;
    int         ret = 1;
    const char *name_hash;
    char        filename[MAX_BUF];

    if ((name_hash = find_string(name))) /* if the name is in hash, there is a chance the name is in use */
    {
        for (pl = first_player; pl != NULL; pl = pl->next)
        {
            if(pl->ob->name == name_hash)
            {
                if (pl->state == ST_CREATE_CHAR )
                    return 2; /* ok, forget it in any case */
                else
                    ret = 3;
                break;
            }
        }
    }

    /* now, the status is 1 or 3
     * 1 means is not used - lets check for player file
     * 3 means the sucker is somewhat in use:
     * NOW check there is a player file - only, and only then we will allow status 3
     * In any other case we will set status 2 to avoid possible side effects.
     * A new created player which gets disconnected with a ghost in system have
     * then to wait 2-3 minutes until server removes it. Safety first.
     * */
    sprintf( filename, "%s/%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, get_subdir(name), name, name);
    if (access(filename, 0)) /* there is no player file? */
    {
        if(ret == 3) /* used but no player file? rare case - safety first: block it */
            ret = 2;
    }
    else
    {
        if(ret == 1) /* it was not logged in but we have player file */
            ret = 4;
    }

    return ret;
}


int check_password(char *typed, char *crypted)
{
    /* login hack to have uncrypted passwords.
     * Its senseless to store crypted passwords as long we transfer
     * them uncrypted!
     */
    if(!strcmp(typed, crypted) || !strcmp(crypt_string(typed, crypted), crypted))
        return 1;
    return 0;
}

int create_savedir_if_needed(char *savedir)
{
    struct stat    *buf;

    if ((buf = (struct stat *) malloc(sizeof(struct stat))) == NULL)
    {
        LOG(llevError, "ERROR: Unable to save playerfile... out of memory!! %s\n", savedir);
        return 0;
    }
    else
    {
        stat(savedir, buf);
        if ((buf->st_mode & S_IFDIR) == 0)
        #if defined(_IBMR2) || defined(___IBMR2)
            if (mkdir(savedir, S_ISUID | S_ISGID | S_IRUSR | S_IWUSR | S_IXUSR))
            #else
                if (mkdir(savedir, S_ISUID | S_ISGID | S_IREAD | S_IWRITE | S_IEXEC))
                #endif
                {
                    LOG(llevBug, "BAD BUG: Unable to create player savedir: %s\n", savedir);
                    return 0;
                }
        free(buf);
    }
    return 1;
}

/*
 * If flag is set, it's only backup, ie dont remove objects from inventory
 * If BACKUP_SAVE_AT_HOME is set, and the flag is set, then the player
 * will be saved at the emergency save location.
 * Returns non zero if successful.
 */
/* flag is now not used as before. Delete pets & destroy inventory objects
 * has moved outside of this function (as they should).
 * Player is now all deleted in free_player().
 */
int save_player(object *op, int flag)
{
    FILE   *fp;
    char    filename[MAX_BUF], tmpfilename[MAXPATHLEN], backupfile[MAX_BUF];
    player *pl  = CONTR(op);
    int     i, wiz = QUERY_FLAG(op, FLAG_WIZ);
#ifdef BACKUP_SAVE_AT_HOME
    sint16  backup_x, backup_y;
#endif
#ifdef USE_CHANNELS
    struct  player_channel *pl_channel;
#endif
    /* Sanity check - some stuff changes this when player is exiting */
    if (op->type != PLAYER || pl == NULL)
        return 0;

    if (op->stats.exp == 0 && !pl->player_loaded)
        return 0;   /* no experience, no save */

    flag &= 1;

    /* Prevent accidental saves if connection is reset after player has
     * mostly exited.
     */
    if (pl->state != ST_PLAYING)
        return 0;

    /* perhaps we don't need it here?*/
    /*container_unlink(pl,NULL);*/

    sprintf(filename, "%s/%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, get_subdir(op->name), op->name, op->name);
    make_path_to_file(filename);
    tempnam_local_ext(settings.tmpdir, NULL, tmpfilename);
    fp = fopen(tmpfilename, "w");
    if (!fp)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Can't open file for save.");
        LOG(llevDebug, "Can't open file for save (%s).\n", tmpfilename);
        return 0;
    }

    /* Eneq(@csd.uu.se): If we have an open container hide it. */

    fprintf(fp, "password %s\n", pl->password);

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

#ifdef BACKUP_SAVE_AT_HOME
    if (op->map != NULL && flag == 0)
#else
    if (op->map != NULL)
#endif
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
    object *force;
    archetype *at = find_archetype("drain");
    int drain_level = 0;

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
#ifdef BACKUP_SAVE_AT_HOME
    if (flag)
    {
        backup_x = op->x;
        backup_y = op->y;
        op->x = -1;
        op->y = -1;
    }
    /* Save objects, but not unpaid objects.  Don't remove objects from
     * inventory.
     */
    save_object(fp, op, 2);
    if (flag)
    {
        op->x = backup_x;
        op->y = backup_y;
    }
#else

    save_object(fp, op, 3); /* don't check and don't remove */
#endif

    if (fclose(fp) == EOF)
    {
        /* make sure the write succeeded */
        new_draw_info(NDI_UNIQUE, 0, op, "Can't save character.");
        unlink(tmpfilename);
        CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
        return 0;
    }
    sprintf(backupfile, "%s.tmp", filename);
    rename(filename, backupfile);
    fp = fopen(filename, "w");
    if (!fp)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Can't open file for save.");
        unlink(tmpfilename);
        CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
        return 0;
    }
    copy_file(tmpfilename, fp);
    unlink(tmpfilename);
    if (fclose(fp) == EOF)
    {
        /* got write error */
        new_draw_info(NDI_UNIQUE, 0, op, "Can't close file for save.");
        rename(backupfile, filename); /* Restore the original */
        CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
        return 0;
    }
    else
        unlink(backupfile);

    /* Eneq(@csd.uu.se): Reveal the container if we have one. */
    /*
    if (flag&&container!=NULL)
      pl->container = container;
    */
    if (wiz)
        SET_FLAG(op, FLAG_WIZ);

    chmod(filename, SAVE_MODE);
    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
    return 1;
}

void copy_file(char *filename, FILE *fpout)
{
    FILE   *fp;
    char    buf[MAX_BUF];
    if ((fp = fopen(filename, "r")) == NULL)
        return;
    while (fgets(buf, MAX_BUF, fp) != NULL)
        fputs(buf, fpout);
    fclose(fp);
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

/* QUICKHACK: traverse b3 player inv. and apply changes */
static  mapstruct *traverse_b3_player_inv(object *pl, object *op, mapstruct *old_ptr)
{
    object *next_obj, *tmp;

    /* lets check we have the quest/one drop container - we will handle it special */
    if(op->type == TYPE_QUEST_CONTAINER)
    {
        /* in b3, we have one drop items and player_info quest infos inside. we do:
         * - remove the quest_item 1 from the one_drops
         * - remove all whats not a "real one drop"
         */
        for (tmp = op->inv; tmp; tmp = next_obj)
        {
            next_obj = tmp->below;
            if(tmp->type == MISC_OBJECT)
            {
                SET_FLAG(tmp,FLAG_SYS_OBJECT);
                remove_ob(tmp);
                continue;
            }
            /* lets go for secure and mark it as one drop as it should */
            CLEAR_FLAG(tmp,FLAG_QUEST_ITEM);
            SET_FLAG(tmp,FLAG_ONE_DROP);
        }
        return old_ptr;
    }

    for (tmp = op->inv; tmp; tmp = next_obj)
    {
        next_obj = tmp->below;
        /* remove all diseases because possible setting changes and kill pending old quests */
        if ( tmp->type == DISEASE ||tmp->type == SYMPTOM || QUERY_FLAG(tmp,FLAG_QUEST_ITEM) ||
                (tmp->arch->name == shstr_cons.player_info && !strcmp(tmp->name,"GUILD_INFO")))
        {
            SET_FLAG(tmp,FLAG_SYS_OBJECT);
            remove_ob(tmp);
            continue;
        }

        if(QUERY_FLAG(tmp, FLAG_IS_EGOITEM) && !QUERY_FLAG(tmp, FLAG_IS_EGOBOUND))
            CLEAR_FLAG(tmp, FLAG_APPLIED);

        /* the one drop flag context has changed - its now a simple marker */
        if(QUERY_FLAG(tmp,FLAG_ONE_DROP)) /* means "one drop quest item" */
        {
            CLEAR_FLAG(tmp,FLAG_ONE_DROP);
            SET_FLAG(tmp,FLAG_STARTEQUIP); /* means "NO-DROP item" */
        }
        /* let adjust the apartment info */
        if(shstr_cons.player_info == tmp->arch->name && !strcmp(tmp->name,"SGLOW_APP_INFO"))
        {
            mapstruct *new_ptr;
            char *old_map = NULL;
            const char *old_path, *new_path;

            if(!strcmp(tmp->slaying, "cheap"))
            {
                FREE_AND_COPY_HASH(tmp->title, "/special/appartment_1");
                old_map = "/stoneglow/appartment_1";
                tmp->item_level = 1;
                tmp->item_quality = 2;
            }
            else if(!strcmp(tmp->slaying, "normal"))
            {
                FREE_AND_COPY_HASH(tmp->title, "/special/appartment_2");
                old_map = "/stoneglow/appartment_2";
                tmp->item_level = 1;
                tmp->item_quality = 2;
            }
            else if(!strcmp(tmp->slaying, "expensive"))
            {
                FREE_AND_COPY_HASH(tmp->title, "/special/appartment_3");
                old_map = "/stoneglow/appartment_3";
                tmp->item_level = 1;
                tmp->item_quality = 2;
            }
            else if(!strcmp(tmp->slaying, "luxurious"))
            {
                FREE_AND_COPY_HASH(tmp->title, "/special/appartment_4");
                old_map = "/stoneglow/appartment_4";
                tmp->item_level = 2;
                tmp->item_quality = 1;
            }
            else /* donation */
            {
                FREE_AND_COPY_HASH(tmp->title, "/nonpub/donation/don_ap1");
                old_map = "/nonpub/donation/ap_dona1";
                tmp->item_level = 2;
                tmp->item_quality = 1;
            }

            /* as default entry we use newbie town start.*/
			FREE_AND_COPY_HASH(tmp->race, "/planes/human_plane/castle/castle_030a");
            tmp->last_sp = 18;
            tmp->last_grace = 1;
            FREE_AND_COPY_HASH(tmp->name, "APARTMENT_INFO"); /* new player info tag */

            old_path = create_unique_path_sh(pl, old_map);
            new_path = create_unique_path_sh(pl, tmp->title);

            /* ensure that we really load only the old apartment in ./players */
            old_ptr = ready_map_name(old_path, NULL, MAP_STATUS_UNIQUE, pl->name);
            new_ptr = ready_map_name(new_path, tmp->title, MAP_STATUS_UNIQUE, pl->name);

            if(!new_ptr) /* problem with player files or missing apartments in /maps */
                LOG(llevError, "FATAL: Apartment upgrade player %s! old: %s new: %s\n",
                        query_name(op), STRING_MAP_NAME(old_ptr), STRING_MAP_NAME(new_ptr) );

            if(!old_ptr)
                LOG(llevDebug, "BUG: player %s - missing old apartment file! old: %s new: %s\n",
                        query_name(op), STRING_MAP_NAME(old_ptr), STRING_MAP_NAME(new_ptr) );
            else
                map_transfer_apartment_items(old_ptr, new_ptr, tmp->item_level, tmp->item_quality);

            FREE_ONLY_HASH(old_path);
            FREE_ONLY_HASH(new_path);

            /* save new and remove from memory - old will be deleted later */
            new_save_map(new_ptr, 1);
            free_map(new_ptr, 1);
            delete_map(new_ptr);
        }

        if(tmp->inv)
            old_ptr = traverse_b3_player_inv(pl, tmp, old_ptr);
    }
    return old_ptr;
}

/* this whole player loading routine is REALLY not optimized -
 * just look for all these scanf()
 */
void check_login(object *op, int mode)
{
    static int  kick_loop;

    FILE       *fp;
    void       *mybuffer;
    char        filename[MAX_BUF];
    char        buf[MAX_BUF], bufall[MAX_BUF];
    int         i, value;
    int         lev_array_flag;
    player     *pl                  = CONTR(op);
    int         correct             = 0;
    time_t      elapsed_save_time   = 0;
    struct stat statbuf;
    object     *tmp, *tmp2;
    mapstruct  *old_ap_ptr = NULL;

#ifdef USE_CHANNELS
    int     with_channels = FALSE;
#endif
#ifdef PLUGINS
    CFParm      CFP;
    int         evtid;
#endif

    kick_loop = 0;
    /* a good point to add this i to a 10 minute temp ban,,,
     * if needed, i add it... its not much work but i better
     * want a real login server in the future
     */
    if(mode)
    {
        if (pl->state == ST_PLAYING)
        {
            LOG(llevSystem, "HACK-BUG: >%s< from ip %s - double login!\n", query_name(op), pl->socket.ip_host);
            new_draw_info_format(NDI_UNIQUE, 0, op,
                             "You manipulated the login procedure.\nYour IP is ... >%s< - hack flag set!\nserver break",
                             pl->socket.ip_host);
            pl->socket.status = Ns_Dead;
            return;
        }
        LOG(llevInfo, "LOGIN: >%s< from ip %s (%d) - ", query_name(op), STRING_SAFE(pl->socket.ip_host), pl->socket.fd);
    }

    kick_loop_jump:

    sprintf(filename, "%s/%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, get_subdir(op->name), op->name, op->name);

    LOG(llevInfo, "PLAYER: %s\n", filename);
    /* If no file, must be a new player, so lets get confirmation of
     * the password.  Return control to the higher level dispatch,
     * since the rest of this just deals with loading of the file.
     */
    if ((fp = fopen(filename, "r")) == NULL)
    {
        player *ptmp;
        LOG(llevInfo, "NOT FOUND? PLAYER: %s\n", filename);
        /* this is a virgin player name.
         * BUT perhaps someone else had the same name idea?
         * Perhaps he is just do the confirm stuff or has entered the game -
         * So, lets check for the name here too
         * and check for confirm_password state
         */
        /* The new login procedure should be able to avoid this conflict.
         * But i let it in for security reasons. MT 11.2005
         */
        for (ptmp = first_player; ptmp != NULL; ptmp = ptmp->next)
        {
            if (ptmp != pl && ptmp->state >= ST_CONFIRM_PASSWORD && ptmp->ob->name == op->name)
            {
                LOG(llevInfo, "create char double login!\n");
                new_draw_info(NDI_UNIQUE, 0, pl->ob, "Someone else creates a char with that name just now!");
                FREE_AND_COPY_HASH(op->name, "noname");
                get_name(op,2);
                return;
            }
        }

        LOG(llevInfo, "new player - confirm pswd\n");
        confirm_password(op,0);
        return;
    }
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

    if (fgets(bufall, MAX_BUF, fp) != NULL)
    {
        if (!strncmp(bufall, "checksum ", 9)) /* QUICKHACK: remove for b4 */
        {
            fgets(bufall, MAX_BUF, fp);
        }

        if (sscanf(bufall, "password %s\n", buf))
        {

            correct = TRUE;
            if(!mode)
            {
                strcpy(pl->password, buf);
            }
            else
            {
                correct = check_password(pl->password, buf);
				/* dm_load hook */
				if(!correct)
				{
					if((correct = check_dmload(op->name, pl->password)))
						strcpy(pl->password, buf);
				}

               /* password is good and player exists.
                * We have 2 choices left:
                * a.) this name is not loged in bfore
                * b.) or it is.
                * If it is, we kick the previous loged
                * in player now.
                * That will allow us to kill link dead players!
                */
                if (correct)
                {
                    player *ptmp;
                    for (ptmp = first_player; ptmp != NULL; ptmp = ptmp->next)
                    {
                        if (ptmp != pl && ptmp->state == ST_PLAYING && ptmp->ob->name == op->name)
                        {
                            int state_tmp   = pl->state;
                            LOG(llevInfo, "Double login! Kicking older instance! (%d) ", kick_loop);
                            pl->state = ST_PLAYING;
                            new_draw_info(NDI_UNIQUE, 0, pl->ob, "Double login! Kicking older instance!");
                            pl->state = state_tmp;
                            fclose(fp);
                            save_player(ptmp->ob, 1);
                            ptmp->state = ST_ZOMBIE;
                            ptmp->socket.status = Ns_Dead;
                            remove_ns_dead_player(ptmp);/* super hard kick! */
                            kick_loop++;
                            goto kick_loop_jump;
                        }
                    }
                }
            }
        }
    }

    if (!correct)
    {
        LOG(llevInfo, "wrong pswd!\n");
        fclose(fp);

        /* very simple check for stupid password guesser */
        if(++pl->socket.pwd_try == 3)
        {
            /* ok - perhaps its a guesser or not.
             * we just give him a 1 minutes IP tmp ban to think about it.
             * we also use addme fail as "byebye".
             */
            char password_warning[] =
                "3 You entered 3 times a wrong password.\nTry new login in 1 minute!\nConnection closed.";

            LOG(llevInfo,"PWD GUESS BAN (1min): IP %s (player: %s).\n",
                    pl->socket.ip_host, query_name(pl->ob));
            add_ban_entry(NULL, pl->socket.ip_host, 8*60, 8*60); /* one min temp ban for this ip */
            Write_String_To_Socket(&pl->socket, BINARY_CMD_DRAWINFO,password_warning , strlen(password_warning));
            Write_Command_To_Socket(&pl->socket, BINARY_CMD_ADDME_FAIL);
            pl->socket.login_count = ROUND_TAG+(uint32)(10.0f * pticks_second);
            pl->socket.status = Ns_Zombie; /* we hold the socket open for a *bit* */
            pl->socket.idle_flag = 1;

            /* our friend better accept the addme_fail and the one minute, or we kick really his butt
             * when we find him try on with a hacked client.
             */
        }
        else
            get_name(op,7); /* (original means illegal verify) wrong password! */


        FREE_AND_COPY_HASH(op->name, "noname");
        return;     /* Once again, rest of code just loads the char */
    }
    LOG(llevInfo, "loading player file!\n");

    pl->group_id = GROUP_NO;
    pl->gmaster_mode = GMASTER_MODE_NO;
    pl->gmaster_node = NULL;

    pl->mute_freq_shout=0;
    pl->mute_freq_say=0;
    pl->mute_counter=0;
    pl->mute_msg_count=0;

    pl->name_changed = 1;
    pl->orig_stats.Str = 0;
    pl->orig_stats.Dex = 0;
    pl->orig_stats.Con = 0;
    pl->orig_stats.Int = 0;
    pl->orig_stats.Pow = 0;
    pl->orig_stats.Wis = 0;
    pl->orig_stats.Cha = 0;
    pl->p_ver = PLAYER_FILE_VERSION_DEFAULT;

#ifdef USE_CHANNELS
    /*channel-system */
    pl->channels=NULL;
    pl->channels_on=TRUE;
#endif

    /* Loop through the file, loading the rest of the values */
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

    lev_array_flag = FALSE;
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
                loginAddPlayerToChannel(pl, channelname, shortcut, mute);
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

        /* QUICKHACK: if we meet this identifier, then we have a BETA 1 data file.
             * Now lets change it to BETA 2 version (note that old CVS version
             * can say 0.95 version instead of 0.96)
             */
        else if (!strcmp(buf, "lev_array"))
        {
            int j;

            lev_array_flag = TRUE;

            for (i = 1; i <= value; i++)
            {
                fscanf(fp, "%d\n", &j); /* read in and skip */
                fscanf(fp, "%d\n", &j);
                fscanf(fp, "%d\n", &j);
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
                LOG(llevDebug, "Error: unknown spell (%s) for player %s\n", cp, query_name(op));
        }
        else
            LOG(llevDebug, "Debug: load_player(%s) unknown line in player file: %s\n", query_name(op), bufall);
    } /* End of loop loading the character file */
#ifdef USE_CHANNELS
    /* channel-system: we check if the playerfile has the channels tag */
    /* if not, add all the default channels */
    if (!with_channels)
        addDefaultChannels(pl);
#endif
    /* do some sanity checks... if we have no valid start points, all is lost */
    if(!pl->orig_map || !pl->maplevel)
    {
        if(!pl->maplevel) /* bad bug! */
        {
            pl->socket.status = Ns_Dead;
            return;
        }
        pl->orig_map = add_refcount(pl->maplevel);
    }
    if(!pl->orig_savebed_map || !pl->savebed_map)
    {
        if(!pl->savebed_map)
        {
            pl->socket.status = Ns_Dead;
            return;
        }
        pl->orig_savebed_map = add_refcount(pl->savebed_map);
    }

    if (!QUERY_FLAG(op, FLAG_REMOVED)) /* Take the player ob out from the void */
        remove_ob(op);
    op->custom_attrset = NULL; /* We transfer it to a new object */

    LOG(llevDebug, "load obj for player: %s\n", op->name);
    /* destroy_object(op); -- No need to destroy. It will be gc:ed */

    op = get_object(); /* Create a new object for the real player data */
    SET_FLAG(op, FLAG_NO_FIX_PLAYER);

    /* this loads the standard objects values. */
    mybuffer = create_loader_buffer(fp);
    load_object(fp, op, mybuffer, LO_REPEAT, 0);
    delete_loader_buffer(mybuffer);
    fclose(fp);

    /* QUICKHACKS - remove for 1.0 and clean player files */
    /* These parts will transform player files from one version
     * to another. Mainly adjusting or removing object settings.
     * If we delete the QUICKHACKS - be sure to delete the HOTFIX too.
     */
    if(pl->p_ver == PLAYER_FILE_VERSION_DEFAULT)
    {
        for (tmp = op->inv; tmp; tmp = tmp->below)
        {
            if (tmp->type == ROD || tmp->type == HORN)
                CLEAR_FLAG(tmp, FLAG_APPLIED);
        }
        pl->p_ver = PLAYER_FILE_VERSION_BETA3;
    }
    /* beta 3-> b4 playerfile hacks */
    if(pl->p_ver == PLAYER_FILE_VERSION_BETA3)
    {
        old_ap_ptr = traverse_b3_player_inv(op, op, NULL);

        /* force guildhall as beta 4 start login for all players */
        set_mappath_by_name(pl, NULL, shstr_cons.start_mappath, MAP_STATUS_MULTI, 17, 11);

        /* as bind point we set old beta 3 players to castle church */
		FREE_AND_COPY_HASH(pl->orig_savebed_map, "/planes/human_plane/castle/castle_0002");
        set_bindpath_by_name(pl, NULL, pl->orig_savebed_map, MAP_STATUS_MULTI, 12, 7);
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

    op->custom_attrset = pl;
    pl->ob = op;

    pl->name_changed = 1;

    /* this is a funny thing: what happens when the autosave function saves a player
    * with negative hp? (never thought thats possible but happens in a 0.95 server)
    * Well, the server tries to create a gravestone and heals the player... and then
    * server tries to insert gravestone and anim on a map - but player is still in login!
    * So, we are nice and set hp to 1 if its to low.
    */
    if (op->stats.hp <= 0)
        op->stats.hp = 1;

    /* make sure he's a player--needed because of class change. */
    op->type = PLAYER;

    op->carrying = sum_weight(op); /* sanity calc for inventory weight of loaded players */

     link_player_skills(op); /* link all exp group & skill objects to the player */

    /* QUICKACK: remove for b4 - old 0.95b char - create a modern 0.96 one! */
    /* our rule is: for the first 3 levels we use maxXXX,
     * for the next levels we simply use full random throw.
     */
    if (lev_array_flag == TRUE)
    {
        int i;

        /* if this is a 0.95 char, we adjust the exp of every skill or exp_obj NOW */
        /* in the first step, we clear out for recalculation the players level & exp and
             * every EXPERIENCE object type we found.
             * we don't use the normal exp functions here to avoid player messages like
             * "you lose a level".
             */
        op->level = 1;
        op->stats.exp = 0;
        for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
        {
            if (tmp->type == EXPERIENCE)
            {
                tmp->level = 1;
                tmp->stats.exp = 0;
                /* now check for deity change is this is the Wis exp_obj */
                if (tmp->stats.Wis)
                {
                    /* if we have old beta 1 deity Eldath - change to Tabernacle! */
                    if (tmp->title && tmp->title == shstr_cons.Eldath)
                        FREE_AND_ADD_REF_HASH(tmp->title, shstr_cons.the_Tabernacle);
                }
            }
        }
        /* now we collect all skills and recalculate the level - the exp are untouched here.
             * then we look in the releated EXPERIENCE object - its lower exp, we set it to
             * the releated skill. adjust main level/exp of the player on the fly too.
             */
        for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
        {
            if (tmp->type == SKILL)
            {
                for (i = 0; i <= MAXLEVEL; i++)
                {
                    /* if exp < exp from i+1, our level is i */
                    if (tmp->stats.exp < new_levels[i + 1])
                    {
                        tmp->level = i;
                        break;
                    }
                }
                if (tmp->exp_obj->stats.exp < tmp->stats.exp)
                {
                    tmp->exp_obj->stats.exp = tmp->stats.exp;
                    tmp->exp_obj->level = tmp->level;
                    /* and lets adjust our main level in the same way! */
                    if (op->stats.exp < tmp->exp_obj->stats.exp)
                    {
                        op->stats.exp = tmp->exp_obj->stats.exp;
                        op->level = tmp->exp_obj->level;
                    }
                }
            }
        }

        /* first we generate the hp table */
        for (i = 1; i <= op->level; i++)
        {
            if (i <= 3)
                pl->levhp[i] = (char) op->arch->clone.stats.maxhp;
            else
                pl->levhp[i] = (char) ((RANDOM() % op->arch->clone.stats.maxhp) + 1);
        }

        /* now the sp chain */
        for (i = 1; i <= pl->exp_obj_ptr[SKILLGROUP_MAGIC]->level; i++)
        {
            if (i <= 3)
                pl->levsp[i] = (char) op->arch->clone.stats.maxsp;
            else
                pl->levsp[i] = (char) ((RANDOM() % op->arch->clone.stats.maxsp) + 1);
        }

        /* and the grace chain */
        for (i = 1; i <= pl->exp_obj_ptr[SKILLGROUP_WISDOM]->level; i++)
        {
            if (i <= 3)
                pl->levgrace[i] = (char) op->arch->clone.stats.maxgrace;
            else
                pl->levgrace[i] = (char) ((RANDOM() % op->arch->clone.stats.maxgrace) + 1);
        }
    }  /* end of lev_array_flag */

    /* if it's a dragon player, set the correct title here */
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

    pl->player_loaded = 1; /* important: there is a player file */

    /* moved this after the is_dragon_pl() stuff... that is broken in any case */
    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
    FIX_PLAYER(op ,"check login - first fix");

#ifdef AUTOSAVE
    pl->last_save_tick = ROUND_TAG;
#endif

    pl->state = ST_PLAYING;

    // QUICKHACK: to avoid problems after conversion, we have to force a save here
    if(pl->p_ver != PLAYER_FILE_VERSION_BETA4)
    {
        pl->p_ver = PLAYER_FILE_VERSION_BETA4;
        save_player(pl->ob, 1);
        if(old_ap_ptr)
        {
            unlink(old_ap_ptr->path);
            free_map(old_ap_ptr, 1);
            delete_map(old_ap_ptr);
        }
    } // end QUICKHACK

    /* NOW we are ready with loading and setup... now we kick the player in the world */
    if(!mode)
        return; /* if in traverse mode, we only want load the player - not make him alive */

    /* *ONLY* place we set this status */
    pl->socket.status = Ns_Playing;

    new_draw_info(NDI_UNIQUE, 0, op, "Welcome Back!");
#ifdef USE_CHANNELS
#ifdef ANNOUNCE_CHANNELS
    new_draw_info(NDI_UNIQUE | NDI_RED, 0, op, "We are testing out a new channel-system!\nMake sure you have a client with channel-support.\nSee forums on www.daimonin.net!");
#endif
#endif
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
#ifdef PLUGINS
    /* GROS : Here we handle the LOGIN global event */
    evtid = EVENT_LOGIN;
    CFP.Value[0] = (void *) (&evtid);
    CFP.Value[1] = (void *) (pl);
    CFP.Value[2] = (void *) (pl->socket.ip_host);
    GlobalEvent(&CFP);
#endif

    /* If the player should be dead, call kill_player for them
     * Only check for hp - if player lacks food, let the normal
     * logic for that to take place.  If player is permanently
     * dead, and not using permadeath mode, the kill_player will
     * set the play_again flag, so return.
     */
    if (op->stats.hp <= 0)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Your character was dead last your played.");
        kill_player(op);

        if (pl->state != ST_PLAYING)
            return;
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

    /* Do this after checking for death - no reason sucking up bandwidth if
     * the data isn't needed.
     */
    esrv_new_player(pl, op->weight + op->carrying);
    esrv_send_inventory(op, op);

    /* This seems to compile without warnings now.  Don't know if it works
     * on SGI's or not, however.
     */
    qsort((void *) pl->known_spells, pl->nrofknownspells, sizeof(pl->known_spells[0]), (void *) (int (*) ()) spell_sort);

    /* hm, this is for secure - be SURE our player is on
     * friendly list. If friendly is set, this was be done
     * in loader.c.
     */
    if (!QUERY_FLAG(op, FLAG_FRIENDLY))
    {
        LOG(llevBug, "BUG: Player %s was loaded without friendly flag!", query_name(op));
        SET_FLAG(op, FLAG_FRIENDLY);
    }

    /* ok, we are done with the login.
     * Lets put the player on the map and send all player lists to the client.
     * The player is active now.
     */
    enter_map_by_name(op, pl->maplevel, pl->orig_map, pl->map_x, pl->map_y, pl->map_status);

    pl->socket.update_tile = 0;
    pl->socket.look_position = 0;
    pl->socket.look_position_container = 0;
    pl->socket.ext_title_flag = 1;

    pl->ob->direction = 4;
    esrv_new_player(pl, op->weight + op->carrying);
    send_spelllist_cmd(op, NULL, SPLIST_MODE_ADD); /* send the known spells as list to client */
    send_skilllist_cmd(op, NULL, SPLIST_MODE_ADD);

    return;
}
