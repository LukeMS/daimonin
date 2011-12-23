/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003-2006 Michael Toennies

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

    kerbholz.c - Kill Stats by Alderan
    code mainly c&p from ingnore.c....:)

*/
#include <include.h>

/* TODO: use the NPC GUI or some other gui of the client to make the kill list graphical */

struct kills_list *kills_list_start = NULL;

/* add an entry to the kill list
 * return 1 if new kill (mob with tha name killed for the first time)
 * 2 if 'old' kill
 */
int addKill(char *name)
{
    struct kills_list *node=NULL;
    // Check for already existing entry
    node=kills_list_start;
    while (node)
    {
        if (!strnicmp(node->name, name, strlen(name)))
            break;
        node=node->next;
    }
    if (!node) //Not found? lets add a new entry...
    {
        addNewKill(name, 1, 1);
        kill_list_save();
        return 1;
    }
    else                            //increment the counters...
    {
        node->kills++;
        node->session++;
        kill_list_save();
        return 2;
    }

}

/* creates a new kill-entry in the sorted kills-list
 * we need the second parameter to use this function also for loading
 */
void addNewKill(char *name, unsigned int kills, unsigned int session)
{
    /* We make a sorted list, better for listing */
    struct kills_list *node;
    struct kills_list *ptr, *ptr1;

    MALLOC(node, sizeof(struct kills_list));
    node->kills = kills;
    node->session=session;
    node->next=NULL;
    strncpy(node->name,name,64);

    /* First Element of List? */
    if(kills_list_start==NULL)
    {
        kills_list_start=node;
        node->next=NULL;
    }
    else
    {
        ptr=ptr1=kills_list_start;
        while(ptr != NULL && (stricmp(ptr->name,name) < 0))
        {
            ptr1=ptr;
            ptr=ptr->next;
        }
        /* If node 'smaller' as first element, insert at beginning */
        if(ptr==kills_list_start)
        {
            kills_list_start=node;
            node->next=ptr;
        }
        /* last position, or in the middle ptr1 holds the forerunner */
        else
        {
            ptr1->next=node;
            node->next=ptr;
        }
    } //Ende else

    return;
}

/* show the killed monsters as list to the player
 * TODO: make this list better formatted, with proportional font a little difficult
 * or use some gui...
 */
void kill_list_show(int type)
{
    struct kills_list *node = kills_list_start;
    int                i = 0;
    /* trim string - remove all white spaces */

    textwin_show_string(0, NDI_COLR_WHITE,
                       "\n       KILLS LIST %s\n"\
                       "------------------------------------------",
                       (type == 1) ? "SESSION" : "TOTAL");

    for (; node; i++, node = node->next)
    {
        textwin_show_string(0, NDI_COLR_WHITE, "%4d/%12d ... %s",
                           node->session, node->kills, node->name);
    }

    textwin_show_string(0, NDI_COLR_WHITE, "\n%d different monsters killed.", i);
}

/* clear the list, free all memory */
void kill_list_clear(void)
{
    struct kills_list *node, *tmp;

    for(node = kills_list_start;node;node = tmp)
    {
        tmp = node->next;
        FREE(node);
    }
    kills_list_start=NULL;
}

/* clear the list and load it clean from file */
void kill_list_load(void)
{
    char         buf[SMALL_BUF];
    PHYSFS_File *handle;

    sprintf(buf, "%s/%s.%s", DIR_SETTINGS, cpl.name, FILE_KILL);

    if (!(handle = load_client_file(buf)))
    {
        return;
    }

    kill_list_clear();

    while (PHYSFS_readString(handle, buf, sizeof(buf)) > 0)
    {
        char *cp;

        if (buf[0] == '#')
        {
            continue;
        }
        else if (!(cp = strchr(buf, '|')))
        {
            /* Unfortunately pre-0.10.6 clients used the standard comment
             * introducer as a separator! */
            if (!(cp = strchr(buf, '#')))
            {
                LOG(LOG_ERROR, "Ignoring malformed line >%s<!\n", buf);

                continue;
            }
        }

        *cp++ = '\0';
        addNewKill(buf, atoi(cp), 0);
    }

    PHYSFS_close(handle);
}

/* save the list to the kill file. Overwrite it */
void kill_list_save(void)
{
    char               buf[SMALL_BUF];
    PHYSFS_File       *handle;
    struct kills_list *kl;

    sprintf(buf, "%s/%s.%s", DIR_SETTINGS, cpl.name, FILE_KILL);

    if (!(handle = save_client_file(buf)))
    {
        return;
    }

    for (kl = kills_list_start; kl; kl = kl->next)
    {
        sprintf(buf, "%s|%d\n", kl->name, kl->kills);
        PHYSFS_writeString(handle, buf);
    }

    PHYSFS_close(handle);
}

/* parse a /kill <cmd> part (without "/kill " part) */
void kill_command(char *cmd)
{
    int i;

    /* trim string - remove all white spaces */
    cmd[60]=0;
    while (isspace(*cmd))
        cmd++;
    i = strlen(cmd)-1;
    while (isspace(cmd[i--]))
        cmd[i+1]=0;


    if(*cmd == 0) /* pure /kill command = list */
        kill_list_show(1);
    else if (!stricmp(cmd, "total"))
    {
        kill_list_show(2);
    }
    else if (!stricmp(cmd, "reset"))
    {
        kill_list_clear();
        kill_list_save();
    }
}
_kills_list *getKillEntry(char *name)
{
    struct kills_list *node=NULL;
    node=kills_list_start;
    while (node)
    {
        if (!strnicmp(node->name, name, strlen(name)))
            break;
        node=node->next;
    }
    return node;

}

