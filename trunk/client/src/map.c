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

extern _Sprite         *test_sprite;

static struct Map       the_map;

static struct MapCell  *TheMapCache = NULL;

_mapdata                MapData; /* Current shown map: mapname, length, etc */

/* we need this to parse the map and sort the multi tile monsters */
typedef struct _map_object_parse
{
    int                         face;
    int                         x;
    int                         y;
    struct _map_object_parse   *next;
}
_map_object_parse;

struct _map_object_parse   *start_map_object_parse  = NULL;

_multi_part_obj             MultiArchs[16];

/* TODO: do a real adjust... we just clear here the cache.
 */
void adjust_map_cache(int xpos, int ypos)
{
    int x, y /*, i*/;
//    register struct MapCell                    *map;
    int             xreal=0, yreal=0;

    memset(TheMapCache, 0, 9 * (MapData.xlen * MapData.ylen) * sizeof(struct MapCell));
    for (y = 0; y < MapStatusY; y++)
    {
        for (x = 0; x < MapStatusX; x++)
        {
            xreal = xpos + (x - (MAP_MAX_SIZE - 1) / 2) + MapData.xlen;
            yreal = ypos + (y - (MAP_MAX_SIZE - 1) / 2) + MapData.ylen;
            if (xreal < 0 || yreal < 0 || xreal >= MapData.xlen * 3 || xreal >= MapData.ylen * 3)
                continue;

            /*
                        map = TheMapCache + (yreal * MapData.xlen * 3) + xreal;

                        map->fog_of_war = FALSE;
                        map->darkness = the_map.cells[x][y].darkness;

                        for (i = 0; i < MAXFACES; i++)
                        {
                            map->faces[i] = the_map.cells[x][y].faces[i];
                            map->ext[i] = the_map.cells[x][y].ext[i];
                            map->pos[i] = the_map.cells[x][y].pos[i];
                            map->probe[i] = the_map.cells[x][y].probe[i];
                        }
            */
        }
    }
}


/* load the multi arch offsets */
void load_mapdef_dat(void)
{
    FILE   *stream;
    int     i, ii, x, y, d[32];
    char    line[256];

    if (!(stream = fopen_wrapper(ARCHDEF_FILE, "r")))
    {
        LOG(LOG_ERROR, "ERROR: Can't find file %s\n", ARCHDEF_FILE);
        return;
    }
    for (i = 0; i < 16; i++)
    {
        if (fgets(line, 255, stream) == NULL)
            break;

        sscanf(line,
               "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
               &x, &y, &d[0], &d[1], &d[2], &d[3], &d[4], &d[5], &d[6], &d[7], &d[8], &d[9], &d[10], &d[11], &d[12],
               &d[13], &d[14], &d[15], &d[16], &d[17], &d[18], &d[19], &d[20], &d[21], &d[22], &d[23], &d[24], &d[25],
               &d[26], &d[27], &d[28], &d[29], &d[30], &d[31]);
        MultiArchs[i].xlen = x;
        MultiArchs[i].ylen = y;

        for (ii = 0; ii < 16; ii++)
        {
            MultiArchs[i].part[ii].xoff = d[ii * 2];
            MultiArchs[i].part[ii].yoff = d[ii * 2 + 1];
        }
    }
    fclose(stream);
}


void clear_map(void)
{
    memset(&the_map, 0, sizeof(Map));
}

void display_mapscroll(int dx, int dy)
{
    int         x, y;
    struct Map  newmap;

    for (x = 0; x < MapStatusX; x++)
    {
        for (y = 0; y < MapStatusY; y++)
        {
            if (x + dx < 0 || x + dx >= MapStatusX || y + dy < 0 || y + dy >= MapStatusY)
            {
                memset((char *) &(newmap.cells[x][y]), 0, sizeof(struct MapCell));
            }
            else
            {
                memcpy((char *) &(newmap.cells[x][y]), (char *) &(the_map.cells[x + dx][y + dy]), sizeof(struct MapCell));
            }
        }
    }
    memcpy((char *) &the_map, (char *) &newmap, sizeof(struct Map));
}

