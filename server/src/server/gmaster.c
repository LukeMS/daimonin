/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2005 Michael Toennies

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

/* gmaster.c
 * Handle the gmaster_file and the settings to different "game master"
 * levels for players.
 * This system allows different rights for gm's and player as a better control.
 * Rights & permissions can be changed at runtime and are stored between sessions.
 */

#ifndef __GMASTER_C
#define __GMASTER_C
#endif

#include <global.h>

/* lists of the active ingame gmasters */
objectlink *gmaster_list;
objectlink *gmaster_list_VOL;
objectlink *gmaster_list_GM;
objectlink *gmaster_list_MW;
objectlink *gmaster_list_MM;
objectlink *gmaster_list_SA;

/* returns a objectlink with gmaster_struct
 * we use for both the memorypool system
 */
static objectlink *get_gmaster_node(void)
{
    objectlink        *ol   = get_objectlink(OBJLNK_FLAG_GM);
    struct _gmaster_struct  *gptr = (struct _gmaster_struct *) get_poolchunk(pool_gmasters);

    memset(gptr, 0, sizeof(gmaster_struct));
    ol->objlink.gm = gptr;

    return ol;
}

/* free the gmaster node and the used objectlink
 */
static void free_gmaster_node(objectlink *ol)
{
    return_poolchunk(ol->objlink.gm, pool_gmasters);
    return_poolchunk(ol, pool_objectlink);
}


/* add a player with a activated gmaster mode to the global lists
 */
static struct oblnk *add_gmaster_list(player *pl)
{
    objectlink *ol;

    if(pl->gmaster_mode == GMASTER_MODE_NO)
        return NULL;

    ol = get_objectlink(OBJLNK_FLAG_OB);
    ol->objlink.ob = pl->ob;

    if (pl->gmaster_mode == GMASTER_MODE_MW)
        objectlink_link(&gmaster_list_MW, NULL, NULL, gmaster_list_MW, ol);
    else if (pl->gmaster_mode == GMASTER_MODE_VOL)
        objectlink_link(&gmaster_list_VOL, NULL, NULL, gmaster_list_VOL, ol);
    else if (pl->gmaster_mode == GMASTER_MODE_GM)
        objectlink_link(&gmaster_list_GM, NULL, NULL, gmaster_list_GM, ol);
    else if (pl->gmaster_mode == GMASTER_MODE_MM)
        objectlink_link(&gmaster_list_MM, NULL, NULL, gmaster_list_MM, ol);
    else
        objectlink_link(&gmaster_list_SA, NULL, NULL, gmaster_list_SA, ol);

    return ol;
}

/* remove a player from global gmaster lists
 */
void remove_gmaster_list(player *pl)
{
    if (pl->gmaster_mode == GMASTER_MODE_NO)
        return;
    if (pl->gmaster_mode == GMASTER_MODE_MW)
        objectlink_unlink(&gmaster_list_MW, NULL, pl->gmaster_node);
    else if (pl->gmaster_mode == GMASTER_MODE_VOL)
        objectlink_unlink(&gmaster_list_VOL, NULL, pl->gmaster_node);
    else if (pl->gmaster_mode == GMASTER_MODE_GM)
        objectlink_unlink(&gmaster_list_GM, NULL, pl->gmaster_node);
    else if (pl->gmaster_mode == GMASTER_MODE_MM)
        objectlink_unlink(&gmaster_list_MM, NULL, pl->gmaster_node);
    else
        objectlink_unlink(&gmaster_list_SA, NULL, pl->gmaster_node);
}

/* This performs some basic checks on the parameters to make sure they all add
 * up to a valid entry. If they do, GMASTER_MODE_FOO is returned according to
 * <mode>. If they don't, GMASTER_MODE_NO is returned. */
