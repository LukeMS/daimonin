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

#include "include.h"

locator_t locator;

static void   GetHostIP(char *ip, gameserver_geoloc_t *geoloc);
static size_t ParseHostIP(void *ptr, size_t size, size_t nmemb, void *data);
static sint16 GetX(sint16 lx);
static sint16 GetY(sint16 ly);
static void   Plot(sint16 x, sint16 y, sint16 lx, sint16 ly, int marker);

/* Initialises the locator structure. The width <w> and height <h> specify the
 * size of the locator window. It is actually positioned and drawn with
 * locator_show() below. */
void locator_init(uint16 w, uint16 h)
{
    gameserver_t *node;

    locator.map_wh = skin_sprites[SKIN_SPRITE_LOCATOR_MAP]->bitmap->w;
    locator.map_ht = skin_sprites[SKIN_SPRITE_LOCATOR_MAP]->bitmap->h; 

    /* Get the location of the client. */
    GetHostIP(NULL, &locator.client);

    /* Get the locations of all known servers. */
    for (node = gameserver_1st; node; node = node->next)
    {
        char *cp;

        locator.server = node;

        /* Local server must be where the client is. */
        if (!strcmp(node->address, "127.0.0.1"))
        {
            node->geoloc.lx = locator.client.lx;
            node->geoloc.ly = locator.client.ly;
        }
        else if (!(cp = get_ip_from_hostname(node->address)))
        {
            node->geoloc.lx = 0;
            node->geoloc.ly = 0;
        }
        else
        {
            GetHostIP(cp, &node->geoloc);
        }
    }

    locator.box.w = w;
    locator.box.h = h;
    locator_focus(locator.client.lx, locator.client.ly);
}

/* Clears player details. If server is non-NULL, then only for that server. If
 * it is NULL, then all player details are cleared. */
void locator_clear_players(gameserver_t *server)
{
    locator_player_t *lp = locator.player;

    while (lp)
    {
        locator_player_t *prev = lp->prev,
                         *next = lp->next;

        if (!server ||
            lp->server == server)
        {
            if (lp == locator.player)
            {
                locator.player = NULL;
            }

            FREE(lp->name);
            FREE(lp->race);
            FREE(lp);

            if (prev)
            {
                prev->next = next;
            }

            if (next)
            {
                next->prev = prev;
            }
        }

        lp = next;
    }
}

/* Adds details of a new player. */
void locator_add_player(gameserver_t *server, const char *name, uint8 gender,
                        const char *race, sint16 lx, sint16 ly)
{
    locator_player_t *new;

    MALLOC(new, sizeof(locator_player_t));
    new->server = server;
    MALLOC_STRING(new->name, name);
    new->gender = gender;
    MALLOC_STRING(new->race, race);
    new->geoloc.lx = lx;
    new->geoloc.ly = ly;

    if (!locator.player)
    {
        locator.player = new;
    }
    else
    {
        locator_player_t *lp = locator.player;

        while (lp->next)
        {
            lp = lp->next;
        }

        new->prev = lp;
        lp->next = new;
    }
}

void locator_show_players(gameserver_t *server)
{
    if (server)
    {
        locator_player_t *lp;

        textwin_show_string(0, NDI_COLR_WHITE, "There %s %d player%s online.\n",
                           (server->players == 1) ? "is" : "are",
                           MAX(0, server->players),
                           (server->players == 1) ? "" : "s");

        for (lp = locator.player; lp; lp = lp->next)
        {
            if (lp->server != server)
            {
                continue;
            }

            textwin_show_string(0, NDI_COLR_WHITE, "%s %u %s %d %d",
                                lp->name, lp->gender, lp->race, lp->geoloc.lx,
                                lp->geoloc.ly);
        }
    }
}

/* Centers the locator window on <lx>, <ly>. */
void locator_focus(sint16 lx, sint16 ly)
{
    locator.box.x = (sint16)(GetX(lx) - locator.box.w / 2);
    locator.box.y = (sint16)(GetY(ly) - locator.box.h / 2);
}

