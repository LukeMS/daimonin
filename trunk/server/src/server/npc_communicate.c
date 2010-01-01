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

        if ((csp = find_command_element(buf, EmoteCommands, EmoteCommandsSize)))
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

/* Handle a player attempting to /talk.
 *
 * The way it all works is:
 * 
 * When a player gives a /talk command first the server checks if he has a
 * marked inv item to talk to. If not, it checks his target. If he doesn't have
 * one or it's a player or invalid, we do a normal target cycle but ignoring
 * players (so both enemies and friends are fine),
 * 
 * If we still can't find a target, a message is given (There's no-one around
 * to talk to!).
 * 
 * If we found, or already had, a target, try to talk to it. Here we query its
 * sensing range (which is halved if the mob is asleep). If it's not in range
 * (or is out of LOS) we say so.
 * 
 * If it is in range, we run the attached talk script, if any, as normal. After
 * this is run we wake up the mob. This means the script can check me.f_sleep
 * and respond accordingly if the player woke up the mob. ;) */
void talk_to_npc(player *pl, char *topic)
{
    object *t_obj;

    /* this happens when whitespace only string was submited */
//    if (!topic ||
//        !(topic = cleanup_chat_string(topic)))
    if (!topic)
    {
        gui_npc(pl->ob, GUI_NPC_MODE_NO, NULL);

        return;
    }

    /* If we have a marked, talkable object in the inventory, talk to it. */
    if ((t_obj = find_marked_object(pl->ob)) &&
       (t_obj->event_flags & EVENT_FLAG_TALK))
    {
        trigger_object_plugin_event(EVENT_TALK, t_obj, pl->ob, NULL, topic,
                                    NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR);

        return;
    }

    /* If we have no or an invalid target or a valid target which is a player
     * (talk is player-mob only), look for a new one. */
    if (!OBJECT_VALID(pl->target_object, pl->target_object_count) ||
        pl->target_object->type == PLAYER)
    {
        pl->target_object = NULL;

        command_target(pl->ob, "3");
    }

    /* If we now have a valid target and it's in LOS and within it's sensing
     * range (modified if it is asleep), talk to it. */
    if (OBJECT_VALID(pl->target_object, pl->target_object_count))
    {
        rv_vector rv;

        t_obj = pl->target_object;

        /* Is the target on this mapset and not too far away? */
        if (get_rangevector(pl->ob, t_obj, &rv, 0))
        {
            unsigned int range = MAX(1, t_obj->stats.Wis);

            if (QUERY_FLAG(t_obj, FLAG_SLEEP))
            {
                range = MAX(1, range / 2);
            }

            if (rv.distance <= range)
            {
                int x = pl->socket.mapx_2 + rv.distance_x,
                    y = pl->socket.mapy_2 + rv.distance_y;

                /* Is it visible to the player? */
                if (pl->blocked_los[x][y] <= BLOCKED_LOS_BLOCKSVIEW)
                {
                    if (t_obj->event_flags & EVENT_FLAG_TALK)
                    {
                        trigger_object_plugin_event(EVENT_TALK, t_obj, pl->ob,
                                                    NULL, topic, NULL, NULL,
                                                    NULL, SCRIPT_FIX_ACTIVATOR);
                    }
                    else
                    {
                        gui_npc(pl->ob, GUI_NPC_MODE_NO, NULL);

                        if(t_obj->msg)
                        {
                            new_draw_info(NDI_NAVY | NDI_UNIQUE, 0, pl->ob,
                                          t_obj->msg);
                        }
                        else
                        {
                            new_draw_info_format(NDI_NAVY | NDI_UNIQUE, 0,
                                                 pl->ob, "%s has nothing to say.",
                                                 query_name(t_obj));
                        }
                    }

                    /* Wake up target. Do it hear so the script can check
                     * me.f_sleep to see if the player's yakking interrupted
                     * the mob's snooze. */
                    CLEAR_FLAG(t_obj, FLAG_SLEEP);

                    return;
                }
            }
        }
    }

    gui_npc(pl->ob, GUI_NPC_MODE_NO, NULL);

    /* If we have a target, it must be out of range. */
    if (t_obj)
    {
        new_draw_info(NDI_UNIQUE, 0, pl->ob, "Your talk target is not in range!");
    }
    else
    {
        new_draw_info(NDI_UNIQUE, 0, pl->ob, "There's no-one around to talk to!");
    }

    return;
}

/* open a (npc) gui communication interface */
void gui_npc(object *who, uint8 mode, const char *text)
{
    NewSocket *ns = &CONTR(who)->socket;

    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_SMALL);

    if (mode == GUI_NPC_MODE_NO ||
        mode >= GUI_NPC_MODE_END)
    {
        SockBuf_AddChar(ACTIVE_SOCKBUF(ns), GUI_NPC_MODE_NO);
    }
    else
    {
        SockBuf_AddChar(ACTIVE_SOCKBUF(ns), mode);

        if (text)
        {
            SockBuf_AddString(ACTIVE_SOCKBUF(ns), text, strlen(text));
        }
    }

    SOCKBUF_REQUEST_FINISH(ns, BINARY_CMD_INTERFACE, SOCKBUF_DYNAMIC);
}
