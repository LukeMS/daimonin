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

#include <OgreKeyEvent.h>
#include "logger.h"
#include "option.h"
#include "item.h"
#include "events.h"
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
void Events::keyPressed(KeyEvent *e)
{
    static Real g_pitch = 0.2;
    mIdleTime =0;

    // ////////////////////////////////////////////////////////////////////
    // GUI keyEvents.
    // ////////////////////////////////////////////////////////////////////
    if (GuiManager::getSingleton().keyEvent(e->getKeyChar(), e->getKey()))
    {
        e->consume();
        return;
    }

    // ////////////////////////////////////////////////////////////////////
    // InGame keyEvent.
    // ////////////////////////////////////////////////////////////////////
    if (Option::getSingleton().getGameStatus() < Option::GAME_STATUS_PLAY) return;
    mShiftDown = e->isShiftDown();
    switch (e->getKey())
    {
        case KC_A:
        {
            static int animNr= 0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_ANIMATION, 0, ObjectAnimate::ANIM_GROUP_IDLE, animNr);
            if (++animNr >= 16) animNr= 0;
            break;
        }

        case KC_B:
        {
            static int animNr= 0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_ANIMATION, 0, ObjectAnimate::ANIM_GROUP_ATTACK, animNr);
            if (++animNr >= 16) animNr= 0;
            break;
        }

        case KC_C:
        {
            static int animNr= 0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_ANIMATION, 0, ObjectAnimate::ANIM_GROUP_ABILITY, animNr);
            if (++animNr >= 16) animNr= 0;

            //ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, 0,ObjectAnimate::STATE_CAST1);
            break;
        }

        case KC_D:
            //ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, 0,ObjectAnimate::STATE_DEATH1);
            break;

        case KC_F:
        {
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
            break;
        }

        case KC_G:
