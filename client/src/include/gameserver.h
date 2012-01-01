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

    The author can be reached via e-mail to info@daimonin.org
*/

#ifndef __GAMESERVER_H
#define __GAMESERVER_H

/* The default address for the metaserver. */
#define GAMESERVER_META_ADDRESS "www.daimonin.org"

/* The default ports for the two types of gameserver. */
#define GAMESERVER_PLAY_PORT 13327
#define GAMESERVER_META_PORT 13326

/* The max size of the metastring (IDK why it is this huge value -- presumably
 * optimism (128 servers?). */
#define GAMESERVER_MAX_METASTRING (128 * 2013)

typedef enum gameserver_id_t
{
    GAMESERVER_META_ID = -1,
    GAMESERVER_LOCAL_ID,
    GAMESERVER_MAIN_ID,
    GAMESERVER_TEST_ID,
    GAMESERVER_DEV_ID,

    GAMESERVER_NROF
}
gameserver_id_t;

typedef struct gameserver_geoloc_t
{
    sint16 lx;
    sint16 ly;
}
gameserver_geoloc_t;

typedef struct gameserver_t
{
    struct gameserver_t *next;

    char                *name;
    char                *address;
    uint16               port;
    char                *version;
    sint16               players;
    char                *info;
    sint16               ping;
    uint32               ping_server;
    char                *pingstring;
    gameserver_geoloc_t  geoloc;
}
gameserver_t;

extern gameserver_t *gameserver_1st,
                    *gameserver_sel;

extern void gameserver_init(void);
extern void gameserver_add(gameserver_id_t id);
extern void gameserver_query_meta(uint8 force);
extern void gameserver_parse_pingstring(gameserver_t *server);

#endif /* ifndef __GAMESERVER_H */
