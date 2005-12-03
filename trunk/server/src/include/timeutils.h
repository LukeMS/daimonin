/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2005 Michael Toennies

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
/* Copyright 2005 Björn Axelsson */

#ifndef _DAIMONIN_TIME_H_
#define _DAIMONIN_TIME_H_

#include <sys/time.h>

/* Generic function for simple timeval arithmetic (addition & subtraction) */
static inline void add_time(struct timeval *dst, struct timeval *a, struct timeval *b)
{
    dst->tv_sec = a->tv_sec + b->tv_sec;
    dst->tv_usec = a->tv_usec + b->tv_usec;

    if(dst->tv_sec < 0 || (dst->tv_sec == 0 && dst->tv_usec < 0))
    {
        while(dst->tv_usec < -1000000) {
            dst->tv_sec -= 1;
            dst->tv_usec += 1000000;
        }
    } else
    {
        while(dst->tv_usec < 0) {
            dst->tv_sec -= 1;
            dst->tv_usec += 1000000;
        }
        while(dst->tv_usec > 1000000) {
            dst->tv_sec += 1;
            dst->tv_usec -= 1000000;
        }
    }
}

#endif /* _DAIMONIN_TIME_H_ */
