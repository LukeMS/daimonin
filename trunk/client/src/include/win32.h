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
#define _CRT_SECURE_NO_DEPRECATE

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
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>

/* Many defines to redirect unix functions or fake standard unix values */
#define inline __inline
#define unlink(__a) _unlink(__a)
#define mkdir(__a, __b) _mkdir(__a)
#define getpid() _getpid()
#define popen(__a, __b) _popen(__a, __b)
#define pclose(__a) _pclose(__a)
#define strdup(__a) _strdup(__a)
#define stricmp(__a, __b) _stricmp(__a, __b)
#define strnicmp(__a, __b, __c) _strnicmp(__a, __b, __c)
#define getcwd(__a, __b) _getcwd(__a, __b)
#define chdir(__a) _chdir(__a)
#define access(__a, __b) _access(__a, __b)
#define chmod(__a, __b) _chmod(__a, __b)
#define hypot(__a, __b) _hypot(__a, __b)
#ifndef MINGW
#define fileno(__a) _fileno(__a)
#endif
#define umask(__a) _umask(__a)
#define lseek(__a, __b, __c) _lseek(__a, __b, __c)

#define _malloc(__d,__s) malloc(__d)

#define inline __inline

#define HAVE_STRICMP
#define HAVE_STRNICMP

#define MSG_DONTWAIT 0

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "info@daimonin.net"

/* Define to the full name of this package. */
#define PACKAGE_NAME "Daimonin SDL Client"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "Daimonin SDL Client 0.9.7.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "daimonin-sdl-client"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.9.7.1"

/* Installation prefix */
#define PREFIX "../../../client-0.9.7.1"

/* Use the SDL_mixer sound system. Remove when you have no sound card or slow
   computer */
#define INSTALL_SOUND

#ifndef Boolean
#define Boolean int
#endif

#endif
#endif