int validate_gmaster_params(char *name, char *host, char *mode)
{
    int mode_id,
        len;

    /* First validate mode and set mode_id. */
    if (!strcmp(mode, "VOL"))
    {
        mode_id = GMASTER_MODE_VOL;
    }
    else if (!strcmp(mode, "GM"))
    {
        mode_id = GMASTER_MODE_GM;
    }
    else if (!strcmp(mode, "MW"))
    {
        mode_id = GMASTER_MODE_MW;
    }
    else if (!strcmp(mode, "MM"))
    {
        mode_id = GMASTER_MODE_MM;
    }
    else if (!strcmp(mode, "SA"))
    {
        mode_id = GMASTER_MODE_SA;
    }
    else
    {
        mode_id = GMASTER_MODE_NO;
        LOG(llevInfo, "INFO:: validate_gmaster_mode(): mode '%s' invalid!\n",
            mode);
    }

    /* Validate name. */
    if (!name ||
        (((len = strlen(name)) == 1 &&
          *name != '*') &&
         (len < MIN_ACCOUNT_NAME ||
          len > MAX_ACCOUNT_NAME)))
    {
        mode_id = GMASTER_MODE_NO;
        LOG(llevInfo, "INFO:: validate_gmaster_mode(): invalid name '%s'!\n",
            name);
    }

    /* Validate host. */
    if (!host ||
        (((len = strlen(host)) == 1 &&
          *host != '*') &&
         (len <= 6 ||
          len >= 120)))
    {
        mode_id = GMASTER_MODE_NO;
        LOG(llevInfo, "INFO:: validate_gmaster_mode(): invalid host '%s'!\n",
            host);
    }

    return mode_id;
}

/* check a file entry.
 * Return GMASTER_MODE_NO for a invalid entry.
 */
int check_gmaster_file_entry(char *name, char *host, char *mode)
{
    int mode_id = GMASTER_MODE_NO;

    if(strlen(name) > MAX_ACCOUNT_NAME)
    {
        LOG(llevBug, "BUG: load_gmaster_file): name %s too long: %d\n", name, (int)strlen(name));
        return mode_id;
    }
    if(strlen(host) >= 120)
    {
        LOG(llevBug, "BUG: load_gmaster_file): host %s too long: %d\n", host, (int)strlen(host));
        return mode_id;
    }

    if(!strcasecmp(mode,"VOL"))
        mode_id = GMASTER_MODE_VOL;
    else if(!strcasecmp(mode,"GM"))
        mode_id = GMASTER_MODE_GM;
    else if(!strcasecmp(mode,"MW"))
        mode_id = GMASTER_MODE_MW;
    else if(!strcasecmp(mode,"MM"))
        mode_id = GMASTER_MODE_MM;
    else if(!strcasecmp(mode,"SA"))
        mode_id = GMASTER_MODE_SA;

    if(mode_id == GMASTER_MODE_NO)
        LOG(llevBug, "BUG: load_gmaster_file): invalid mode tag: %d\n", mode_id);

    return mode_id;
}

/* load & parse the gmaster_file.
 * Setup the gmaster rights list.
 */
int load_gmaster_file(void)
{
    FILE   *dmfile;
    char    buf[HUGE_BUF];
    char    line_buf[MEDIUM_BUF], name[MEDIUM_BUF], host[MEDIUM_BUF], mode[MEDIUM_BUF], dummy[MEDIUM_BUF];

    LOG(llevInfo,"loading gmaster_file....\n");
    sprintf(buf, "%s/%s", settings.localdir, GMASTER_FILE);
    if ((dmfile = fopen(buf, "r")) == NULL)
    {
        LOG(llevDebug, "Could not find gmaster_file file.\n");
        return(0);
    }
    while (fgets(line_buf, 160, dmfile) != NULL)
    {
        if (line_buf[0] == '#')
            continue;
        if (sscanf(line_buf, "%[^/]/%[^/]/%s%[\n\r]", name, host, mode, dummy) < 3)
            LOG(llevBug, "BUG: malformed gmaster_file entry: %s\n", line_buf);
        else
        {

            int mode_id = check_gmaster_file_entry(name, host, mode);

            if(mode_id == GMASTER_MODE_NO)
                continue;

            /* all ok, setup the gmaster node and add it to our list */
            add_gmaster_file_entry(name, host, mode_id);
        }

    }
    fclose(dmfile);
    return (0);
}

