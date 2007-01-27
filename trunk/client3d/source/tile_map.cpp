/*-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include <string>
#include "logger.h"
#include "sound.h"
#include "tile_map.h"
#include "tile_map_wrapper.h"
#include "tile_manager.h"
#include "object_manager.h"
#include "particle_manager.h"

using namespace Ogre;

//================================================================================================
// .
//================================================================================================
TileMap::TileMap()
{
    TheMapCache = 0;
    ObjectWrapper::getSingleton().add3dNames(); // MUST be called after each change of the converter file.
    ObjectWrapper::getSingleton().readObjects();
}

//================================================================================================
// .
//================================================================================================
TileMap::~TileMap()
{
    delete[] TheMapCache;
}

//================================================================================================
// .
//================================================================================================
void TileMap::clear_map(void)
{
    memset(&the_map, 0, sizeof(Map));
}

//================================================================================================
// TODO: do a real adjust... atm, we just clear the cache.
//================================================================================================
void TileMap::adjust_map_cache(int xpos, int ypos)
{
    memset(TheMapCache, 0, 9 * (mMapData.xlen * mMapData.ylen) * sizeof(MapCell));
}

//================================================================================================
// .
//================================================================================================
void TileMap::map_draw_map_clear(void)
{
    /*
    #define MAP_TILE_XOFF 12
    #define MAP_TILE_YOFF 24
    #define MAP_START_XOFF 376
    #define MAP_START_YOFF 143

        register int ypos, xpos;
        for (register int y = 0; y < CHUNK_SIZE_Z; y++)
        {
            for (register int x = 0; x < CHUNK_SIZE_X; x++)
            {
                xpos = MAP_START_XOFF + x * MAP_TILE_YOFF - y * MAP_TILE_YOFF;
                ypos = MAP_START_YOFF + x * MAP_TILE_XOFF + y * MAP_TILE_XOFF;
                //sprite_blt(Bitmaps[BITMAP_BLACKTILE], xpos, ypos, NULL, NULL);
            }
        }
    */
}

//================================================================================================
// .
//================================================================================================
void TileMap::InitMapData(const char *name, int xl, int yl, int px, int py)
{
    char *tag;
    int  music_fade  = 0;

    // Tag stuff.
    if ((tag = strchr((char*)name, '�')))
    {
        mMapData.music = tag;
        if (tag)
        {
            char *p1 = strchr(tag, '|');
            char *p2 = strrchr(tag, '|');
            if (!p1 || !p2)
            {
                //LOG(LOG_MSG, "MediaTagError: Parameter == NULL (%x %x)\n", p1, p2);
            }
            *p1++ = 0;
            *p2++ = 0;
            if (strstr(tag + 1, ".ogg"))
            {
                Sound::getSingleton().playStream(tag + 1, true);
                //sound_play_music(tag + 1, options.music_volume, 2000, atoi(p2), atoi(p1), MUSIC_MODE_NORMAL);
                music_fade = 1;
            }
            else if (strstr(tag + 1, ".png"))
            {}
            //media_show_update--; // perhaps we have a png - simulate a step = map_scroll.
            *tag = 0;
        }
    }
    if (!music_fade) // there was no music tag or playon tag in this map - fade out.
    {
        // sound_fadeout_music(0);
    }

    if (name)
        mMapData.name = name;
    if (xl != -1)
        mMapData.xlen = xl;
    if (yl != -1)
        mMapData.ylen = yl;
    if (px != -1)
        mMapData.posx = px;
    if (py != -1)
        mMapData.posy = py;

    if (xl > 0)
    {
        clear_map();
        delete[] TheMapCache;
        // we allocate 9 times the map... in tiled maps, we can have 8 connected
        // maps to our map - we want cache a map except its 2 maps away.
        // WARNING: tiled maps must be of same size... don't attach a 32x32
        // map on a 16x16 map. Its possible to handle this here, but then we need
        // to know the sizes of the attached maps here
        TheMapCache = new char[9 * xl * yl * sizeof(MapCell)];
        memset(TheMapCache, 0, 9 * xl * yl * sizeof(MapCell));
    }
}

