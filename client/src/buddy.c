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

	node = (struct buddy_list *) malloc(sizeof(struct buddy_list));
	strcpy(node->name, name);
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

			free(node);
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

	draw_info("\nBUDDY LIST", COLOR_WHITE);
	draw_info("--------------------------", COLOR_WHITE);
	for(node = buddy_list_start;node;i++, node = node->next)
	{
		draw_info(node->name, COLOR_WHITE);
	}

	draw_info_format(COLOR_WHITE, "\n%d name(s) on your list.", i);
}

/* clear the list, free all memory */
void buddy_list_clear(void)
{
	struct buddy_list *node, *tmp;

	for(node = buddy_list_start;node;node = tmp)
	{
		tmp = node->next;
		free(node);
	}
    buddy_list_start=NULL;
}

/* clear the list and load it clean from file */
void buddy_list_load(void)
{
	int i;
	char buf[64];
	char filename[255];
	FILE   *stream;

    sprintf(filename,"settings/%s.buddy.list",cpl.name);
    LOG(LOG_DEBUG,"Trying to open buddy file: %s\n",filename);

	buddy_list_clear();

	if (!(stream = fopen_wrapper(filename, "r")))
		return; /* no list - no buddys - no problem */

	while (fgets(buf, 60, stream) != NULL)
	{
		i = strlen(buf)-1;
		while (isspace(buf[i--]))
			buf[i+1]=0;
		buddy_entry_add(buf);
	}

	fclose(stream);
}

/* save the list to the buddy file. Overwrite it */
void buddy_list_save(void)
{
	struct buddy_list *node;
	char filename[255];

	FILE *stream;

    sprintf(filename,"settings/%s.buddy.list",cpl.name);
    LOG(LOG_DEBUG,"Trying to open buddy file: %s\n",filename);

	if (!(stream = fopen_wrapper(filename, "w")))
		return;

	for(node = buddy_list_start;node;node = node->next)
	{
		fputs(node->name, stream);
		fputs("\n", stream);
	}

	fclose(stream);

}


/* check player <name> is on the buddy list.
 * return TRUE: player is on the buddy list
 */
int buddy_check(char *name)
{
	struct buddy_list *node;

	for(node = buddy_list_start;node;node = node->next)
	{
		/*draw_info_format(COLOR_WHITE, "compare >%s< with >%s<", name, node->name);*/
		if(!stricmp(name, node->name))
			return TRUE;
	}
	return FALSE;
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
			draw_info_format(COLOR_WHITE, "removed %s from buddy list.", cmd);
		}
		else
		{
			buddy_entry_add(cmd);
			draw_info_format(COLOR_WHITE, "added %s to buddy list.", cmd);
		}

		buddy_list_save();
	}
}