/* add a gmaster entry to the gmaster file list
 */
void add_gmaster_file_entry(char *name, char *host, int mode_id)
{
    objectlink *ol;
    uint8      i;

    ol = get_gmaster_node();

    *name = toupper(*name);

    for (i = 1; *(name + i) != '\0'; i++)
    {
        *(name + i) = tolower(*(name + i));
    }

    sprintf(ol->objlink.gm->entry, "%s/%s/%s",
            name, host, ((mode_id == GMASTER_MODE_VOL) ? "VOL" :
                         ((mode_id == GMASTER_MODE_GM) ? "GM" :
                          ((mode_id == GMASTER_MODE_MW) ? "MW" :
                           ((mode_id == GMASTER_MODE_MM) ? "MM" : "SA")))));
    strcpy(ol->objlink.gm->name, name);
    strcpy(ol->objlink.gm->host, host);
    ol->objlink.gm->mode = mode_id;

    /* lifo list */
    objectlink_link(&gmaster_list, NULL, NULL, gmaster_list, ol);
}

void remove_gmaster_file_entry(objectlink *ol)
{
    objectlink_unlink(&gmaster_list, NULL, ol);
}

/* a player has given a /dm,/gm,/vol commands or its triggered by login.
 * Check the gmaster list its allowed.
 * return: TRUE= allowed, FALSE: disallowed
 */
int check_gmaster_list(player *pl, int mode)
{
    objectlink *ol;

    for(ol = gmaster_list;ol;ol=ol->next)
    {
        /*LOG(llevNoLog,"CHECK: %s - %s -%d\n",ol->objlink.gm->name,
                ol->objlink.gm->host,ol->objlink.gm->mode );*/
        if ((!strcmp(ol->objlink.gm->name, "*") ||
             !strcasecmp(pl->account_name, ol->objlink.gm->name)) &&
           (!strcmp(ol->objlink.gm->host, "*") ||
            !strcasecmp(pl->socket.ip_host, ol->objlink.gm->host)))
        {
             /* Obviously you can become the GMASTER you're listed as. */
            if (ol->objlink.gm->mode == mode)
                return TRUE;
            /* GMs can become VOLs. */
            else if (ol->objlink.gm->mode == GMASTER_MODE_GM &&
                mode == GMASTER_MODE_VOL)
                return TRUE;
            /* MMs can become MWs. */
            else if (ol->objlink.gm->mode == GMASTER_MODE_MM &&
                mode == GMASTER_MODE_MW)
                return TRUE;
            /* SAs can become anything. */
            else if (ol->objlink.gm->mode == GMASTER_MODE_SA &&
                     (mode == GMASTER_MODE_VOL ||
                      mode == GMASTER_MODE_GM ||
                      mode == GMASTER_MODE_MW ||
                      mode == GMASTER_MODE_MM))
                return TRUE;
        }
    }

    return FALSE;
}

/* Free the whole gmaster list
 */
void free_gmaster_list()
{
    objectlink *ol;

    LOG(llevDebug, "Freeing all gmaster entries\n");

    for(ol = gmaster_list; ol; ol=ol->next)
        free_gmaster_node(ol);
}

/* set a gmaster mode to a player: SA, MM, MW, GM, or VOL
 */
