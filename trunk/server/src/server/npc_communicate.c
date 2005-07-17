/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001 Michael Toennies

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

#include <global.h>

/*
 * When parsing a message-struct, the msglang struct is used
 * to contain the values.
 * This struct will be expanded as new features are added.
 * When things are stable, it will be parsed only once.
 */

typedef struct _msglang
{
    char           **messages;  /* An array of messages */
    char           ***keywords; /* For each message, an array of strings to match */
} msglang;

/*
 * Non-scripted chat functions
 */

static void free_messages(msglang *msgs)
{
    int messages, keywords;

    if (!msgs)
        return;
    for (messages = 0; msgs->messages[messages]; messages++)
    {
        if (msgs->keywords[messages])
        {
            for (keywords = 0; msgs->keywords[messages][keywords]; keywords++)
                free(msgs->keywords[messages][keywords]);
            free(msgs->keywords[messages]);
        }
        free(msgs->messages[messages]);
    }
    free(msgs->messages);
    free(msgs->keywords);
    free(msgs);
}

static msglang * parse_message(const char *msg)
{
    msglang    *msgs;
    int         nrofmsgs, msgnr, i;
    char       *cp, *line, *last, *tmp;
    char       *buf = strdup_local(msg);

    /* First find out how many messages there are.  A @ for each. */
    for (nrofmsgs = 0, cp = buf; *cp; cp++)
        if (*cp == '@')
            nrofmsgs++;
    if (!nrofmsgs)
    {
        free(buf);
        return NULL;
    }

    msgs = (msglang *) malloc(sizeof(msglang));
    msgs->messages = (char * *) malloc(sizeof(char *) * (nrofmsgs + 1));
    msgs->keywords = (char * **) malloc(sizeof(char * *) * (nrofmsgs + 1));
    for (i = 0; i <= nrofmsgs; i++)
    {
        msgs->messages[i] = NULL;
        msgs->keywords[i] = NULL;
    }

    for (last = NULL, cp = buf, msgnr = 0; *cp; cp++)
        if (*cp == '@')
        {
            int nrofkeywords, keywordnr;
            *cp = '\0'; cp++;
            if (last != NULL)
            {
                msgs->messages[msgnr++] = strdup_local(last);
                tmp = msgs->messages[msgnr - 1];
                for (i = (int) strlen(tmp); i; i--)
                {
                    if (*(tmp + i) && *(tmp + i) != 0x0a && *(tmp + i) != 0x0d)
                        break;
                    *(tmp + i) = 0;
                }
            }
            if (strncmp(cp, "match", 5))
            {
                LOG(llevBug, "BUG: parse_message(): Unsupported command in message.\n");
                free(buf);
                return NULL;
            }
            for (line = cp + 6, nrofkeywords = 0; *line != '\n' && *line; line++)
                if (*line == '|')
                    nrofkeywords++;
            if (line > cp + 6)
                nrofkeywords++;
            if (nrofkeywords < 1)
            {
                LOG(llevBug, "BUG: parse_message():Too few keywords in message.\n");
                free(buf);
                free_messages(msgs);
                return NULL;
            }
            msgs->keywords[msgnr] = (char * *) malloc(sizeof(char * *) * (nrofkeywords + 1));
            msgs->keywords[msgnr][nrofkeywords] = NULL;
            last = cp + 6;
            cp = strchr(cp, '\n');
            if (cp != NULL)
                cp++;
            for (line = last, keywordnr = 0; line < cp && *line; line++)
                if (*line == '\n' || *line == '|')
                {
                    *line = '\0';
                    if (last != line)
                        msgs->keywords[msgnr][keywordnr++] = strdup_local(last);
                    else
                    {
                        if (keywordnr < nrofkeywords)
                        {
                            /* Whoops, Either got || or |\n in @match. Not good */
                            msgs->keywords[msgnr][keywordnr++] = strdup_local("xxxx");
                            /* We need to set the string to something sensible to    *
                                * prevent crashes later. Unfortunately, we can't set to *
                                * NULL, as that's used to terminate the for loop in     *
                                * talk_to_npc.  Using xxxx should also help map         *
                                * developers track down the problem cases.              */
                            LOG(llevBug, "BUG: parse_message(): Tried to set a zero length message in parse_message\n");
                            /* I think this is a error worth reporting at a reasonably *
                                * high level. When logging gets redone, this should       *
                                * be something like MAP_ERROR, or whatever gets put in    *
                                * place. */
                            if (keywordnr > 1)
                                           /* This is purely addtional information, should *
                                            * only be gieb if asked */
                                LOG(llevDebug, "Msgnr %d, after keyword %s\n", msgnr + 1,
                                    msgs->keywords[msgnr][keywordnr - 2]);
                            else
                                LOG(llevDebug, "Msgnr %d, first keyword\n", msgnr + 1);
                        }
                    }
                    last = line + 1;
                }
            /*
               * your eyes aren't decieving you, this is code repetition.  However,
               * the above code doesn't catch the case where line<cp going into the
               * for loop, skipping the above code completely, and leaving undefined
                    * data in the keywords array.  This patches it up and solves a crash
               * bug.  garbled 2001-10-20
               */
            if (keywordnr < nrofkeywords)
            {
                LOG(llevBug, "BUG: parse_message(): Map developer screwed up match statement" " in parse_message\n");
                if (keywordnr > 1)
                               /* This is purely addtional information, should *
                                * only be gieb if asked */
                    LOG(llevDebug, "Msgnr %d, after keyword %s\n", msgnr + 1, msgs->keywords[msgnr][keywordnr - 2]);
                else
                    LOG(llevDebug, "Msgnr %d, first keyword\n", msgnr + 1);
#if 0
/* Removed this block - according to the compiler, this has no effect,
 * and looking at the if statement above, the certainly appears to be the
 * case.
 */
          for(keywordnr; keywordnr <= nrofkeywords; keywordnr++)
              msgs->keywords[msgnr][keywordnr] = strdup_local("xxxx");
#endif
            }
            last = cp;
        }
    if (last != NULL)
        msgs->messages[msgnr++] = strdup_local(last);

    tmp = msgs->messages[msgnr - 1];
    for (i = (int) strlen(tmp); i; i--)
    {
        if (*(tmp + i) && *(tmp + i) != 0x0a && *(tmp + i) != 0x0d)
            break;
        *(tmp + i) = 0;
    }
    free(buf);
    return msgs;
}

