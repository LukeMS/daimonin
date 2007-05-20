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

    The author can be reached via e-mail to info@daimonin.net
*/
#include <include.h>

/* TODO: use the NPC GUI or some other gui of the client to make the ignore list graphical */

#define IGNORE_FILE_NAME "ignore.list"

typedef struct ignore_list
{
    struct ignore_list *next;
    char name[64];
}
_ignore_list;

struct ignore_list *ignore_list_start = NULL;

/* add an entry to the ignore list */
static void ignore_entry_add(char *name)
{
    struct ignore_list *node;

    node = (struct ignore_list *) malloc(sizeof(struct ignore_list));
    strcpy(node->name, name);
    node->next = ignore_list_start;
    ignore_list_start = node;
}

/* remove an entry from the ignore list */
static void ignore_entry_remove(char *name)
{
    struct ignore_list *node, *tmp=NULL;

    for (node = ignore_list_start;node;node = node->next)
    {
        if (!stricmp(name, node->name))
        {
            if (tmp)
                tmp->next = node->next;
            else
                ignore_list_start = node->next;

            free(node);
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

    draw_info("\nIGNORE LIST", COLOR_WHITE);
    draw_info("--------------------------", COLOR_WHITE);
    for (node = ignore_list_start;node;i++, node = node->next)
    {
        draw_info(node->name, COLOR_WHITE);
    }

    draw_info_format(COLOR_WHITE, "\n%d name(s) ignored", i);
}

/* clear the list, free all memory */
void ignore_list_clear(void)
{
    struct ignore_list *node, *tmp;

    for (node = ignore_list_start;node;node = tmp)
    {
        tmp = node->next;
        free(node);
    }
}

/* clear the list and load it clean from file */
void ignore_list_load(void)
{
    int i;
    char buf[64];
    FILE   *stream;

    ignore_list_clear();

    if (!(stream = fopen_wrapper(IGNORE_FILE_NAME, "r")))
        return; /* no list - no ignores - no problem */

    while (fgets(buf, 60, stream) != NULL)
    {
        i = strlen(buf)-1;
        while (isspace(buf[i--]))
            buf[i+1]=0;
        ignore_entry_add(buf);
    }

    fclose(stream);
}

/* save the list to the ignore file. Overwrite it */
void ignore_list_save(void)
{
    struct ignore_list *node;
    FILE *stream;

    if (!(stream = fopen_wrapper(IGNORE_FILE_NAME, "w")))
        return;

    for (node = ignore_list_start;node;node = node->next)
    {
        fputs(node->name, stream);
        fputs("\n", stream);
    }

    fclose(stream);

}


/* check player <name> is on the ignore list.
 * return TRUE: player is on the ignore list
 */
int ignore_check(char *name)
{
    struct ignore_list *node;

    for (node = ignore_list_start;node;node = node->next)
    {
//        draw_info_format(COLOR_WHITE, "compare >%s< with >%s<", name, node->name);
        if (!stricmp(name, node->name))
            return TRUE;
    }
    return FALSE;
}

/* parse a /ignore <cmd> part (without "/ignore " part) */
void ignore_command(char *cmd)
{
    int i;

    /* trim string - remove all white spaces */
    cmd[60]=0;
    while (isspace(*cmd))
        cmd++;
    i = strlen(cmd)-1;
    while (isspace(cmd[i--]))
        cmd[i+1]=0;

    /*LOG(LOG_DEBUG, "IGNORE CMD: >%s<\n", cmd);*/

    if (*cmd == 0) /* pure /ignore command = list */
        ignore_list_show();
    else
    {
        /* syntax: if the name is in the list, remove it.
         * if its new, add it. save the new list then
         */
        *cmd = toupper(*cmd);
        if (ignore_check(cmd) )
        {
            ignore_entry_remove(cmd);
            draw_info_format(COLOR_WHITE, "removed %s from ignore list.", cmd);
        }
        else
        {
            ignore_entry_add(cmd);
            draw_info_format(COLOR_WHITE, "added %s to ignore list.", cmd);
        }

        ignore_list_save();
    }
}
