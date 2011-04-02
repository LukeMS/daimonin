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

#ifndef __PORTPROTO_H
#define __PORTPROTO_H

/* porting.c */
extern char            *tempnam_local_ext(char *dir, char *pfx, char *name);
extern void             remove_directory(const char *path);
extern char            *strdup_local(const char *str);
extern long             strtol_local(register char *str, char **ptr, register int base);
extern char            *strerror_local(int errnum);
extern int              isqrt(int n);
extern char            *ltostr10(signed long n);
extern void             save_long(char *buf, char *name, long n);
extern void             make_path_to_file(char *filename);

#endif /* ifndef __PORTPROTO_H */