/* i changed this... This function is not to understimate when player talk alot
 * in areas which alot if map objects... This is one of this little extra cpu eaters
 * which adds cput time here and there.
 * i added P_MAGIC_EAR as map flag - later we should use a chained list in the map headers
 * perhaps. I also removed the npcs from the map search and use the target system.
 * This IS needed because in alot of cases in the past you was not able to target the
 * npc you want - if the search routine find another npc first, the other was silenced.
 * MT-2003
 */
void communicate(object *op, char *txt)
{
    object     *npc;
    mapstruct  *m;
    int         i, xt, yt;

    char        buf[HUGE_BUF];

    if (!txt)
        return;


    /* with target, only player can talk to npc... for npc to npc talk we need so or so a script,
    * and there we have then to add the extra interface.
    */

    /* thats the whisper code - i will add a /whisper for it and remove it from here */
    /*
    if(op->type == PLAYER)
    {
        if(op->contr->target_object && op->contr->target_object_count==op->contr->target_object->count)
        {
            if(op->contr->target_object->type == PLAYER)
            {
                if(op != op->contr->target_object)
                {
                    sprintf(buf, "%s whispers to you: ",query_name(op));
                    strncat(buf, txt, MAX_BUF - strlen(buf)-1);
                    buf[MAX_BUF-1]=0;
                    new_draw_info(NDI_WHITE|NDI_GREEN,0, op->contr->target_object, buf);
                    sprintf(buf, "you whispers to %s: ",query_name(op->contr->target_object));
                    strncat(buf, txt, MAX_BUF - strlen(buf)-1);
                    buf[MAX_BUF-1]=0;
                    new_draw_info(NDI_WHITE|NDI_GREEN,0, op, buf);
                    sprintf(buf, "%s whispers something to %s.",query_name(op),query_name(op->contr->target_object));
                    new_info_map_except2(NDI_WHITE,op->map, op, op->contr->target_object, buf);
                    if(op->contr->target_object->map && op->contr->target_object->map != op->map)
                        new_info_map_except2(NDI_WHITE,op->contr->target_object->map, op, op->contr->target_object, buf);
                }
                else
                {
                    sprintf(buf, "%s says: ",query_name(op));
                    strncat(buf, txt, MAX_BUF - strlen(buf)-1);
                    buf[MAX_BUF-1]=0;
                    new_info_map(NDI_WHITE,op->map, buf);
                }
            }
            else
            {
                sprintf(buf, "%s says to %s: ",query_name(op),query_name(op->contr->target_object));
                strncat(buf, txt, MAX_BUF - strlen(buf)-1);
                buf[MAX_BUF-1]=0;
                new_info_map_except(NDI_WHITE,op->map, op, buf);
                if(op->contr->target_object->map && op->contr->target_object->map != op->map)
                    new_info_map_except(NDI_WHITE,op->contr->target_object->map, op, buf);
                sprintf(buf, "you say to %s: ",query_name(op->contr->target_object));
                strncat(buf, txt, MAX_BUF - strlen(buf)-1);
                buf[MAX_BUF-1]=0;
                new_draw_info(NDI_WHITE,0, op, buf);
                talk_to_npc(op, op->contr->target_object,txt);
            }
        }
        else
        {
            sprintf(buf, "%s says: ",query_name(op));
            strncat(buf, txt, MAX_BUF - strlen(buf)-1);
            buf[MAX_BUF-1]=0;
            new_info_map(NDI_WHITE,op->map, buf);
        }
    }
    else
    {
        sprintf(buf, "%s says: ",query_name(op));
        strncat(buf, txt, MAX_BUF - strlen(buf)-1);
        buf[MAX_BUF-1]=0;
        new_info_map(NDI_WHITE,op->map, buf);
    }
    */

    /* npc chars can hook in here with
     * monster.Communicate("/kiss Fritz")
     * we need to catch the emote here.
     */
    if (*txt == '/' && op->type != PLAYER)
    {
        CommArray_s    *csp;
        char           *cp  = NULL;

        /* remove the command from the parameters */
        strncpy(buf, txt, HUGE_BUF - 1);
        buf[HUGE_BUF - 1] = '\0';

        cp = strchr(buf, ' ');

        if (cp)
        {
            *(cp++) = '\0';
            cp = cleanup_string(cp);
            if (cp && *cp == '\0')
                cp = NULL;
        }

        csp = find_command_element(buf, CommunicationCommands, CommunicationCommandSize);
        if (csp)
        {
            csp->func(op, cp);
            return;
        }
        return;
    }

    sprintf(buf, "%s says: ", query_name(op));
    strncat(buf, txt, MAX_BUF - strlen(buf) - 1);
    buf[MAX_BUF - 1] = 0;
    new_info_map(NDI_WHITE, op->map, op->x, op->y, MAP_INFO_NORMAL, buf);

    /* Players can chat with a marked object in their inventory */
    if(op->type == PLAYER && (npc = find_marked_object(op)))
        trigger_object_plugin_event(EVENT_SAY, npc, op, NULL,
                txt, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR);

    for (i = 0; i <= SIZEOFFREE2; i++)
    {
        xt = op->x + freearr_x[i];
        yt = op->y + freearr_y[i];
        if ((m = out_of_map(op->map, &xt, &yt)))
        {
            if (GET_MAP_FLAGS(m, xt, yt) & (P_MAGIC_EAR | P_IS_ALIVE)) /* quick check we have a magic ear */
            {
                /* ok, browse now only on demand */
                for (npc = get_map_ob(m, xt, yt); npc != NULL; npc = npc->above)
                {
                    /* avoid talking to self */
                    if (op != npc)
                    {
                        /* the ear ... don't break because it can be mutiple on a node or more in the area */
                        if (npc->type == MAGIC_EAR)
                            (void) talk_to_wall(npc, txt); /* Maybe exit after 1. success? */
                        else if (QUERY_FLAG(npc, FLAG_ALIVE))
                            talk_to_npc(op, npc, txt);
                    }
                }
            }
        }
    }
}