void map_draw_map_clear(void)
{
    register int ypos, xpos, x,y;

    for (y = 0; y < MapStatusY; y++)
    {
        for (x = 0; x < MapStatusX; x++)
        {
            xpos = options.mapstart_x + x * MAP_TILE_YOFF - y * MAP_TILE_YOFF;
            ypos = options.mapstart_y + x * MAP_TILE_XOFF + y * MAP_TILE_XOFF;
            sprite_blt_map(Bitmaps[BITMAP_BLACKTILE], xpos, ypos, NULL, NULL);
        }
    }
}

void UpdateMapName(char *name)
{
    char   *tmp;
    int     music_fade  = 0;

    if (name == NULL)
        return;

    if ((tmp = strchr(name, '�')))
    {
        strcpy(MapData.music, tmp);
        if (init_media_tag(tmp))
            music_fade = 1;
        media_show_update--; /* perhaps we have a png - simulate a step = map_scroll */

        *tmp = 0;
    }

    if (!music_fade) /* there was no music tag or playon tag in this map - fade out */
    {
        /* now a interesting problem - when we have some seconds before a fadeout
         * to a file (and not to "mute") and we want mute now - is it possible that
         * the mixer callback is called in a different thread? and then this thread
         * stuck BEHIND the music_new.flag = 1 check - then the fadeout of this mute
         * will drop whatever - the callback will play the old file.
         * that the classic thread/semphore problem.
         */
        sound_fadeout_music(0);
    }
    cur_widget[MAPNAME_ID].wd = get_string_pixel_length(name, &SystemFont);
    strcpy(MapData.name, name);
}

void InitMapData(int xl, int yl, int px, int py)
{
    void   *tmp_free;

    if (xl != -1)
        MapData.xlen = xl;
    if (yl != -1)
        MapData.ylen = yl;
    if (px != -1)
        MapData.posx = px;
    if (py != -1)
        MapData.posy = py;

    if (xl > 0)
    {
        clear_map();
        if (TheMapCache)
        {
            tmp_free = &TheMapCache;
            FreeMemory(tmp_free);
        }
        /* we allocate 9 times the map... in tiled maps, we can have 8 connected
        * maps to our map - we want cache a map except its 2 maps aways-
        * WARNING: tiled maps must be of same size... don't attach a 32x32
        * map on a 16x16 map. Its possible to handle this here, but then we need
        * to know the sizes of the attached maps here
        */
        TheMapCache = malloc(9 * xl * yl * sizeof(struct MapCell));
        memset(TheMapCache, 0, 9 * xl * yl * sizeof(struct MapCell));
    }
}

void set_map_ext(int x, int y, int layer, int ext, int probe)
{
//    register struct MapCell                    *map;
    int             xreal, yreal;

    the_map.cells[x][y].ext[layer] = ext;
    if (probe != -1)
        the_map.cells[x][y].probe[layer] = probe;

    xreal = MapData.posx + (x - (MAP_MAX_SIZE - 1) / 2) + MapData.xlen;
    yreal = MapData.posy + (y - (MAP_MAX_SIZE - 1) / 2) + MapData.ylen;
    if (xreal < 0 || yreal < 0 || xreal >= MapData.xlen * 3 || yreal >= MapData.ylen * 3)
        return;
    /*
        map = TheMapCache + (yreal * MapData.xlen * 3) + xreal;

        map->ext[layer] = ext;
        if (probe != -1)
            map->probe[layer] = probe;
    */
}

void set_map_face(int x, int y, int layer, int face, int pos, int ext, char *name)
{
//   register struct MapCell                    *map;
    int             xreal, yreal/*, i*/;

    the_map.cells[x][y].faces[layer] = face;
    if (!face)
        ext = 0;
    if (ext != -1)
        the_map.cells[x][y].ext[layer] = ext;
    the_map.cells[x][y].pos[layer] = pos;

    strcpy(the_map.cells[x][y].pname[layer], name);

    xreal = MapData.posx + (x - (MAP_MAX_SIZE - 1) / 2) + MapData.xlen;
    yreal = MapData.posy + (y - (MAP_MAX_SIZE - 1) / 2) + MapData.ylen;
    if (xreal < 0 || yreal < 0 || xreal >= MapData.xlen * 3 || yreal >= MapData.ylen * 3)
        return;
    /*
        map = TheMapCache + (yreal * MapData.xlen * 3) + xreal;

        map->fog_of_war = FALSE;
        map->darkness = the_map.cells[x][y].darkness;

        for (i = 0; i < MAXFACES; i++)
        {
            map->faces[i] = the_map.cells[x][y].faces[i];
            map->ext[i] = the_map.cells[x][y].ext[i];
            map->pos[i] = the_map.cells[x][y].pos[i];
            map->probe[i] = the_map.cells[x][y].probe[i];
            strcpy(map->pname[i], the_map.cells[x][y].pname[i]);
        }
    */
}

