/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#include "logger.h"
#include "option.h"
#include "item.h"
#include "events.h"
#include "sound.h"
#include "network.h"
#include "gui_manager.h"
#include "object_manager.h"
#include "object_visuals.h"
#include "tile_manager.h"
#include "particle_manager.h"

using namespace Ogre;

//================================================================================================
// Buffered Key Events.
//================================================================================================
bool Events::keyPressed( const OIS::KeyEvent &e)
{
    mIdleTime =0;
    static int fogStart = 450;
    // ////////////////////////////////////////////////////////////////////
    // GUI keyEvents.
    // ////////////////////////////////////////////////////////////////////
    if (GuiManager::getSingleton().keyEvent(e.key, e.text))
    {
        //e->consume();
        return true;
    }
    // ////////////////////////////////////////////////////////////////////
    // InGame keyEvent.
    // ////////////////////////////////////////////////////////////////////
    if (Option::getSingleton().getGameStatus() < Option::GAME_STATUS_PLAY) return true;
    //mShiftDown = e->isShiftDown();
    switch (e.key)
    {
        case OIS::KC_A:
        {
            //Item::getSingleton().getInventoryItemFromFloor(0);
            static int animNr= 0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_ANIMATION, 0, ObjectAnimate::ANIM_GROUP_IDLE, animNr);
            if (++animNr >= 16) animNr= 0;
            break;
        }

        case OIS::KC_B:
        {
            static int animNr= 0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_ANIMATION, 0, ObjectAnimate::ANIM_GROUP_ATTACK, animNr);
            //ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_ANIMATION, 0, ObjectAnimate::ANIM_GROUP_EMOTE, animNr);
            if (++animNr >= 16) animNr= 0;
            break;
        }

        case OIS::KC_C:
        {
            TileManager::getSingleton().updateHeighlightVertexPos(0, 1);
            /*
            static int animNr= 0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_ANIMATION, 0, ObjectAnimate::ANIM_GROUP_ABILITY, animNr);
            if (++animNr >= 16) animNr= 0;

            //ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, 0,ObjectAnimate::STATE_CAST1);
            */
            break;
        }

        case OIS::KC_D:
        {
            TileManager::getSingleton().updateHeighlightVertexPos(-1, 0);
            //ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, 0,ObjectAnimate::STATE_DEATH1);
            break;
        }

        case OIS::KC_E:
        {
            //ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, 0,ObjectAnimate::STATE_DEATH1);
            break;
        }

        case OIS::KC_F:
        {
            TileManager::getSingleton().updateHeighlightVertexPos(1, 0);
            /*
            static TextureFilterOptions mFiltering = TFO_BILINEAR;
            static int mAniso = 1;
            switch (mFiltering)
            {
                case TFO_BILINEAR:
                    mFiltering = TFO_TRILINEAR;
                    mAniso = 1;
                    break;
                case TFO_TRILINEAR:
                    mFiltering = TFO_ANISOTROPIC;
                    mAniso = 8;
                    break;
                case TFO_ANISOTROPIC:
                    mFiltering = TFO_BILINEAR;
                    mAniso = 1;
                    break;
                default:
                    break;
            }
            MaterialManager::getSingleton().setDefaultTextureFiltering(mFiltering);
            MaterialManager::getSingleton().setDefaultAnisotropy(mAniso);
            */
            break;
        }

        case OIS::KC_G:
        {
            /*
                        char    buf[100];
                        int nrof =1;
                        int loc = Item::getSingleton().mActGrndContainerID;
                        int tag = Item::getSingleton().HeroTileGround[0]->tag;
                        sprintf(buf, "mv %d %d %d", loc, tag, nrof);
                        Network::getSingleton().cs_write_string(buf);
            */
            static bool grid = true;
            TileManager::getSingleton().setGrid(grid);
            grid = !grid;
            break;
        }

        case OIS::KC_H:
        {
            static int val = 100;
            val=0;
            //GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, StringConverter::toString(val).c_str());
            //GuiManager::getSingleton().setValue(GuiManager::STATUSBAR_PLAYER_MANA, val);
            //ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, 0,ObjectAnimate::STATE_HIT1);
            //GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Test 1");
            //GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Test 2");
            GuiManager::getSingleton().addItem(GuiManager::LIST_MSGWIN, "gbn", 0xffffffff, "tooltt");
            GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Test 3");
            GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Test 4");
            break;
        }

        case OIS::KC_I:
        {
            GuiManager::getSingleton().showWindow(GuiManager::WIN_EQUIPMENT, true);
            GuiManager::getSingleton().showWindow(GuiManager::WIN_INVENTORY, true);
            //GuiManager::getSingleton().setSlotBusyTime(GuiManager::WIN_INVENTORY, 0, 6);
            //GuiManager::getSingleton().setSlotBusy(GuiManager::WIN_INVENTORY, 0);
            //GuiManager::getSingleton().showWindow(GuiManager::WIN_TRADE, true);
            //GuiManager::getSingleton().showWindow(GuiManager::WIN_SHOP, true);
            GuiManager::getSingleton().showWindow(GuiManager::WIN_PLAYERTARGET, true);
            GuiManager::getSingleton().showWindow(GuiManager::WIN_CONTAINER, true);
            // ObjectManager::getSingleton().setPlayerEquipment(ObjectManager::OBJECT_PLAYER, ObjectNPC::BONE_HEAD, 1);
            break;
        }

        case OIS::KC_J:
        {
            GuiManager::getSingleton().showWindow(GuiManager::WIN_EQUIPMENT, false);
            GuiManager::getSingleton().showWindow(GuiManager::WIN_INVENTORY, false);
            GuiManager::getSingleton().showWindow(GuiManager::WIN_CONTAINER, false);

            /*
                        Vector3 pos = TileManager::getSingleton().getTileInterface()->get_Selection();
                        sObject obj;
                        obj.meshName = "tree1.mesh";
                        obj.nickName = "tree";
                        obj.type = ObjectManager::OBJECT_STATIC;
                        obj.posX = (int)pos.x;
                        obj.posY = (int)pos.z;
                        obj.facing = 0;
                        ObjectManager::getSingleton().addMobileObject(obj);
            */
            break;
        }

        case OIS::KC_K:
        {
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_HIT,0, 5);
            break;
        }

        case OIS::KC_L:
        {
            TileManager::getSingleton().loadLvl();
            break;
        }

        case OIS::KC_M:
        {
            String strMemUsage = "Memory used for Textures: " + StringConverter::toString(TextureManager::getSingleton().getMemoryUsage()/1024) + " KB";
            GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, strMemUsage.c_str());
            strMemUsage = "Memory used for Meshes: " + StringConverter::toString(MeshManager::getSingleton().getMemoryUsage()/1024) + " KB";
            GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, strMemUsage.c_str());
            break;
        }

        case OIS::KC_O:
        {
            GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, "Show inventory");
            // ObjectManager::getSingleton().setPlayerEquipment(ObjectManager::OBJECT_PLAYER, ObjectNPC::BONE_SHIELD_HAND, 1);
            break;
        }

        case OIS::KC_P:
        {
            bool ready = ObjectManager::getSingleton().isPrimaryWeaponReady(ObjectNPC::HERO);
            ObjectManager::getSingleton().readyPrimaryWeapon(ObjectNPC::HERO, !ready);
            break;
        }

        case OIS::KC_Q:
        {
            if (ObjectManager::getSingleton().isPrimaryWeaponReady(ObjectNPC::HERO))
                ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_ANIMATION, 0, ObjectAnimate::ANIM_GROUP_ATTACK, 1);
            else if (ObjectManager::getSingleton().isSecondaryWeaponReady(ObjectNPC::HERO))
                ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_ANIMATION, 0, ObjectAnimate::ANIM_GROUP_ATTACK, 6);
            break;
        }

        case OIS::KC_R:
        {
            TileManager::getSingleton().updateHeighlightVertexPos(0, -1);
            break;
        }

        case OIS::KC_S:
        {
            TileManager::getSingleton().saveLvl();
            /*
                        bool ready = ObjectManager::getSingleton().isSecondaryWeaponReady(ObjectNPC::HERO);
                        ObjectManager::getSingleton().readySecondaryWeapon(ObjectNPC::HERO, !ready);
            */
            break;
        }

        case OIS::KC_T:
        {
            //Item::getSingleton().printAllItems();
            /*
                   case KEYFUNC_TARGET_ENEMY:
                     send_command("/target 0", -1, SC_NORMAL);
                     break;
                   case KEYFUNC_TARGET_FRIEND:
                     send_command("/target 1", -1, SC_NORMAL);
                     break;
                   case KEYFUNC_TARGET_SELF:
                     send_command("/target 2", -1, SC_NORMAL);
                     break;
                   case KEYFUNC_COMBAT:
                     send_command("/combat", -1, SC_NORMAL);
                     break;
            */
            break;
        }


        case OIS::KC_W:
        {
            GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, "Client3d commands:");
            //Network::getSingleton().send_command("/apply", -1, SC_NORMAL);
            //GuiManager::getSingleton().addTextline(WIN_TEXTWINDOW, LIST_CHATWIN, "apply");
            break;
        }

        case OIS::KC_X:
        {
            /*
            for (int z=22; z ; --z)
            {
                for (int x = z; x < 47; ++x)
                {
                    TileManager::getSingleton().setMap(x, z, 15, 2);
                }
            }
            */
            /*
                        String filename = "./shadowtest.txt";
                        std::ifstream txtFile;
                        txtFile.open(filename.c_str(), std::ios::in | std::ios::binary);
                        if (!txtFile)
                        {
                            Logger::log().error() << "Error on file " << filename;
                        }
                        else
                        {
                            while (1)
                            {
                                int heightTL, tex1, tex2, fil1, shadow;
                                //Logger::log().error() << "Error on file " << filename;
                                if (!getline(txtFile, filename)) goto ennn;
                                if (!getline(txtFile, filename)) goto ennn;
                                if (!getline(txtFile, filename)) goto ennn;
                                if (!getline(txtFile, filename)) goto ennn;
                                for (int y = 0; y < 21; ++y)
                                {
                                    for (int x = 0; x < 21; ++x)
                                    {
                                        if (!getline(txtFile, filename)) goto ennn;
                                        sscanf(filename.c_str(), "%d %d %d %d %d", &heightTL, &tex1, &tex2, &fil1, &shadow);
                                        TileManager::getSingleton().setMap(x, y, heightTL, tex1, tex2, fil1, shadow, 0);
                                    }
                                    if (!getline(txtFile, filename)) goto ennn;
                                    if (!getline(txtFile, filename)) goto ennn;
                                }
                            }
                            ennn:
                            txtFile.close();


                            TileManager::getSingleton().changeChunks();

                            break;
                        }
                        break;
            */
        }
        break;

        case OIS::KC_Y:
        {
            static int sceneDetailIndex =0;
            sceneDetailIndex = (++sceneDetailIndex)%3;
            switch (sceneDetailIndex)
            {
                case 0:
                    mCamera->setPolygonMode(PM_SOLID);
                    break ;
                case 1:
                    mCamera->setPolygonMode(PM_WIREFRAME);
                    break ;
                case 2:
                    mCamera->setPolygonMode(PM_POINTS);
                    break ;
            }
        }
        break;

        case OIS::KC_1:
        {
            /*
            GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "ABCDEF^Found me! Dies ist ein besonders langer link! Der Link geht ueber mehr als eine Zeile. Aber egal ^Kein Link mehr ^123^11111");
            GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "ABCDEF^Link1^1234^Link2^5678^Link3^90123");
            GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "ABCDEF|Hello|90123");
            GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "ABCDEF~#ff0000ff12345678901234567890qwedghldflghdffdjghft34z89t6gfdiug76349zbh4oeu8jlghdfljgrtzuiop~");
            break;
            */
            //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_WEAPON_HAND, 1);
            //static int color =0;
            //ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE, 0, ObjectNPC::TEXTURE_POS_SKIN, color++);
            TileManager::getSingleton().updateTileHeight(+1);
            break;
        }

        case OIS::KC_2:
        {
            //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_SHIELD_HAND, 1);
            //static int color =0;
            //ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE, 0,ObjectNPC::TEXTURE_POS_FACE, color++);
            TileManager::getSingleton().updateTileHeight(-1);
            break;
        }

        case OIS::KC_3:
        {
            //ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TEXTURE,0, -1);
            //static int color =0;
            //ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE, 0,ObjectNPC::TEXTURE_POS_HAIR, color++);
            TileManager::getSingleton().updateTileGfx(+1);
            //TileManager::getSingleton().changeMapset(1,-1);
            break;
        }

        case OIS::KC_4:
        {
            //static int color =0;
            //ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE,0, ObjectNPC::TEXTURE_POS_BODY, color++);
            TileManager::getSingleton().updateTileGfx(-1);
            break;
        }

        case OIS::KC_5:
        {
            TileManager::getSingleton().setTileGfx();
            /*
            static int color =0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE,0, ObjectNPC::TEXTURE_POS_LEGS, color++);
            */
            break;
        }

        case OIS::KC_6:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE,0, ObjectNPC::TEXTURE_POS_BELT, color++);
            break;
        }

        case OIS::KC_7:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE,0, ObjectNPC::TEXTURE_POS_SHOES, color++);
            break;
        }

        case OIS::KC_8:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE, 0,ObjectNPC::TEXTURE_POS_HANDS, color++);
            //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_HEAD, 1);
            break;
        }

        case OIS::KC_9:
        {
            for (int y=0; y < 32; ++y)
                for (int x=0; x < 32; ++x)
                    TileManager::getSingleton().setMap(x, y, 60, 1, 0);

            // Mask demo.
            int gfxBG = 1, gfx0  = 2, gfx1  = 7, shadow = 255;
            int x = 2, y = 4; // Mask 0
            TileManager::getSingleton().setMap(x+0, y+0, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+1, y+0, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+2, y+0, 60, gfx0 , 0);
            TileManager::getSingleton().setMap(x+0, y+1, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+1, y+1, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+2, y+1, 60, gfx1 , 0);
            TileManager::getSingleton().setMap(x+0, y+2, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+1, y+2, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+2, y+2, 60, gfx0 , 0);
            x+= 4;            // Mask 1
            TileManager::getSingleton().setMap(x+0, y+0, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+1, y+0, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+2, y+0, 60, gfx1 , 0);
            TileManager::getSingleton().setMap(x+0, y+1, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+1, y+1, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+2, y+1, 60, gfx0 , 0);
            TileManager::getSingleton().setMap(x+0, y+2, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+1, y+2, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+2, y+2, 60, gfx1 , 0);
            y+= 4; x= 2;      // Mask 2
            TileManager::getSingleton().setMap(x+0, y+0, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+1, y+0, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+2, y+0, 60, gfxBG, 0);
            TileManager::getSingleton().setMap(x+0, y+1, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+1, y+1, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+2, y+1, 60, gfx1 , 0);
            TileManager::getSingleton().setMap(x+0, y+2, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+1, y+2, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+2, y+2, 60, gfxBG, 0);
            x+= 4;            // Mask 3
            TileManager::getSingleton().setMap(x+0, y+0, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+1, y+0, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+2, y+0, 60, gfxBG, 0);
            TileManager::getSingleton().setMap(x+0, y+1, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+1, y+1, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+2, y+1, 60, gfx0 , 0);
            TileManager::getSingleton().setMap(x+0, y+2, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+1, y+2, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+2, y+2, 60, gfxBG, 0);
            y+= 4;  x= 2;     // Mask 4
            TileManager::getSingleton().setMap(x+0, y+0, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+1, y+0, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+2, y+0, 60, gfx1 , 0);
            TileManager::getSingleton().setMap(x+0, y+1, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+1, y+1, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+2, y+1, 60, gfxBG, 0);
            TileManager::getSingleton().setMap(x+0, y+2, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+1, y+2, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+2, y+2, 60, gfx1 , 0);
            x+= 4;            // Mask 5
            TileManager::getSingleton().setMap(x+0, y+0, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+1, y+0, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+2, y+0, 60, gfx0 , 0);
            TileManager::getSingleton().setMap(x+0, y+1, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+1, y+1, 60, gfx1 , 0); TileManager::getSingleton().setMap(x+2, y+1, 60, gfxBG, 0);
            TileManager::getSingleton().setMap(x+0, y+2, 60, gfx0 , 0); TileManager::getSingleton().setMap(x+1, y+2, 60, gfxBG, 0); TileManager::getSingleton().setMap(x+2, y+2, 60, gfx0 , 0);
            // Hard edges demo.
            x = 9, y = 5;
            gfx0 = TileManager::getSingleton().getMapLayer0(x  , y  );  TileManager::getSingleton().setMap(x  , y  , 60, gfx0 ,0 ,shadow ,8);
            gfx0 = TileManager::getSingleton().getMapLayer0(x  , y+1);  TileManager::getSingleton().setMap(x  , y+1, 60, gfx0 ,0 ,shadow ,8);
            gfx0 = TileManager::getSingleton().getMapLayer0(x+1, y  );  TileManager::getSingleton().setMap(x+1, y  , 60, gfx0 ,0 ,shadow ,8);
            gfx0 = TileManager::getSingleton().getMapLayer0(x+1, y+1);  TileManager::getSingleton().setMap(x+1, y+1, 60, gfx0 ,0 ,shadow ,8);
            // Shadow demo.
            x = 11, y = 8, gfx0 = 1, shadow = 80;
            TileManager::getSingleton().setMap(x  , y  , 10, gfx0, 0, shadow); TileManager::getSingleton().setMap(x+1, y  , 10, gfx0, 0, shadow); TileManager::getSingleton().setMap(x+2, y  , 10, gfx0, 0, shadow);
            TileManager::getSingleton().setMap(x  , y+1, 10, gfx0, 0, shadow); TileManager::getSingleton().setMap(x+1, y+1, 10, gfx0, 0, shadow); TileManager::getSingleton().setMap(x+2, y+1, 10, gfx0, 0, shadow);
            TileManager::getSingleton().setMap(x  , y+2, 10, gfx0, 0, shadow); TileManager::getSingleton().setMap(x+1, y+2, 10, gfx0, 0, shadow); TileManager::getSingleton().setMap(x+2, y+2, 10, gfx0, 0, shadow);
            // Some water.
            x = 11, y = 14, gfx0 = 1, shadow = 0;
            TileManager::getSingleton().setMap(x  ,  y  , 0, 6, 48);
            TileManager::getSingleton().setMap(x  ,  y+1, 0, 6, 48);
            TileManager::getSingleton().setMap(x+1,  y  , 0, 6, 48);
            TileManager::getSingleton().setMap(x+1,  y+1, 0, 6, 48);
            TileManager::getSingleton().setMap(x+2,  y  , 0, 6, 48);
            TileManager::getSingleton().setMap(x+2,  y+1, 0, 6, 48);
            /*
                    // Some height.
                    TileManager::getSingleton().setMap(5,  4,  99, 4, 0);
                    TileManager::getSingleton().setMap(6,  4, 200, 4, 0);
                    TileManager::getSingleton().setMap(5,  5,  60, 4, 0);
                    TileManager::getSingleton().setMap(6,  5,  80, 4, 0);
                    TileManager::getSingleton().setMap(7,  5,  80, 4, 0);
            */
            /*
                    // Some water.
                    TileManager::getSingleton().setMap(10,  4, 30, 6, 48);
                    TileManager::getSingleton().setMap(11,  4, 30, 6, 48);
                    TileManager::getSingleton().setMap(10,  5, 30, 6, 48);
                    TileManager::getSingleton().setMap(11,  5, 30, 6, 48);
                    TileManager::getSingleton().setMap(12,  5, 30, 6, 48);
            */
            // Some water.
            /*
                    TileManager::getSingleton().setMap(10,  8, 30, 6, 0, 200);
                    TileManager::getSingleton().setMap(11,  8, 30, 6, 0, 200);
                    TileManager::getSingleton().setMap(10,  9, 30, 6, 0, 200);
                    TileManager::getSingleton().setMap(11,  9, 30, 6, 0, 200);
                    TileManager::getSingleton().setMap(12,  9, 30, 6, 0, 200);
            */
            /*
            TileManager::getSingleton().setMap(16,  2, 30, 6, 48, 200);

            TileManager::getSingleton().setMap(10,  8, 30, 6, 48, 200);
            TileManager::getSingleton().setMap(11,  8, 30, 6, 48, 200);
            TileManager::getSingleton().setMap(10,  9, 30, 6, 48, 200);
            TileManager::getSingleton().setMap(11,  9, 30, 6, 48, 200);
            TileManager::getSingleton().setMap(12,  9, 30, 6, 48, 200);

            TileManager::getSingleton().setMap(11,  4, 30, 6, 48, 200);
            TileManager::getSingleton().setMap(12,  4, 30, 6, 48, 200);
            TileManager::getSingleton().setMap(11,  5, 30, 6, 48, 200);
            TileManager::getSingleton().setMap(12,  5, 30, 6, 48, 200);
            TileManager::getSingleton().setMap(13,  5, 30, 6, 48, 200);
            */

            TileManager::getSingleton().updateChunks();
            //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_BODY, 1);
        }
        break;

        // ////////////////////////////////////////////////////////////////////
        // Player Movemment.
        // ////////////////////////////////////////////////////////////////////
        case OIS::KC_UP:
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_CURSOR_WALK, 0,  1);
            break;
        case OIS::KC_DOWN:
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_CURSOR_WALK, 0, -1);
            break;
        case OIS::KC_RIGHT:
            //mCamera->  moveRelative (Vector3(100,0,0));
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_CURSOR_TURN, 0, -1);
            break;
        case OIS::KC_LEFT:
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_CURSOR_TURN, 0,  1);
            //mCamera->  moveRelative (Vector3(-100,0,0));
            break;

        case OIS::KC_F1:
        {
            static bool tst = 1;
            tst = !tst;
            GuiManager::getSingleton().setVisible(GuiManager::BUTTON_TEST, tst);
            break;

            Vector3 pos = mCamera->getPosition();
            pos.y+= 15;
            Logger::log().error() << "camera pos: " << pos.x << "   " <<pos.y << "   " << pos.z;
            mCamera->setPosition(pos);
        }
        break;

        case OIS::KC_F2:
        {
            Vector3 pos = mCamera->getPosition();
            pos.y-= 15;
            Logger::log().error() << "camera pos: " << pos.x << "   " <<pos.y << "   " << pos.z;
            mCamera->setPosition(pos);
        }
        break;

        case OIS::KC_F3:
        {
            Vector3 pos = mCamera->getPosition();
            pos.z+= 15;
            Logger::log().error() << "camera pos: " << pos.x << "   " << pos.y << "   " << pos.z;
            mCamera->setPosition(pos);
        }
        break;

        case OIS::KC_F4:
        {
            Vector3 pos = mCamera->getPosition();
            pos.z-= 15;
            Logger::log().error() << "camera pos: " << pos.x << "   "<< pos.y << "   " << pos.z;
            mCamera->setPosition(pos);
        }
        break;

        case OIS::KC_HOME:
        {
            static int cAdd =0;
            ++cAdd;
            Logger::log().error() << "camera pitch add: " << cAdd;
            mCamera->pitch(Degree(+1));
            mCamera->moveRelative (Vector3(0,-30, -30));

            //mCameraRotating = FREEZE;
        }
        break;

        case OIS::KC_END:
        {
            static int cAdd =0;
            ++cAdd;
            Logger::log().error() << "camera pitch sub: " << cAdd;
            mCamera->pitch(Degree(-1));
            mCamera->moveRelative (Vector3(0, 30, 30));
        }
        break;

        case OIS::KC_F7:
        {
            fogStart+=5;
            mSceneManager->setFog(FOG_LINEAR , ColourValue(0,0,0), 0, fogStart, 600);
        }
        break;

        case OIS::KC_F8:
        {
            fogStart-=5;
            mSceneManager->setFog(FOG_LINEAR , ColourValue(0,0,0), 0, fogStart, 600);
        }
        break;


        case OIS::KC_SUBTRACT:
        {
            if (mCameraZoom < MAX_CAMERA_ZOOM)
                mCameraZoom += 5;
            mCamera->setFOVy(Degree(mCameraZoom));
            break;
        }

        case OIS::KC_ADD:
        {
            if (mCameraZoom > MIN_CAMERA_ZOOM)
                mCameraZoom -= 5;
            mCamera->setFOVy(Degree(mCameraZoom));
            break;
        }

        case OIS::KC_PGUP:
            mCameraRotating = POSITIVE;
            break;

        case OIS::KC_PGDOWN:
            mCameraRotating = NEGATIVE;
            break;

            // ////////////////////////////////////////////////////////////////////
            // Screenshot.
            // ////////////////////////////////////////////////////////////////////
        case OIS::KC_SYSRQ:
        {
            static int mNumScreenShots=0;
            String strTemp = "Client3d_" + StringConverter::toString(++mNumScreenShots,2,'0') + ".png";
            mWindow->writeContentsToFile(strTemp.c_str());
            //mTimeUntilNextToggle = 0.5;
            break;
        }
        // ////////////////////////////////////////////////////////////////////
        // Exit game.
        // ////////////////////////////////////////////////////////////////////
        case OIS::KC_ESCAPE:
            mQuitGame = true;
            break;

        default:
            break;
    }
    return true;
