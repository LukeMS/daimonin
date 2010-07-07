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

    The author can be reached via e-mail to info@daimonin.org
*/

#ifndef __STATS_H
#define __STATS_H

#define STATS_EVENT

typedef enum stats_event_type
{
    STATS_EVENT_FIRST,          /* Only used to indicate start of list */
    STATS_EVENT_STARTUP,        /* Use this event to log server starts */
    STATS_EVENT_SHUTDOWN,       /* Use this event to log server shutdowns */
    STATS_EVENT_PLAYER_DEATH,   /* player dies, format: char *name */
    STATS_EVENT_PVP_DEATH,      /* player dies in pvp: char *name, char *killername */
    STATS_EVENT_MESSAGE,        /* Use this to log any message string, format: char *msg */
    STATS_EVENT_ROLLBACK,       /* Use this message to inform db of a backup restore */
    STATS_EVENT_LAST            /* Only used to indicate end of list */
} stats_event_type;

#endif /* ifndef __STATS_H */
