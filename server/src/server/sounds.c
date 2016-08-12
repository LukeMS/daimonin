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

#include <global.h>

/* This is only used for new client/server sound.  If the sound source
 * on the map is farther away than this, we don't sent it to the client.
 */
#define MAX_SOUND_DISTANCE 12
#define MAX_SOUND_DISTANCE_SQUARED SQR(MAX_SOUND_DISTANCE)

static _sounds sounds = {0, NULL};

/*
 * Load sound definitions from sound file
 */

void init_sounds()
{
    char    buf[MEDIUM_BUF];
    FILE    *fp;
    int     state = 0;
    char    name[32];
    int     type_count  = 0;
    int     type_index  = -1;
    int     sound_count = 0;
    int     sound_index = -1;

    sprintf(buf, "%s/client_sounds", settings.datadir);
    LOG(llevDebug, "Reading sound definitions from %s...", STRING_SAFE(buf));
    if ((fp = fopen(buf, "r")) == NULL)
        LOG(llevError, "ERROR: Can not open sound definitions file Filename=%s\n", STRING_SAFE(buf));
    while (fgets(buf, MEDIUM_BUF - 1, fp) != NULL)
    {
        // Strip trailing newline character(s) (allow for \r\n or \n)
        buf[strcspn(buf, "\r\n")] = '\0';

        if ((strlen(buf) == 0) || (buf[0] == '#'))
            continue;

        if (!strcmp(buf, "*end"))
            break;

        switch (state)
        {
        case 0:
            // Looking for start line
            if (strncmp(buf, "*start", 6) == 0)
            {
                strtok(buf, "|"); // discard *start
                sscanf(strtok(NULL, "|"), "%d", &type_count); // count of soundtypes
                sounds.count = type_count;

                // Allocate memory
                sounds.types = malloc(type_count * sizeof(_soundtype));

                state++;
            }
            break;

        case 1:
            // Looking for soundtype introducer
            if ((type_count > 0) && (buf[0] == '*'))
            {
                // New soundtype
                type_count--;
                type_index++;
                sscanf(strtok(buf, "|"), "*%d", &sounds.types[type_index].id);
                strcpy(name, strtok(NULL, "|"));
                strtok(NULL, "|"); // discard prefix
                sscanf(strtok(NULL, "|"), "%d", &sound_count);
                sounds.types[type_index].count = sound_count;
                sounds.types[type_index].name = strdup_local(name);
                sounds.types[type_index].sounds = malloc(sound_count * sizeof(_sound)); // space for sounds
                sound_index = -1;
                state++;
            }
            break;

        case 2:
            // Process sound
            if ((sound_count > 0) && (buf[0] == '+'))
            {
                // Process sound
                sound_count--;
                sound_index++;
                sscanf(strtok(buf, "|"), "+%d", &sounds.types[type_index].sounds[sound_index].id);
                strcpy(name, strtok(NULL, "|"));
                sounds.types[type_index].sounds[sound_index].name = strdup_local(name);
            }

            if (sound_count == 0)
                state--;            // Look for next soundtype
            break;
        }
    }
    fclose(fp);
    LOG(llevDebug, "done.\n");
}

void free_sounds()
{
    int     i, j;

    LOG(llevDebug, "Freeing sound definitions\n");
    for (i = 0; i < sounds.count; i++)
    {
        free(sounds.types[i].name);
        for (j = 0; j < sounds.types[i].count; j++)
        {
            free(sounds.types[i].sounds[j].name);
        }
        free(sounds.types[i].sounds);
    }
    free(sounds.types);
}

int lookup_sound(int type_id, const char* soundname)
{
    _soundtype  *type = NULL;
    int         id = -1;
    int         i;

    for (i = 0; i < sounds.count; i++)
    {
        if (sounds.types[i].id == type_id)
        {
            type = &sounds.types[i];
            break;
        }
    }

    if (type)
    {
        for (i = 0; i < type->count; i++)
        {
            if (strcasecmp(type->sounds[i].name, soundname) == 0)
            {
                id = i;
                break;
            }
        }
    }
    return id;
}

void play_sound_player_only(player_t *pl, int soundnum, int soundtype, int x, int y)
{
    NewSocket *ns = &pl->socket;

    if (!pl->socket.sound) /* player has disabled sound */
        return;

    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_SMALL);
    SockBuf_AddChar(ACTIVE_SOCKBUF(ns), (char) x);
    SockBuf_AddChar(ACTIVE_SOCKBUF(ns), (char) y);
    SockBuf_AddShort(ACTIVE_SOCKBUF(ns), (uint16) soundnum);
    SockBuf_AddChar(ACTIVE_SOCKBUF(ns), (char) soundtype);
    SOCKBUF_REQUEST_FINISH(ns, SERVER_CMD_SOUND, SOCKBUF_DYNAMIC);

}

/* CHECKPLAYERS() is a convenience macro used in play_sound_map() to avoid
 * large areas of repeat code. Essentially it finds all the players within
 * range of the sound and calls play_sound_player_only() for them. */
#define CHECKPLAYERS(_O_, _M_, _X_, _Y_, _N_, _T_) \
    for ((_O_) = (_M_)->player_first; (_O_); (_O_) = CONTR((_O_))->map_above) \
    { \
        if (SQR((_O_)->x - (_X_)) + SQR((_O_)->y - (_Y_)) <= MAX_SOUND_DISTANCE_SQUARED) \
        { \
            play_sound_player_only(CONTR((_O_)), (_N_), (_T_), (_X_) - (_O_)->x, (_Y_) - (_O_)->y); \
        } \
    }

/* Plays some sound on map at x,y using a distance counter.
 * This is now nicely optimized - we use the player map list
 * and testing only the main map and the possible 8 attached maps.
 * Now, we don't must fear about increasing performance lose with
 * high player numbers. mt - 04.02.04 */
void play_sound_map(msp_t *msp, int sound_num, int sound_type)
{
    map_t    *m,
             *m2;
    sint16    x,
              y,
              x2,
              y2;
    object_t *this;

    if (!msp ||
        msp->map->in_memory != MAP_MEMORY_ACTIVE)
    {
        return;
    }

    m = msp->map;
    x = msp->x;
    y = msp->y;
    CHECKPLAYERS(this, m, x, y, sound_num, sound_type);

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_NORTH]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x;
        y2 = y + m2->height;
        CHECKPLAYERS(this, m2, x2, y2, sound_num, sound_type);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_EAST]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x - m->width;
        y2 = y;
        CHECKPLAYERS(this, m2, x2, y2, sound_num, sound_type);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_SOUTH]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x;
        y2 = y - m->height;
        CHECKPLAYERS(this, m2, x2, y2, sound_num, sound_type);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_WEST]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x + m2->width;
        y2 = y;
        CHECKPLAYERS(this, m2, x2, y2, sound_num, sound_type);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_NORTHEAST]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x - m->width;
        y2 = y + m2->height;
        CHECKPLAYERS(this, m2, x2, y2, sound_num, sound_type);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_SOUTHEAST]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x - m->width;
        y2 = y - m->height;
        CHECKPLAYERS(this, m2, x2, y2, sound_num, sound_type);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_SOUTHWEST]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x + m2->width;
        y2 = y - m->height;
        CHECKPLAYERS(this, m2, x2, y2, sound_num, sound_type);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_NORTHWEST]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x + m2->width;
        y2 = y + m2->height;
        CHECKPLAYERS(this, m2, x2, y2, sound_num, sound_type);
    }
}

#undef CHECKPLAYERS
