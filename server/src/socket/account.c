/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2008 Michael Toennies

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

/* reset and clear the account data of socket ns */
void account_clear(Account *ac)
{
    int i;

    ac->pwd[0] = 0;
    FREE_AND_CLEAR_HASH(ac->name);
    FREE_AND_CLEAR_HASH(ac->create_name); /* sanity freeing .. should not needed */
    ac->nrof_chars = 0;

    for(i=0;i<ACCOUNT_MAX_PLAYER;i++)
    {
        ac->level[i] = ac->race[i] = ac->gender[i] = 0;
        ac->charname[i][0] = 0;
    }
}

/* save the account to file in a save way */
account_status account_save(Account *ac, const char *name)
{
    int i;
    FILE *fp;
    char filename[MEDIUM_BUF];
    char filepath[MEDIUM_BUF];
    char tmpfilename[MEDIUM_BUF];

    sprintf(filepath, "%s/%s/%s/%s", settings.localdir, settings.accountdir, get_subdir(name), name);
    sprintf(filename, "%s/%s.acc", filepath, name);
    make_path_to_file(filename); /* sanity pathing */
    LOG(llevInfo, "Save Account: %s\n", filename);
    tempnam_local_ext(filepath, NULL, tmpfilename);

    /* save the account in the tmporary file name next to the old account file */
    fp = fopen(tmpfilename, "w");
    if (!fp)
    {
        LOG(llevDebug, "Can't open account file for save (%s).\n", filename);
        return ACCOUNT_STATUS_NOSAVE;
    }

    fprintf(fp, "pwd %s\n", ac->pwd);

    for(i=0;i < ACCOUNT_MAX_PLAYER;i++)
    {
        if(ac->level[i]) /* we have an entry */
        {
            /* save the lvl first, thats our "new index" trigger for the read function */
            fprintf(fp, "lvl %d\n", ac->level[i]);
            fprintf(fp, "name %s\n", ac->charname[i]);
            fprintf(fp, "race %d\n", ac->race[i]);
            fprintf(fp, "gender %d\n", ac->gender[i]);
        }
    }

    if (fclose(fp) == EOF)
    {
        unlink(tmpfilename);
        return ACCOUNT_STATUS_NOSAVE;
    }

    /* we have now 2 valid files - remove now the old one and rename the tmp file to it */
    unlink(filename);
    rename(tmpfilename, filename);

    return ACCOUNT_STATUS_OK;

}

/* create a new account (file) - check the file don't exist and setup the initial values */
account_status account_create(Account *ac, const char *name, char *pass)
{
    FILE *fp;
    char filename[HUGE_BUF];

    sprintf(filename, "%s/%s/%s/%s/%s.acc", settings.localdir, settings.accountdir, get_subdir(name), name, name);

    /* sanity check... we already have an account of this name? */
    if (access(filename, F_OK) == 0)
    {
        LOG(llevDebug,"Bug: account_create(): Account %s already exists!\n", filename);
        return ACCOUNT_STATUS_EXISTS;
    }

    make_path_to_file(filename);
    LOG(llevInfo, "Create Account: %s\n", filename);

    fp = fopen(filename, "w");
    if (!fp)
    {
        LOG(llevDebug, "Can't open account file for save (%s).\n", filename);
        return ACCOUNT_STATUS_CORRUPT;
    }
    fprintf(fp, "pwd %s\n", pass);

    /* this is a fresh account - no character data to write */

    fclose(fp);

    return ACCOUNT_STATUS_OK;
}

