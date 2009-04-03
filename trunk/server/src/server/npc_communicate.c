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

/* lets check the alive object has an say event script.
 * TODO: Iam not sure it would make sense for a FLAG_ANSWERSAY flag.
 * which drops the ->msg as default answer. I don't think so, lets use
 * scripts for it.
 * Return 1 or 0 according to event.returnvalue. I am not sure why we even have
 * this func. Why not simply call trigger_object_plugin_event() directly below?
 * -- Smacky 20090403
 */
static int talk_to_object(object *op, object *npc, char *txt)
{
    return (trigger_object_plugin_event(EVENT_SAY,
                                        npc, op, NULL, txt,
                                        NULL, NULL, NULL,
                                        SCRIPT_FIX_ACTIVATOR));
}

/* This function is not to understimate when player talk alot
 * in areas which alot if map objects... This is one of this little extra cpu eaters
 * which adds cpu time here and there.
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
    int         flags, i, xt, yt;

    char        buf[HUGE_BUF];

    if (!txt)
        return;

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

    /* Players can chat with a marked object in their inventory */
    if(op->type == PLAYER && (npc = find_marked_object(op)))
    {
        /* If script returns 1, the say is not passed on.
         * This simulates the player whispering to something in his inv. */
        if (trigger_object_plugin_event(EVENT_SAY,
                                        npc, op, NULL, txt,
                                        NULL, NULL, NULL,
                                        SCRIPT_FIX_ACTIVATOR))
            return;
    }

    /* Search nearby for NPCs and magic ears. */
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
                    /* search but avoid talking to self */
                    if ((npc->type == MAGIC_EAR || QUERY_FLAG(npc, FLAG_ALIVE)) && op != npc)
                    {
                        /* If script returns 1, the say is not passed on.
                         * Mappers must take care when a say script return 1
                         * because subsequent say scripts in the area won't get
                         * a look in and players will not hear the message!
                         * TODO: Prioritise which objects get to react to
                         * SAYs -- eg, NPCs first then magic ears. */
                        if (talk_to_object(op, npc, txt))
                            return;
                    }
                }
            }
        }
    }

    /* Broadcast the say to others (players) nearby. */
    sprintf(buf, "%s says: ", query_name(op));
    strncat(buf, txt, MAX_BUF - strlen(buf) - 1);
    buf[MAX_BUF - 1] = 0;

    flags = NDI_WHITE;
    if(op->type == PLAYER)
        flags |= (NDI_SAY | NDI_PLAYER);

    new_info_map(flags, op->map, op->x, op->y, MAP_INFO_NORMAL, buf);
}

/* open a (npc) gui communication interface */
void gui_interface(object *who, int mode, const char *text, const char *tail)
{
	NewSocket *ns = &CONTR(who)->socket;

	SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_SMALL);

    /* NPC_INTERFACE_MODE_NO will send a clear body = remove interface to the client */
    if(mode != NPC_INTERFACE_MODE_NO)
    {
        SockBuf_AddChar(ACTIVE_SOCKBUF(ns), (char)mode);
		SockBuf_AddString(ACTIVE_SOCKBUF(ns), text, strlen(text));
        if(tail)
			SockBuf_AddString(ACTIVE_SOCKBUF(ns), tail, strlen(tail));
    }

	SOCKBUF_REQUEST_FINISH(ns, BINARY_CMD_INTERFACE, SOCKBUF_DYNAMIC);
}

/* Send a "remove NPC interface" command to the client.
* For example if a player->npc communication stops because
* the npc is gone (moved away, dead) or stops talking.
* This is needed to end the asynchron communication way.
*/
void send_clear_interface(player *pl)
{
	SOCKBUF_REQUEST_BUFFER(&pl->socket, SOCKET_SIZE_SMALL);
	SOCKBUF_REQUEST_FINISH(&pl->socket, BINARY_CMD_INTERFACE, SOCKBUF_DYNAMIC);
}

