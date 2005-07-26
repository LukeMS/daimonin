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

/* Delete character with name.  if new is set, also delete the new
 * style directory, otherwise, just delete the old style playfile
 * (needed for transition)
 */
void delete_character(const char *name, int new)
{
    char    buf[MAX_BUF];

    sprintf(buf, "%s/%s/%s.pl", settings.localdir, settings.playerdir, name);
    if (unlink(buf) == -1)
    {
        LOG(llevBug, "BUG:: delete_character(): unlink(%s) failed! (dir not removed too)\n", buf);
        return;
    }
    if (new)
    {
        sprintf(buf, "%s/%s/%s", settings.localdir, settings.playerdir, name);
        /* this effectively does an rm -rf on the directory */
        remove_directory(buf);
    }
}

/* lets check the player name is used.
 * We only deny here when
 * a.) the name is illegal
 * b.) some other user use this name atm to create a new char (bad timing)
 * If the player is playing or just had submited the name,
 * we don't deny it here.
 * we wait for the password first!
 */
int check_name(player *me, char *name)
{
    player     *pl;
    const char *name_hash;

    if (!playername_ok(name))
    {
        new_draw_info(NDI_UNIQUE, 0, me->ob, "That name contains illegal characters.");
        return 0;
    }

    if (!(name_hash = find_string(name)))
        return 1; /* perfect - no hash name, no player */

    for (pl = first_player; pl != NULL; pl = pl->next)
    {
        if (pl != me && (pl->state == ST_CONFIRM_PASSWORD || pl->state == ST_CREATE_CHAR) && pl->ob->name == name_hash)
        {
            new_draw_info(NDI_UNIQUE, 0, me->ob, "Someone else creates a char with that name just now!");
            return 0;
        }
    }

    return 1;
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
    char    filename[MAX_BUF], *tmpfilename, backupfile[MAX_BUF];
    player *pl  = CONTR(op);
    int     i, wiz = QUERY_FLAG(op, FLAG_WIZ);
    long    checksum;
#ifdef BACKUP_SAVE_AT_HOME
    sint16  backup_x, backup_y;
#endif

    if (!op->stats.exp && (!CONTR(op) || !CONTR(op)->player_loaded))
        return 0;   /* no experience, no save */

    flag &= 1;

    /* Sanity check - some stuff changes this when player is exiting */
    if (op->type != PLAYER)
        return 0;

    /* Prevent accidental saves if connection is reset after player has
     * mostly exited.
     */
    if (pl->state != ST_PLAYING)
        return 0;

    /* perhaps we don't need it here?*/
    /*container_unlink(pl,NULL);*/

    /* Delete old style file */
    sprintf(filename, "%s/%s/%s.pl", settings.localdir, settings.playerdir, op->name);
    unlink(filename);

    sprintf(filename, "%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, op->name, op->name);
    make_path_to_file(filename);
    tmpfilename = tempnam_local(settings.tmpdir, NULL);
    fp = fopen(tmpfilename, "w");
    if (!fp)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Can't open file for save.");
        LOG(llevDebug, "Can't open file for save (%s).\n", tmpfilename);
        free(tmpfilename);
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
	fprintf(fp, "dm_stealth %d\n", pl->dm_stealth);
    fprintf(fp, "silent_login %d\n", pl->silent_login);
    fprintf(fp, "gen_hp %d\n", pl->gen_hp);
    fprintf(fp, "gen_sp %d\n", pl->gen_sp);
    fprintf(fp, "gen_grace %d\n", pl->gen_grace);
    fprintf(fp, "listening %d\n", pl->listening);
    fprintf(fp, "spell %d\n", pl->chosen_spell);
    fprintf(fp, "shoottype %d\n", pl->shoottype);
    fprintf(fp, "digestion %d\n", pl->digestion);
    fprintf(fp, "pickup %d\n", pl->mode);
    fprintf(fp, "skill_group %d %d %d\n", pl->base_skill_group[0],pl->base_skill_group[1],pl->base_skill_group[2]);

    /* Match the enumerations but in string form */
    fprintf(fp, "usekeys %s\n",
            pl->usekeys == key_inventory ? "key_inventory" : (pl->usekeys == keyrings ? "keyrings" : "containers"));

#ifdef BACKUP_SAVE_AT_HOME
    if (op->map != NULL && flag == 0)
    #else
        if (op->map != NULL)
        #endif
            fprintf(fp, "map %s\n", op->map->path);
        else
            fprintf(fp, "map %s\n", EMERGENCY_MAPPATH);

    fprintf(fp, "savebed_map %s\n", pl->savebed_map);
    fprintf(fp, "bed_x %d\nbed_y %d\n", pl->bed_x, pl->bed_y);
    fprintf(fp, "Str %d\n", pl->orig_stats.Str);
    fprintf(fp, "Dex %d\n", pl->orig_stats.Dex);
    fprintf(fp, "Con %d\n", pl->orig_stats.Con);
    fprintf(fp, "Int %d\n", pl->orig_stats.Int);
    fprintf(fp, "Pow %d\n", pl->orig_stats.Pow);
    fprintf(fp, "Wis %d\n", pl->orig_stats.Wis);
    fprintf(fp, "Cha %d\n", pl->orig_stats.Cha);

    /* save hp table */
    fprintf(fp, "lev_hp %d\n", op->level);
    for (i = 1; i <= op->level; i++)
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
        free(tmpfilename);
        CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
        return 0;
    }
    checksum = calculate_checksum(tmpfilename, 0);
    sprintf(backupfile, "%s.tmp", filename);
    rename(filename, backupfile);
    fp = fopen(filename, "w");
    if (!fp)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Can't open file for save.");
        unlink(tmpfilename);
        free(tmpfilename);
        CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
        return 0;
    }
    fprintf(fp, "checksum %lx\n", checksum);
    copy_file(tmpfilename, fp);
    unlink(tmpfilename);
    free(tmpfilename);
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

