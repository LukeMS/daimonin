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

/* There are two types of gameserver: a playserver; and a metaserver.
 *
 * A playserver is what players actually play the game on. A metaserver simply
 * holds information about the available playservers. This module also has some
 * default info about the playservers. TODO: These defaults will be maintained
 * more intelligently in future.
 *
 * So first we connect to a metaserver which sends us a string (the metastring)
 * of info about all available playservers. We parse this metastring to build a
 * list of playservers or, if the metaserver fails or we choose to skip it, use
 * the defaults.
 *
 * Next we ping each playserver on the list to get some details about it such
 * as whether it is actually up and our approximate connection speed, and who
 * is online (the pingstring).
 *
 * Finally, we hopefully connect to one of these playservers and play Daimonin.
 *
 * Note that this module only deals with this very high level process of
 * querying the metaserver, parsing the metastrings and pingstrings, and
 * building the list of playservers. The low level business of actually
 * connecting to the servers is handled in the socket module, and
 * communications to and from a playserver are handled in the client_cmd and
 * command modules.
 *
 * -- Smacky 20111231 */

#include "include.h"

static gameserver_t Default[GAMESERVER_NROF] =
{
    {
        NULL,
        "LOCAL", "127.0.0.1", GAMESERVER_PLAY_PORT, "UNKNOWN", -1,
        "Your local server.",
        -1, 0, NULL, { 0, 0 }
    },
    {
        NULL,
        "Main", "188.40.40.149", GAMESERVER_PLAY_PORT, "UNKNOWN", -1,
        "Best for simply playing the game",
        -1, 0, NULL, { 0, 0 }
    },
    {
        NULL,
        "Test", "62.75.168.180", GAMESERVER_PLAY_PORT, "UNKNOWN", -1,
        "Best for testing new content (maps), both official and unofficial",
        -1, 0, NULL, { 0, 0 }
    },
    {
        NULL,
        "Dev", "62.75.248.130", GAMESERVER_PLAY_PORT, "UNKNOWN", -1,
        "Best for testing new code (features)",
        -1, 0, NULL, { 0, 0 }
    },
};

static uint8 GetMetastring(SOCKET fd);
static uint8 ParseMetastring(char *metastring);
static void  CharToSpace(char *str, char c);
static int   SortPlayservers(const void *a, const void *b);
static uint8 AddPlayserver(char *name, char *server,
                           uint16 port, sint16 players, char *version,
                           char *info);

gameserver_t *gameserver_1st,
             *gameserver_sel;

void gameserver_init(void)
{
    gameserver_t *node,
                 *next = NULL;

    for (node = gameserver_1st; node; node = next)
    {
        next = node->next;
        FREE(node->name);
        FREE(node->address);
        FREE(node->version);
        FREE(node->info);
        FREE(node->pingstring);
        FREE(node);
    }

    gameserver_1st = NULL;
    gameserver_sel = NULL;
}

void gameserver_add(gameserver_id_t id)
{
    if (id == GAMESERVER_META_ID ||
        id == GAMESERVER_NROF)
    {
        return;
    }

    AddPlayserver(Default[id].name, Default[id].address, Default[id].port,
                  Default[id].players, Default[id].version, Default[id].info);
}

void gameserver_query_meta(uint8 force)
{
    static uint8 done = 0;
    uint8        meta;

    if (!force &&
        done)
    {
        return;
    }

    interface_mode = GUI_NPC_MODE_NO;
    clear_group();
    map_udate_flag = 2;

    if (options.gameserver_showlocal)
    {
        gameserver_add(GAMESERVER_LOCAL_ID);
    }

    if (options.gameserver_nometa)
    {
        textwin_show_string(0, NDI_COLR_OLIVE, "Metaserver ignored (using default list)!");
        meta = 0;
    }
    else
    {
        SOCKET sock = SOCKET_NO;

        if (!SOCKET_OpenSocket(&sock, GAMESERVER_META_ADDRESS,
                               GAMESERVER_META_PORT) ||
            !GetMetastring(sock))
        {
            textwin_show_string(0, NDI_COLR_SILVER, "Query metaserver (%s:%d)... ~FAILED~ (using default list)!",
                               GAMESERVER_META_ADDRESS, GAMESERVER_META_PORT);
            meta = 0;
        }
        else
        {
            textwin_show_string(0, NDI_COLR_SILVER, "Query metaserver (%s:%d)... ~OK~!",
                               GAMESERVER_META_ADDRESS, GAMESERVER_META_PORT);
            meta = 1;
        }

        SOCKET_CloseSocket(sock);
    }

    if (!meta)
    {
        gameserver_id_t id;

        for (id = GAMESERVER_MAIN_ID; id < GAMESERVER_NROF; id++)
        {
            gameserver_add(id);
        }
    }

    gameserver_sel = gameserver_1st;
    locator_init(330, 248);
    textwin_show_string(0, NDI_COLR_SILVER, "Select a server.");
    done = 1;
}