/* load a account file and fill up the account data */
account_status account_load(Account *ac, char *name, char *pass)
{
    FILE        *fp;
    int         i = -1, val, ret = ACCOUNT_STATUS_OK;
    char        filename[HUGE_BUF];
    char        buf[MEDIUM_BUF], bufall[MEDIUM_BUF];

    sprintf(filename, "%s/%s/%s/%s/%s.acc", settings.localdir, settings.accountdir, get_subdir(name), name, name);
    LOG(llevInfo, "Login to Account: %s\n", filename);
    account_clear(ac);

    if ((fp = fopen(filename, "r")) == NULL)
    {
        return ACCOUNT_STATUS_UNKNOWN; /* unknown account */
    }

    if (fgets(bufall, MEDIUM_BUF, fp) == NULL)
    {
        LOG(llevDebug, "\nBUG: corrupt account file %s!\n", filename);
        fclose(fp);
        return ACCOUNT_STATUS_CORRUPT;
    }
    else
    {
        if (sscanf(bufall, "pwd %s\n", buf))
        {
            /* thats the final check - if pwd matches we have a valid login */
            if(strcmp(buf, crypt_string(pass)))
            {
                /* TODO: enable password try handling for accounts */
                /*
                if(++ns->pwd_try == 3)
                {
                    char password_warning[] =
                        "3 You entered 3 times a wrong password.\nTry new login in 1 minute!\nConnection closed.";
                    LOG(llevInfo,"PWD GUESS BAN (1min): IP %s (tried name: %s).\n", ns->ip_host, name);
                    add_ban_entry(NULL, ns->ip_host, 8*60, 8*60);
                    Write_String_To_Socket(ns, SERVER_CMD_DRAWINFO,password_warning , strlen(password_warning));
                    player_addme_failed(ns, ADDME_MSG_DISCONNECT);
                    ns->inactive_when = ROUND_TAG + INACTIVE_ZOMBIE * pticks_second;
                    ns->status = Ns_Zombie;
                    ns->inactive_flag = 1;
                }
                */

                LOG(llevDebug, "Wrong account password: %s!\n", filename);
                fclose(fp);
                return ACCOUNT_STATUS_WRONGPWD;
            }
            strcpy(ac->pwd, buf);
        }
        else
        {
            LOG(llevDebug, "\nBUG: corrupt (2) account file %s!\n", filename);
            fclose(fp);
            return ACCOUNT_STATUS_CORRUPT;
        }
    }

    while (fgets(bufall, MEDIUM_BUF, fp) != NULL)
    {
        if (!strncmp(bufall, "name ", 5))
        {
            if(i < 0 || !bufall[5])
                ret = ACCOUNT_STATUS_CORRUPT;
            else
                sscanf(bufall, "name %s\n", ac->charname[i]); /* use sscanf to safely remove OS based \n */
        }
        else
        {
            if (sscanf(bufall, "%s %d\n", buf, &val) != 2) /* sanity check */
                ret = ACCOUNT_STATUS_CORRUPT;
            else if (!strcmp(buf, "lvl"))
            {
                /* new lvl = new index */
                if(++i >= ACCOUNT_MAX_PLAYER)
                    ret = ACCOUNT_STATUS_CORRUPT;
                else
                {
                    ac->nrof_chars = i+1; /* index is zero based, nrof_chars based on one */
                    ac->level[i] = val;
                }
            }
            else if (!strcmp(buf, "race"))
            {
                if(i < 0)
                    ret = ACCOUNT_STATUS_CORRUPT;
                else
                    ac->race[i] = val;
            }
            else if (!strcmp(buf, "gender"))
            {
                if(i < 0)
                    ret = ACCOUNT_STATUS_CORRUPT;
                else
                    ac->gender[i] = val;
            }
            else
                ret = ACCOUNT_STATUS_CORRUPT;
        }

        if(ret != ACCOUNT_STATUS_OK)
        {
            LOG(llevDebug, "\nBUG: corrupt ID:(%d) account file %s!\n", ret, filename);
            account_clear(ac);
            break;
        }
    }

    fclose(fp);

    /* finally remember our account name */
    if(ret == ACCOUNT_STATUS_OK)
        FREE_AND_COPY_HASH(ac->name, name);

    return ret;
}

/* The client had asked us to use <name> for a new account.
 * msg says yes OR no
 */
void account_create_msg(NewSocket *ns, int msg)
{
    sockbuf_struct	*sbptr;

    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_SMALL);
    sbptr = ACTIVE_SOCKBUF(ns);
    SockBuf_AddChar(sbptr, msg);
    SOCKBUF_REQUEST_FINISH(ns, SERVER_CMD_ACCNAME_SUC, SOCKBUF_DYNAMIC);
}

/* send our account data to the client or tell client why its not possible
 */
void account_send_client(NewSocket *ns, int stats)
{
    int i;
    Account *ac = &ns->pl_account;
    sockbuf_struct	*sbptr;

    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_SMALL);
    sbptr = ACTIVE_SOCKBUF(ns);

    SockBuf_AddChar(sbptr, stats);

    /* check status - send status only when its not OK, in the other case send the collected info */
    if(stats == ACCOUNT_STATUS_OK)
    {
        /* flush the account data to the client - the client can login with simple name later */
        for(i=0;i < ACCOUNT_MAX_PLAYER;i++)
        {
            if(ac->level[i])
            {
                SockBuf_AddString(sbptr, ac->charname[i], strlen(ac->charname[i]));
                SockBuf_AddChar(sbptr, ac->level[i]);
                SockBuf_AddChar(sbptr, ac->race[i]);
                SockBuf_AddChar(sbptr, ac->gender[i]);
            }
        }
    }

    SOCKBUF_REQUEST_FINISH(ns, SERVER_CMD_ACCOUNT, SOCKBUF_DYNAMIC);
}

/* remove a player from an account by moving the player dir inside the account
 * we use the naming system <name>.timestamp for it, where timestamp will make it unique.
 * We also remove the player name from the account and do some sanity checks and repair when needed.
 * We don't save or send account data, thats up to the caller (allows virtual handling
 * of non logged in acounts).
 * return values:
 * player deleted and account updated: ACCOUNT_STATUS_OK
 * name not part of account: ACCOUNT_STATUS_EXISTS
 * player name don't exists: ACCOUNT_STATUS_NOSAVE (but we removed name from account!!)
 * player name can't be moved/accessed/renamed: ACCOUNT_STATUS_CORRUPT
 * (we also remove name from account - in this case the player name should be blocked which
 * will effect a newchar with that name but did no harm)
 */