/* Shows the locator map in a window (as defined by locator.box, see
 * locator_init() above) with the top left corner at <x>, <y>. */
void locator_show(sint16 x, sint16 y)
{
    SDL_Rect      box;
    gameserver_t *node;

    box.x = x;
    box.y = y;
    box.w = locator.box.w;
    box.h = locator.box.h;
    SDL_SetClipRect(ScreenSurface, &box);
    sprite_blt(skin_sprites[SKIN_SPRITE_LOCATOR_MAP], x, y, &locator.box, NULL);

//    Plot(x, y, 22, 485, SKIN_SPRITE_LOCATOR_PLAYER_THAT); // Paris, France
//    Plot(x, y, 132, 523, SKIN_SPRITE_LOCATOR_PLAYER_THAT); // Berlin, Germany
//    Plot(x, y, -216, 641, SKIN_SPRITE_LOCATOR_PLAYER_THAT); // Reykjavik, Iceland
//    Plot(x, y, 123, 415, SKIN_SPRITE_LOCATOR_PLAYER_THAT); // Rome, Italy
//    Plot(x, y, 373, 554, SKIN_SPRITE_LOCATOR_PLAYER_THAT); // Moskva, Russian Federation
//    Plot(x, y, -34, 402, SKIN_SPRITE_LOCATOR_PLAYER_THAT); // Madrid, Spain
//    Plot(x, y, 180, 592, SKIN_SPRITE_LOCATOR_PLAYER_THAT); // Stockholm, Sweden
//    Plot(x, y, -0, 514, SKIN_SPRITE_LOCATOR_PLAYER_THAT); // London, UK
//    Plot(x, y, -770, 399, SKIN_SPRITE_LOCATOR_PLAYER_THAT); // Washington DC, USA
//    Plot(x, y, -1600, -213, SKIN_SPRITE_LOCATOR_PLAYER_THAT); // Rarotonga, Cook Islands

    /* Plot all servers EXCEPT the currently selected one. */
    for (node = gameserver_1st; node; node = node->next)
    {
        if (node == locator.server)
        {
            continue;
        }

        Plot(x, y, node->geoloc.lx, node->geoloc.ly,
             SKIN_SPRITE_LOCATOR_SERVER_THAT);
    }

    /* Plot the currently selected server. */
    Plot(x, y, locator.server->geoloc.lx, locator.server->geoloc.ly,
         SKIN_SPRITE_LOCATOR_SERVER_THIS);

    if (locator.player)
    {
        locator_player_t *lp;

        /* Plot all the players on non-selected servers. */
        for (lp = locator.player; lp; lp = lp->next)
        {
            if (lp->server == gameserver_sel)
            {
                continue;
            }

            Plot(x, y, lp->geoloc.lx, lp->geoloc.ly, SKIN_SPRITE_LOCATOR_PLAYER_THAT);
        }

        /* Plot all the players on this server. */
        for (lp = locator.player; lp; lp = lp->next)
        {
            if (lp->server != gameserver_sel)
            {
                continue;
            }

            Plot(x, y, lp->geoloc.lx, lp->geoloc.ly, SKIN_SPRITE_LOCATOR_PLAYER_THIS);
        }
    }

    /* Plot this client. */
    Plot(x, y, locator.client.lx, locator.client.ly, SKIN_SPRITE_LOCATOR_CLIENT);

    SDL_SetClipRect(ScreenSurface, NULL);
    draw_frame(x - 1, y - 1, locator.box.w + 1, locator.box.h + 1);
}