void display_map_clearcell(long x, long y)
{
//    register struct MapCell                    *map;
    int             xreal, yreal, i;


    the_map.cells[x][y].darkness = 0;
    for (i = 0; i < MAXFACES; i++)
    {
        the_map.cells[x][y].pname[i][0] = 0;
        the_map.cells[x][y].faces[i] = 0;
        the_map.cells[x][y].ext[i] = 0;
        the_map.cells[x][y].pos[i] = 0;
        the_map.cells[x][y].probe[i] = 0;
    }

    xreal = MapData.posx + (x - (MAP_MAX_SIZE - 1) / 2) + MapData.xlen;
    yreal = MapData.posy + (y - (MAP_MAX_SIZE - 1) / 2) + MapData.ylen;
    if (xreal < 0 || yreal < 0 || xreal >= MapData.xlen * 3 || yreal >= MapData.ylen * 3)
        return;
    /*
        map = TheMapCache + (yreal * MapData.xlen * 3) + xreal;

        map->fog_of_war = TRUE;
        map->darkness = 0;
        for (i = 0; i < MAXFACES; i++)
        {
            if (map->faces[i] & 0x8000)
                map->faces[i] = 0;
            map->ext[i] = 0;
            map->pname[i][0] = 0;
            map->probe[i] = 0;
        }
    */
}


void set_map_darkness(int x, int y, uint8 darkness)
{
    //  register struct MapCell                    *map;
    int             xreal, yreal;

    if (darkness != the_map.cells[x][y].darkness)
        the_map.cells[x][y].darkness = darkness;

    xreal = MapData.posx + (x - (MAP_MAX_SIZE - 1) / 2) + MapData.xlen;
    yreal = MapData.posy + (y - (MAP_MAX_SIZE - 1) / 2) + MapData.ylen;
    if (xreal < 0 || yreal < 0 || xreal >= MapData.xlen * 3 || yreal >= MapData.ylen * 3)
        return;
    /*
        map = TheMapCache + (yreal * MapData.xlen * 3) + xreal;

        if (darkness != map->darkness)
            map->darkness = darkness;
    */
}

/** Figure out if name is the same as rankandname.
 * Note: I don't see the tricky part. rank is always appended to name both in the
 * name parameter and in the rankandname parameter.
 * Perhaps titles will be a problem in the future?
 * /Gecko 2006-12-28
 */
static int namecmp(const char *name, const char *rankandname)
{
    return stricmp(name, rankandname);
}