void set_gmaster_mode(player *pl, int mode)
{
#ifdef USE_CHANNELS
    struct channels        *channel;
    extern struct channels *channel_list_start;
#endif
    /* remove first the old mode if there is one */
    if(pl->gmaster_mode != GMASTER_MODE_NO)
        remove_gmaster_mode(pl);

    pl->gmaster_mode = mode;
    pl->gmaster_node = add_gmaster_list(pl); /* link player to list of gmasters */

#ifdef USE_CHANNELS
    /* Add player to appropriate gmaster channels. */
    for (channel = channel_list_start; channel; channel = channel->next)
    {
        if (channel->gmaster_mode != GMASTER_MODE_NO &&
            compare_gmaster_mode(channel->gmaster_mode, mode))
            addPlayerToChannel(pl, channel->name, NULL);
    }
#endif

#ifdef _TESTSERVER
    if (mode == GMASTER_MODE_MW ||
        mode == GMASTER_MODE_MM ||
        mode == GMASTER_MODE_SA)
#else
    if (mode == GMASTER_MODE_MM ||
        mode == GMASTER_MODE_SA)
#endif
    {
        SET_FLAG(pl->ob, FLAG_WIZ);
        pl->wizpass = 1;
        esrv_send_inventory(pl->ob, pl->ob);
        clear_los(pl->ob);
        pl->socket.update_tile = 0; /* force a draw_look() */
        pl->update_los = 1;
    }

    pl->socket.ext_title_flag =1;
    new_draw_info(NDI_UNIQUE, 0, pl->ob, "%s mode activated for %s!",
                  ((mode == GMASTER_MODE_VOL) ? "VOL" :
                   ((mode == GMASTER_MODE_GM) ? "GM" :
                    ((mode == GMASTER_MODE_MW) ? "MW" :
                     ((mode == GMASTER_MODE_MM) ? "MM" : "SA")))), pl->ob->name);
}

/* remove the current gmaster mode
 */
void remove_gmaster_mode(player *pl)
{
    int                     mode;
#ifdef USE_CHANNELS
    struct player_channel  *pl_channel;
#endif

    mode = pl->gmaster_mode;

    new_draw_info(NDI_UNIQUE, 0, pl->ob, "%s mode deactivated.",
                  ((mode == GMASTER_MODE_VOL) ? "VOL" :
                   ((mode == GMASTER_MODE_GM) ? "GM" :
                    ((mode == GMASTER_MODE_MW) ? "MW" :
                     ((mode == GMASTER_MODE_MM) ? "MM" : "SA")))));

    remove_gmaster_list(pl);
    pl->gmaster_mode = GMASTER_MODE_NO;

#ifdef USE_CHANNELS
    /* Remove player from all gmaster channels. */
    for (pl_channel = pl->channels; pl_channel; pl_channel = pl_channel->next_channel)
    {
        if (pl_channel->channel->gmaster_mode != GMASTER_MODE_NO)
        {
            char buf[MEDIUM_BUF];

            sprintf(buf, "You leave channel %s", pl_channel->channel->name);
            removeChannelFromPlayer(pl, pl_channel, buf);
        }
    }
#endif

#ifdef _TESTSERVER
    if (mode == GMASTER_MODE_MW ||
        mode == GMASTER_MODE_MM ||
        mode == GMASTER_MODE_SA)
#else
    if (mode == GMASTER_MODE_MM ||
        mode == GMASTER_MODE_SA)
#endif
    {
        /* remove the power settings */
        CLEAR_FLAG(pl->ob, FLAG_WIZ);
        pl->wizpass = 0;
        /* bit of a cheat, but by doing this we avoid a fix when going into wiz
         * mode and slight confusion. */
        pl->dm_invis = 0;
        FIX_PLAYER(pl->ob, "remove wiz mode");
        pl->socket.update_tile = 0;
        esrv_send_inventory(pl->ob, pl->ob);
        pl->update_los = 1;
    }

    pl->socket.ext_title_flag =1;
}

/* write back the gmaster file.
 * (triggered after a dm_set add/remove command)
 */
void write_gmaster_file(void)
{
    char    filename[MEDIUM_BUF];
    objectlink *ol;
    FILE   *fp;

    sprintf(filename, "%s/%s", settings.localdir,GMASTER_FILE);
    if ((fp = fopen(filename, "w")) == NULL)
    {
        LOG(llevBug, "BUG: Cannot open %s for writing\n", filename);
        return;
    }
    fprintf(fp, "# GMASTER_FILE (file is changed from server at runtime)\n");
    fprintf(fp, "#\n");
    fprintf(fp, "# <name>/<host>/<mode>\n");
    fprintf(fp, "#\n");
    fprintf(fp, "# <name> is a account name. May be '*' for any name. Must not contain the '/'\n");
    fprintf(fp, "# character.\n");
    fprintf(fp, "#\n");
    fprintf(fp, "# <host> is an IP address. May be '*' for any host. Must not contain the '/'\n");
    fprintf(fp, "# character.\n");
    fprintf(fp, "#\n");
    fprintf(fp, "# <mode> is one of MW, VOL, GM, MM, or SA.\n");

    for(ol = gmaster_list;ol;ol=ol->next)
        fprintf(fp, "%s\n", ol->objlink.gm->entry);

    fclose(fp);
}

