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

    The author can be reached via e-mail to info@daimonin.net
*/

#include <global.h>

/* This is only used for new client/server sound.  If the sound source
 * on the map is farther away than this, we don't sent it to the client.
 */
#define MAX_SOUND_DISTANCE 12
#define MAX_SOUND_DISTANCE_SQUARED POW2(MAX_SOUND_DISTANCE)

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

void play_sound_player_only(player *pl, int soundnum, int soundtype, int x, int y)
{
    NewSocket *ns = &pl->socket;

    if (!pl->socket.sound) /* player has disabled sound */
        return;

    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_SMALL);
    SockBuf_AddChar(ACTIVE_SOCKBUF(ns), (char) x);
    SockBuf_AddChar(ACTIVE_SOCKBUF(ns), (char) y);
    SockBuf_AddShort(ACTIVE_SOCKBUF(ns), (uint16) soundnum);
    SockBuf_AddChar(ACTIVE_SOCKBUF(ns), (char) soundtype);
    SOCKBUF_REQUEST_FINISH(ns, BINARY_CMD_SOUND, SOCKBUF_DYNAMIC);

}

/* Plays some sound on map at x,y using a distance counter.
 * This is now nicely optimized - we use the player map list
 * and testing only the main map and the possible 8 attached maps.
 * Now, we don't must fear about increasing performance lose with
 * high player numbers. mt - 04.02.04
 * the function looks a bit bloated, but for speed reasons, we just
 * cloned all the 8 loops with native settings for each direction.
 */
void play_sound_map(mapstruct *map, int x, int y, int sound_num, int sound_type)
{
    int     xt, yt;
    object *tmp;

    if (!map || map->in_memory != MAP_IN_MEMORY)
        return;

    if (map->player_first) /* any player on this map? */
    {
        for (tmp = map->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - x) + POW2(tmp->y - y)) <= MAX_SOUND_DISTANCE_SQUARED)
                play_sound_player_only(CONTR(tmp), sound_num, sound_type, x - tmp->x, y - tmp->y);
        }
    }

    if (map->tile_map[TILED_MAPS_NORTH] && map->tile_map[TILED_MAPS_NORTH]->in_memory == MAP_IN_MEMORY && map->tile_map[TILED_MAPS_NORTH]->player_first)
    {
        yt = y + MAP_HEIGHT(map->tile_map[TILED_MAPS_NORTH]);
        for (tmp = map->tile_map[TILED_MAPS_NORTH]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - x) + POW2(tmp->y - yt)) <= MAX_SOUND_DISTANCE_SQUARED)
                play_sound_player_only(CONTR(tmp), sound_num, sound_type, x - tmp->x, yt - tmp->y);
        }
    }
    if (map->tile_map[TILED_MAPS_EAST] && map->tile_map[TILED_MAPS_EAST]->in_memory == MAP_IN_MEMORY && map->tile_map[TILED_MAPS_EAST]->player_first)
    {
        xt = x - MAP_WIDTH(map);
        for (tmp = map->tile_map[TILED_MAPS_EAST]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - xt) + POW2(tmp->y - y)) <= MAX_SOUND_DISTANCE_SQUARED)
                play_sound_player_only(CONTR(tmp), sound_num, sound_type, xt - tmp->x, y - tmp->y);
        }
    }
    if (map->tile_map[TILED_MAPS_SOUTH] && map->tile_map[TILED_MAPS_SOUTH]->in_memory == MAP_IN_MEMORY && map->tile_map[TILED_MAPS_SOUTH]->player_first)
    {
        yt = y - MAP_HEIGHT(map);
        for (tmp = map->tile_map[TILED_MAPS_SOUTH]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - x) + POW2(tmp->y - yt)) <= MAX_SOUND_DISTANCE_SQUARED)
                play_sound_player_only(CONTR(tmp), sound_num, sound_type, x - tmp->x, yt - tmp->y);
        }
    }
    if (map->tile_map[TILED_MAPS_WEST] && map->tile_map[TILED_MAPS_WEST]->in_memory == MAP_IN_MEMORY && map->tile_map[TILED_MAPS_WEST]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[TILED_MAPS_WEST]);
        for (tmp = map->tile_map[TILED_MAPS_WEST]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - xt) + POW2(tmp->y - y)) <= MAX_SOUND_DISTANCE_SQUARED)
                play_sound_player_only(CONTR(tmp), sound_num, sound_type, xt - tmp->x, y - tmp->y);
        }
    }
    if (map->tile_map[TILED_MAPS_NORTHEAST] && map->tile_map[TILED_MAPS_NORTHEAST]->in_memory == MAP_IN_MEMORY && map->tile_map[TILED_MAPS_NORTHEAST]->player_first)
    {
        yt = y + MAP_HEIGHT(map->tile_map[TILED_MAPS_NORTHEAST]);
        xt = x - MAP_WIDTH(map);
        for (tmp = map->tile_map[TILED_MAPS_NORTHEAST]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= MAX_SOUND_DISTANCE_SQUARED)
                play_sound_player_only(CONTR(tmp), sound_num, sound_type, xt - tmp->x, yt - tmp->y);
        }
    }
    if (map->tile_map[TILED_MAPS_SOUTHEAST] && map->tile_map[TILED_MAPS_SOUTHEAST]->in_memory == MAP_IN_MEMORY && map->tile_map[TILED_MAPS_SOUTHEAST]->player_first)
    {
        xt = x - MAP_WIDTH(map);
        yt = y - MAP_HEIGHT(map);
        for (tmp = map->tile_map[TILED_MAPS_SOUTHEAST]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= MAX_SOUND_DISTANCE_SQUARED)
                play_sound_player_only(CONTR(tmp), sound_num, sound_type, xt - tmp->x, yt - tmp->y);
        }
    }
    if (map->tile_map[TILED_MAPS_SOUTHWEST] && map->tile_map[TILED_MAPS_SOUTHWEST]->in_memory == MAP_IN_MEMORY && map->tile_map[TILED_MAPS_SOUTHWEST]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[TILED_MAPS_SOUTHWEST]);
        yt = y - MAP_HEIGHT(map);
        for (tmp = map->tile_map[TILED_MAPS_SOUTHWEST]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= MAX_SOUND_DISTANCE_SQUARED)
                play_sound_player_only(CONTR(tmp), sound_num, sound_type, xt - tmp->x, yt - tmp->y);
        }
    }
    if (map->tile_map[TILED_MAPS_NORTHWEST] && map->tile_map[TILED_MAPS_NORTHWEST]->in_memory == MAP_IN_MEMORY && map->tile_map[TILED_MAPS_NORTHWEST]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[TILED_MAPS_NORTHWEST]);
        yt = y + MAP_HEIGHT(map->tile_map[TILED_MAPS_NORTHWEST]);
        for (tmp = map->tile_map[TILED_MAPS_NORTHWEST]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= MAX_SOUND_DISTANCE_SQUARED)
                play_sound_player_only(CONTR(tmp), sound_num, sound_type, xt - tmp->x, yt - tmp->y);
        }
    }
}