//e->consume();
}

//================================================================================================
// .
//================================================================================================
bool Events::keyReleased(const OIS::KeyEvent &e)
{
    //mShiftDown = e->isShiftDown();
    switch (e.key)
    {
            // ////////////////////////////////////////////////////////////////////
            // Player Movemment.
            // ////////////////////////////////////////////////////////////////////
        case OIS::KC_UP:
        case OIS::KC_DOWN:
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_CURSOR_WALK, 0);
            break;

        case OIS::KC_RIGHT:
        case OIS::KC_LEFT:
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_CURSOR_TURN, 0);
            break;

        case OIS::KC_J:
        case OIS::KC_K:
            // ObjectManager::getSingleton().Event(ObjectManager::OBJECT_NPC, OBJ_TURN,  0);
            break;

        case OIS::KC_G:
            // ObjectManager::getSingleton().Event(ObjectManager::OBJECT_NPC, OBJ_WALK,  0);
            break;

        case OIS::KC_PGUP:
        case OIS::KC_PGDOWN:
            mCameraRotating = TURNBACK;
            break;

        default:
            break;
    }
    return true;
}

//================================================================================================
// Buffered Mouse Events.
//================================================================================================
bool Events::mouseMoved(const OIS::MouseEvent &e)
{
    const int MOUSE_POINTER_SIZE = 10;
    mMouse.x = e.state.X.abs;
    mMouse.y = e.state.Y.abs;
    mMouse.z = e.state.Z.rel; // Mouse-wheel.
    if (mMouse.x > e.state.width - MOUSE_POINTER_SIZE) mMouse.x = e.state.width - MOUSE_POINTER_SIZE;
    if (mMouse.y > e.state.height- MOUSE_POINTER_SIZE) mMouse.y = e.state.height- MOUSE_POINTER_SIZE;
    GuiManager::getSingleton().mouseEvent(GuiManager::MOUSE_MOVEMENT, mMouse);
    return true;
}

