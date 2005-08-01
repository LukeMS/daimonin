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

    The author can be reached via e-mail to daimonin@nord-com.net
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

    if(pl->gmaster_mode == GMASTER_MODE_VOL)
        objectlink_link(&gmaster_list_VOL, NULL, NULL, gmaster_list_VOL, ol);
    else if(pl->gmaster_mode == GMASTER_MODE_GM)
        objectlink_link(&gmaster_list_GM, NULL, NULL, gmaster_list_GM, ol);
    else /* DM list mode */
        objectlink_link(&gmaster_list_DM, NULL, NULL, gmaster_list_DM, ol);

    return ol;
}

/* remove a player from global gmaster lists
 */
void remove_gmaster_list(player *pl)
{
    if(pl->gmaster_mode == GMASTER_MODE_NO)
        return;

    if(pl->gmaster_mode == GMASTER_MODE_VOL)
        objectlink_unlink(&gmaster_list_VOL, NULL, pl->gmaster_node);
    else if(pl->gmaster_mode == GMASTER_MODE_GM)
        objectlink_unlink(&gmaster_list_GM, NULL, pl->gmaster_node);
    else /* DM list mode */
        objectlink_unlink(&gmaster_list_DM, NULL, pl->gmaster_node);
}

/* check a file entry.
 * Return GMASTER_MODE_NO for a invalid entry.
 */
int check_gmaster_file_entry(char *name, char *passwd, char *host, char *mode)
{
    int mode_id = GMASTER_MODE_NO;

    if(strlen(name) >= MAX_PLAYER_NAME)
    {
        LOG(llevBug, "BUG: load_gmaster_file): name %s to long: %d\n", name, strlen(name));
        return mode_id;
    }
    if(strlen(host) >= 120)
    {
        LOG(llevBug, "BUG: load_gmaster_file): host %s to long: %d\n", host, strlen(host));
        return mode_id;
    }
    if(strlen(passwd) >= MAX_PLAYER_PASSWORD)
    {
        LOG(llevBug, "BUG: load_gmaster_file): passwd %s to long: %d\n", passwd, strlen(passwd));
        return mode_id;
    }

    if(!strcasecmp(mode,"VOL"))
        mode_id = GMASTER_MODE_VOL;
    else if(!strcasecmp(mode,"GM"))
        mode_id = GMASTER_MODE_GM;
    else if(!strcasecmp(mode,"DM"))
        mode_id = GMASTER_MODE_DM;

    if(mode_id == GMASTER_MODE_NO)
        LOG(llevBug, "BUG: load_gmaster_file): invalid mode tag: %s\n", mode_id);

    return mode_id;
}

/* load & parse the gmaster_file.
 * Setup the gmaster rights list.
 */
int load_gmaster_file(void)
{
    FILE   *dmfile;
    char    buf[HUGE_BUF];
    char    line_buf[MAX_BUF], name[MAX_BUF], passwd[MAX_BUF], host[MAX_BUF], mode[MAX_BUF], dummy[MAX_BUF];

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
        if (sscanf(line_buf, "%[^:]:%[^:]:%[^:]:%s%[\n\r]", name, passwd, host, mode, dummy) < 3)
            LOG(llevBug, "BUG: malformed gmaster_file entry: %s\n", line_buf);
        else
        {

            int mode_id = check_gmaster_file_entry(name, passwd, host, mode);

            if(mode_id == GMASTER_MODE_NO)
                continue;

            /* all ok, setup the gmaster node and add it to our list */
            add_gmaster_file_entry(name, passwd, host, mode_id);
        }

    }
    fclose(dmfile);
    return (0);
}

/* add a gmaster entry to the gmaster file list
 */
