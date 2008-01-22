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

    2007-01-14 Alderan:
    chatfilter - customizable incoming unwanted f*-word filter
    Mostly c&p code from ignore
*/
#include <include.h>

/* TODO: use the NPC GUI or some other gui of the client to make the chatfilter list graphical */

typedef struct chatfilter_list
{
    struct chatfilter_list *next;
    char word[64];
}
_chatfilter_list;

struct chatfilter_list *chatfilter_list_start = NULL;

char replacechar='*';

/* add an word to the chatfilter list */
static void chatfilter_entry_add(char *word)
{
    struct chatfilter_list *node;

    node = (struct chatfilter_list *) malloc(sizeof(struct chatfilter_list));
    strcpy(node->word, word);
    node->next = chatfilter_list_start;
    chatfilter_list_start = node;
}

/* remove an word from the chatfilter list */
static void chatfilter_entry_remove(char *word)
{
    struct chatfilter_list *node, *tmp=NULL;

    for (node = chatfilter_list_start;node;node = node->next)
    {
        if (!stricmp(word, node->word))
        {
            if (tmp)
                tmp->next = node->next;
            else
                chatfilter_list_start = node->next;

            free(node);
            return;

        }
        tmp = node;
    }
}

/* show the ignored words as list to the player */
static void chatfilter_list_show(void)
{
    struct chatfilter_list *node;
    int i=0;

    draw_info("\nCHATFILTER LIST", COLOR_WHITE);
    draw_info("--------------------------", COLOR_WHITE);
    for (node = chatfilter_list_start;node;i++, node = node->next)
    {
        draw_info(node->word, COLOR_WHITE);
    }
    draw_info_format(COLOR_WHITE, "\n%d word(s) ignored", i);
    draw_info("HELP: see '/cfilter ?'",COLOR_WHITE);
}

/* clear the list, free all memory */
void chatfilter_list_clear(void)
{
    struct chatfilter_list *node, *tmp;

    for (node = chatfilter_list_start;node;node = tmp)
    {
        tmp = node->next;
        free(node);
    }
    chatfilter_list_start=NULL;
}

/* clear the list and load it clean from file */
void chatfilter_list_load(void)
{
    int i;
    char buf[64];
	char filename[255];
    FILE   *stream;

    sprintf(filename,"settings/%s.cfilter.list",cpl.name);
    LOG(LOG_DEBUG,"Trying to open cfilter file: %s\n",filename);

    chatfilter_list_clear();

    if (!(stream = fopen_wrapper(filename, "r")))
        return; /* no list - no words - no problem */

    /* first line is replacementchar */
    if (fgets(buf, 60, stream) != NULL)
    {
        replacechar=buf[0];
    }
    while (fgets(buf, 60, stream) != NULL)
    {
        i = strlen(buf)-1;
        while (isspace(buf[i--]))
            buf[i+1]=0;
        chatfilter_entry_add(buf);
    }

    fclose(stream);
}

/* save the list to the chatfilterlist file. Overwrite it */
void chatfilter_list_save(void)
{
    struct chatfilter_list *node;
    char filename[255];
    FILE *stream;

    sprintf(filename,"settings/%s.cfilter.list",cpl.name);
    LOG(LOG_DEBUG,"Trying to open cfilter file: %s\n",filename);

    if (!(stream = fopen_wrapper(filename, "w")))
        return;

    fputc(replacechar,stream);
    fputs(" <- Replacement Char, change if you want.\n",stream);

    for (node = chatfilter_list_start;node;node = node->next)
    {
        fputs(node->word, stream);
        fputs("\n", stream);
    }

    fclose(stream);

}

/* replace all f*words with replacechar
 * TODO: make in configurable to replace only whole words, or any occurence
 */

void chatfilter_filter(char *msg)
{
    struct chatfilter_list *node;
    char *stemp;
    int i;
    char buf[1024];

    strncpy(buf,msg,1024); /* we need a lowercase string as reference */

    for (i=0;i<(int) strlen(buf);i++)
    {
        buf[i]=tolower(buf[i]);
    }

    for (node = chatfilter_list_start;node;node = node->next)
    {
        while ((stemp=strstr(buf,node->word))!=NULL)
        {
            for (i=0;i<(int)strlen(node->word);i++)
            {
                msg[(stemp-buf)+i]=replacechar; /* replace with replacechar */
                stemp[i]=replacechar;           /* reference string must also replaced to prevend endless loop */
            }
        }
    }
}

/* checks if an word is already on the list
 * returns TRUE if in list
 */
int chatfilter_check(char *word)
{
    struct chatfilter_list *node;

    for (node = chatfilter_list_start;node;node = node->next)
    {
        if (!stricmp(word, node->word))
            return TRUE;
    }
    return FALSE;
}


/* parse a /cfilter <cmd> part (without "/ignore " part) */
void chatfilter_command(char *cmd)
{
    int i;

    /* trim string - remove all white spaces */
    cmd[60]=0;
    while (isspace(*cmd))
        cmd++;
    i = strlen(cmd)-1;
    while (isspace(cmd[i--]))
        cmd[i+1]=0;

//    LOG(LOG_DEBUG, "CHATFILTER CMD: >%s<\n", cmd);

    if (*cmd == 0) /* pure /cfilter command = list */
        chatfilter_list_show();
    else if (cmd[0]=='+')
    {
        options.chatfilter=TRUE;
        draw_info("Chatfilter enabled.",COLOR_WHITE);
        save_options_dat();
    }
    else if (cmd[0]=='-')
    {
        options.chatfilter=FALSE;
        draw_info("Chatfilter disabled.",COLOR_WHITE);
        save_options_dat();
    }
    else if (cmd[0]=='?')
    {
        draw_info("HELP:",COLOR_WHITE);
        draw_info("'/cfilter' - shows list of filtered words.",COLOR_WHITE);
        draw_info("'/cfilter <word>' - adds word to list, or if its on list remove it from list.",COLOR_WHITE);
        draw_info("'/cfilter [+|-]' - enables or disables chatfiltering.",COLOR_WHITE);
        draw_info("'/cfilter !<char>' - sets replcement character.",COLOR_WHITE);
    }
    else if ((cmd[0]=='!') && (cmd[1]!=0))
    {
        replacechar=cmd[1];
        draw_info_format(COLOR_WHITE,"Replacement character changed to '%c'.",replacechar);
        chatfilter_list_save();
    }
    else
    {
        /* syntax: if the word is in the list, remove it.
         * if its new, add it. save the new list then
         */
        for (i=0;i<(int)strlen(cmd);i++)
        {
            cmd[i]=tolower(cmd[i]);
        }
        if (chatfilter_check(cmd))
        {
            chatfilter_entry_remove(cmd);
            draw_info_format(COLOR_WHITE, "Removed >%s< from chatfilter list.", cmd);
        }
        else
        {
            chatfilter_entry_add(cmd);
            draw_info_format(COLOR_WHITE, "Added >%s< to chatfilter list.", cmd);
        }
        chatfilter_list_save();
    }
}
