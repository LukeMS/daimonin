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

    The author can be reached via e-mail to daimonin@nord-com.net
*/

#if !defined(__DIALOG_H)
#define __DIALOG_H

#define OPTWIN_MAX_TAB 20
#define OPTWIN_MAX_OPT 26
#define OPTWIN_MAX_KEYS 100

extern void show_optwin(void);
extern void show_newplayer_server(void);
extern void show_login_server(void);
extern void show_meta_server(_server *node, int metaserver_start, int metaserver_sel);
extern void accept_char();
#endif
