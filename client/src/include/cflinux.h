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
#if !defined(__LINUX_H)
#define __LINUX_H
#ifdef __LINUX

#ifdef HAVE_SYS_STAT_H
#   include <sys/stat.h>
#endif

#ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
#endif

#include <time.h>

#ifdef HAVE_STRING_H
#   include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif

#ifdef HAVE_DMALLOC_H
#  include <dmalloc.h>
#endif

#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <SDL.h>
#include <SDL_main.h>
#include <SDL_image.h>

#define _malloc(__d,__s) malloc(__d)

#ifndef O_BINARY
#define O_BINARY 0x0
#endif

#define Boolean int
#define UINT32 uint32
#endif

#define SOCKET int
#define SOCKET_TIMEOUT_SEC 8

#endif