/* Scrolls the locator map in its window according to the keys pressed. */
uint8 locator_scroll(SDLKey key, SDLMod mod)
{
    // TODO: Make size of steps dependent on axis -- requires code reversal.
    uint8 steps = MIN(locator.map_wh, locator.map_ht) / 10;

    if ((mod & KMOD_SHIFT))
    {
        steps /= 2;
    }

    if ((mod & KMOD_CTRL))
    {
        steps /= 5;
    }

    switch (key)
    {
        case SDLK_LEFT:
            locator.box.x = MAX(0, locator.box.x - locator.map_wh / steps);

            break;

        case SDLK_UP:
            locator.box.y = MAX(0, locator.box.y - locator.map_ht / steps);

            break;

        case SDLK_DOWN:
            locator.box.y = MIN(locator.map_ht - locator.box.h,
                                locator.box.y + locator.map_ht / steps);

            break;

        case SDLK_RIGHT:
            locator.box.x = MIN(locator.map_wh - locator.box.w,
                                locator.box.x + locator.map_wh / steps);

            break;

        default:
            break;
    }

    return 0;
}

/* Gets data from hostip.info for IP <ip>, returning longitude and latitude in
 * <lx> and <ly>. */
static void GetHostIP(char *ip, gameserver_geoloc_t *geoloc)
{
    CURL *curlp;
    char  buf[TINY_BUF],
          url[SMALL_BUF];

    if (!(curlp = curl_easy_init()))
    {
        LOG(LOG_ERROR, "%s/curl_easy_init() failed!\n", __FILE__);

        return;
    }

    if (!ip)
    {
        buf[0] = '\0';
    }
    else
    {
        sprintf(buf, "ip=%s&", ip);
    }

    curl_easy_reset(curlp);
    sprintf(url, "http://api.hostip.info/get_html.php?%sposition=true", buf);
    curl_easy_setopt(curlp, CURLOPT_URL, url);
    curl_easy_setopt(curlp, CURLOPT_WRITEFUNCTION, ParseHostIP);
    curl_easy_setopt(curlp, CURLOPT_WRITEDATA, (void *)geoloc);
    (void)curl_easy_perform(curlp);
    curl_easy_cleanup(curlp);
}

/* Actually parses the data provided by hostip.info. */
static size_t ParseHostIP(void *ptr, size_t size, size_t nmemb, void *data)
{
    char                *cp;
    gameserver_geoloc_t *geoloc = (gameserver_geoloc_t *)data;

    if ((cp = strstr((char *)ptr, "Longitude")))
    {
        geoloc->lx = (sint16)(atof(cp + 11) * 10);
    }

    if ((cp = strstr((char *)ptr, "Latitude")))
    {
        geoloc->ly = (sint16)(atof(cp + 10) * 10);
    }

    return size * nmemb;
}

/* Returns a longitude <lx> as an pixel coordinate <x>. */
static sint16 GetX(sint16 lx)
{
    sint16 x = (sint16)((180.0f + (float)(lx * 0.1)) *
                        ((float)locator.map_wh / 360.0f));

    return x;
}

/* Returns a latitude <ly> as an pixel coordinate <y>. */
static sint16 GetY(sint16 ly)
{
    sint16 y = (locator.map_ht -
                (sint16)((90.0f + (float)(ly * 0.1)) *
                         ((float)locator.map_ht / 180.0f)));

    return y;
}

/* Plots the image <marker> on the locator map. */
static void Plot(sint16 x, sint16 y, sint16 lx, sint16 ly, int marker)
{
    sint16 xx,
           yy;

    if (lx < -1800 ||
        lx > 1800 ||
        ly < -900 ||
        ly > 900)
    {
        LOG(LOG_ERROR, "Longitude/latitude out of range: longitude is %f, must be -180:180 / Latitude is %f, must be -90:90!\n",
            (float)(lx * 0.1), (float)(ly * 0.1));

        return;
    }

    xx = GetX(lx);
    yy = GetY(ly);

    // draw_info_format(NDI_COLR_WHITE, "%f,%f = %d,%d", lx, ly, xx, yy);

    if (xx >= locator.box.x &&
        xx <= locator.box.x + locator.box.w &&
        yy >= locator.box.y &&
        yy <= locator.box.y + locator.box.h)
    {
        sprite_blt(skin_sprites[marker],
                   (x + (xx - locator.box.x) - skin_sprites[marker]->bitmap->w / 2),
                   (y + (yy - locator.box.y) - skin_sprites[marker]->bitmap->h / 2),
                   NULL, NULL);
    }
}