/* Parses the ping string, if there is one, for the specified server and, if it
 * is complete, adds a new player to the locator. */
void gameserver_parse_pingstring(gameserver_t *server)
{
    if (server &&
        server->pingstring)
    {
        char *cp_start,
             *cp_end;

        for (cp_start = server->pingstring; *cp_start; cp_start = cp_end + 1)
        {
            char          name[TINY_BUF],
                          race[TINY_BUF];
            unsigned int  gender;
            int           lx,
                          ly;
            
            if ((cp_end = strchr(cp_start, '\n')))
            {
                if (sscanf(cp_start, "%s %u %s %d %d",
                    name, &gender, race, &lx, &ly) == 5)
                {
                    char *cp;

                    /* Strip any gmaster tag from the player name. */
                    if ((cp = strchr(name, '[')))
                    {
                        *cp = '\0';
                    }

                    locator_add_player(server, name, (uint8)gender, race,
                                       (sint16)lx, (sint16)ly);
                }
            }
        }
    }
}

/* Gets the metastring from the metaserver. */
static uint8 GetMetastring(SOCKET fd)
{
    uint16  end = 0;
    uint8   count;
    char   *buf_in,
           *buf_out;

    MALLOC(buf_in, GAMESERVER_MAX_METASTRING);
    MALLOC(buf_out, GAMESERVER_MAX_METASTRING);

    while (1)
    {
        int stat;

#if WIN32
        stat = recv(fd, buf_in, GAMESERVER_MAX_METASTRING, 0);

        if (stat == -1 &&
            WSAGetLastError() != WSAEWOULDBLOCK)
        {
            LOG(LOG_ERROR, "Error reading metaserver data!: %d\n",
                WSAGetLastError());

            break;
        }
#else // elif LINUX
        /* FIXME: Select on fd instead of this never-ending (in case of error)
         * busy-loop */
        for (stat = -1; stat == -1;
             stat = recv(fd, buf_in, GAMESERVER_MAX_METASTRING, 0))
        {
        }

        /* FIXME: Obviously this can never be reached. */
        if (stat == -1)
        {
            LOG(LOG_ERROR, "Error reading metaserver data!\n");

            break;
        }
#endif
        else if (stat > 0)
        {
            if (end + stat >= GAMESERVER_MAX_METASTRING)
            {
                memcpy(buf_out + end, buf_in,
                       end + stat - GAMESERVER_MAX_METASTRING - 1);
                end += stat;

                break;
            }

            memcpy(buf_out + end, buf_in, stat);
            end += stat;
        }
        /* Connect closed by meta */
        else if (stat == 0)
        {
            break;
        }
    }

    buf_out[end] = '\0';
    count = ParseMetastring(buf_out);
    FREE(buf_in);
    FREE(buf_out);

    return count;
}

/* We have one big string holding all servers from the metaserver
 * we do simple castings of 2 placeholders ( | and _ ) to ' ' whitespace
 * and use then sscanf to get the info. */