/*
 * calculate_checksum:
 * Evil scheme to avoid tampering with the player-files 8)
 * The cheat-flag will be set if the file has been changed.
 */

long calculate_checksum(char *filename, int checkdouble)
{
#ifdef USE_CHECKSUM
    long    checksum    = 0;
    int     offset      = 0;
    FILE   *fp;
    char    buf[MAX_BUF], *cp;
    if ((fp = fopen(filename, "r")) == NULL)
        return 0;
    while (fgets(buf, MAX_BUF, fp))
    {
        if (checkdouble && !strncmp(buf, "checksum", 8))
            continue;
        for (cp = buf; *cp; cp++)
        {
            if (++offset > 28)
                offset = 0;
            checksum ^= (*cp << offset);
        }
    }
    fclose(fp);
    return checksum;
#else
    return 0;
#endif
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

/* this whole player loading routine is REALLY not optimized -
 * just look for all these scanf()
 */
void check_login(object *op)
{
    static int  kick_loop;

    FILE       *fp;
    void       *mybuffer;
    char        filename[MAX_BUF];
    char        buf[MAX_BUF], bufall[MAX_BUF];
    int         i, value;
    int         lev_array_flag;
    long        checksum            = 0;
    player     *pl                  = CONTR(op);
    int         correct             = 0;
    time_t      elapsed_save_time   = 0;
    struct stat statbuf;
    object     *tmp, *tmp2;

#ifdef PLUGINS
    CFParm      CFP;
    int         evtid;
#endif

    /* we need this why? */
    //strcpy(pl->maplevel,EXIT_PATH(&map_archeytpe->clone) );

    kick_loop = 0;
    /* a good point to add this i to a 10 minute temp ban,,,
     * if needed, i add it... its not much work but i better
     * want a real login server in the future
     */
    if (pl->state == ST_PLAYING)
    {
        LOG(llevSystem, "HACK-BUG: >%s< from ip %s - double login!\n", op->name, pl->socket.host);
        new_draw_info_format(NDI_UNIQUE, 0, op,
                             "You manipulated the login procedure.\nYour IP is ... >%s< - hack flag set!\nserver break",
                             pl->socket.host);
        pl->socket.status = Ns_Dead;
        return;
    }

    LOG(llevInfo, "LOGIN: >%s< from ip %s (%d) - ", STRING_SAFE(op->name), STRING_SAFE(pl->socket.host), pl->socket.fd);

    kick_loop_jump:
    /* First, lets check for newest form of save */
    sprintf(filename, "%s/%s/%s/%s.pl", settings.localdir, settings.playerdir, op->name, op->name);
    if (access(filename, F_OK) == -1)
    {
        /* not there,  Try the old style */

        sprintf(filename, "%s/%s/%s.pl", settings.localdir, settings.playerdir, op->name);
        /* Ok - old style exists.  Lets make the new style directory */
        if (access(filename, F_OK) == 0)
        {
            sprintf(buf, "%s/%s/%s", settings.localdir, settings.playerdir, op->name);
            make_path_to_file(buf);
        }
    }

    /* If no file, must be a new player, so lets get confirmation of
     * the password.  Return control to the higher level dispatch,
     * since the rest of this just deals with loading of the file.
     */
    if ((fp = fopen(filename, "r")) == NULL)
    {
        player *ptmp;
        /* this is a virgin player name.
             * BUT perhaps someone else had the same name idea?
             * Perhaps he is just do the confirm stuff or has entered the game -
             * So, lets check for the name here too
             * and check for confirm_password state
             */
        for (ptmp = first_player; ptmp != NULL; ptmp = ptmp->next)
        {
            if (ptmp != pl && ptmp->state >= ST_CONFIRM_PASSWORD && ptmp->ob->name == op->name)
            {
                LOG(llevInfo, "create char double login!\n");
                new_draw_info(NDI_UNIQUE, 0, pl->ob, "Someone else creates a char with that name just now!");
                FREE_AND_COPY_HASH(op->name, "noname");
                pl->last_value = -1;
                get_name(op);
                return;
            }
        }

        LOG(llevInfo, "new player - confirm pswd\n");
        confirm_password(op);
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
        if (!strncmp(bufall, "checksum ", 9))
        {
            checksum = strtol_local(bufall + 9, (char * *) NULL, 16);
            fgets(bufall, MAX_BUF, fp);
        }

        if (sscanf(bufall, "password %s\n", buf))
        {
            correct = check_password(pl->password, buf);

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
                        remove_ns_dead_player(ptmp);/* super hard kick! */
                        kick_loop++;
                        goto kick_loop_jump;
                    }
                }
            }
        }
    }

    if (!correct)
    {
        LOG(llevInfo, "wrong pswd!\n");
        new_draw_info(NDI_RED, 0, op, " ** wrong password ***");
        fclose(fp);
        FREE_AND_COPY_HASH(op->name, "noname");
        pl->last_value = -1;
        get_name(op);
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

    strcpy(pl->savebed_map,EXIT_PATH(&map_archeytpe->clone) );
    pl->bed_x = map_archeytpe->clone.stats.hp;
    pl->bed_y = map_archeytpe->clone.stats.sp;


    /* Loop through the file, loading the rest of the values */
    lev_array_flag = FALSE;
    while (fgets(bufall, MAX_BUF, fp) != NULL)
    {
        if (!strcmp(bufall, "skill_group "))
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
        else if (!strcmp(buf, "gen_hp"))
            pl->gen_hp = value;
        else if (!strcmp(buf, "shoottype"))
            pl->shoottype = (rangetype) value;
        else if (!strcmp(buf, "gen_sp"))
            pl->gen_sp = value;
        else if (!strcmp(buf, "gen_grace"))
            pl->gen_grace = value;
        else if (!strcmp(buf, "spell"))
            pl->chosen_spell = value;
        else if (!strcmp(buf, "listening"))
            pl->listening = value;
        else if (!strcmp(buf, "digestion"))
            pl->digestion = value;
        else if (!strcmp(buf, "pickup"))
            pl->mode = value;
        else if (!strcmp(buf, "map"))
            sscanf(bufall, "map %s", pl->maplevel);
        else if (!strcmp(buf, "savebed_map"))
            sscanf(bufall, "savebed_map %s", pl->savebed_map);
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

        /* if we meet this identifier, then we have a BETA 1 data file.
             * Now lets change it to BETA 2 version (note that old CVS version
             * can say 0.95 version instead of 0.96)
             */
        else if (!strcmp(buf, "lev_array"))
        {
            int j;
            for (i = 1; i <= value; i++)
            {
                fscanf(fp, "%d\n", &j); /* read in and skip */
                fscanf(fp, "%d\n", &j);
                fscanf(fp, "%d\n", &j);
            }
            lev_array_flag = TRUE;
            /*
                for(i=1;i<=value;i++) {
                int j;
                fscanf(fp,"%d\n",&j);
                pl->levhp[i]=j;
                fscanf(fp,"%d\n",&j);
                pl->levsp[i]=j;
                fscanf(fp,"%d\n",&j);
                pl->levgrace[i]=j;
                }*/

            /* spell_array code removed - don't know when that was last used.
             * Even the load code below will someday be replaced by spells being
             * objects.
             */
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
        /* Remove confkeys, pushkey support - very old */
    } /* End of loop loading the character file */
    /*leave_map(op);*/

    if (!QUERY_FLAG(op, FLAG_REMOVED)) /* Take the player ob out from the void */
        remove_ob(op);
    op->custom_attrset = NULL; /* We transfer it to a new object */

    LOG(llevDebug, "load obj for player: %s\n", op->name);
    destroy_object(op);

    op = get_object(); /* Create a new object for the real player data */

    /* this loads the standard objects values. */
    mybuffer = create_loader_buffer(fp);
    load_object(fp, op, mybuffer, LO_REPEAT, 0);
    delete_loader_buffer(mybuffer);
    fclose(fp);

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
    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);

    strncpy(pl->title, op->arch->clone.name, MAX_NAME);

    /* If the map where the person was last saved does not exist,
     * restart them on their home-savebed. This is good for when
     * maps change between versions
     * First, we check for partial path, then check to see if the full
     * path (for unique player maps)
     */
    if (check_path(pl->maplevel, 1) == -1)
    {
        if (check_path(pl->maplevel, 0) == -1)
        {
            strcpy(pl->maplevel, pl->savebed_map);
            op->x = pl->bed_x, op->y = pl->bed_y;
        }
    }

    /* If player saved beyond some time ago, and the feature is
     * enabled, put the player back on his savebed map.
     */
    if ((settings.reset_loc_time > 0) && (elapsed_save_time > settings.reset_loc_time))
    {
        strcpy(pl->maplevel, pl->savebed_map);
        op->x = pl->bed_x, op->y = pl->bed_y;
    }

    /* make sure he's a player--needed because of class change. */
    op->type = PLAYER;

    /* this is a funny thing: what happens when the autosave function saves a player
     * with negative hp? (never thought thats possible but happens in a 0.95 server)
     * Well, the sever tries to create a gravestone and heals the player... and then
     * server tries to insert gravestone and anim on a map - but player is still in login!
     * So, we are nice and set hp to 1 if here negative-.
     */
    if (op->stats.hp < 0)
        op->stats.hp = 1;

    pl->name_changed = 1;
    pl->state = ST_PLAYING;
#ifdef AUTOSAVE
    pl->last_save_tick = ROUND_TAG;
#endif
    op->carrying = sum_weight(op);

     link_player_skills(op); /* link all exp group & skill objects to the player */

    /* old 0.95b char - create a modern 0.96 one! */
    /* our rule is: for the first 3 levels we use maxXXX,
     * for the next levels we simply use full random throw.
     */

    /* our second mission here is to change all old players deity from Eldath
     * to the Tabernacle.
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
                    if (tmp->title && tmp->title == shstr.Eldath)
                        FREE_AND_ADD_REF_HASH(tmp->title, shstr.the_Tabernacle);
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
                    if ((uint32) tmp->stats.exp < new_levels[i + 1])
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
    }

    if (!legal_range(op, pl->shoottype))
        pl->shoottype = range_none;

    fix_player(op);

    /* if it's a dragon player, set the correct title here */
    if (is_dragon_pl(op) && op->inv != NULL)
    {
        object *tmp, *abil = NULL, *skin = NULL;
        for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
        {
            if (tmp->type == FORCE)
            {
                if (tmp->arch->name == shstr.dragon_ability_force)
                    abil = tmp;
                else if (tmp->arch->name == shstr.dragon_skin_force)
                    skin = tmp;
            }
        }
        set_dragon_name(op, abil, skin);
    }

    pl->player_loaded = 1; /* important: there is a player file */

    new_draw_info(NDI_UNIQUE, 0, op, "Welcome Back!");
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
    CFP.Value[2] = (void *) (pl->socket.host);
    GlobalEvent(&CFP);
#endif
#ifdef ENABLE_CHECKSUM
    LOG(llevDebug, "Checksums: %x %x\n", checksum, calculate_checksum(filename, 1));
    if (calculate_checksum(filename, 1) != checksum)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Since your savefile has been tampered with,");
        new_draw_info(NDI_UNIQUE, 0, op, "you will not be able to save again.");
        set_cheat(op);
    }
#endif
    /* If the player should be dead, call kill_player for them
     * Only check for hp - if player lacks food, let the normal
     * logic for that to take place.  If player is permanently
     * dead, and not using permadeath mode, the kill_player will
     * set the play_again flag, so return.
     */
    if (op->stats.hp < 0)
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

    pl->last_value = -1;

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
    enter_exit(op, NULL); /* kick player on map - load map if needed */

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