/*
            char    buf[100];
            int nrof =1;
            int loc = Item::getSingleton().mActGrndContainerID;
            int tag = Item::getSingleton().HeroTileGround[0]->tag;
            sprintf(buf, "mv %d %d %d", loc, tag, nrof);
            Network::getSingleton().cs_write_string(buf);
*/

            TileManager::getSingleton().toggleGrid();
            break;

        case KC_H:
            //ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_ANIMATION, 0,ObjectAnimate::STATE_HIT1);
            break;

        case KC_I:
            // ObjectManager::getSingleton().setPlayerEquipment(ObjectManager::OBJECT_PLAYER, ObjectNPC::BONE_HEAD, 1);
            break;

        case KC_J:
            // ObjectManager::getSingleton().keyEvent(OBJECT_NPC, OBJ_TURN,  1);
            //mCamera->yaw(Degree(10));
        {
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
        }
        break;

        case KC_K:
        {
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_HIT,0, 5);
            break;
        }

        case KC_O:
            // ObjectManager::getSingleton().setPlayerEquipment(ObjectManager::OBJECT_PLAYER, ObjectNPC::BONE_SHIELD_HAND, 1);
            break;

        case KC_P:
        {
            bool ready = ObjectManager::getSingleton().isPrimaryWeaponReady(ObjectNPC::HERO);
            ObjectManager::getSingleton().readyPrimaryWeapon(ObjectNPC::HERO, !ready);
            break;
        }

        case KC_Q:
        {
            if (ObjectManager::getSingleton().isPrimaryWeaponReady(ObjectNPC::HERO))
                ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_ANIMATION, 0, ObjectAnimate::ANIM_GROUP_ATTACK, 1);
            else if (ObjectManager::getSingleton().isSecondaryWeaponReady(ObjectNPC::HERO))
                ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_ANIMATION, 0, ObjectAnimate::ANIM_GROUP_ATTACK, 6);
            break;
        }

        case KC_S:
        {
            Item::getSingleton().clearContainer(Item::getSingleton().mActOpenContainerID);
            break;

            bool ready = ObjectManager::getSingleton().isSecondaryWeaponReady(ObjectNPC::HERO);
            ObjectManager::getSingleton().readySecondaryWeapon(ObjectNPC::HERO, !ready);
            break;
        }

        /*
                case KC_S:
                {
              bool ready = ObjectManager::getSingleton().isSecondaryWeaponReady(ObjectNPC::HERO);
                    ObjectManager::getSingleton().readySecondaryWeapon(ObjectNPC::HERO, !ready);
           break;
                }
        */
        case KC_T:
            Item::getSingleton().printAllItems();
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

        case KC_W:
            //Network::getSingleton().send_command("/apply", -1, SC_NORMAL);
            //GuiManager::getSingleton().addTextline(GUI_WIN_TEXTWINDOW, GUI_LIST_MSGWIN, "apply");
            if (mDayTime)
            {
                mDayTime =0;
                // mLight[LIGHT_VOL ]->setVisible(false);
                // mLight[LIGHT_SPOT]->setVisible(true);
            }
            else
            {
                mDayTime =15;
                //mLight[LIGHT_VOL ]->setVisible(true);
                //mLight[LIGHT_SPOT]->setVisible(false);
            }
            break;

        case KC_X:
        {
            static int pixel =128;
            //change pixel size of terrain textures
            pixel /= 2; // shrink pixel value
            if (pixel < TileManager::MIN_TEXTURE_PIXEL)
                pixel = 128; // if value is too low resize to maximum
            TileManager::getSingleton().setMaterialLOD(pixel);
            mTimeUntilNextToggle = .5;
            break;
        }

        case KC_Y:
            mSceneDetailIndex = (mSceneDetailIndex+1)%3;
            switch (mSceneDetailIndex)
            {
                case 0 :
                    mCamera->setPolygonMode(PM_SOLID);
                    break ;
                case 1 :
                    mCamera->setPolygonMode(PM_WIREFRAME);
                    break ;
                case 2 :
                    mCamera->setPolygonMode(PM_POINTS);
                    break ;
            }
            break;

        case KC_1:
            //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_WEAPON_HAND, 1);
        {
            static int color =0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE, 0, ObjectNPC::TEXTURE_POS_SKIN, color++);
            break;
        }

        case KC_2:
            //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_SHIELD_HAND, 1);
        {
            static int color =0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE, 0,ObjectNPC::TEXTURE_POS_FACE, color++);
            break;
        }

        case KC_3:
            //ObjectManager::getSingleton().keyEvent(OBJECT_PLAYER, OBJ_TEXTURE,0, -1);
        {
            static int color =0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE, 0,ObjectNPC::TEXTURE_POS_HAIR, color++);
            break;
        }

        case KC_4:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE,0, ObjectNPC::TEXTURE_POS_BODY, color++);
            break;
        }

        case KC_5:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE,0, ObjectNPC::TEXTURE_POS_LEGS, color++);
            break;
        }

        case KC_6:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE,0, ObjectNPC::TEXTURE_POS_BELT, color++);
            break;
        }

        case KC_7:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE,0, ObjectNPC::TEXTURE_POS_SHOES, color++);
            break;
        }

        case KC_8:
        {
            static int color =0;
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_TEXTURE, 0,ObjectNPC::TEXTURE_POS_HANDS, color++);
            //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_HEAD, 1);
            break;
        }

        case KC_9:
            //ObjectManager::getSingleton().toggleMesh(OBJECT_PLAYER, BONE_BODY, 1);
            break;

            // ////////////////////////////////////////////////////////////////////
            // Player Movemment.
            // ////////////////////////////////////////////////////////////////////
        case KC_UP:
            //      ObjectManager::getSingleton().Event(OBJECT_PLAYER, OBJ_WALK, 0, 1);
            //mCamera->  moveRelative (Vector3(0,100,0));
            break;

        case KC_DOWN:
            //      ObjectManager::getSingleton().Event(OBJECT_PLAYER, 9, OBJ_WALK,0, -1);
            //mCamera->  moveRelative (Vector3(0,-100,0));
            break;

        case KC_RIGHT:
            //mCamera->  moveRelative (Vector3(100,0,0));
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_CURSOR_TURN, 0, -1);
            break;

        case KC_LEFT:
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_CURSOR_TURN, 0,  1);
            //mCamera->  moveRelative (Vector3(-100,0,0));
            break;

        case KC_F1:
            break;

        case KC_F2:
            break;

        case KC_PGUP:
        {
            mCamera->pitch(Radian(-0.1));
            g_pitch -= 0.1;
            Vector3 pos = mCamera->getPosition();
            pos.y += 30;
            mCamera->setPosition(pos);
            break;
        }

        case KC_PGDOWN:
        {
            mCamera->pitch(Radian(+0.1));
            g_pitch += 0.1;
            Vector3 pos = mCamera->getPosition();
            pos.y -= 30;
            mCamera->setPosition(pos);
            break;
        }

        case KC_SUBTRACT:
        {
            //if (mCameraZoom < MAX_CAMERA_ZOOM)
            mCameraZoom += 5;
            mCamera->setFOVy(Degree(mCameraZoom));
            break;
        }

        case KC_ADD:
        {
            if (mCameraZoom > MIN_CAMERA_ZOOM)
                mCameraZoom -= 5;
            mCamera->setFOVy(Degree(mCameraZoom));
            break;
        }

        // ////////////////////////////////////////////////////////////////////
        // Screenshot.
        // ////////////////////////////////////////////////////////////////////
        case KC_SYSRQ:
        {
            static int mNumScreenShots=0;
            String strTemp = "screenshot_" + StringConverter::toString(++mNumScreenShots,2,'0') + ".png";
            mWindow->writeContentsToFile(strTemp.c_str());
            mTimeUntilNextToggle = 0.5;
            break;
        }

        // ////////////////////////////////////////////////////////////////////
        // Exit game.
        // ////////////////////////////////////////////////////////////////////
        case KC_ESCAPE:
            mQuitGame = true;
            break;

        default:
            break;
    }
    e->consume();
}

void Events::keyClicked(KeyEvent* )
{}

