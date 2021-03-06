/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

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
/* This is a config file for the client.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef DAI_DEVELOPMENT
/* this gives feedback text based on input */
//#define DEBUG_TEXT

/* NEVER enable this on productions clients, and NEVER on mapmaker clients
 * use it only for profiling, the log will grow quite big
 */
//#define PROFILING
//#define PROFILING_WIDGETS

//#define WIDGET_SNAP
#endif

/* uncomment this to enable the channel support in the client */
#define USE_CHANNELS

#define USE_NEW_ANIM

#define USE_TILESTRETCHER

/* this define does 2 things:
 * 1) if the framerate of the client is too slow, frames will be skipped if needed
 * 2) animations should always look the same...
 * hopefully i didn't do the calcs wrong
 */
//#define ANIM_FRAMESKIP

#define DEFAULT_SERVER_PORT 13327
#define DEFAULT_METASERVER_PORT 13326

#ifndef SYSPATH
#define SYSPATH "./"
#endif

#define DIR_BITMAPS "bitmaps"
#define DIR_CACHE "cache"
#define DIR_GFX_USER "gfx_user"
#define DIR_ICONS "icons"
#define DIR_LOGS "logs"
#define DIR_MUSIC "media"
#define DIR_SETTINGS "settings"
#define DIR_SOUNDS "sfx"
#define DIR_SRV_FILES "srv_files"

#define FILE_DAIMONIN_P0 "daimonin.p0"
#define FILE_BMAPS_P0 "bmaps.p0"
#define FILE_BMAPS_TMP "srv_files/bmaps.tmp"
#define FILE_ANIMS_TMP "srv_files/anims.tmp"
#define FILE_ANIMS_TMP2 "srv_files/newanims.tmp"
#define FILE_CLIENT_SOUNDS "srv_files/client_sounds"
#define FILE_CLIENT_SPELLS "srv_files/client_spells"
#define FILE_CLIENT_SKILLS "srv_files/client_skills"
#define FILE_CLIENT_SETTINGS "srv_files/client_settings"
#define FILE_CLIENT_BMAPS "srv_files/client_bmap"
#define FILE_CLIENT_ANIMS "srv_files/client_anims"
#define FILE_OPTION "options.dat"
#define FILE_KEYBIND "keys.dat"
#define ARCHDEF_FILE "archdef.dat"
#define FILE_INTERFACE "interface.gui"
#define FILE_LOG "client.log"

#define CLIENT_ICON_NAME "icon.png"

/* socket timeout value */
#define MAX_TIME 0

/* Default Screen
 * TODO: allowing different screen sizes.
 * Because i want a fixed map size (gaming issue), bigger screens will give
 * only more space for menus. Some work to do - i used some fixed positions.
 */
#define SCREEN_XLEN 800
#define SCREEN_YLEN 600

/* Increase when we got MANY new face... Hopefully,we need to increase this
 * in the future...
 */
#define MAX_FACE_TILES 20000

/* Alderan 2008-04-23:
 * just to be sure i increase that value a bit, with the new client_anim code, anims are only really loaded
 * when used the first time, the base-structure memory overhead isn't that much
 */
#define MAXANIM 3000

#define MAP_MAX_SIZE    17

/* Careful when changing this, should be no need */
#define MAX_INPUT_STRING 256                /* max. string len in input string*/
#define MAX_HISTORY_LINES 20                           /* max input history lines */

/* The numbers of our dark levels */
/* for each level-1 we store a own bitmap copy, so be careful */
#define DARK_LEVELS 7

#define DENOMINATIONS 4 // number of coin types

#endif /* ifndef __CONFIG_H */
