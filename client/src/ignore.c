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
*/
#include <include.h>

/* TODO: use the NPC GUI or some other gui of the client to make the ignore list graphical */

typedef struct ignore_list
{
    struct ignore_list *next;
    char name[64];
    char type[64];
}
_ignore_list;

struct ignore_list *ignore_list_start = NULL;

/* add an entry to the ignore list */
static void ignore_entry_add(char *name, char *type)
{
    struct ignore_list *node;
    MALLOC(node, sizeof(struct ignore_list));
    sprintf(node->name, "%s", name);
    if (type[0]=='\0')
        node->type[0]='\0';
    else
        strncpy(node->type, type,64);
    node->next = ignore_list_start;
    ignore_list_start = node;
}

/* remove an entry from the ignore list */
static void ignore_entry_remove(char *name, char *type)
{
    struct ignore_list *node, *tmp=NULL;

    for (node = ignore_list_start;node;node = node->next)
    {
        if (!stricmp(name, node->name) && !stricmp(type, node->type))
        {
            if (tmp)
                tmp->next = node->next;
            else
                ignore_list_start = node->next;

            FREE(node);
            return;

        }
        tmp = node;
    }
}

/* show the ignored players as list to the player */
static void ignore_list_show(void)
{
    struct ignore_list *node;
    int i=0;

    textwin_showstring(COLOR_WHITE, "\nIGNORE LIST");
    textwin_showstring(COLOR_WHITE, "--------------------------");
    for (node = ignore_list_start;node;i++, node = node->next)
    {
        textwin_showstring(COLOR_WHITE, "%s.%s", (node->type) ? node->type : "*", node->name);
    }
}

/* clear the list, free all memory */
void ignore_list_clear(void)
{
    struct ignore_list *node, *tmp;

    for (node = ignore_list_start;node;node = tmp)
    {
        tmp = node->next;
        FREE(node);
    }
    ignore_list_start=NULL;
}

/* clear the list and load it clean from file */
void ignore_list_load(void)
{
    char         buf[SMALL_BUF];
    PHYSFS_File *handle;

    sprintf(buf, "%s/%s.%s", DIR_SETTINGS, cpl.name, FILE_IGNORE);

    if (!(handle = load_client_file(buf)))
    {
        return;
    }

    ignore_list_clear();

    while (PHYSFS_readString(handle, buf, sizeof(buf)) > 0)
    {
        char *cp;

        if (buf[0] == '#')
        {
            continue;
        }
        else if (!(cp = strchr(buf, ' ')))
        {
            LOG(LOG_ERROR, "Ignoring malformed line >%s<!\n", buf);

            continue;
        }

        *cp++ = '\0';
        ignore_entry_add(buf, (*cp == '*') ? "" : cp);
    }

    PHYSFS_close(handle);
}

/* save the list to the ignore file. Overwrite it */
void ignore_list_save(void)
{
    char                buf[SMALL_BUF];
    PHYSFS_File        *handle;
    struct ignore_list *il;

    sprintf(buf, "%s/%s.%s", DIR_SETTINGS, cpl.name, FILE_IGNORE);

    if (!(handle = save_client_file(buf)))
    {
        return;
    }

    for (il = ignore_list_start; il; il = il->next)
    {
        sprintf(buf, "%s %s\n", il->name, (!il->type[0]) ? "*" : il->type);
        PHYSFS_writeString(handle, buf);
    }

    PHYSFS_close(handle);
}

/* check player <name> is on the ignore list.
 * return 1: player is on the ignore list
 */
int ignore_check(char *name, char *type)
{
    struct ignore_list *node;

    for (node = ignore_list_start;node;node = node->next)
    {
//        textwin_showstring(COLOR_WHITE, "compare >%s< with >%s< (%s with %s)", name, node->name,type, node->type);
        if (!stricmp(name, node->name) && ((!stricmp(type,node->type)) || (!node->type[0])))
            return 1;
    }
    return 0;
}

/* parse a /ignore <cmd> part (without "/ignore " part) */
void ignore_command(char *cmd)
{
    int i;
    char name[64];
    char type[64];
    name[0]='\0';
    type[0]='\0';

    /* trim string - remove all white spaces */
    cmd[120]=0;
    while (isspace(*cmd))
        cmd++;
    i = strlen(cmd)-1;
    while (isspace(cmd[i--]))
        cmd[i+1]=0;

    LOG(LOG_DEBUG, "IGNORE CMD: >%s<\n", cmd);

    if (*cmd == 0) /* pure /ignore command = list */
        ignore_list_show();
    else
    {
        /* syntax: if the name is in the list, remove it.
         * if its new, add it. save the new list then
         */
        if (sscanf(cmd,"%s %s",name, type)==EOF)
        {
            textwin_showstring(COLOR_WHITE,"Syntax: /ignore <name> <'channel'>");
            textwin_showstring(COLOR_WHITE,"Syntax: /ignore <name> *  for all 'channels'");
            textwin_showstring(COLOR_WHITE,"channel can be somthing like: 'say', 'shout', 'tell', 'emote'");
        }
        else if ((name[0]=='\0') || (type[0]=='\0'))
        {
            textwin_showstring(COLOR_WHITE,"Syntax: /ignore <name> <'channel'>");
            textwin_showstring(COLOR_WHITE,"Syntax: /ignore <name> *  for all 'channels'");
            textwin_showstring(COLOR_WHITE,"channel can be somthing like: 'say', 'shout', 'tell', 'emote'");

        }
        else
        {
            i=0;
            while (name[i]!='\0')
            {
                name[i]=tolower(name[i]);
                i++;
            }
            i=0;
            while (type[i]!='\0')
            {
                type[i]=tolower(type[i]);
                i++;
            }

            name[0] = tolower(name[0]);
            if (type[0]=='*')
                type[0]='\0';
            if (ignore_check(name, type) )
            {
                ignore_entry_remove(name, type);
                textwin_showstring(COLOR_WHITE, "removed %s (%s) from ignore list.", name, type);
            }
            else
            {
                ignore_entry_add(name, type);
                textwin_showstring(COLOR_WHITE, "added %s (%s) to ignore list.", name, type);
            }

            ignore_list_save();
        }
    }
}
