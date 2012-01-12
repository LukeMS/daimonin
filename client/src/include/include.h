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

#ifndef __INCLUDE_H
#define __INCLUDE_H

/* This is for the DevCpp IDE */
#ifndef __WIN_32
#ifdef WIN32
#define __WIN_32
#endif
#endif

#if WIN32
# include "win32.h"
#else // elif LINUX
# include "define.h"
# include "cflinux.h"
#endif

#include "config.h"

/* Just some handy ones I like to use */
#ifndef ABS
#define ABS(x) ((x)<0?-(x):(x))
#endif
#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

typedef unsigned int    uint32;
typedef signed int      sint32;
typedef unsigned short  uint16;
typedef signed short    sint16;
typedef unsigned char   uint8;
typedef signed char     sint8;

/* ok, here we define for what we want compile */
/* later this should be insert a makefile */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <curl/curl.h>
#include <physfs.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#ifdef INSTALL_SOUND
#include <SDL/SDL_mixer.h>
#endif

#include "protocol.h" /* shared header between server & client */
#include "widget.h"
#include "wrapper.h"
#include "zlib.h"
#include "socket.h"
#include "gameserver.h"
#include "face.h"
#include "font.h"
#include "client_cmd.h"
#include "server_cmd.h"
#include "item.h"
#include "chatfilter.h"
#include "ignore.h"
#include "buddy.h"
#include "kerbholz.h"
#include "main.h"
#include "player.h"
#include "misc.h"
#include "event.h"
#include "sound.h"
#include "sprite.h"
#include "map.h"
#include "anim.h"
#include "tile_stretcher.h"
#include "interface.h"
#include "book.h"
#include "textwin.h"
#include "inventory.h"
#include "menu.h"
#include "dialog.h"
#include "group.h"
#include "filewrap.h"
#include "physfsrwops.h"
#include "srvfile.h"
#include "locator.h"
#include "skin.h"

#endif /* ifndef __INCLUDE_H */
