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

    kerbholz - Kill Stats by Alderan
*/
#if !defined(__KERBHOLZ_H)
#define __KERBHOLZ_H

typedef struct kills_list
{
    struct kills_list *next;
    char name[64];
    unsigned int kills;
    unsigned int session;
} _kills_list;



int addKill(char *name);
void addNewKill(char *name, unsigned int kills, unsigned int session);
void kill_list_show(int type);
void kill_list_clear(void);
void kill_list_load(void);
void kill_list_save(void);
void kill_command(char *cmd);
_kills_list *getKillEntry(char *name);



#endif