static uint8 ParseMetastring(char *metastring)
{
    char         *cp_start,
                 *cp_end;
    uint8         count = 0,
                  i;
    gameserver_t  playserver[16]; // 16 should be more than ample (at 20120101
                                  // there are 3).

#ifdef DEBUG_GAMESERVER
    LOG(LOG_DEBUG, "PARSING METASTRING: %s\n", metastring);
#endif

    CharToSpace(metastring, '|');

    for (cp_start = metastring; cp_start &&
                                *cp_start; cp_start = cp_end + 1)
    {
        int    ticks,
               port,
               players;
        char   name[TINY_BUF],
               address[TINY_BUF],
               version[TINY_BUF],
               info[LARGE_BUF];

        if (!(cp_end = strchr(cp_start, '\n')))
        {
            break;
        }

        *cp_end = '\0';

        if (sscanf(cp_start, "%d %s %s %d %s %d %s",
                   &ticks, name, address, &port, version, &players, info) != 7)
        {
            LOG(LOG_ERROR, "Malformed playserver string: >%s<\n", cp_start);

            continue;
        }

        CharToSpace(name, '_');
        CharToSpace(version, '_');
        CharToSpace(info, '_');
        MALLOC_STRING(playserver[count].name, name);
        MALLOC_STRING(playserver[count].address, address);
        playserver[count].port = (uint16)port;
        playserver[count].players = (sint16)players;
        MALLOC_STRING(playserver[count].version, version);
        MALLOC_STRING(playserver[count].info, info);
        count++;
    }

    qsort(playserver, count, sizeof(gameserver_t), SortPlayservers);

    for (i = 0; i < count; i++)
    {
        AddPlayserver(playserver[i].name, playserver[i].address,
                      playserver[i].port, playserver[i].players,
                      playserver[i].version, playserver[i].info);
        FREE(playserver[i].name);
        FREE(playserver[i].address);
        FREE(playserver[i].version);
        FREE(playserver[i].info);
    }

#ifdef DEBUG_GAMESERVER
    LOG(LOG_DEBUG, "METASTRING PARSED. %u playservers found!\n", count);
#endif

    return count;
}

/* Replaces all occurrences of specified char in string with space */
static void CharToSpace(char *str, char c)
{
    uint16 i;

    for (i = 0; *(str + i); i++)
    {
        if (*(str + i) == c)
        {
            *(str + i) = ' ';
        }
    }
}

static int SortPlayservers(const void *a, const void *b)
{
    gameserver_id_t id_a,
                    id_b;

    for (id_a = 0; id_a < GAMESERVER_NROF; id_a++)
    {
        if (!strcmp(((gameserver_t *)a)->name, Default[id_a].name))
        {
            break;
        }
    }

    if (id_a == GAMESERVER_NROF)
    {
        return 1;
    }

    for (id_b = 0; id_b < GAMESERVER_NROF; id_b++)
    {
        if (!strcmp(((gameserver_t *)b)->name, Default[id_b].name))
        {
            break;
        }
    }

    if (id_b == GAMESERVER_NROF)
    {
        return -1;
    }

    return id_a - id_b;
}

/* Adds a playserver to the end of the gameserver_1st list. */
static uint8 AddPlayserver(char *name, char *address,
                           uint16 port, sint16 players, char *version,
                           char *info)
{
    gameserver_t *new;

    if (!gameserver_1st)
    {
        MALLOC(new, sizeof(gameserver_t));
        gameserver_1st = new;
        MALLOC_STRING(new->address, address);
        new->port = port;
    }
    else
    {
        gameserver_t *node;

        for (node = gameserver_1st; node; node = node->next)
        {
            if (node->address &&
                !strcmp(node->address, address) &&
                node->port == port)
            {
                new = node;
                FREE(new->name);
                FREE(new->version);
                FREE(new->info);

                break;
            }
            else if (!node->next)
            {
                MALLOC(new, sizeof(gameserver_t));
                node->next = new;
                MALLOC_STRING(new->address, address);
                new->port = port;

                break;
            }
        }
    }

    new->players = players;
    new->ping = -1; // UNKNOWN
    MALLOC_STRING(new->name, name);
    MALLOC_STRING(new->version, version);
    MALLOC_STRING(new->info, info);
#ifdef DEBUG_GAMESERVER
    LOG(LOG_DEBUG, "ADD PLAYSERVER: %s %s %d %s %d {%s}\n",
        name, address, port, version, players, info);
#endif

    return 1;
}
