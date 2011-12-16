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

    MALLOC(node, sizeof(struct chatfilter_list));
    sprintf(node->word, "%s", word);
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

            FREE(node);
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

    textwin_showstring(0, NDI_COLR_WHITE, "\nCHATFILTER LIST");
    textwin_showstring(0, NDI_COLR_WHITE, "--------------------------");
    for (node = chatfilter_list_start;node;i++, node = node->next)
    {
        textwin_showstring(0, NDI_COLR_WHITE, "%s", node->word);
    }
    textwin_showstring(0, NDI_COLR_WHITE, "\n%d word(s) ignored\nHELP: see '/cfilter ?'",
                       i);
}

/* clear the list, free all memory */
void chatfilter_list_clear(void)
{
    struct chatfilter_list *node, *tmp;

    for (node = chatfilter_list_start;node;node = tmp)
    {
        tmp = node->next;
        FREE(node);
    }
    chatfilter_list_start=NULL;
}

/* clear the list and load it clean from file */
void chatfilter_list_load(void)
{
    char         buf[SMALL_BUF];
    PHYSFS_File *handle;

    sprintf(buf, "%s/%s.%s", DIR_SETTINGS, cpl.name, FILE_CHATFILTER);

    if (!(handle = load_client_file(buf)))
    {
        return;
    }

    chatfilter_list_clear();

    PHYSFS_readString(handle, buf, sizeof(buf));
    replacechar = buf[0];

    while (PHYSFS_readString(handle, buf, sizeof(buf)) > 0)
    {
        if (buf[0] == '#')
        {
            continue;
        }

        /* Strangely, pre-0.10.6 clients would chop multi-word lines off at
         * the last space -- so 'this is a bad phrase' would be censored to
         * '************* phrase'. 0.10.6 censors the whole phrase. */
        chatfilter_entry_add(buf);
    }

    PHYSFS_close(handle);
}

/* save the list to the chatfilterlist file. Overwrite it */
void chatfilter_list_save(void)
{
    char                    buf[SMALL_BUF];
    PHYSFS_File            *handle;
    struct chatfilter_list *cl;

    sprintf(buf, "%s/%s.%s", DIR_SETTINGS, cpl.name, FILE_CHATFILTER);

    if (!(handle = save_client_file(buf)))
    {
        return;
    }

    sprintf(buf, "%c # Replacement Char, change if you want.\n", replacechar);
    PHYSFS_writeString(handle, buf);

    for (cl = chatfilter_list_start; cl; cl = cl->next)
    {
        sprintf(buf, "%s\n", cl->word);
        PHYSFS_writeString(handle, buf);
    }

    PHYSFS_close(handle);
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
 * returns 1 if in list
 */
int chatfilter_check(char *word)
{
    struct chatfilter_list *node;

    for (node = chatfilter_list_start;node;node = node->next)
    {
        if (!stricmp(word, node->word))
            return 1;
    }
    return 0;
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
        options.chatfilter=1;
        textwin_showstring(0, NDI_COLR_WHITE, "Chatfilter enabled.");
        save_options_dat();
    }
    else if (cmd[0]=='-')
    {
        options.chatfilter=0;
        textwin_showstring(0, NDI_COLR_WHITE, "Chatfilter disabled.");
        save_options_dat();
    }
    else if (cmd[0]=='?')
    {
        textwin_showstring(0, NDI_COLR_WHITE,
                           "HELP:\n"\
                           "'/cfilter' - shows list of filtered words.\n"\
                           "'/cfilter <word>' - adds word to list, or if its on list remove it from list.\n"\
                           "'/cfilter [+|-]' - enables or disables chatfiltering.\n"\
                           "'/cfilter !<char>' - sets replcement character.");
    }
    else if ((cmd[0]=='!') && (cmd[1]!=0))
    {
        replacechar=cmd[1];
        textwin_showstring(0, NDI_COLR_WHITE,"Replacement character changed to '%c'.",replacechar);
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
            textwin_showstring(0, NDI_COLR_WHITE, "Removed >%s< from chatfilter list.", cmd);
        }
        else
        {
            chatfilter_entry_add(cmd);
            textwin_showstring(0, NDI_COLR_WHITE, "Added >%s< to chatfilter list.", cmd);
        }
        chatfilter_list_save();
    }
}