//================================================================================================
// .
//================================================================================================
bool Events::mousePressed(const OIS::MouseEvent &e, const OIS::MouseButtonID button)
{
    // ////////////////////////////////////////////////////////////////////
    // First check if the mouse action is within the gui.
    // ////////////////////////////////////////////////////////////////////
    int ret =  GuiManager::getSingleton().mouseEvent(GuiManager::BUTTON_PRESSED, mMouse);
    if (ret == GuiManager::EVENT_CHECK_DONE)
        return true;
    // ////////////////////////////////////////////////////////////////////
    // Right button for selection and menu.
    // ////////////////////////////////////////////////////////////////////
    if (button == OIS::MB_Right)
    {
        {
            RaySceneQuery *mRaySceneQuery = mSceneManager->createRayQuery(Ray());
            mRaySceneQuery->setRay(mCamera->getCameraToViewportRay(mMouse.x / e.state.width, mMouse.y / e.state.height));
            mRaySceneQuery->setQueryMask(ObjectManager::QUERY_NPC_MASK | ObjectManager::QUERY_CONTAINER);
            RaySceneQueryResult &result = mRaySceneQuery->execute();
            if (!result.empty())
            {
                //Logger::log().warning() << result.size();
                RaySceneQueryResult::iterator itr = result.begin();
                ObjectManager::getSingleton().selectObject(itr->movable);
            }
            else
            {
                // nothing selected, but if we are in attack mode -> attack selected enemy.
                //ObjectManager::getSingleton().selectObject(0);
            }
            mSceneManager->destroyQuery(mRaySceneQuery);
            mIdleTime =0;
        }
    }
    // ////////////////////////////////////////////////////////////////////
    // Left button for standard action.
    // ////////////////////////////////////////////////////////////////////
    else if (button == OIS::MB_Left)
    {
        RaySceneQuery *mRaySceneQuery = mSceneManager->createRayQuery(Ray());
        mRaySceneQuery->setRay(mCamera->getCameraToViewportRay(mMouse.x / e.state.width, mMouse.y / e.state.height));
        mRaySceneQuery->setQueryMask(ObjectManager::QUERY_NPC_MASK | ObjectManager::QUERY_CONTAINER);
        RaySceneQueryResult &result = mRaySceneQuery->execute();
        if (!result.empty())
        {
            // An object waas clicked.
            RaySceneQueryResult::iterator itr = result.begin();
            ObjectManager::getSingleton().mousePressed(itr->movable, mShiftDown);
        }
        else
        {
            // A tile was clicked.
            TileManager::getSingleton().tileClick(mMouse.x / e.state.width, mMouse.y / e.state.height);
        }
        mSceneManager->destroyQuery(mRaySceneQuery);
        mIdleTime =0;
    }
    //e->consume();
    return true;
}

