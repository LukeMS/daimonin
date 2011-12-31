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

#ifndef __LOCATOR_H
#define __LOCATOR_H

typedef struct locator_player_t
{
    struct locator_player_t *prev,
                            *next;

    gameserver_t        *server;
    char                *name;
    uint8                gender;
    char                *race;
    gameserver_geoloc_t  geoloc;
}
locator_player_t;

typedef struct locator_t
{
    uint16               map_wh;
    uint16               map_ht;
    gameserver_geoloc_t  client;
    SDL_Rect             box;
    gameserver_t        *server;
    locator_player_t    *player;
}
locator_t;

extern locator_t locator;

extern void  locator_init(uint16 w, uint16 h);
extern void  locator_clear_players(gameserver_t *server);
extern void  locator_add_player(gameserver_t *server, const char *name,
                                uint8 gender, const char *race, sint16 lx,
                                sint16 ly);
extern void  locator_show_players(gameserver_t *server);
extern void  locator_focus(sint16 lx, sint16 ly);
extern void  locator_show(sint16 x, sint16 y);
extern uint8 locator_scroll(SDLKey key, SDLMod mod);

#endif /* ifndef __LOCATOR_H */