void add_gmaster_file_entry(char *name, char *passwd, char *host, int mode_id)
{
    objectlink *ol;

    ol = get_gmaster_node();

    sprintf( ol->objlink.gm->entry, "%s:%s:%s:%s", name, passwd, host,
             mode_id==GMASTER_MODE_DM?"DM":(mode_id==GMASTER_MODE_GM?"GM":"VOL"));
    strcpy(ol->objlink.gm->name,name);
    strcpy(ol->objlink.gm->password,passwd);
    strcpy(ol->objlink.gm->host,host);
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
        /*LOG(-1,"CHECK: %s - %s - %s -%d\n",ol->objlink.gm->name,
                ol->objlink.gm->password,ol->objlink.gm->host,ol->objlink.gm->mode );*/
        if ( ol->objlink.gm->mode >= mode /* allow a GM to activate VOL mode for example */
             && (!strcmp(ol->objlink.gm->name, "*") || !strcasecmp(pl->ob->name, ol->objlink.gm->name))
             && (!strcmp(ol->objlink.gm->password, "*") || !strcmp(pl->password, ol->objlink.gm->password))
             && (!strcmp(ol->objlink.gm->host, "*") || !strcasecmp(pl->socket.host, ol->objlink.gm->host)))
            return TRUE;
    }

    return FALSE;
}

/* set a gmaster mode to a player: DM, GM or VOL
 */
void set_gmaster_mode(player *pl, int mode)
{
    /* remove first the old mode if there is one */
    if(pl->gmaster_mode != GMASTER_MODE_NO)
        remove_gmaster_mode(pl);

    pl->gmaster_mode = mode;
    pl->gmaster_node = add_gmaster_list(pl); /* link player to list of gmasters */

    if(mode == GMASTER_MODE_DM)
    {
        SET_FLAG(pl->ob, FLAG_WIZ);
        SET_FLAG(pl->ob, FLAG_WIZPASS);
        SET_MULTI_FLAG(pl->ob, FLAG_FLYING);
        esrv_send_inventory(pl->ob, pl->ob);
        clear_los(pl->ob);
        pl->socket.update_tile = 0; /* force a draw_look() */
        pl->update_los = 1;
    }

    pl->socket.ext_title_flag =1;
    new_draw_info_format( NDI_UNIQUE, 0, pl->ob, "%s mode activated for %s!",
                          mode==GMASTER_MODE_DM ? "DM" : (mode==GMASTER_MODE_GM ?"GM" : "VOL") ,pl->ob->name);
}


/* remove the current gmaster mode
 */
void remove_gmaster_mode(player *pl)
{
    new_draw_info_format(NDI_UNIQUE, 0, pl->ob, "%s mode deactivated.",
        pl->gmaster_mode==GMASTER_MODE_DM ? "DM" : (pl->gmaster_mode==GMASTER_MODE_GM ?"GM" : "VOL"));

    remove_gmaster_list(pl);

    if(pl->gmaster_mode == GMASTER_MODE_DM)
    {
        /* remove the DM power settings */
        CLEAR_FLAG(pl->ob, FLAG_WIZ);
        CLEAR_FLAG(pl->ob, FLAG_WIZPASS);
        CLEAR_MULTI_FLAG(pl->ob, FLAG_FLYING);
        fix_player(pl->ob);
        pl->socket.update_tile = 0;
        esrv_send_inventory(pl->ob, pl->ob);
        pl->update_los = 1;
    }

    pl->socket.ext_title_flag =1;
    pl->gmaster_mode = GMASTER_MODE_NO;
}

/* write back the gmaster file.
 * (triggered after a dm_set add/remove command)
 */
void write_gmaster_file(void)
{
    char    filename[MAX_BUF];
    objectlink *ol;
    FILE   *fp;

    sprintf(filename, "%s/%s", settings.localdir,GMASTER_FILE);
    if ((fp = fopen(filename, "w")) == NULL)
    {
        LOG(llevBug, "BUG: Cannot open %s for writing\n", filename);
        return;
    }
    fprintf(fp, "# GMASTER_FILE (file is changed from server at runtime)\n");
    fprintf(fp, "# entry <name>:*:*:GM will allow player <name> to be GM or VOL\n");

    for(ol = gmaster_list;ol;ol=ol->next)
        fprintf(fp, "%s\n", ol->objlink.gm->entry);

    fclose(fp);
}

/* check the rights of all DM/VOL/GM
 * (triggered after a dm_set add/remove command)
 */
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

    for(ol = gmaster_list_DM;ol;ol=ol_tmp)
    {
        ol_tmp = ol->next;
        if(!check_gmaster_list(CONTR(ol->objlink.ob), GMASTER_MODE_DM))
            remove_gmaster_mode(CONTR(ol->objlink.ob));
    }
}