/* Returns TRUE if p has sufficient gmaster_mode to access t, FALSE if not. */
int compare_gmaster_mode(int t, int p)
{
    switch (t)
    {
        /* Only SA p can access a SA t. */
        case GMASTER_MODE_SA:
            if (p == GMASTER_MODE_SA)
                return TRUE;

            return FALSE;

        /* SA or MM p can access a MM t. */
        case GMASTER_MODE_MM:
            if (p == GMASTER_MODE_SA ||
                p == GMASTER_MODE_MM)
                return TRUE;

            return FALSE;

        /* MW, MM, or SA p can access a MW t. */
        case GMASTER_MODE_MW:
            if (p == GMASTER_MODE_SA ||
                p == GMASTER_MODE_MM ||
                p == GMASTER_MODE_MW)
                return TRUE;

            return FALSE;

        /* SA or GM p can access a GM t. */
        case GMASTER_MODE_GM:
            if  (p == GMASTER_MODE_SA ||
                 p == GMASTER_MODE_GM)
                return TRUE;

            return FALSE;

        /* VOL, GM, or SA p can access a VOL t. */
        case GMASTER_MODE_VOL:
            if (p == GMASTER_MODE_VOL ||
                p == GMASTER_MODE_GM ||
                p == GMASTER_MODE_SA)
                return TRUE;

            return FALSE;

        /* t is not gmaster restricted. */
        case GMASTER_MODE_NO:
            return TRUE;

        /* t is an unrecognised gmaster_mode, so restrict access. */
        default:
            LOG(llevBug, "BUG:: %s/compare_gmaster_mode(): Unrecognised gmaster_mode: %d!",
                __FILE__, t);

            return FALSE;
    }
}

/* check the rights of all gmaster_modes.
 * (triggered after a /gmasterfile add/remove command). */
void update_gmaster_file(void)
{
    objectlink *ol, *ol_tmp;

    for(ol = gmaster_list_VOL;ol;ol=ol_tmp)
    {
        ol_tmp = ol->next;
        if(!check_gmaster_list(CONTR(ol->objlink.ob), GMASTER_MODE_VOL))
            remove_gmaster_mode(CONTR(ol->objlink.ob));
    }

    for(ol = gmaster_list_GM;ol;ol=ol_tmp)
    {
        ol_tmp = ol->next;
        if(!check_gmaster_list(CONTR(ol->objlink.ob), GMASTER_MODE_GM))
            remove_gmaster_mode(CONTR(ol->objlink.ob));
    }

    for(ol = gmaster_list_MW;ol;ol=ol_tmp)
    {
        ol_tmp = ol->next;
        if(!check_gmaster_list(CONTR(ol->objlink.ob), GMASTER_MODE_MW))
            remove_gmaster_mode(CONTR(ol->objlink.ob));
    }
    for(ol = gmaster_list_MM;ol;ol=ol_tmp)
    {
        ol_tmp = ol->next;
        if(!check_gmaster_list(CONTR(ol->objlink.ob), GMASTER_MODE_MM))
            remove_gmaster_mode(CONTR(ol->objlink.ob));
    }

    for(ol = gmaster_list_SA;ol;ol=ol_tmp)
    {
        ol_tmp = ol->next;
        if(!check_gmaster_list(CONTR(ol->objlink.ob), GMASTER_MODE_SA))
            remove_gmaster_mode(CONTR(ol->objlink.ob));
    }

}