//================================================================================================
// .
//================================================================================================
bool Events::mouseReleased(const OIS::MouseEvent &e, const OIS::MouseButtonID id)
{
    int ret = GuiManager::getSingleton().mouseEvent(GuiManager::BUTTON_RELEASED, mMouse);
    if (ret == GuiManager::EVENT_USER_ACTION)
        elementClicked(GuiManager::getSingleton().getElementPressed());
    else if (ret == GuiManager::EVENT_DRAG_DONE)
    {
        //GuiManager::getSingleton().getDragDestElement();
        //Item::getSingleton().dropItem(mDragSrcWin, mDragSrcSlot, mDragDstWin, mDragDstSlot);
        GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, "Drag done.");
    }
    return true;
}

//================================================================================================
// The user clicked an element.
//================================================================================================
void Events::elementClicked(int element)
{
    Sound::getSingleton().playStream(Sound::BUTTON_CLICK);
    const char *buf;
    switch (element)
    {
        case GuiManager::BUTTON_CLOSE:
            GuiManager::getSingleton().closeParentWin(element);
            break;
        case GuiManager::LIST_MSGWIN:
        case GuiManager::LIST_CHATWIN:
            buf = GuiManager::getSingleton().getKeyword(element);
            if (buf) GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, buf);
            GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, "");
            break;
        case GuiManager::LIST_NPC:
            Network::getSingleton().send_game_command(GuiManager::getSingleton().getKeyword(element));
            break;
        default:
            GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, "Anonymous element was clicked.");
    }
}