void Events::keyReleased(KeyEvent* e)
{
    mShiftDown = e->isShiftDown();
    switch (e->getKey())
    {
            // ////////////////////////////////////////////////////////////////////
            // Player Movemment.
            // ////////////////////////////////////////////////////////////////////
        case KC_UP:
        case KC_DOWN:
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_WALK, 0);
            break;

        case KC_RIGHT:
        case KC_LEFT:
            ObjectManager::getSingleton().Event(ObjectManager::OBJECT_PLAYER, ObjectManager::OBJ_CURSOR_TURN, 0);
            break;

        case KC_J:
        case KC_K:
            // ObjectManager::getSingleton().Event(ObjectManager::OBJECT_NPC, OBJ_TURN,  0);
            break;

        case KC_G:
            // ObjectManager::getSingleton().Event(ObjectManager::OBJECT_NPC, OBJ_WALK,  0);
            break;

        default:
            break;
    }
}

//================================================================================================
// Buffered Mouse Events.
//================================================================================================
void Events::mouseMoved (MouseEvent *e)
{
    mMouse.x = e->getX();
    if (mMouse.x > 0.998)
        mMouse.x = 0.998;
    mMouse.y = e->getY();
    if (mMouse.y > 0.995)
        mMouse.y = 0.995;
    mMouse.z = e->getRelZ();
    if (GuiManager::getSingleton().mouseEvent(GuiWindow::MOUSE_MOVEMENT, mMouse))
        return;

    if (Option::getSingleton().getGameStatus() >= Option::GAME_STATUS_PLAY)
    {}}

void Events::mousePressed (MouseEvent *e)
{
    // ////////////////////////////////////////////////////////////////////
    // Right button for selection and menu.
    // ////////////////////////////////////////////////////////////////////
    if (Option::getSingleton().getGameStatus() < Option::GAME_STATUS_PLAY) return; // TODO: ServerSelection by mouse.
    mMouse.x = e->getX();
    mMouse.y = e->getY();
    if (GuiManager::getSingleton().mouseEvent(GuiWindow::BUTTON_PRESSED, mMouse)) return;

    // ////////////////////////////////////////////////////////////////////
    // Right button for selection and menu.
    // ////////////////////////////////////////////////////////////////////
    if (Option::getSingleton().getGameStatus() < Option::GAME_STATUS_PLAY) return;
    int button = e->getButtonID();
#ifdef WIN32
    if (button & MouseEvent::BUTTON1_MASK )
#else
    if (button & MouseEvent::BUTTON2_MASK )
#endif
    {
        RaySceneQuery *mRaySceneQuery = mSceneManager->createRayQuery(Ray());
        mRaySceneQuery->setRay(mCamera->getCameraToViewportRay(mMouse.x, mMouse.y));
        mRaySceneQuery->setQueryMask(ObjectManager::QUERY_NPC_MASK | ObjectManager::QUERY_CONTAINER);
        RaySceneQueryResult &result = mRaySceneQuery->execute();
        if (!result.empty())
        {
            //Logger::log().info() << result.size();
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

    // ////////////////////////////////////////////////////////////////////
    // Left button for movement & standard action.
    // ////////////////////////////////////////////////////////////////////
    else if (button & MouseEvent::BUTTON0_MASK ) // LeftButton.
    {
        TileManager::getSingleton().getTileInterface()->pickTile(mMouse.x, mMouse.y);
        RaySceneQuery *mRaySceneQuery = mSceneManager->createRayQuery(Ray());
        mRaySceneQuery->setRay(mCamera->getCameraToViewportRay(mMouse.x, mMouse.y));
        mRaySceneQuery->setQueryMask(ObjectManager::QUERY_NPC_MASK | ObjectManager::QUERY_CONTAINER);
        RaySceneQueryResult &result = mRaySceneQuery->execute();
        if (!result.empty())
        {
            RaySceneQueryResult::iterator itr = result.begin();
            ObjectManager::getSingleton().mousePressed(itr->movable, TileManager::getSingleton().getTileInterface()->getSelectedTile(), mShiftDown);
        }
        else
        {
            ParticleManager::getSingleton().addFreeObject(TileManager::getSingleton().getTileInterface()->getSelectedPos(), "Particle/SelectionDust", 0.8);
            ObjectManager::getSingleton().mousePressed(0, TileManager::getSingleton().getTileInterface()->getSelectedTile(), mShiftDown);
        }
        mSceneManager->destroyQuery(mRaySceneQuery);
        mIdleTime =0;
    }
    e->consume();
}

void Events::mouseDragged(MouseEvent *e)
{
    mouseMoved(e);
    e->consume();
}

void Events::mouseClicked (MouseEvent *e)
{
    mouseMoved(e);
    e->consume();
}

void Events::mouseEntered (MouseEvent *e)
{
    mouseMoved(e);
    e->consume();
}

void Events::mouseExited  (MouseEvent *e)
{
    mouseMoved(e);
    e->consume();
}

void Events::mouseReleased(MouseEvent *e)
{
    GuiManager::getSingleton().mouseEvent(GuiWindow::BUTTON_RELEASED, mMouse);
    e->consume();
}
