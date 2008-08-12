/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Application

    IP-COMPARE component written by: Brian Angeletti (gramlath)

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

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
#define NEED_INET_PTON
#endif

#ifdef DEBUG_IPCOMPARE_MAIN
#define llevDebug 0
#define LOG(x,y,args...) printf(y,## args)
#else
#include <logger.h>
#endif

#ifdef NEED_INET_PTON
/* prototype of local version */
int inet_pton(int af, const char *src, void *dst);

#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT    97
#endif

#ifndef INET6
#define INET6
#endif

#ifndef AF_INET
#define AF_INET 2
#endif

#ifndef AF_INET6
#define AF_INET6 10
#endif

#else
#include <arpa/inet.h>
#endif
