/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003-2006 Michael Toennies

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
#if !defined(__IGNORE_H)
#define __IGNORE_H


void ignore_list_clear(void);
void ignore_list_load(void);
void ignore_list_save(void);
int ignore_check(char *name, char *type);
void ignore_command(char *cmd);

#endif
