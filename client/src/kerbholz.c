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

    node = (struct kills_list *) malloc(sizeof(struct kills_list));
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
    struct kills_list *node;
    int i=0;
    char buf[256];
    /* trim string - remove all white spaces */

    switch (type)
    {
    case 1:
        draw_info("\n       KILLS LIST SESSION", COLOR_WHITE);
        draw_info("-------------------------------------------", COLOR_WHITE);
        for(node = kills_list_start;node;i++, node = node->next)
        {
            if (node->session>0)
            {
                sprintf(buf, "%4d/%12d ... %s", node->session, node->kills,node->name);
                draw_info(buf, COLOR_WHITE);
            }
            else
                i--;
        }
        draw_info_format(COLOR_WHITE, "\n%d different monster this session killed.", i);
    break;

    case 2:
        draw_info("\n       KILLS LIST TOTAL", COLOR_WHITE);
        draw_info("-------------------------------------------", COLOR_WHITE);
        for(node = kills_list_start;node;i++, node = node->next)
        {
            sprintf(buf, "%4d/%12d ... %s", node->session, node->kills,node->name);
            draw_info(buf, COLOR_WHITE);
        }
        draw_info_format(COLOR_WHITE, "\n%d different monster at all killed.", i);
    break;
    }
}

/* clear the list, free all memory */
void kill_list_clear(void)
{
    struct kills_list *node, *tmp;

    for(node = kills_list_start;node;node = tmp)
    {
        tmp = node->next;
        free(node);
    }
    kills_list_start=NULL;
}

/* clear the list and load it clean from file */
void kill_list_load(void)
{
    char buf[128];
	char filename[255];
	FILE   *stream;
	unsigned int kills;
	char *name;
	char *kill;

    sprintf(filename,"settings/%s.kills.list",cpl.name);
    LOG(LOG_DEBUG,"Trying to open kill file: %s\n",filename);


    kill_list_clear();

    if (!(stream = fopen_wrapper(filename, "r")))
        return; /* no list - no kills - no problem */

    while (fgets(buf, 128, stream) != NULL)
    {
        name=buf;
        if (!(kill=strchr(buf,'#'))) continue;
        kill[0]='\0';
        kill++;
        kills=atoi(kill);
        addNewKill(name, kills, 0);
    }

    fclose(stream);
}

/* save the list to the kill file. Overwrite it */
void kill_list_save(void)
{
    struct kills_list *node;
    FILE *stream;
    char buf[512];
    char filename[255];

    sprintf(filename,"settings/%s.kills.list",cpl.name);
    LOG(LOG_DEBUG,"Trying to open kill file: %s\n",filename);

    if (!(stream = fopen_wrapper(filename, "w")))
        return;

    for(node = kills_list_start;node;node = node->next)
    {
        sprintf(buf,"%s#%d\n",node->name, node->kills);
        fputs(buf, stream);
    }

    fclose(stream);

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

