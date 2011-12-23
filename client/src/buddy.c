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

typedef struct buddy_list
{
	struct buddy_list *next;
	char name[64];
} _buddy_list;

struct buddy_list *buddy_list_start = NULL;

/* add an entry to the buddy list */
static void buddy_entry_add(char *name)
{
	struct buddy_list *node;

	MALLOC(node, sizeof(struct buddy_list));
	sprintf(node->name, "%s", name);
	node->next = buddy_list_start;
	buddy_list_start = node;
}

/* remove an entry from the buddy list */
static void buddy_entry_remove(char *name)
{
	struct buddy_list *node, *tmp=NULL;

	for(node = buddy_list_start;node;node = node->next)
	{
		if(!stricmp(name, node->name))
		{
			if(tmp)
				tmp->next = node->next;
			else
				buddy_list_start = node->next;

			FREE(node);
			return;

		}
		tmp = node;
	}
}

/* show the buddy-players as list to the player */
static void buddy_list_show(void)
{
	struct buddy_list *node;
	int i=0;

	textwin_show_string(0, NDI_COLR_WHITE, "\nBUDDY LIST");
	textwin_show_string(0, NDI_COLR_WHITE, "--------------------------");
	for(node = buddy_list_start;node;i++, node = node->next)
	{
		textwin_show_string(0, NDI_COLR_WHITE, "%s", node->name);
	}

	textwin_show_string(0, NDI_COLR_WHITE, "\n%d name(s) on your list.", i);
}

/* clear the list, free all memory */
void buddy_list_clear(void)
{
	struct buddy_list *node, *tmp;

	for(node = buddy_list_start;node;node = tmp)
	{
		tmp = node->next;
		FREE(node);
	}
    buddy_list_start=NULL;
}

/* clear the list and load it clean from file */
void buddy_list_load(void)
{
    char         buf[SMALL_BUF];
    PHYSFS_File *handle;

    sprintf(buf, "%s/%s.%s", DIR_SETTINGS, cpl.name, FILE_BUDDY);

    if (!(handle = load_client_file(buf)))
    {
        return;
    }

    buddy_list_clear();

    while (PHYSFS_readString(handle, buf, sizeof(buf)) > 0)
    {
        if (buf[0] == '#')
        {
            continue;
        }
        else if (!player_name_valid(buf))
        {
            LOG(LOG_ERROR, "Ignoring malformed line >%s<!\n", buf);

            continue;
        }

        buddy_entry_add(buf);
    }

    PHYSFS_close(handle);
}

/* save the list to the buddy file. Overwrite it */
void buddy_list_save(void)
{
    char                buf[SMALL_BUF];
    PHYSFS_File        *handle;
    struct buddy_list  *bl;

    sprintf(buf, "%s/%s.%s", DIR_SETTINGS, cpl.name, FILE_BUDDY);

    if (!(handle = save_client_file(buf)))
    {
        return;
    }

    for (bl = buddy_list_start; bl; bl = bl->next)
    {
        sprintf(buf, "%s\n", bl->name);
        PHYSFS_writeString(handle, buf);
    }

    PHYSFS_close(handle);
}

/* check player <name> is on the buddy list.
 * return 1: player is on the buddy list
 */
int buddy_check(char *name)
{
	struct buddy_list *node;

	for(node = buddy_list_start;node;node = node->next)
	{
		/*textwin_show_string(0, NDI_COLR_WHITE, "compare >%s< with >%s<", name, node->name);*/
		if(!stricmp(name, node->name))
			return 1;
	}
	return 0;
}

/* parse a /buddy <cmd> part (without "/buddy " part) */
void buddy_command(char *cmd)
{
	int i;

	/* trim string - remove all white spaces */
	cmd[60]=0;
	while (isspace(*cmd))
		cmd++;
	i = strlen(cmd)-1;
	while (isspace(cmd[i--]))
		cmd[i+1]=0;

	/*LOG(LOG_DEBUG, "buddy CMD: >%s<\n", cmd);*/

	if(*cmd == 0) /* pure /buddy command = list */
		buddy_list_show();
	else
	{
		/* syntax: if the name is in the list, remove it.
		 * if its new, add it. save the new list then
		 */
		*cmd = toupper(*cmd);
		if(buddy_check(cmd) )
		{
			buddy_entry_remove(cmd);
			textwin_show_string(0, NDI_COLR_WHITE, "removed %s from buddy list.", cmd);
		}
		else
		{
			buddy_entry_add(cmd);
			textwin_show_string(0, NDI_COLR_WHITE, "added %s to buddy list.", cmd);
		}

		buddy_list_save();
	}
}