/* this communication thingy is ugly - ugly in the way it use malloc for getting
buffers for storing parts of the msg text??? There should be many smarter ways
to handle it
*/
int talk_to_npc(object *op, object *npc, char *txt)
{
    msglang    *msgs;
    int         i, j;

    trigger_object_plugin_event(EVENT_SAY, 
            npc, op, NULL, txt, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR);

/* Gecko 2005-05-15: I disabled this because it makes little sense. Talking to
 * objects in your own inventory is now handled by first marking the object,
 * which is a much better compromise with performance
 * This also seemed very much broken. */
#if 0
    /* GROS - Here we let the objects inside inventories hear and answer, too. */
    /* This allows the existence of "intelligent" weapons you can discuss with */
    for (cobj = npc->inv; cobj != NULL; cobj = cobj->below)
    {
        if (cobj->event_flags & EVENT_FLAG_SAY)
            trigger_object_plugin_event(EVENT_SAY, 
                    cobj, op, npc, txt, NULL, NULL, NULL, SCRIPT_FIX_ALL);
        cobj = cobj->below;
    }
#endif

    if (npc->msg == NULL || *npc->msg != '@')
    {
        /*new_draw_info_format(NDI_UNIQUE,0,op, "%s has nothing to say.", query_name(npc));*/
        return 0;
    }
    if ((msgs = parse_message(npc->msg)) == NULL)
        return 0;
#if 0 /* Turn this on again when enhancing parse_message() */
  if(debug)
    dump_messages(msgs);
#endif
    for (i = 0; msgs->messages[i]; i++)
        for (j = 0; msgs->keywords[i][j]; j++)
            if (msgs->keywords[i][j][0] == '*' || re_cmp(txt, msgs->keywords[i][j]))
            {
                char    buf[MAX_BUF];
                if (op->type != PLAYER) /* a npc talks to another one - show both in white */
                {
                    /* if a message starts with '/', we assume a emote */
                    /* we simply hook here in the emote msg list */
                    if (*msgs->messages[i] == '/')
                    {
                        CommArray_s    *csp;
                        char           *cp  = NULL;
                        char            buf[MAX_BUF];

                        strncpy(buf, msgs->messages[i], MAX_BUF - 1);
                        buf[MAX_BUF - 1] = '\0';

                        cp = strchr(buf, ' ');
                        if (cp)
                        {
                            *(cp++) = '\0';
                            cp = cleanup_string(cp);
                            if (cp && *cp == '\0')
                                cp = NULL;

                            if (cp && *cp == '%')
                                cp = (char *) op->name;
                        }

                        csp = find_command_element(buf, CommunicationCommands, CommunicationCommandSize);
                        if (csp)
                            csp->func(npc, cp);
                    }
                    else
                    {
                        sprintf(buf, "%s says: %s", query_name(npc), msgs->messages[i]);
                        new_info_map_except(NDI_UNIQUE, op->map, op->x, op->y, MAP_INFO_NORMAL, op, op, buf);
                    }
                }
                else /* if a npc is talking to a player, is shown navy and with a seperate "xx says:" line */
                {
                    /* if a message starts with '/', we assume a emote */
                    /* we simply hook here in the emote msg list */
                    if (*msgs->messages[i] == '/')
                    {
                        CommArray_s    *csp;
                        char           *cp  = NULL;
                        char            buf[MAX_BUF];

                        strncpy(buf, msgs->messages[i], MAX_BUF - 1);
                        buf[MAX_BUF - 1] = '\0';

                        cp = strchr(buf, ' ');
                        if (cp)
                        {
                            *(cp++) = '\0';
                            cp = cleanup_string(cp);
                            if (cp && *cp == '\0')
                                cp = NULL;

                            if (cp && *cp == '%')
                                cp = (char *) op->name;
                        }

                        csp = find_command_element(buf, CommunicationCommands, CommunicationCommandSize);
                        if (csp)
                            csp->func(npc, cp);
                    }
                    else
                    {
                        sprintf(buf, "%s says:", query_name(npc));
                        new_draw_info(NDI_NAVY | NDI_UNIQUE, 0, op, buf);
                        new_draw_info(NDI_NAVY | NDI_UNIQUE, 0, op, msgs->messages[i]);
                        sprintf(buf, "%s talks to %s.", query_name(npc), query_name(op));
                        new_info_map_except(NDI_UNIQUE, op->map, op->x, op->y, MAP_INFO_NORMAL, op, op, buf);
                    }
                }
                free_messages(msgs);
                return 1;
            }
    free_messages(msgs);
    return 0;
}

int talk_to_wall(object *npc, char *txt)
{
    msglang    *msgs;
    int         i, j;

    if (npc->msg == NULL || *npc->msg != '@')
        return 0;
    if ((msgs = parse_message(npc->msg)) == NULL)
        return 0;
#if 0 /* Turn this on again when enhancing parse_message() */
  if(settings.debug >= llevDebug)
    dump_messages(msgs);
#endif
    for (i = 0; msgs->messages[i]; i++)
        for (j = 0; msgs->keywords[i][j]; j++)
            if (msgs->keywords[i][j][0] == '*' || re_cmp(txt, msgs->keywords[i][j]))
            {
                if (msgs->messages[i] && *msgs->messages[i] != 0)
                    new_info_map(NDI_NAVY | NDI_UNIQUE, npc->map, npc->x, npc->y, MAP_INFO_NORMAL, msgs->messages[i]);
                free_messages(msgs);
                use_trigger(npc);
                return 1;
            }
    free_messages(msgs);
    return 0;
}
