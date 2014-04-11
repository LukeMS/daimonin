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

#ifndef __GMASTER_H
#define __GMASTER_H

#define GMASTER_MODE_NO  0
#define GMASTER_MODE_MW  (1 << 0)
#define GMASTER_MODE_VOL (1 << 1)
#define GMASTER_MODE_GM  (1 << 2)
#define GMASTER_MODE_MM  (1 << 3)
#define GMASTER_MODE_SA  (1 << 4)

/* Tests any object for gmaster_mode. */
#define GET_GMASTER_MODE(_WHO_) \
    (((_WHO_)->type == PLAYER && \
     CONTR((_WHO_))) ? CONTR((_WHO_))->gmaster_wizpass : GMASTER_MODE_NO)

/* Tests if an *object* (use pl->ob if we already have a player struct --
 * eventually this should expect a player struct as input but for now many
 * funcs which are only applicable to players anyway still work with an object
 * parameter so it is easiest to go with the flow for now) is a gmaster with
 * wiz powers. These powers are very basically defined and one size fits all
 * ATM. In future, we will better define them so that for example, MWs only
 * have certain wiz powers and only on their own maps. */
#define IS_GMASTER_WIZ(_WHO_) \
    ((_WHO_)->type == PLAYER && \
     CONTR((_WHO_)) && \
     CONTR((_WHO_))->gmaster_wiz)

#define IS_GMASTER_WIZPASS(_WHO_) \
    ((_WHO_)->type == PLAYER && \
     CONTR((_WHO_)) && \
     CONTR((_WHO_))->gmaster_wizpass)

#define IS_GMASTER_STEALTH(_WHO_) \
    ((_WHO_)->type == PLAYER && \
     CONTR((_WHO_)) && \
     CONTR((_WHO_))->gmaster_stealth)

#define IS_GMASTER_INVIS(_WHO_) \
    ((_WHO_)->type == PLAYER && \
     CONTR((_WHO_)) && \
     CONTR((_WHO_))->gmaster_invis)

typedef struct _gmaster_struct
{
    char                    entry[196]; /* unparsed gmaster_file entry for this node */
    char                    host[MEDIUM_BUF];
    char                    name[MAX_ACCOUNT_NAME+1];
    int                     mode;
} gmaster_struct;

/* lists of the active ingame gmasters */
extern objectlink *gmaster_list;
extern objectlink *gmaster_list_VOL;
extern objectlink *gmaster_list_GM;
extern objectlink *gmaster_list_MW;
extern objectlink *gmaster_list_MM;
extern objectlink *gmaster_list_SA;

#endif /* ifndef __GMASTER_H */