account_status account_delete_player(NewSocket *ns, shstr_t *name)
{
    Account *ac = &ns->pl_account;
    int i, ret = ACCOUNT_STATUS_EXISTS;
    char ac_fname[MEDIUM_BUF], pl_fname[MEDIUM_BUF];
    struct timeval   now;
    player_t *ptmp = first_player;

    /* first, lets check we have that name in our account - in any case we remove it
     * from the account!
     */
    for(i=0;i < ACCOUNT_MAX_PLAYER;i++)
    {
        if(ac->level[i] && !strcmp(name, ac->charname[i]))
        {
            ac->level[i] = ac->race[i] = ac->gender[i] = 0;
            ac->charname[i][0] = 0;
            ac->nrof_chars -= 1;
            ret = ACCOUNT_STATUS_OK;
            /* to do NOT a break here will delete any entry in the case we have the same name
             * here more as one time (which should not happen and is a bug. But so we try on
             * the fly to repair it
             */
            /*break;*/
        }
    }

    if(ret != ACCOUNT_STATUS_OK)
        return ret;

    /* ok, lets try to move the player - but first some sanity check - we need a account name */
    if(!ac->name)
        return ACCOUNT_STATUS_CORRUPT; /* something is really sick */

    for (; ptmp; ptmp = ptmp->next)
    {
        /* If a the player is playing, kick him BEFORE we delete the player
         * file for this new instance. */
        if ((ptmp->state & ST_PLAYING) &&
            ptmp->ob->name == name)
        {
            LOG(llevInfo, "INFO:: Online player %s deleted!\n", name);
            ptmp->state &= ~ST_PLAYING;
            ptmp->state |= ST_ZOMBIE;
            ptmp->socket.status = Ns_Dead;
            remove_ns_dead_player(ptmp);/* super hard kick! */

            continue;
        }
    }

    /* we move the whole player dir */
    sprintf(pl_fname, "%s/%s/%s/%s", settings.localdir, settings.playerdir, get_subdir(name), name);

    GETTIMEOFDAY(&now);

    /* inside the account */
    sprintf(ac_fname, "%s/%s/%s/%s/%s.%lu", settings.localdir, settings.accountdir, get_subdir(ac->name), ac->name, name, now.tv_sec);

    /* this will rename the directory and adjust the path too like a move cmd */
    if(rename(pl_fname, ac_fname))
        ret = ACCOUNT_STATUS_CORRUPT; /* there is *something* wrong with the player files. but account is ok... */

    return ret;
}

Account *account_get_from_object(object_t *op)
{
    player_t    *pl;
    NewSocket *ns;
    Account   *ac;

    if (!op ||
        !(pl = CONTR(op)))
        return NULL;

    if (!(ns = &pl->socket))
        return NULL;

    if (!(ac = &ns->pl_account))
        return NULL;

    return ac;
}

int account_update(Account *ac, object_t *op)
{
    int        i;
    object_t    *force;
    int        drain_level = 0;

    if (!ac)
        return 0;

    // Find correct entry in the account file, corresponding to current player
    for (i=0; i < ACCOUNT_MAX_PLAYER; i++)
    {
        if (ac->level[i]) // we have an entry
        {
            if (!strcmp(STRING_OBJ_NAME(op), ac->charname[i]))
            {
                /* check for drain so we know the proper level of the player */
                if ((force = present_arch_in_ob(archetype_global._drain, op)))
                    drain_level = force->level;

                ac->level[i] = op->level + drain_level;
                return 1;
            }
        }
    }

    // Didn't find a match - probably going through char creation process
    return 0;
}

// These 2 functions basically copied from their find_player() equivalents
Account *find_account(char *acname)
{
    char name[MAX_ACCOUNT_NAME+1];
    const char *name_hash;

    int name_len = strlen(acname); /* we assume a legal string */
    if (name_len <= 1 || name_len > MAX_ACCOUNT_NAME)
        return NULL;

    strcpy(name, acname); /* we need to copy it because we access the string */
    transform_account_name_string(name);
    if (!(name_hash = find_string(name)))
        return NULL;

    return find_account_hash(name_hash);
}

/* nearly the same as above except we
 * have the hash string when we call
 */
Account *find_account_hash(const char *acname)
{
    player_t *pl;

    if(acname)
    {
        for (pl = first_player; pl != NULL; pl = pl->next)
        {
            if (pl->ob && !QUERY_FLAG(pl->ob, FLAG_REMOVED) && pl->account_name == acname)
                return &pl->socket.pl_account;
        }
    }
    return NULL;
}
