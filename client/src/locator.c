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

typedef struct locator_hostip_info_t
{
    float lx;
    float ly;
}
locator_hostip_info_t;

locator_t locator;

static size_t ParseHostIP(void *ptr, size_t size, size_t nmemb, void *data);
static sint16 GetX(float lx);
static sint16 GetY(float ly);
static void   Plot(sint16 x, sint16 y, float lx, float ly, int marker);

/* Initialises the locator structure. The width <w> and height <h> specify the
 * size of the locator window. It is actually positioned and drawn with
 * locator_show() below. */
void locator_init(uint16 w, uint16 h)
{
    _server *node;

    locator.map_wh = Bitmaps[BITMAP_LOCATOR_MAP]->bitmap->w;
    locator.map_ht = Bitmaps[BITMAP_LOCATOR_MAP]->bitmap->h; 
    /* Get the location of THIS client. */
    locator_get_hostip_info(NULL, &locator.client);

    for (node = start_server; node; node = node->next)
    {
        char ip[TINY_BUF];

        locator.server = node;
        get_ip_from_hostname(node->nameip, ip);
        locator_get_hostip_info(ip, &node->geoloc);
    }

    locator.box.w = w;
    locator.box.h = h;
    locator_focus(locator.client.lx, locator.client.ly);
}

/* Gets data from hostip.info for IP <ip>, returning longitude and latitude in
 * <lx> and <ly>. */
void locator_get_hostip_info(char *ip, geolocation_t *geoloc)
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

/* Centers the locator window on <lx>, <ly>. */
void locator_focus(float lx, float ly)
{
    locator.box.x = (sint16)(GetX(lx) - locator.box.w / 2);
    locator.box.y = (sint16)(GetY(ly) - locator.box.h / 2);
}

/* Shows the locator map in a window (as defined by locator.box, see
 * locator_init() above) with the top left corner at <x>, <y>. */
void locator_show(sint16 x, sint16 y)
{
    SDL_Rect box;
    _server  *node;

    box.x = x;
    box.y = y;
    box.w = locator.box.w;
    box.h = locator.box.h;
    SDL_SetClipRect(ScreenSurface, &box);
    sprite_blt(Bitmaps[BITMAP_LOCATOR_MAP], x, y, &locator.box, NULL);

//    Plot(x, y, 2.20f, 48.50f, BITMAP_LOCATOR_PLAYER); // Paris, France
//    Plot(x, y, 13.25f, 52.30f, BITMAP_LOCATOR_PLAYER); // Berlin, Germany
//    Plot(x, y, -21.57f, 64.10f, BITMAP_LOCATOR_PLAYER); // Reykjavik, Iceland
//    Plot(x, y, 12.29f, 41.54f, BITMAP_LOCATOR_PLAYER); // Rome, Italy
//    Plot(x, y, 37.35f, 55.45f, BITMAP_LOCATOR_PLAYER); // Moskva, Russian Federation
//    Plot(x, y, -3.45f, 40.25f, BITMAP_LOCATOR_PLAYER); // Madrid, Spain
//    Plot(x, y, 18.03f, 59.20f, BITMAP_LOCATOR_PLAYER); // Stockholm, Sweden
//    Plot(x, y, -0.05f, 51.36f, BITMAP_LOCATOR_PLAYER); // London, UK
//    Plot(x, y, -77.02f, 39.91f, BITMAP_LOCATOR_PLAYER); // Washington DC, USA
//    Plot(x, y, -160.0f, -21.3f, BITMAP_LOCATOR_PLAYER); // Rarotonga, Cook Islands

    /* Plot all servers/players EXCEPT the currently selected one. */
    for (node = start_server; node; node = node->next)
    {
        if (node == locator.server)
        {
            continue;
        }

        Plot(x, y, node->geoloc.lx, node->geoloc.ly,
             BITMAP_LOCATOR_SERVER_THAT);
    }

    /* Plot the currently selected server/players. */
    Plot(x, y, locator.server->geoloc.lx, locator.server->geoloc.ly,
         BITMAP_LOCATOR_SERVER_THIS);

    /* Plot this client. */
    Plot(x, y, locator.client.lx, locator.client.ly, BITMAP_LOCATOR_CLIENT);

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

/* Actually parses the data provided by hostip.info. */
static size_t ParseHostIP(void *ptr, size_t size, size_t nmemb, void *data)
{
    char             *cp;
    geolocation_t *geoloc = (geolocation_t *)data;

    if ((cp = strstr((char *)ptr, "Longitude")))
    {
        geoloc->lx = atof(cp + 11);
    }

    if ((cp = strstr((char *)ptr, "Latitude")))
    {
        geoloc->ly = atof(cp + 10);
    }

    return size * nmemb;
}

/* Returns a longitude <lx> as an pixel coordinate <x>. */
static sint16 GetX(float lx)
{
    sint16 x = (sint16)((180.0f + lx) *
//                      (float)cos(0.0f) *
                        ((float)locator.map_wh / 360.0f));

    return x;
}

/* Returns a latitude <ly> as an pixel coordinate <y>. */
static sint16 GetY(float ly)
{
    sint16 y = (locator.map_ht -
                (sint16)((90.0f + ly) *
                         ((float)locator.map_ht / 180.0f)));

    return y;
}

/* Plots the image <marker> on the locator map. */
static void Plot(sint16 x, sint16 y, float lx, float ly, int marker)
{
    sint16 xx,
           yy;

    if (lx < -180.0f ||
        lx > 180.0f ||
        ly < -90.0f ||
        ly > 90.0f)
    {
        LOG(LOG_ERROR, "Longitude/latitude out of range: longitude is %f, must be -180:180 / Latitude is %f, must be -90:90!\n",
            lx, ly);

        return;
    }

    xx = GetX(lx);
    yy = GetY(ly);

    // draw_info_format(COLOR_WHITE, "%f,%f = %d,%d", lx, ly, xx, yy);

    if (xx >= locator.box.x &&
        xx <= locator.box.x + locator.box.w &&
        yy >= locator.box.y &&
        yy <= locator.box.y + locator.box.h)
    {
        sprite_blt(Bitmaps[marker],
                   (x + (xx - locator.box.x) - Bitmaps[marker]->bitmap->w / 2),
                   (y + (yy - locator.box.y) - Bitmaps[marker]->bitmap->h / 2),
                   NULL, NULL);
    }
}