void map_draw_map(void)
{
    register struct MapCell                    *map;
    _Sprite        *face_sprite;
    register int ypos, xpos;
    int         x, y, k, xl, yl, temp, kk, kt, yt, xt, alpha;
    int         xml, xmpos, xtemp = 0;
    uint16      index, index_tmp;
    int         mid, mnr, xreal, yreal;
    _BLTFX      bltfx;
    SDL_Rect    rect;

    /* we should move this later to a better position, this only for testing here */
    _Sprite     player_dummy;
    SDL_Surface bmap;
    int         player_posx, player_posy;
    int         player_pixx, player_pixy;

    if (!TheMapCache)
        return;
    player_posx = MapStatusX - (MapStatusX / 2) - 1;
    player_posy = MapStatusY - (MapStatusY / 2) - 1;
    player_pixx = MAP_START_XOFF + player_posx * MAP_TILE_YOFF - player_posy * MAP_TILE_YOFF + 20;
    player_pixy = 0 + player_posx * MAP_TILE_XOFF + player_posy * MAP_TILE_XOFF - 14;
    player_dummy.border_left = -5;
    player_dummy.border_right = 0;
    player_dummy.border_up = 0;
    player_dummy.border_down = -5;
    player_dummy.bitmap = &bmap;
    bmap.h = 33;
    bmap.w = 35;
    player_pixy = (player_pixy + MAP_TILE_POS_YOFF) - bmap.h;
    bltfx.surface = NULL;
    bltfx.alpha = 128;

    for (kk = 0; kk < MAXFACES - 1; kk++)    /* we draw floor & mask as layer wise (layer 0 & 1) */
    {
        for (alpha = 0; alpha < MAP_MAX_SIZE; alpha++)
        {
            xt = yt = -1;
            while (xt < alpha || yt < alpha)
            {
                if (xt < alpha) /* draw x row from 0 to alpha with y = alpha */
                {
                    x = ++xt;
                    y = alpha;
                }
                else /* x row is drawn, now draw y row from 0 to alpha with x = alpha */
                {
                    y = ++yt;
                    x = alpha;
                }

                if (kk < 2) /* and we draw layer 2 and 3 at once on a node */
                    kt = kk;
                else
                    kt = kk + 1;
                for (k = kk; k <= kt; k++)
                {
                    xpos = MAP_START_XOFF + x * MAP_TILE_YOFF - y * MAP_TILE_YOFF;
                    ypos = 0 + x * MAP_TILE_XOFF + y * MAP_TILE_XOFF;
                 //   if (!k)
                   //     sprite_blt_map(Bitmaps[BITMAP_BLACKTILE], xpos, ypos, NULL, NULL);
                    if (!debug_layer[k])
                        continue;

                    xreal = MapData.posx - (MAP_MAX_SIZE - 1) / 2 + x + MapData.xlen;
                    yreal = MapData.posy - (MAP_MAX_SIZE - 1) / 2 + y + MapData.ylen;

                    if (xreal < 0 || yreal < 0 || xreal >= MapData.xlen * 3 || yreal >= MapData.ylen * 3)
                        continue;
                    /*LOG(-1,"MAPCACHE: x:%d y:%d l:%d\n", xreal,yreal,(yreal*MapData.xlen*3)+xreal);*/
//                    map = TheMapCache + (yreal * MapData.xlen * 3) + xreal;

                    map = &the_map.cells[x][y];
                    if ((index_tmp = map->faces[k]) > 0)
                    {
                        index = index_tmp & ~0x8000;
                        face_sprite = FaceList[index].sprite;
                        if (!face_sprite)
                        {
                            index = MAX_FACE_TILES - 1;
                            face_sprite = FaceList[index].sprite;
                        }
                        if (face_sprite)
                        {
                            if (map->pos[k]) /* we have a set quick_pos = multi tile*/
                            {
                                mnr = map->pos[k];
                                mid = mnr >> 4;
                                mnr &= 0x0f;
                                xml = MultiArchs[mid].xlen;
                                yl = ypos
                                     - MultiArchs[mid].part[mnr].yoff
                                     + MultiArchs[mid].ylen
                                     - face_sprite->bitmap->h;
                                /* we allow overlapping x borders - we simply center then
                                 */
                                xl = 0;
                                if (face_sprite->bitmap->w > MultiArchs[mid].xlen)
                                    xl = (MultiArchs[mid].xlen - face_sprite->bitmap->w) >> 1;
                                xmpos = xpos - MultiArchs[mid].part[mnr].xoff;
                                xl += xmpos;

                                /*sprintf(buf,"ID:%d NR:%d yoff:%d yl:%d", mid,mnr,MultiArchs[mid].part[mnr].yoff, yl);
                                draw_info(buf,COLOR_RED);*/
                            }
                            else /* single tile... */
                            {
                                /* first, we calc the shift positions */
                                xml = MAP_TILE_POS_XOFF;
                                yl = (ypos + MAP_TILE_POS_YOFF) - face_sprite->bitmap->h;
                                xmpos = xl = xpos;
                                if (face_sprite->bitmap->w > MAP_TILE_POS_XOFF)
                                    xl -= (face_sprite->bitmap->w - MAP_TILE_POS_XOFF) / 2;
                            }
                            /* blt the face in the darkness level, the tile pos has */
                            temp = map->darkness;

                            if (temp == 210)
                                bltfx.dark_level = 0;
                            else if (temp == 180)
                                bltfx.dark_level = 1;
                            else if (temp == 150)
                                bltfx.dark_level = 2;
                            else if (temp == 120)
                                bltfx.dark_level = 3;
                            else if (temp == 90)
                                bltfx.dark_level = 4;
                            else if (temp == 60)
                                bltfx.dark_level = 5;
                            else if (temp == 0)
                                bltfx.dark_level = 7;
                            else
                                bltfx.dark_level = 6;

                            /* all done, just blt the face */
                            bltfx.flags = 0;
                            if (k && ((x > player_posx && y >= player_posy) || (x >= player_posx && y > player_posy)))
                            {
                                if (face_sprite && face_sprite->bitmap && k > 1)
                                {
                                    if (sprite_collision(player_pixx, player_pixy, xl, yl, &player_dummy, face_sprite))
                                        bltfx.flags = BLTFX_FLAG_SRCALPHA;
                                }
                            }

                            if (map->fog_of_war == TRUE)
                                bltfx.flags |= BLTFX_FLAG_FOW;
                            else if (cpl.stats.flags & SF_INFRAVISION && index_tmp & 0x8000 && map->darkness < 150)
                                bltfx.flags |= BLTFX_FLAG_RED;
                            else if (cpl.stats.flags & SF_XRAYS)
                                bltfx.flags |= BLTFX_FLAG_GREY;
                            else
                                bltfx.flags |= BLTFX_FLAG_DARK;

                            if (map->ext[k] & FFLAG_INVISIBLE && !(bltfx.flags & BLTFX_FLAG_FOW))
                            {
                                bltfx.flags &= ~BLTFX_FLAG_DARK;
                                bltfx.flags |= BLTFX_FLAG_SRCALPHA | BLTFX_FLAG_GREY;
                            }
                            else if (map->ext[k] & FFLAG_ETHEREAL && !(bltfx.flags & BLTFX_FLAG_FOW))
                            {
                                bltfx.flags &= ~BLTFX_FLAG_DARK;
                                bltfx.flags |= BLTFX_FLAG_SRCALPHA;
                            }
                            if (FaceList[index].flags & FACE_FLAG_UP)
                            {
                                if (FaceList[index].flags & FACE_FLAG_D1)
                                {
                                    if (y < (MAP_MAX_SIZE - 1) / 2)
                                        sprite_blt_map(face_sprite, xl, yl, NULL, &bltfx);
                                }
                                if (FaceList[index].flags & FACE_FLAG_D3)
                                {
                                    if (x < (MAP_MAX_SIZE - 1) / 2 || y < (MAP_MAX_SIZE - 1) / 2)
                                        sprite_blt_map(face_sprite, xl, yl, NULL, &bltfx);
                                }
                            }
                            else
                                sprite_blt_map(face_sprite, xl, yl, NULL, &bltfx);

                            /* here we handle high & low walls - for example when
                             * you enter a house or something. The wall will be drawn
                             * low and higher wall mask will be removed, when the wall
                             * is in front of you.
                             */
                            if (FaceList[index].flags)
                            {
                                if (FaceList[index].flags & FACE_FLAG_DOUBLE)
                                {
                                    if (FaceList[index].flags & FACE_FLAG_D1)
                                    {
                                        if (y < (MAP_MAX_SIZE - 1) / 2)
                                            sprite_blt_map(face_sprite, xl, yl - 22, NULL, &bltfx);
                                    }
                                    if (FaceList[index].flags & FACE_FLAG_D3)
                                    {
                                        if (x < (MAP_MAX_SIZE - 1) / 2 || y < (MAP_MAX_SIZE - 1) / 2)
                                            sprite_blt_map(face_sprite, xl, yl - 22, NULL, &bltfx);
                                    }
                                }
                            }

                            /* have we a playername? then print it! */
                            if (options.player_names && map->pname[k][0])
                            {
                                if (options.player_names == 1 /* all names */
                                        || (options.player_names == 2 && namecmp(map->pname[k], cpl.rankandname)) /* names from other players only */
                                        || (options.player_names == 3 && !namecmp(map->pname[k], cpl.rankandname))) /* only you */
                                {
                                    int s, col = COLOR_DEFAULT;

                                    for (s = 0; s < GROUP_MAX_MEMBER; s++)
                                    {
                                        char *name_tmp = strchr(map->pname[k], ' ');
                                        int len = 0;

                                        if (name_tmp)
                                        {
                                            len = name_tmp - map->pname[k];
                                            if (len != strlen(&group[s].name[0]))
                                                len = 0;
                                        }

                                        if (group[s].name[0] != '\0' && (!strcmp(&group[s].name[0], map->pname[k])||
                                                                         (len && !strncmp(&group[s].name[0], map->pname[k], len)) ))
                                        {
                                            col = COLOR_GREEN;
                                            break;
                                        }
                                    }
                                    StringBlt(ScreenSurfaceMap, &Font6x3Out, map->pname[k],
                                              xpos - (strlen(map->pname[k]) * 2) + 22, ypos - 48, col, NULL, NULL);
                                }
                            }

                            /* perhaps the objects has a marked effect, blt it now */
                            if (map->ext[k])
                            {
                                if (map->ext[k] & FFLAG_SLEEP)
                                    sprite_blt_map(Bitmaps[BITMAP_SLEEP], xl + face_sprite->bitmap->w / 2, yl - 5, NULL,
                                               NULL);
                                if (map->ext[k] & FFLAG_CONFUSED)
                                    sprite_blt_map(Bitmaps[BITMAP_CONFUSE], xl + face_sprite->bitmap->w / 2 - 1, yl - 4,
                                               NULL, NULL);
//                                if (map->ext[k] & FFLAG_SCARED)
//                                    sprite_blt(Bitmaps[BITMAP_SCARED], xl + face_sprite->bitmap->w / 2 + 10, yl - 4,
//                                               NULL, NULL);
                                if (map->ext[k] & FFLAG_EATING)
                                    sprite_blt_map(Bitmaps[BITMAP_WARN_FOOD], xpos + 17, yl - 13, NULL,
                                               NULL);
                                if (map->ext[k] & FFLAG_PARALYZED)
                                {
                                    sprite_blt_map(Bitmaps[BITMAP_PARALYZE], xl + face_sprite->bitmap->w / 2 + 2, yl + 3,
                                               NULL, NULL);
                                    sprite_blt_map(Bitmaps[BITMAP_PARALYZE], xl + face_sprite->bitmap->w / 2 + 9, yl + 3,
                                               NULL, NULL);
                                }
                                if (map->ext[k] & FFLAG_PROBE)
                                {
                                    if (face_sprite)
                                    {
                                        /* 2007-01-15 Alderan: modifying/extend robed's HP patch */
                                        int hp_col;
                                        Uint32 sdl_col;


                                             if (cpl.target_hp > 90) hp_col = COLOR_GREEN;
                                        else if (cpl.target_hp > 75) hp_col = COLOR_DGOLD;
                                        else if (cpl.target_hp > 50) hp_col = COLOR_HGOLD;
                                        else if (cpl.target_hp > 25) hp_col = COLOR_ORANGE;
                                        else if (cpl.target_hp > 10) hp_col = COLOR_YELLOW;
                                        else                         hp_col = COLOR_RED;

                                        if(xml == MAP_TILE_POS_XOFF)
                                            xtemp = (int) (((double)xml/100.0)*25.0);
                                        else
                                            xtemp = (int) (((double)xml/100.0)*20.0);

                                        temp = (xml-xtemp*2)-1;
                                        if (temp <=0)
                                            temp = 1;
                                        if(temp >= 300)
                                            temp = 300;
                                        mid = map->probe[k];
                                        if(mid<=0)
                                            mid = 1;
                                        if (mid>100)
                                            mid = 100;
                                        temp = (int)(((double)temp/100.0)*(double)mid);
                                        rect.h=2 ;
                                        rect.w=temp ;
                                        rect.x = 0;
                                        rect.y = 0;

                                        sdl_col = SDL_MapRGB(ScreenSurfaceMap->format,
                                               Bitmaps[BITMAP_PALETTE]->bitmap->format->palette->colors[hp_col].r,
                                               Bitmaps[BITMAP_PALETTE]->bitmap->format->palette->colors[hp_col].g,
                                               Bitmaps[BITMAP_PALETTE]->bitmap->format->palette->colors[hp_col].b);

                                        /* first we draw the bar: */
                                        rect.x = xmpos+xtemp-1;
                                        rect.y = yl-9;
                                        rect.h = 1;
                                        SDL_FillRect(ScreenSurfaceMap,&rect,sdl_col);
                                        /* horizontal lines of left bracked */
                                        rect.h=1;
                                        rect.w=3;
                                        rect.x = xmpos+xtemp-3;
                                        rect.y = yl-11;
                                        SDL_FillRect(ScreenSurfaceMap,&rect,sdl_col);
                                        rect.y = yl-7;
                                        SDL_FillRect(ScreenSurfaceMap,&rect,sdl_col);
                                        /* hor. lines of right bracked */
                                        rect.x = xmpos+xtemp+(xml-xtemp*2)-3;
                                        SDL_FillRect(ScreenSurfaceMap,&rect,sdl_col);
                                        rect.y = yl-11;
                                        SDL_FillRect(ScreenSurfaceMap,&rect,sdl_col);
                                        /* vertical lines */
                                        rect.w = 1;
                                        rect.h = 5;
                                        rect.x = xmpos+xtemp-3;
                                        rect.y = yl-11;
                                        SDL_FillRect(ScreenSurfaceMap,&rect,sdl_col);
                                        rect.x = xmpos+xtemp+(xml-xtemp*2)-1;
                                        SDL_FillRect(ScreenSurfaceMap,&rect,sdl_col);

                                        /* Draw the name of target if it's not a player */
                                        if (!(options.player_names && map->pname[k][0]))
                                            StringBlt(ScreenSurfaceMap, &Font6x3Out, cpl.target_name, xpos - (strlen(cpl.target_name)*2) + 22, yl - 26, cpl.target_color, NULL, NULL);
                                        /* Draw HP remaining percent */
                                        if (cpl.target_hp>0)
                                        {
                                            char hp_text[9];
                                            int hp_len;
                                            hp_len = sprintf((char *)hp_text, "HP: %d%%", cpl.target_hp);
                                            StringBlt(ScreenSurfaceMap, &Font6x3Out, hp_text, xpos - hp_len*2 + 22, yl - 36, hp_col, NULL, NULL);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


#define TILE_ISO_XLEN 48
/* this +1 is the trick to catch the one pixel line between
 * 2 y rows - the tiles don't touch there!
 */
#define TILE_ISO_YLEN (23+1)

/******************************************************************
* clac the tile-pos(tx,ty) from mouse-pos(x,y).
* ret: 0 ok  ;  <0 not a valid position.
******************************************************************/
int get_tile_position(int x, int y, int *tx, int *ty)
{
//    x +=2*MAP_TILE_POS_XOFF;
//    y +=142+options.mapstart_y;

    if (x < (int)((options.mapstart_x+384)*(options.zoom/100.0)))
        x -= (int)(MAP_TILE_POS_XOFF*(options.zoom/100.0));
    x -= (int)((options.mapstart_x+384)*(options.zoom/100.0));
    y -= (int)((options.mapstart_y)*(options.zoom/100.0));
    *tx = x / (int)(MAP_TILE_POS_XOFF*(options.zoom/100.0)) + y / (int)(MAP_TILE_YOFF*(options.zoom/100.0));
    *ty = y / (int)(MAP_TILE_YOFF*(options.zoom/100.0)) - x / (int)(MAP_TILE_POS_XOFF*(options.zoom/100.0));

    if (x < 0)
    {
        x += ((int)(MAP_TILE_POS_XOFF*(options.zoom/100.0)) << 3) - 1;
 //       draw_info_format(COLOR_GREEN,"x<0 wert: %d",((int)(MAP_TILE_POS_XOFF*(options.zoom/100.0)) << 3) - 1);


    }


    x %= (int)(MAP_TILE_POS_XOFF*(options.zoom/100.0));
    y %= (int)(MAP_TILE_YOFF*(options.zoom/100.0));

    if (x < (int)(MAP_TILE_POS_XOFF2*(options.zoom/100.0)))
    {
        if (x + y + y < (int)(MAP_TILE_POS_XOFF2*(options.zoom/100.0)))
            --(*tx);
        else if (y - x > 0)
            ++(*ty);
    }
    else
    {
        x -= (int)(MAP_TILE_POS_XOFF2*(options.zoom/100.0));
        if (x - y - y > 0)
            --(*ty);
        else if (x + y + y > (int)(MAP_TILE_POS_XOFF*(options.zoom/100.0)))
            ++(*tx);
    }

    if (*tx <0 || *tx>16 || *ty <0 || *ty>16)
        return -1;
    return 0;
}

