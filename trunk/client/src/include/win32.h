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

    The author can be reached via e-mail to info@daimonin.net
*/

#if !defined(__WIN32_H)
#define __WIN32_H
#ifdef __WIN_32

#define STRICT

#if _MSC_VER > 1000
#pragma once
#endif /* _MSC_VER > 1000 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <windowsx.h>
#include <mmsystem.h>
#include <winsock2.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <SDL/SDL_image.h>

#define _malloc(__d,__s) malloc(__d)

#define inline __inline

#define HAVE_STRICMP
#define HAVE_STRNICMP

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "info@daimonin.net"

/* Define to the full name of this package. */
#define PACKAGE_NAME "Daimonin SDL Client"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "Daimonin SDL Client BETA3-0.966"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "daimonin-sdl-client"

/* Define to the version of this package. */
#define PACKAGE_VERSION "BETA3-0.966"

/* Installation prefix */
#define PREFIX "../../../client-BETA3-0.966"

/* Use the SDL_mixer sound system. Remove when you have no sound card or slow
   computer */
#define INSTALL_SOUND

#ifndef Boolean
#define Boolean int
#endif

#define SOCKET_TIMEOUT_SEC 8

#endif
#endif