//================================================================================================
// .
//================================================================================================
void TileMap::set_map_ext(int x, int y, int layer, int ext, int probe)
{
    the_map.cells[x][y].ext[layer] = ext;
    if (probe != -1)
        the_map.cells[x][y].probe[layer] = probe;
    /*
    int xreal = MapData.posx + (x - (MAP_MAX_SIZE - 1) / 2) + MapData.xlen;
    if (xreal < 0 || xreal >= MapData.xlen * 3) return;
    int yreal = MapData.posy + (y - (MAP_MAX_SIZE - 1) / 2) + MapData.ylen;
    if (yreal < 0 || yreal >= MapData.ylen * 3) return;
    map = TheMapCache + (yreal * MapData.xlen * 3) + xreal;
    map->ext[layer] = ext;
    if (probe != -1)
        map->probe[layer] = probe;
    */
}

//================================================================================================
// Replace client2d faces by client3d faces.
//================================================================================================
void TileMap::set_map_face(int x, int y, int layer, int face, int pos, int ext, char *name)
{
    enum {LAYER_TILES, LAYER_TODO1, LAYER_TODO2, LAYER_OBJECTS};

    if (layer == LAYER_TILES)
    {
        const char *strTile = ObjectWrapper::getSingleton().getMeshName(face & ~0x8000);
        if (!strTile || !strTile[0])
        {
            //Logger::log().error() << "Tile face: " << face << " pos: " << x << ", " << y;
            TileManager::getSingleton().setMap(x, y, 30, 3, 0);
            return;
        }
        // "TilePos_x,y,h"
        int texture_col = strTile[ 8]-'0';
        int texture_row = strTile[10]-'0';
        int height      =(strTile[12]-'0') *10;
        TileManager::getSingleton().setMap(x, y, height, texture_row, texture_col);
        return;
    }


    if (layer == LAYER_OBJECTS)
    {
        //  Logger::log().error() << "object: " << ObjectWrapper::getSingleton().getMeshName(face & ~0x8000) << "  " << (int) (face & ~0x8000);
        String meshName = ObjectWrapper::getSingleton().getMeshName(face & ~0x8000);
        if (meshName.empty())
        {
            //Logger::log().error() << "Object face: " << face << " pos: " << x << ", " << y;
            return;
        }

        if (meshName == "Smitty.mesh")
        {
            static bool once = true;
            if (once)
            {
                once = false;
                ObjectStatic::sObject obj;
                obj.meshName  = "Smitty.mesh";
                //obj.meshName  = "Ogre_Big.mesh";
                obj.nickName  = "Nick_Smith";
                obj.type      = ObjectManager::OBJECT_NPC;
                obj.boundingRadius = 2;
                obj.friendly  = 20;
                obj.attack    = 50;
                obj.defend    = 50;
                obj.maxHP     = 50;
                obj.maxMana   = 50;
                obj.maxGrace  = 50;
                obj.pos.x     = x;
                obj.pos.z     = y;
                obj.pos.subX  = 3;
                obj.pos.subZ  = 3;
                obj.level     = 0;
                obj.facing    = 30;
                obj.particleNr=-1;
                ObjectManager::getSingleton().addMobileObject(obj);
            }
        }
        else if (meshName == "Monk.mesh")
        {
            static bool once = true;
            if (once)
            {
                once = false;
                ObjectStatic::sObject obj;
                /*
                                obj.pos.x     = x+2;
                                obj.pos.z     = y;
                                obj.pos.subX  = 0;
                                obj.pos.subZ  = 0;
                                for (int i = 0 ; i <8; ++i)
                                {
                                    TileManager::getSingleton().setWalkablePos(obj.pos, i, 0xff);
                                }

                                obj.pos.x     = x+2;
                                obj.pos.z     = y-1;
                                obj.pos.subX  = 0;
                                obj.pos.subZ  = 0;
                                for (int i = 0 ; i <8; ++i)
                                {
                                    TileManager::getSingleton().setWalkablePos(obj.pos, i, 0xff);
                                }


                                obj.pos.x     = x+2;
                                obj.pos.z     = y-2;
                                obj.pos.subX  = 0;
                                obj.pos.subZ  = 0;
                                for (int i = 0 ; i <8; ++i)
                                {
                                    TileManager::getSingleton().setWalkablePos(obj.pos, i, 0xff);
                                }
                */
                obj.meshName  = "Smitty.mesh";
                obj.nickName  = "Nick_Monk";
                obj.type      = ObjectManager::OBJECT_NPC;
                obj.boundingRadius = 2;
                obj.friendly  = 20;
                obj.attack    = 50;
                obj.defend    = 50;
                obj.maxHP     = 50;
                obj.maxMana   = 50;
                obj.maxGrace  = 50;
                obj.pos.x     = x;
                obj.pos.z     = y;
                obj.pos.subX  = 3;
                obj.pos.subZ  = 3;
                obj.level     = 0;
                obj.facing    = -60;
                obj.particleNr=-1;
                ObjectManager::getSingleton().addMobileObject(obj);

                obj.meshName  = "Tentacle_N_Small.mesh";
                obj.nickName  = "Nick_Tentacle";
                obj.type      = ObjectManager::OBJECT_NPC;
                obj.boundingRadius = 2;
                obj.friendly  = -20;
                obj.attack    = 50;
                obj.defend    = 50;
                obj.maxHP     = 50;
                obj.maxMana   = 50;
                obj.maxGrace  = 50;
                obj.pos.x     = x;
                obj.pos.z     = y+4;
                obj.pos.subX  = 3;
                obj.pos.subZ  = 3;
                obj.level     = 0;
                obj.facing    = -60;
                obj.particleNr=-1;
                ObjectManager::getSingleton().addMobileObject(obj);

                obj.meshName  = "Box_D.mesh";
                obj.nickName  = "Nick_Box2";
                obj.type      = ObjectManager::OBJECT_CONTAINER;
                obj.boundingRadius = 2;
                obj.friendly  = 0;
                obj.attack    = 50;
                obj.defend    = 50;
                obj.maxHP     = 50;
                obj.maxMana   = 50;
                obj.maxGrace  = 50;
                obj.pos.x     = x;
                obj.pos.z     = y+5;
                obj.pos.subX  = 3;
                obj.pos.subZ  = 3;
                obj.level     = 0;
                obj.facing    = 120;
                obj.particleNr=-1;
                ObjectManager::getSingleton().addMobileObject(obj);
            }
        }
        else if (meshName == "Hero.mesh")
        {
            static bool once = true;
            if (once)
            {
                once = false;
                TilePos pos;
                pos.x = x;
                pos.z = 17-5;
                pos.subX  =3;
                pos.subZ  =3;
                ObjectManager::getSingleton().setPosition(ObjectNPC::HERO, pos);
                //Logger::log().error() << "we got the Hero face: " << face;
            }
        }
        else if (meshName.find("Wall",  1) != std::string::npos)
        {
            ObjectStatic::sObject obj;
            obj.meshName  = meshName.substr(1, meshName.size());
            obj.nickName  = "Nick_wall";
            obj.type      = ObjectManager::OBJECT_CONTAINER;
            obj.boundingRadius = 2;
            obj.friendly  = 0;
            obj.attack    = 50;
            obj.defend    = 50;
            obj.maxHP     = 50;
            obj.maxMana   = 50;
            obj.maxGrace  = 50;
            obj.pos.x     = x;
            obj.pos.z     = y;
            obj.pos.subX  = 7;
            if (meshName[0] =='v')
            {
                obj.pos.subZ  = 0;
                obj.facing    = 90;
            }
            else
            {
                obj.pos.subZ  = 7;
                obj.facing    = 0;
            }
            obj.level     = 0;

            obj.particleNr=-1;
            ObjectManager::getSingleton().addMobileObject(obj);
        }
        /*
                else if (meshName == "Wall_90")
                {
                        ObjectStatic::sObject obj;
                        obj.meshName  = "Wall_H_B.mesh";
                        obj.nickName  = "Nick_wall";
                        obj.type      = ObjectManager::OBJECT_CONTAINER;
                        obj.boundingRadius = 2;
                        obj.friendly  = 0;
                        obj.attack    = 50;
                        obj.defend    = 50;
                        obj.maxHP     = 50;
                        obj.maxMana   = 50;
                        obj.maxGrace  = 50;
                        obj.pos.x     = x;
                        obj.pos.z     = y;
                        obj.pos.subX  = 7;
                        obj.pos.subZ  = 0;
                        obj.level     = 0;
                        obj.facing    = 90;
                        obj.particleNr=-1;
                        ObjectManager::getSingleton().addMobileObject(obj);
                }
        */
        else if (meshName == "Sack_N.mesh")
        {
            static bool once = true;
            if (once)
            {
                once = false;
                ObjectStatic::sObject obj;
                obj.meshName  = "Sack_N.mesh";
                obj.nickName  = "Nick_Sack";
                obj.type      = ObjectManager::OBJECT_CONTAINER;
                obj.boundingRadius = 2;
                obj.friendly  = 0;
                obj.attack    = 50;
                obj.defend    = 50;
                obj.maxHP     = 50;
                obj.maxMana   = 50;
                obj.maxGrace  = 50;
                obj.pos.x     = x;
                obj.pos.z     = y;
                obj.pos.subX  = 3;
                obj.pos.subZ  = 3;
                obj.level     = 0;
                obj.facing    = -60;
                obj.particleNr=-1;
                ObjectManager::getSingleton().addMobileObject(obj);
            }
        }
        else if (meshName == "Object_Anvil.mesh")
        {
            static bool once = true;
            if (once)
            {
                once = false;
                ObjectStatic::sObject obj;
                obj.meshName  = "Object_Anvil.mesh";
                obj.nickName  = "Nick_Anvil";
                obj.type      = ObjectManager::OBJECT_CONTAINER;
                obj.boundingRadius = 2;
                obj.friendly  = 0;
                obj.attack    = 50;
                obj.defend    = 50;
                obj.maxHP     = 50;
                obj.maxMana   = 50;
                obj.maxGrace  = 50;
                obj.pos.x     = x;
                obj.pos.z     = y;
                obj.pos.subX  = 3;
                obj.pos.subZ  = 3;
                obj.level     = 0;
                obj.facing    = 35;
                obj.particleNr=-1;
                ObjectManager::getSingleton().addMobileObject(obj);
            }
        }
    }

    the_map.cells[x][y].faces[layer] = face;
    if (!face)
        ext = 0;
    if (ext != -1)
        the_map.cells[x][y].ext[layer] = ext;
    the_map.cells[x][y].pos[layer] = pos;
    strcpy(the_map.cells[x][y].pname[layer], name);
}

//================================================================================================
// .
//================================================================================================
void TileMap::display_map_clearcell(long x, long y)
{
    the_map.cells[x][y].darkness = 0;
    for (int i = 0; i < MAXFACES; i++)
    {
        the_map.cells[x][y].pname[i][0] = 0;
        the_map.cells[x][y].faces[i]    = 0;
        the_map.cells[x][y].ext[i]      = 0;
        the_map.cells[x][y].pos[i]      = 0;
        the_map.cells[x][y].probe[i]    = 0;
    }
}

//================================================================================================
// .
//================================================================================================
void TileMap::display_mapscroll(int dx, int dy)
{
    if (!dx && !dy)
        TileManager::getSingleton().changeChunks();
}

//================================================================================================
// .
//================================================================================================
void TileMap::map_draw_map()
{
    TileManager::getSingleton().changeChunks();
    TileManager::getSingleton().map_update_flag = 0;
}
