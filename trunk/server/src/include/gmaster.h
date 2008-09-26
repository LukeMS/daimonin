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
#ifndef __GMASTER_H
#define __GMASTER_H

#define GMASTER_MODE_NO        0
#define GMASTER_MODE_MW        1
#define GMASTER_MODE_VOL       2
#define GMASTER_MODE_GM        4
#define GMASTER_MODE_MM        8

typedef struct _gmaster_struct
{
    char                    entry[196]; /* unparsed gmaster_file entry for this node */
    char                    host[MAX_BUF];
    char                    name[MAX_PLAYER_NAME+1];
    char                    password[MAX_ACCOUNT_PASSWORD+1];
    int                     mode;    /* What is this entry? DM, VOL, GM? */
} gmaster_struct;

#endif
